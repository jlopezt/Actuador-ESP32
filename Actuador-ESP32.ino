/*
 * Actuador con E/S
 *
 * Actuador remoto
 * 
 * Reles de conexion y entradas digitales
 * 
 * Servicio web levantado en puerto PUERTO_WEBSERVER
 */
 
/***************************** Defines *****************************/
//Defines generales
#define NOMBRE_FAMILIA   "Actuador/Secuenciador (E/S)"
#define VERSION          "4.2.2 (ESP32 OTA|MQTT|Logic+|Secuenciador)"
#define SEPARADOR        '|'
#define SUBSEPARADOR     '#'
#define KO               -1
#define OK                0
#define MAX_VUELTAS      UINT16_MAX// 32767 

//Ficheros de configuracion
#define GLOBAL_CONFIG_FILE               "/Config.json"
#define GLOBAL_CONFIG_BAK_FILE           "/Config.json.bak"
#define ENTRADAS_SALIDAS_CONFIG_FILE     "/EntradasSalidasConfig.json"
#define ENTRADAS_SALIDAS_CONFIG_BAK_FILE "/EntradasSalidasConfig.json.bak"
#define WIFI_CONFIG_FILE                 "/WiFiConfig.json"
#define WIFI_CONFIG_BAK_FILE             "/WiFiConfig.json.bak"
#define MQTT_CONFIG_FILE                 "/MQTTConfig.json"
#define MQTT_CONFIG_BAK_FILE             "/MQTTConfig.json.bak"
#define SECUENCIADOR_CONFIG_FILE         "/SecuenciadorConfig.json"
#define SECUENCIADOR_CONFIG_BAK_FILE     "/SecuenciadorConfig.json.bak"

// Una vuela de loop son ANCHO_INTERVALO segundos 
#define ANCHO_INTERVALO         100 //Ancho en milisegundos de la rodaja de tiempo
#define FRECUENCIA_OTA            5 //cada cuantas vueltas de loop atiende las acciones
#define FRECUENCIA_ENTRADAS       5 //cada cuantas vueltas de loop atiende las entradas
#define FRECUENCIA_SALIDAS        5 //cada cuantas vueltas de loop atiende las salidas
#define FRECUENCIA_SECUENCIADOR  10 //cada cuantas vueltas de loop atiende al secuenciador
#define FRECUENCIA_SERVIDOR_WEB   1 //cada cuantas vueltas de loop atiende el servidor web
#define FRECUENCIA_MQTT          10 //cada cuantas vueltas de loop envia y lee del broker MQTT
#define FRECUENCIA_ENVIO_DATOS   50 //cada cuantas vueltas de loop envia al broker el estado de E/S
#define FRECUENCIA_ORDENES        2 //cada cuantas vueltas de loop atiende las ordenes via serie 
/***************************** Defines *****************************/

/***************************** Includes *****************************/
#include <FS.h>                   //this needs to be first, or it all crashes and burns...
#include <TimeLib.h>  // download from: http://www.arduino.cc/playground/Code/Time
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <ArduinoJson.h>
#include <SPIFFS.h> //para el ESP32
/***************************** Includes *****************************/

/***************************** variables globales *****************************/


//IPs de los diferentes dispositivos 
IPAddress IPActuador;
IPAddress IPGateway;

//Indica si el rele se activa con HIGH o LOW
int nivelActivo=HIGH; //Se activa con HIGH por defecto

String nombre_dispoisitivo(NOMBRE_FAMILIA);//Nombre del dispositivo, por defecto el de la familia
uint16_t vuelta = MAX_VUELTAS-100;//0; //vueltas de loop
int debugGlobal=0; //por defecto desabilitado
/***************************** variables globales *****************************/

