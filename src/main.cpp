/*
 * Actuador con E/S, secuenciador, maquina de estados, sensores y variables
 *
 * Actuador remoto
 * 
 * Reles de conexion y entradas digitales
 * 
 * Servicio web levantado en puerto PUERTO_WEBSERVER
 */
 
/***************************** Defines *****************************/
// Una vuela de loop son ANCHO_INTERVALO segundos 
#define ANCHO_INTERVALO                 200 //Ancho en milisegundos de la rodaja de tiempo
#define FRECUENCIA_OTA                    5 //cada cuantas vueltas de loop atiende las acciones
#define FRECUENCIA_SENSORES              15 //cada cuantas vueltas de loop lee los sensores
#define FRECUENCIA_ENTRADAS               4 //cada cuantas vueltas de loop atiende las entradas
#define FRECUENCIA_SALIDAS                4 //cada cuantas vueltas de loop atiende las salidas
#define FRECUENCIA_SECUENCIADOR           9 //cada cuantas vueltas de loop atiende al secuenciador
#define FRECUENCIA_MAQUINAESTADOS         9 //cada cuantas vueltas de loop atiende a la maquina de estados
#define FRECUENCIA_SERVIDOR_FTP           3 //cada cuantas vueltas de loop atiende el servidor ftp
#define FRECUENCIA_SERVIDOR_WEBSOCKET     1 //cada cuantas vueltas de loop atiende el servidor web
#define FRECUENCIA_MQTT                  10 //cada cuantas vueltas de loop envia y lee del broker MQTT
#define FRECUENCIA_ENVIO_DATOS           47 //cada cuantas vueltas de loop envia al broker el estado de E/S
#define FRECUENCIA_ORDENES                3 //cada cuantas vueltas de loop atiende las ordenes via serie 
#define FRECUENCIA_PANTALLA              15 //cada cuantas vueltas de loop actualiza la pantalla
#define FRECUENCIA_WIFI_WATCHDOG         60 //cada cuantas vueltas comprueba si se ha perdido la conexion WiFi

//configuracion del watchdog del sistema
#define TIMER_WATCHDOG        0 //Utilizo el timer 0 para el watchdog
#define PREESCALADO_WATCHDOG 80 //el relog es de 80Mhz, lo preesalo entre 80 para pasarlo a 1Mhz
#define TIEMPO_WATCHDOG      1000*1000*ANCHO_INTERVALO //Si en N ANCHO_INTERVALO no se atiende el watchdog, saltara. ESTA EN microsegundos
/***************************** Defines *****************************/
/***************************** Includes *****************************/
#include <Global.h>
#include <configNVS.h>
#include <RedWifi.h>
#include <Sensores.h>
#include <Variables.h> 
#include <Entradas.h>
#include <Salidas.h>
#include <Ficheros.h>
#include <FTP.h>
#include <GoogleHomeNotifier.h>
#include <MaqEstados.h>
#include <Secuenciador.h>
#include <mDNS.h>
#include <MQTT.h>
#include <Ordenes.h>
#include <OTA.h>
#include <SNTP.h>
#include <WebSockets.h>
#include <WebServer.h>
#include <ComprobacionErrores.h>
#include <Pantalla.h>
#include <ClienteHTTP.h>

#include <rom/rtc.h>

//#include "soc/soc.h"
//#include "soc/rtc_cntl_reg.h"
/***************************** Includes *****************************/
/***************************** Prototipos *****************************/
boolean inicializaConfiguracion(boolean debug);
boolean parseaConfiguracionGlobal(String contenido);
void enviaConfiguracion(void);
const char* reset_reason(RESET_REASON reason);
void configuraWatchdog(void);
void alimentaWatchdog(void);
/***************************** Prototipos *****************************/
/***************************** variables globales *****************************/
enum ETAPAS{
  SETUP=0,
  INICIO,
  OTA,
  SENSORES,
  ENTRADAS,
  SECUENCIADOR,
  MAQ_ESTADOS,
  SALIDAS,
  WEBSOCKETS,
  MQTT,
  ENVIO,
  ORDENES,
  PANTALLA,
  WIFI_WD,
  FIN
};

