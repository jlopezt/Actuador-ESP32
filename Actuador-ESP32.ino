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
#define VERSION          "4.3.2 (ESP32 1.0.2|OTA|MQTT|Logic+|Secuenciador|FicherosWeb)" //ping MQTT, separacion de la configuracion de E y S
#define SEPARADOR        '|'
#define SUBSEPARADOR     '#'
#define KO               -1
#define OK                0
#define MAX_VUELTAS      UINT16_MAX// 32767 

//Ficheros de configuracion
#define FICHERO_CANDADO                  "/Candado"
#define GLOBAL_CONFIG_FILE               "/Config.json"
#define GLOBAL_CONFIG_BAK_FILE           "/Config.json.bak"
#define ENTRADAS_CONFIG_FILE             "/EntradasConfig.json"
#define ENTRADAS_CONFIG_BAK_FILE         "/EntradasConfig.json.bak"
#define SALIDAS_CONFIG_FILE              "/SalidasConfig.json"
#define SALIDAS_CONFIG_BAK_FILE          "/SalidasConfig.json.bak"
#define WIFI_CONFIG_FILE                 "/WiFiConfig.json"
#define WIFI_CONFIG_BAK_FILE             "/WiFiConfig.json.bak"
#define MQTT_CONFIG_FILE                 "/MQTTConfig.json"
#define MQTT_CONFIG_BAK_FILE             "/MQTTConfig.json.bak"
#define SECUENCIADOR_CONFIG_FILE         "/SecuenciadorConfig.json"
#define SECUENCIADOR_CONFIG_BAK_FILE     "/SecuenciadorConfig.json.bak"
#define GHN_CONFIG_FILE                  "/GHNConfig.json"
#define GHN_CONFIG_BAK_FILE              "/GHNConfig.json.bak"

// Una vuela de loop son ANCHO_INTERVALO segundos 
#define ANCHO_INTERVALO         100 //Ancho en milisegundos de la rodaja de tiempo
#define FRECUENCIA_OTA            5 //cada cuantas vueltas de loop atiende las acciones
#define FRECUENCIA_ENTRADAS       5 //cada cuantas vueltas de loop atiende las entradas
#define FRECUENCIA_SALIDAS        5 //cada cuantas vueltas de loop atiende las salidas
#define FRECUENCIA_SECUENCIADOR  10 //cada cuantas vueltas de loop atiende al secuenciador
#define FRECUENCIA_SERVIDOR_WEB   1 //cada cuantas vueltas de loop atiende el servidor web
#define FRECUENCIA_MQTT          10 //cada cuantas vueltas de loop envia y lee del broker MQTT
#define FRECUENCIA_ENVIO_DATOS  100 //cada cuantas vueltas de loop envia al broker el estado de E/S
#define FRECUENCIA_ORDENES        2 //cada cuantas vueltas de loop atiende las ordenes via serie 
/***************************** Defines *****************************/

/***************************** Includes *****************************/
//#include <FS.h>                   //this needs to be first, or it all crashes and burns...
//#include <TimeLib.h>  // download from: http://www.arduino.cc/playground/Code/Time
//#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <ArduinoJson.h>
//#include <SPIFFS.h> //para el ESP32
#include <rom/rtc.h> //<esp32/rom/rtc.h>
/***************************** Includes *****************************/

/***************************** variables globales *****************************/
//Indica si el rele se activa con HIGH o LOW
int nivelActivo=HIGH; //Se activa con HIGH por defecto