void setup()
  {
  Serial.begin(115200);
  Serial.printf("\n\n\n");
  Serial.printf("*************** %s ***************\n",NOMBRE_FAMILIA);
  Serial.printf("*************** %s ***************\n",VERSION);
  Serial.println("***************************************************************");
  Serial.println("*                                                             *");
  Serial.println("*             Inicio del setup del modulo                     *");
  Serial.println("*                                                             *");    
  Serial.println("***************************************************************");

  //Configuracion general
  Serial.printf("\n\nInit Config -----------------------------------------------------------------------\n");
  inicializaConfiguracion(debugGlobal);

  //Wifi
  Serial.println("\n\nInit WiFi -----------------------------------------------------------------------\n");
  if (inicializaWifi(1))//debugGlobal)) No tiene sentido debugGlobal, no hay manera de activarlo
    {
    /*----------------Inicializaciones que necesitan red-------------*/
    //OTA
    Serial.println("\n\nInit OTA -----------------------------------------------------------------------\n");
    iniializaOTA(debugGlobal);
    //SNTP
    Serial.printf("\n\nInit SNTP ----------------------------------------------------------------------\n");
    inicializaReloj();    
    //MQTT
    Serial.println("\n\nInit MQTT -----------------------------------------------------------------------\n");
    inicializaMQTT();
    //WebServer
    Serial.println("\n\nInit Web --------------------------------------------------------------------------\n");
    inicializaWebServer();
    }
  else Serial.println("No se pudo conectar al WiFi");

  //Entradas y Salidas
  Serial.println("\n\nInit entradas ------------------------------------------------------------------------\n");
  inicializaEntradasSalidas();

  //Secuenciador
  Serial.println("\n\nInit secuenciador ---------------------------------------------------------------------\n");
  inicializaSecuenciador();
  
  //Ordenes serie
  Serial.println("\n\nInit Ordenes ----------------------------------------------------------------------\n");  
  inicializaOrden();//Inicializa los buffers de recepcion de ordenes desde PC

  Serial.printf("\n\n");
  Serial.println("***************************************************************");
  Serial.println("*                                                             *");
  Serial.println("*               Fin del setup del modulo                      *");
  Serial.println("*                                                             *");    
  Serial.println("***************************************************************");
  Serial.printf("\n\n");  
  }  

void loop()
  {  
  //referencia horaria de entrada en el bucle
  time_t EntradaBucle=0;
  EntradaBucle=millis();//Hora de entrada en la rodaja de tiempo

  //------------- EJECUCION DE TAREAS --------------------------------------
  //Acciones a realizar en el bucle   
  //Prioridad 0: OTA es prioritario.
  if ((vuelta % FRECUENCIA_OTA)==0) ArduinoOTA.handle(); //Gestion de actualizacion OTA
  //Prioridad 2: Funciones de control.
  if ((vuelta % FRECUENCIA_ENTRADAS)==0) consultaEntradas(debugGlobal); //comprueba las entradas
  if ((vuelta % FRECUENCIA_SALIDAS)==0) actualizaSalidas(debugGlobal); //comprueba las salidas
  if ((vuelta % FRECUENCIA_SECUENCIADOR)==0) actualizaSecuenciador(debugGlobal); //Actualiza la salida del secuenciador
  //Prioridad 3: Interfaces externos de consulta    
  if ((vuelta % FRECUENCIA_SERVIDOR_WEB)==0) webServer(debugGlobal); //atiende el servidor web
  if ((vuelta % FRECUENCIA_MQTT)==0) atiendeMQTT(debugGlobal);      
  if ((vuelta % FRECUENCIA_ENVIO_DATOS)==0) enviaDatos(debugGlobal); //publica via MQTT los datos de entradas y salidas, segun configuracion
  if ((vuelta % FRECUENCIA_ORDENES)==0) while(HayOrdenes(debugGlobal)) EjecutaOrdenes(debugGlobal); //Lee ordenes via serie
  //------------- FIN EJECUCION DE TAREAS ---------------------------------  

  //sumo una vuelta de loop, si desborda inicializo vueltas a cero
  vuelta++;//sumo una vuelta de loop  
      
  //Espero hasta el final de la rodaja de tiempo
  while(millis()<EntradaBucle+ANCHO_INTERVALO)
    {
    if(millis()<EntradaBucle) break; //cada 49 dias el contador de millis desborda
    delayMicroseconds(1000);
    }
  }