String nombreEtapas[]={"SETUP","INICIO","OTA","SENSORES","ENTRADAS","SECUENCIADOR","MAQ_ESTADOS","SALIDAS","WEBSOCKETS","MQTT","ENVIO","ORDENES","RELOJ","WIFI_WD","FIN"};

int debugMain=0; //por defecto desabilitado
int debugGlobal=0; //por defecto desabilitado
int nivelActivo;//Indica si el rele se activa con HIGH o LOW
String URLPlataforma=""; //URL de la plataforma Domoticae
hw_timer_t *timer = NULL;//Puntero al timer del watchdog
uint16_t vuelta = 0; //MAX_VUELTAS-100; //vueltas de loop
int etapa=SETUP;
/***************************** variables globales *****************************/

/*************************************** SETUP ***************************************/
void setup()
  {
  //WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable   detector
  Serial.begin(115200);
  configuraLed();
  enciendeLed();
  
  Traza.mensaje("\n\n\n");
  Traza.mensaje("*************** %s ***************\n",NOMBRE_FAMILIA);
  Traza.mensaje("*************** %s ***************\n",VERSION);
  Traza.mensaje("***************************************************************\n");
  Traza.mensaje("*                                                             *\n");
  Traza.mensaje("*             Inicio del setup del modulo                     *\n");
  Traza.mensaje("*                                                             *\n");    
  Traza.mensaje("***************************************************************\n");

  for(int8_t core=0;core<2;core++) Traza.mensaje("Motivo del reinicio (%i): %s\n",core,reset_reason(rtc_get_reset_reason(core)));  

  Traza.mensaje("\n\nInit NVS --------------------------------------------------------------------------\n");
  //NVS - Cargo el ID de device
  inicializaNVS();

  Traza.mensaje("\n\nInit Ficheros ---------------------------------------------------------------------\n");
  //Ficheros - Lo primero para poder leer los demas ficheros de configuracion
  inicializaFicheros(debugGlobal);

  //Configuracion general
  Traza.mensaje("\n\nInit General ---------------------------------------------------------------------\n");
  inicializaConfiguracion(debugGlobal);
  parpadeaLed(1);
  
  //Wifi
  Traza.mensaje("\n\nInit WiFi -----------------------------------------------------------------------\n");
  if (inicializaWifi(1))
    {
    parpadeaLed(5,200); 
    //Traza  
    Traza.begin(0,serie);
    Traza.mensaje("Traza iniciada con nivel %i y salida %i\n",Traza.getNivelDebug(),Traza.getMedio());
    /*----------------Inicializaciones que necesitan red-------------*/
    //Plataforma domoticae
    Traza.mensaje("\n\nInit Plataforma domoticae -------------------------------------------------------\n");
    inicializaPlataforma(debugGlobal);
    parpadeaLed(1);
    //OTA
    Traza.mensaje("\n\nInit OTA ------------------------------------------------------------------------\n");
    inicializaOTA(debugGlobal);
    parpadeaLed(1);
    //SNTP
    Traza.mensaje("\n\nInit SNTP ------------------------------------------------------------------------\n");
    inicializaReloj();    
    parpadeaLed(2);
    //MQTT
    Traza.mensaje("\n\nInit MQTT -----------------------------------------------------------------------\n");
    inicializaMQTT();
    parpadeaLed(3);
    //WebServer
    Traza.mensaje("\n\nInit Web ------------------------------------------------------------------------\n");
    inicializaWebServer();
    parpadeaLed(4);
    //Google Home Notifier
    Traza.mensaje("\n\nInit Google Home Notifier -------------------------------------------------------\n");
    inicializaGHN();
    //WebSockets
    Serial.println("\n\nInit WebSockets -----------------------------------------------------------------\n");
    inicializaWebSockets();
    //mDNS
    Serial.println("\n\nInit mDNS -----------------------------------------------------------------------\n");
    inicializamDNS(NULL);
    parpadeaLed(3);
    //FTPServer
    Serial.println("\n\nInit FTP ----------------------------------------------------------------------------\n");
    //inicializaFTP(debugGlobal);
    }
  else Traza.mensaje("No se pudo conectar al WiFi");
  apagaLed();
  
  //Sensores
  Traza.mensaje("\n\nInit sensores ------------------------------------------------------------------------\n");
  sensores.inicializa();

  //Variables
  Traza.mensaje("\n\nInit variables ------------------------------------------------------------------------\n");
  variables.inicializa();

  //Entradas
  Traza.mensaje("\n\nInit entradas ------------------------------------------------------------------------\n");
  entradas.inicializa();

  //Salidas
  Traza.mensaje("\n\nInit salidas ------------------------------------------------------------------------\n");
  salidas.inicializa();

  //Secuenciador
  Traza.mensaje("\n\nInit secuenciador ---------------------------------------------------------------------\n");
  secuenciador.inicializa();
  
  //Maquina de estados
  Traza.mensaje("\n\nInit maquina de estados----------------------------------------------------------------\n");
  maquinaEstados.inicializa();
  
  //Ordenes serie
  Traza.mensaje("\n\nInit Ordenes ----------------------------------------------------------------------\n");  
  inicializaOrden();//Inicializa los buffers de recepcion de ordenes desde PC

  //Pantalla
#ifdef TIENE_PANTALLA
  Traza.mensaje("\n\nInit Pantalla ----------------------------------------------------------------------\n");  
  inicializaPantalla();
#endif

  compruebaConfiguracion(0);
  enviaConfiguracion();
  parpadeaLed(2);
  apagaLed();//Por si acaso...
  
  Traza.mensaje("\n\n");
  Traza.mensaje("***************************************************************\n");
  Traza.mensaje("*                                                             *\n");
  Traza.mensaje("*               Fin del setup del modulo                      *\n");
  Traza.mensaje("*                                                             *\n");    
  Traza.mensaje("***************************************************************\n");
  Traza.mensaje("\n\n");  

  //activo el watchdog
  configuraWatchdog();  
  }  