String nombre_dispositivo;//(NOMBRE_FAMILIA);//Nombre del dispositivo, por defecto el de la familia
uint16_t vuelta = MAX_VUELTAS-100;//0; //vueltas de loop
int debugGlobal=0; //por defecto desabilitado
boolean candado=false; //Candado de configuracion. true implica que la ultima configuracion fue mal
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

  for(int8_t core=0;core<2;core++) Serial.printf("Motivo del reinicio (%i): %s\n",core,reset_reason(rtc_get_reset_reason(core)));  
  
  Serial.printf("\n\nInit Ficheros ---------------------------------------------------------------------\n");
  //Ficheros - Lo primero para poder leer los demas ficheros de configuracion
  inicializaFicheros(debugGlobal);

  //Compruebo si existe candado, si existe la ultima configuracion fue mal
  if(existeFichero(FICHERO_CANDADO)) 
    {
    Serial.printf("Candado puesto. Configuracion por defecto");
    candado=true; 
    debugGlobal=1;
    }
  else
    {
    candado=false;
    //Genera candado
    if(salvaFichero(FICHERO_CANDADO,"","JSD")) Serial.println("Candado creado");
    else Serial.println("ERROR - No se pudo crear el candado");
    }
 
  //Configuracion general
  Serial.printf("\n\nInit General ---------------------------------------------------------------------\n");
  inicializaConfiguracion(debugGlobal);

  //Wifi
  Serial.println("\n\nInit WiFi -----------------------------------------------------------------------\n");
  if (inicializaWifi(1))//debugGlobal)) No tiene sentido debugGlobal, no hay manera de activarlo
    {
    /*----------------Inicializaciones que necesitan red-------------*/
    //OTA
    Serial.println("\n\nInit OTA ------------------------------------------------------------------------\n");
    iniializaOTA(debugGlobal);
    //SNTP
    Serial.printf("\n\nInit SNTP ------------------------------------------------------------------------\n");
    inicializaReloj();    
    //MQTT
    Serial.println("\n\nInit MQTT -----------------------------------------------------------------------\n");
    inicializaMQTT();
    //WebServer
    Serial.println("\n\nInit Web ------------------------------------------------------------------------\n");
    inicializaWebServer();
    //Google Home Notifier
    Serial.println("\n\nInit Google Home Notifier -------------------------------------------------------\n");
    inicializaGHN();
    }
  else Serial.println("No se pudo conectar al WiFi");

  //Entradas
  Serial.println("\n\nInit entradas ------------------------------------------------------------------------\n");
  inicializaEntradas();

  //Salidas
  Serial.println("\n\nInit salidas ------------------------------------------------------------------------\n");
  inicializaSalidas();

  //Secuenciador
  Serial.println("\n\nInit secuenciador ---------------------------------------------------------------------\n");
  inicializaSecuenciador();
  
  //Ordenes serie
  Serial.println("\n\nInit Ordenes ----------------------------------------------------------------------\n");  
  inicializaOrden();//Inicializa los buffers de recepcion de ordenes desde PC

  //Si ha llegado hasta aqui, todo ha ido bien y borro el candado
  if(borraFichero(FICHERO_CANDADO))Serial.println("Candado borrado");
  else Serial.println("ERROR - No se pudo borrar el candado");
  
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
  if ((vuelta % FRECUENCIA_OTA)==0) gestionaOTA(); //Gestion de actualizacion OTA
  //Prioridad 2: Funciones de control.
  if ((vuelta % FRECUENCIA_ENTRADAS)==0) consultaEntradas(debugGlobal); //comprueba las entradas
  if ((vuelta % FRECUENCIA_SALIDAS)==0) actualizaSalidas(debugGlobal); //comprueba las salidas
  if ((vuelta % FRECUENCIA_SECUENCIADOR)==0) actualizaSecuenciador(debugGlobal); //Actualiza la salida del secuenciador
  //Prioridad 3: Interfaces externos de consulta    
  if ((vuelta % FRECUENCIA_SERVIDOR_WEB)==0) webServer(debugGlobal); //atiende el servidor web
  if ((vuelta % FRECUENCIA_MQTT)==0) atiendeMQTT();      
  if ((vuelta % FRECUENCIA_ENVIO_DATOS)==0) enviaDatos(debugGlobal); //publica via MQTT los datos de entradas y salidas, segun configuracion
  if ((vuelta % FRECUENCIA_ORDENES)==0) while(HayOrdenes(debugGlobal)) EjecutaOrdenes(debugGlobal); //Lee ordenes via serie
  //------------- FIN EJECUCION DE TAREAS ---------------------------------  

  //sumo una vuelta de loop, si desborda inicializo vueltas a cero
  vuelta++;//sumo una vuelta de loop  
      
  //Espero hasta el final de la rodaja de tiempo
  while(millis()<EntradaBucle+ANCHO_INTERVALO)
    {
    if(millis()<EntradaBucle) break; //cada 49 dias el contador de millis desborda
    delay(1);
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
  nombre_dispositivo=String(NOMBRE_FAMILIA); //Nombre del dispositivo, por defecto el de la familia
  nivelActivo=LOW;  
  
  if(!leeFicheroConfig(GLOBAL_CONFIG_FILE, cad))
    {
    Serial.printf("No existe fichero de configuracion global\n");
    cad="{\"nombre_dispositivo\": \"" + String(NOMBRE_FAMILIA) + "\",\"NivelActivo\":0}"; //config por defecto    
    //salvo la config por defecto
    if(salvaFicheroConfig(GLOBAL_CONFIG_FILE, GLOBAL_CONFIG_BAK_FILE, cad)) Serial.printf("Fichero de configuracion global creado por defecto\n"); 
    }

  return parseaConfiguracionGlobal(cad);
  }