/********************************** Funciones de configuracion global **************************************/
/************************************************/
/* Recupera los datos de configuracion          */
/* del archivo global                           */
/************************************************/
boolean inicializaConfiguracion(boolean debug)
  {
  String cad="";
  if (debug) Serial.println("Recupero configuracion de archivo...");

  //cargo el valores por defecto
  IPActuador.fromString("0.0.0.0");
  nivelActivo=LOW;  
  
  if(leeFichero(GLOBAL_CONFIG_FILE, cad)) parseaConfiguracionGlobal(cad);
  else 
    {
    Serial.printf("No existe fichero de configuracion global\n");
    cad="{\"NivelActivo\":0,\"IPActuador\": \"10.68.1.78\",\"IPGateway\":\"10.68.1.1\"}"; //config por defecto
    salvaFichero(GLOBAL_CONFIG_FILE, GLOBAL_CONFIG_BAK_FILE, cad); //salvo la config por defecto
    Serial.printf("Fichero de configuracion global creado por defecto\n");
    if(leeFichero(GLOBAL_CONFIG_FILE, cad)) parseaConfiguracionGlobal(cad);
    else
      {
      Serial.printf("No se pudo recuperar el fichero recien guardado...\n");  
      cad="{\"NivelActivo\":0,\"IPActuador\": \"10.68.1.78\",\"IPGateway\":\"10.68.1.1\"}"; //config por defecto        
      parseaConfiguracionGlobal(cad);
      }
    }

  return true;
  }

/*********************************************/
/* Parsea el json leido del fichero de       */
/* configuracio global                       */
/*********************************************/
boolean parseaConfiguracionGlobal(String contenido)
  {  
  DynamicJsonBuffer jsonBuffer;
  JsonObject& json = jsonBuffer.parseObject(contenido.c_str());
  
  //json.printTo(Serial);
  if (json.success()) 
    {
    Serial.println("parsed json");
//******************************Parte especifica del json a leer********************************
    IPActuador.fromString((const char *)json["IPActuador"]); 
    IPGateway.fromString((const char *)json["IPGateway"]);
    
    if((int)json["NivelActivo"]==0) nivelActivo=LOW;
    else nivelActivo=HIGH;
    
    Serial.printf("Configuracion leida:\nNivelActivo: %i\nIP actuador: %s\nIP Gateway: %s\n", nivelActivo, IPActuador.toString().c_str(),IPGateway.toString().c_str());
//************************************************************************************************
    return true;
    }
  return false;  
  }

/**********************************************************************/
/* Salva la configuracion general en formato json                     */
/**********************************************************************/  
String generaJsonConfiguracionNivelActivo(String configActual, int nivelAct)
  {
  boolean nuevo=true;
  String salida="";

  if(configActual=="") 
    {
    Serial.println("No existe el fichero. Se genera uno nuevo");
    return "{\"NivelActivo\": \"" + String(nivelAct) + "\", \"IPPrimerTermometro\": \"10.68.1.62\",\"IPGateway\":\"10.68.1.1\"}";
    }
    
  DynamicJsonBuffer jsonBuffer;
  JsonObject& json = jsonBuffer.parseObject(configActual.c_str());
  json.printTo(Serial);
  if (json.success()) 
    {
    Serial.println("parsed json");          

//******************************Parte especifica del json a leer********************************
    json["NivelActivo"]=nivelAct;
//    json["IPPrimerTermometro"]=IPSatelites[0].toString().c_str();
//    json["IPGateway"]=IPGateway.toString().c_str();   
//************************************************************************************************

    json.printTo(salida);//pinto el json que he creado
    Serial.printf("json creado:\n#%s#\n",salida.c_str());
    }//la de parsear el json

  return salida;  
  }  