void loop()
  {  
  etapa=INICIO;
  //referencia horaria de entrada en el bucle
  time_t EntradaBucle=0;
  EntradaBucle=millis();//Hora de entrada en la rodaja de tiempo

  //reinicio el watchdog del sistema
  alimentaWatchdog(); // para el esp32 timerWrite(timer, 0);
  
  //------------- EJECUCION DE TAREAS --------------------------------------
  //Acciones a realizar en el bucle   
  //Prioridad 0: OTA es prioritario.
  etapa=OTA;if(debugMain) Serial.printf("Etapa: %s | milis: %i\n", nombreEtapas[etapa],millis());
  if ((vuelta % FRECUENCIA_OTA)==0) gestionaOTA(); //Gestion de actualizacion OTA
  //Prioridad 2: Funciones de control.
  etapa=SENSORES;if(debugMain) Serial.printf("Etapa: %s | milis: %i\n", nombreEtapas[etapa],millis());
  if ((vuelta % FRECUENCIA_SENSORES)==0) {
    sensores.lee(debugMain); //Lee las medidas de los sensores
    variables.lee(debugMain);//Actualiza los valores de las variables
  }
  etapa=ENTRADAS;if(debugMain) Serial.printf("Etapa: %s | milis: %i\n", nombreEtapas[etapa],millis());
  if ((vuelta % FRECUENCIA_ENTRADAS)==0) entradas.actualiza(debugGlobal); //Actualiza las entradas
  etapa=SECUENCIADOR;if(debugMain) Serial.printf("Etapa: %s | milis: %i\n", nombreEtapas[etapa],millis());
  if ((vuelta % FRECUENCIA_SECUENCIADOR)==0) secuenciador.actualiza(debugGlobal); //Actualiza la salida del secuenciador
  etapa=MAQ_ESTADOS;if(debugMain) Serial.printf("Etapa: %s | milis: %i\n", nombreEtapas[etapa],millis());
  if ((vuelta % FRECUENCIA_MAQUINAESTADOS)==0) maquinaEstados.actualiza(debugGlobal); //Actualiza la maquina de estados
  etapa=SALIDAS;if(debugMain) Serial.printf("Etapa: %s | milis: %i\n", nombreEtapas[etapa],millis());
  if ((vuelta % FRECUENCIA_SALIDAS)==0) salidas.actualiza(debugGlobal); //comprueba las salidas
  //Prioridad 3: Interfaces externos de consulta    
  //if ((vuelta % FRECUENCIA_SERVIDOR_FTP)==0) gestionaFTP(); //atiende el servidor ftp
  etapa=WEBSOCKETS;if(debugMain) Serial.printf("Etapa: %s | milis: %i\n", nombreEtapas[etapa],millis());
  if ((vuelta % FRECUENCIA_SERVIDOR_WEBSOCKET)==0) atiendeWebSocket(debugGlobal); //atiende el servidor web
  etapa=MQTT;if(debugMain) Serial.printf("Etapa: %s | milis: %i\n", nombreEtapas[etapa],millis());
  if ((vuelta % FRECUENCIA_MQTT)==0) atiendeMQTT();      
  etapa=ENVIO;if(debugMain) Serial.printf("Etapa: %s | milis: %i\n", nombreEtapas[etapa],millis());
  if ((vuelta % FRECUENCIA_ENVIO_DATOS)==0) enviaDatos(debugGlobal); //publica via MQTT los datos de entradas y salidas, segun configuracion
  etapa=ORDENES;if(debugMain) Serial.printf("Etapa: %s | milis: %i\n", nombreEtapas[etapa],millis());
  if ((vuelta % FRECUENCIA_ORDENES)==0) while(HayOrdenes(debugGlobal)) EjecutaOrdenes(debugGlobal); //Lee ordenes via serie
#ifdef TIENE_PANTALLA  
  etapa=PANTALLA;if(debugMain) Serial.printf("Etapa: %s | milis: %i\n", nombreEtapas[etapa],millis());
  if ((vuelta % FRECUENCIA_PANTALLA)==0) actualizaPantalla(); //actualiza la pantalla
#endif

  //Prioridad 4: WatchDog
  etapa=WIFI_WD;if(debugMain) Serial.printf("Etapa: %s | milis: %i\n", nombreEtapas[etapa],millis());
  if ((vuelta % FRECUENCIA_WIFI_WATCHDOG)==0) WifiWD();      
//------------- FIN EJECUCION DE TAREAS ---------------------------------  
  etapa=FIN;if(debugMain) Serial.printf("Etapa: %s | milis: %i\n", nombreEtapas[etapa],millis());
/*
  if ((vuelta % FRECUENCIA_OTA)==0) gestionaOTA(); //Gestion de actualizacion OTA

  if ((vuelta % FRECUENCIA_SENSORES)==0) sensores.lee(debugGlobal); //Actualiza las entradas  
  if ((vuelta % FRECUENCIA_ENTRADAS)==0) entradas.actualiza(debugGlobal); //Actualiza las entradas
  if ((vuelta % FRECUENCIA_SECUENCIADOR)==0) secuenciador.actualiza(debugGlobal); //Actualiza la salida del secuenciador
  if ((vuelta % FRECUENCIA_MAQUINAESTADOS)==0) maquinaEstados.actualiza(debugGlobal); //Actualiza la maquina de estados
  if ((vuelta % FRECUENCIA_SALIDAS)==0) salidas.actualiza(debugGlobal); //comprueba las salidas

  if ((vuelta % FRECUENCIA_SERVIDOR_WEBSOCKET)==0) atiendeWebSocket(debugGlobal); //atiende el servidor web
  if ((vuelta % FRECUENCIA_MQTT)==0) atiendeMQTT();      
  if ((vuelta % FRECUENCIA_ENVIO_DATOS)==0) enviaDatos(debugGlobal); //publica via MQTT los datos de entradas y salidas, segun configuracion
  if ((vuelta % FRECUENCIA_ORDENES)==0) while(HayOrdenes(debugGlobal)) EjecutaOrdenes(debugGlobal); //Lee ordenes via serie

  if ((vuelta % FRECUENCIA_WIFI_WATCHDOG)==0) WifiWD();    
*/
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
  if (debug) Traza.mensaje("Recupero configuracion de archivo...\n");

  //cargo el valores por defecto
    nivelActivo=LOW;  
  
  if(!leeFichero(GLOBAL_CONFIG_FILE, cad))
    {
    Traza.mensaje("No existe fichero de configuracion global\n");
    cad="{\"NivelActivo\":0}"; //config por defecto    
    //salvo la config por defecto
    if(salvaFichero(GLOBAL_CONFIG_FILE, GLOBAL_CONFIG_BAK_FILE, cad)) Traza.mensaje("Fichero de configuracion global creado por defecto\n"); 
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
 *********************************************/
boolean parseaConfiguracionGlobal(String contenido)
  {  
  DynamicJsonDocument doc(512);
  //JsonObject& json = jsonBuffer.parseObject(contenido.c_str());
  DeserializationError err=deserializeJson(doc,contenido);
  
  if (err) {
    Serial.printf("Error deserializando el json %s\n",err.c_str());
    return false;
  }

  Traza.mensaje("parsed json\n");
//******************************Parte especifica del json a leer********************************
  if (doc.containsKey("NivelActivo")) {
    if(doc["NivelActivo"].as<int>()==0) nivelActivo=LOW;
    else nivelActivo=HIGH;
    }

  if (doc.containsKey("URLPlataforma")) {
    URLPlataforma = doc["URLPlataforma"].as<String>();
    if(URLPlataforma.endsWith("/")) {
      Serial.printf("Acaba en /, lo corto...\n");
      URLPlataforma=URLPlataforma.substring(0,URLPlataforma.length()-1);
      Serial.printf("URLPlataforma: %s\n",URLPlataforma.c_str());
    }
  }
  Traza.mensaje("Configuracion leida:\nNivelActivo: %i | URLPlataforma: %s\n",nivelActivo,URLPlataforma.c_str());
//************************************************************************************************
  return true;
  }
/********************************** Funciones de configuracion global **************************************/
/********************************** Utilidades **************************************/
void configuraLed(void){pinMode(LED_BUILTIN, OUTPUT);}
void enciendeLed(void){digitalWrite(LED_BUILTIN, HIGH);}
void apagaLed(void){digitalWrite(LED_BUILTIN, LOW);}
void parpadeaLed(uint8_t veces, uint16_t espera)
  {
  for(uint8_t i=0;i<2*veces;i++)
    {
    delay(espera/2);
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    }
  }

/**********************************************************************/
/* Salva la configuracion general en formato json                     */
/**********************************************************************/  
String generaJsonConfiguracionNivelActivo(String configActual, int nivelAct)
  {
  String salida="";

  if(configActual=="") 
    {
    Traza.mensaje("No existe el fichero. Se genera uno nuevo\n");
    return "{\"NivelActivo\": \"" + String(nivelAct) + "\"}";
    }
    
  DynamicJsonDocument doc(512);
  DeserializationError err = deserializeJson(doc,configActual);
  serializeJsonPretty(doc,salida);
  Traza.mensaje("json leido:\n#%s#\n",salida.c_str());
  if (err) {
    Serial.printf("Error deserializando el json %s\n",err.c_str());
  }

  Traza.mensaje("parsed json\n");          

//******************************Parte especifica del json a modificar*****************************
  doc["NivelActivo"]=nivelAct;
//************************************************************************************************

  serializeJsonPretty(doc,salida);
  Traza.mensaje("json creado:\n#%s#\n",salida.c_str());

  return salida;  
  } 

/**********************************************************************/
/* Salva la configuracion general en formato json                     */
/**********************************************************************/  
String generaJsonConfiguracionGlobal(void){
  boolean nuevo=true;
  String salida="";

  //Genero el nuevo JSON
  DynamicJsonDocument doc(512);
  
  doc["NivelActivo"]=nivelActivo;

  DeserializationError err = deserializeJson(doc,salida);//pinto el json que he creado
  Traza.mensaje("json creado:\n#%s#\n",salida.c_str());

  return salida;  
}

void salvaConfiguracionGlobal(void)
  {
  String cad="";
  
  Traza.mensaje("---------------------Salvando configuracion general---------------\n");
  Traza.mensaje("Valores que voy a salvar:\nnivel activo : %i\n",nivelActivo);

  cad=generaJsonConfiguracionGlobal();
  if(!salvaFichero(GLOBAL_CONFIG_FILE, GLOBAL_CONFIG_BAK_FILE, cad)) Traza.mensaje("No se pudo salvar el fichero\n");  
  Traza.mensaje("---------------------Fin salvando configuracion---------------\n");
  }
/**********************************************************************/
/*       Vuelca a un json informacion general del dispositivo         */
/**********************************************************************/  
String generaJsonInfo(void)
  {
  String salida="";

  DynamicJsonDocument doc(2*1024);
   
  JsonObject basica = doc.createNestedObject("basica");
  //uptime
  char tempcad[20]="";
  sprintf(tempcad,"%lu", (unsigned long)(esp_timer_get_time()/(unsigned long)1000000)); //la funcion esp_timer_get_time() devuelve el contador de microsegundos desde el arranque. rota cada 292.000 años  
  basica["uptime"] = String(tempcad);      
  basica["hora"] = getHora();
  basica["fecha"] = getFecha();
  basica["nivelActivo"] = String(nivelActivo);

  JsonArray listaSalidas = doc.createNestedArray("infoSalidas");
 
  for(int8_t i=0;i<salidas.getNumSalidas();i++)
    {
    JsonObject salidaNueva = listaSalidas.createNestedObject();

    salidaNueva["id"] = String(i);
    salidaNueva["nombre"] = salidas.getSalida(i).getNombre();
    salidaNueva["estado"] = salidas.getSalida(i).getEstado();
    }
  
  JsonObject wifi = doc.createNestedObject("wifi");  
  wifi["ssid"] = nombreSSID();     
  wifi["ip"] =  WiFi.localIP().toString();
  wifi["potencia"] = String(WiFi.RSSI());

  JsonObject mqtt = doc.createNestedObject("MQTT");  
  mqtt["ipBRoker"] = getIPBroker().toString();     
  mqtt["puertoBroker"] =  getPuertoBroker();
  mqtt["usuario"] = getUsuarioMQTT();
  mqtt["pasword"] = getPasswordMQTT();
  mqtt["topicRoot"] = getTopicRoot();

  JsonObject hardware = doc.createNestedObject("hardware");  
  hardware["FreeHeap"] = String(ESP.getFreeHeap());    
  hardware["ChipId"] =  String(ESP.getChipRevision());
  hardware["SdkVersion"] = String(ESP.getSdkVersion());
  hardware["CpuFreqMHz"] = String(ESP.getCpuFreqMHz());
  hardware["FlashChipSize"] = String(ESP.getFlashChipSize());
  hardware["FlashChipSpeed"] = String(ESP.getFlashChipSpeed());

  serializeJsonPretty(doc,salida);
  return (salida);
  }
/********************************** Utilidades **************************************/
/***************************************WatchDog**************************************************/
/***************************************************************/
/*                                                             */
/*  Funcion de interrupcion del watchdog                       */
/*  reseteara el ESP si no se atiende el watchdog              */
/*                                                             */
/***************************************************************/
void IRAM_ATTR resetModule(void) {
  Serial.printf("Watchdog!!! Etapa: (%i)%s, rebooting....\n",etapa,nombreEtapas[etapa].c_str());
  //ets_printf("Watchdog!!! reboot\n");
  esp_restart();
}

/***************************************************************/
/*                                                             */
/*  Configuracion del watchdog del sistema                     */
/*                                                             */
/***************************************************************/
void configuraWatchdog(void)
  {
  timer = timerBegin(TIMER_WATCHDOG, PREESCALADO_WATCHDOG, true); //timer 0, div 80 para que cuente microsegundos y hacia arriba         //hw_timer_t * timerBegin(uint8_t timer, uint16_t divider, bool countUp);
  timerAttachInterrupt(timer, &resetModule, true);                //asigno la funcion de interrupcion al contador                        //void timerAttachInterrupt(hw_timer_t *timer, void (*fn)(void), bool edge);
  timerAlarmWrite(timer, TIEMPO_WATCHDOG, false);                  //configuro el limite del contador para generar interrupcion en us    //void timerAlarmWrite(hw_timer_t *timer, uint64_t interruptAt, bool autoreload);
  timerWrite(timer, 0);                                           //lo pongo a cero                                                      //void timerWrite(hw_timer_t *timer, uint64_t val);
  timerAlarmEnable(timer);                                        //habilito el contador                                                 //void timerAlarmEnable(hw_timer_t *timer);
  }

void alimentaWatchdog(void) {timerWrite(timer, 0);}

/***************************************************************/
/*                                                             */
/*  Decodifica el motivo del ultimo reset                      */
/*                                                             */
/***************************************************************/
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
/***************************************WatchDog**************************************************/


void enviaConfiguracion(void){
  String servidor = String(URL_PLATAFORMA) + "/configuracion/";
  String contenido="";
  String URL="";

  //test, si no responde 200 salgo
  if(!testHTTP()) return;
  boolean retorno=false;
Serial.printf("Test OK\n");
  //Config
  retorno = enviaFicheroConfig(GLOBAL_CONFIG_FILE);
  Serial.printf("Envio de Config: %s\n",(retorno?"OK":"KO"));
  //WiFi
  retorno = enviaFicheroConfig(WIFI_CONFIG_FILE);
  Serial.printf("Envio de WiFi: %s\n",(retorno?"OK":"KO"));
  //MQTT
  retorno = enviaFicheroConfig(MQTT_CONFIG_FILE);
  Serial.printf("Envio de MQTT: %s\n",(retorno?"OK":"KO"));
  //Entradas
  retorno = enviaFicheroConfig(ENTRADAS_CONFIG_FILE);
  Serial.printf("Envio de entradas: %s\n",(retorno?"OK":"KO")); 
  //Salidas
  retorno = enviaFicheroConfig(SALIDAS_CONFIG_FILE);
  Serial.printf("Envio de salidas: %s\n",(retorno?"OK":"KO"));
  //Secuenciador
  retorno = enviaFicheroConfig(SECUENCIADOR_CONFIG_FILE);
  Serial.printf("Envio de secuenciador: %s\n",(retorno?"OK":"KO"));
  //Maquina estados
  retorno = enviaFicheroConfig(MAQUINAESTADOS_CONFIG_FILE);
  Serial.printf("Envio de maquina de estados: %s\n",(retorno?"OK":"KO"));
  //GHN
  retorno = enviaFicheroConfig(GHN_CONFIG_FILE);
  Serial.printf("Envio de GHN: %s\n",(retorno?"OK":"KO"));
  //Sensores
  retorno = enviaFicheroConfig(SENSORES_CONFIG_FILE);
  Serial.printf("Envio de sensores: %s\n",(retorno?"OK":"KO"));
  //Variables
  retorno = enviaFicheroConfig(VARIABLES_CONFIG_FILE);
  Serial.printf("Envio de variables: %s\n",(retorno?"OK":"KO"));
}