/*********************************************/
/* Parsea el json leido del fichero de       */
/* configuracio global                       */
/*  auto date = obj.get<char*>("date");
    auto high = obj.get<int>("high");
    auto low = obj.get<int>("low");
    auto text = obj.get<char*>("text");
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
    if (json.containsKey("nombre_dispositivo")) nombre_dispositivo=((const char *)json["nombre_dispositivo"]);    
    if(nombre_dispositivo==NULL) nombre_dispositivo=String(NOMBRE_FAMILIA);
 
    if (json.containsKey("NivelActivo")) 
      {
      if((int)json["NivelActivo"]==0) nivelActivo=LOW;
      else nivelActivo=HIGH;
      }
    
    Serial.printf("Configuracion leida:\nNombre dispositivo: %s\nNivelActivo: %i\n",nombre_dispositivo.c_str(),nivelActivo);
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
    return "{\"nombre_dispositivo\": \"Nombre dispositivo\", \"NivelActivo\": \"" + String(nivelAct) + "\"}";
    }
    
  DynamicJsonBuffer jsonBuffer;
  JsonObject& json = jsonBuffer.parseObject(configActual.c_str());
  json.printTo(Serial);
  if (json.success()) 
    {
    Serial.println("parsed json");          

//******************************Parte especifica del json a modificar*****************************
    json["NivelActivo"]=nivelAct;
//************************************************************************************************

    json.printTo(salida);//pinto el json que he creado
    Serial.printf("json creado:\n#%s#\n",salida.c_str());
    }//la de parsear el json

  return salida;  
  }  

const char* reset_reason(RESET_REASON reason)
{
  switch ( reason)
  {
    case 1 : return ("POWERON_RESET");break;          /**<1,  Vbat power on reset*/
    case 3 : return ("SW_RESET");break;               /**<3,  Software reset digital core*/
    case 4 : return ("OWDT_RESET");break;             /**<4,  Legacy watch dog reset digital core*/
    case 5 : return ("DEEPSLEEP_RESET");break;        /**<5,  Deep Sleep reset digital core*/
    case 6 : return ("SDIO_RESET");break;             /**<6,  Reset by SLC module, reset digital core*/
    case 7 : return ("TG0WDT_SYS_RESET");break;       /**<7,  Timer Group0 Watch dog reset digital core*/
    case 8 : return ("TG1WDT_SYS_RESET");break;       /**<8,  Timer Group1 Watch dog reset digital core*/
    case 9 : return ("RTCWDT_SYS_RESET");break;       /**<9,  RTC Watch dog Reset digital core*/
    case 10 : return ("INTRUSION_RESET");break;       /**<10, Instrusion tested to reset CPU*/
    case 11 : return ("TGWDT_CPU_RESET");break;       /**<11, Time Group reset CPU*/
    case 12 : return ("SW_CPU_RESET");break;          /**<12, Software reset CPU*/
    case 13 : return ("RTCWDT_CPU_RESET");break;      /**<13, RTC Watch dog Reset CPU*/
    case 14 : return ("EXT_CPU_RESET");break;         /**<14, for APP CPU, reseted by PRO CPU*/
    case 15 : return ("RTCWDT_BROWN_OUT_RESET");break;/**<15, Reset when the vdd voltage is not stable*/
    case 16 : return ("RTCWDT_RTC_RESET");break;      /**<16, RTC Watch dog reset digital core and rtc module*/
    default : return ("NO_MEAN");
  }
}