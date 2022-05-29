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
#define FRECUENCIA_ENVIO_DATOS           49 //cada cuantas vueltas de loop envia al broker el estado de E/S
#define FRECUENCIA_ORDENES                3 //cada cuantas vueltas de loop atiende las ordenes via serie 
#define FRECUENCIA_WIFI_WATCHDOG         50 //cada cuantas vueltas comprueba si se ha perdido la conexion WiFi

//configuracion del watchdog del sistema
#define TIMER_WATCHDOG        0 //Utilizo el timer 0 para el watchdog
#define PREESCALADO_WATCHDOG 80 //el relog es de 80Mhz, lo preesalo entre 80 para pasarlo a 1Mhz
#define TIEMPO_WATCHDOG      1000*1000*ANCHO_INTERVALO //Si en N ANCHO_INTERVALO no se atiende el watchdog, saltara. ESTA EN microsegundos
/***************************** Defines *****************************/

/***************************** Includes *****************************/
#include <Global.h>
#include <RedWifi.h>
#include <Sensores.h>
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

#include <rom/rtc.h>

//#include "soc/soc.h"
//#include "soc/rtc_cntl_reg.h"
/***************************** Includes *****************************/

boolean inicializaConfiguracion(boolean debug);
boolean parseaConfiguracionGlobal(String contenido);
const char* reset_reason(RESET_REASON reason);
void configuraWatchdog(void);
void alimentaWatchdog(void);

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
  WIFI_WD,
  FIN
};

String nombreEtapas[]={"SETUP","INICIO","OTA","SENSORES","ENTRADAS","SECUENCIADOR","MAQ_ESTADOS","SALIDAS","WEBSOCKETS","MQTT","ENVIO","ORDENES","WIFI_WD","FIN"};

String nombre_dispositivo;//Nombre del dispositivo, por defecto el de la familia
int debugMain=0; //por defecto desabilitado
int debugGlobal=0; //por defecto desabilitado
int nivelActivo;//Indica si el rele se activa con HIGH o LOW

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

  compruebaConfiguracion(0);
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
  if ((vuelta % FRECUENCIA_SENSORES)==0) sensores.lee(debugGlobal); //Actualiza las entradas  
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
  nombre_dispositivo=String(NOMBRE_FAMILIA); //Nombre del dispositivo, por defecto el de la familia
  nivelActivo=LOW;  
  
  if(!leeFichero(GLOBAL_CONFIG_FILE, cad))
    {
    Traza.mensaje("No existe fichero de configuracion global\n");
    cad="{\"nombre_dispositivo\": \"" + String(NOMBRE_FAMILIA) + "\",\"NivelActivo\":0}"; //config por defecto    
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
  DynamicJsonBuffer jsonBuffer;
  JsonObject& json = jsonBuffer.parseObject(contenido.c_str());
  
  if (json.success()) 
    {
    Traza.mensaje("parsed json\n");
//******************************Parte especifica del json a leer********************************
    if (json.containsKey("nombre_dispositivo")) nombre_dispositivo=((const char *)json["nombre_dispositivo"]);    
    if(nombre_dispositivo==NULL) nombre_dispositivo=String(NOMBRE_FAMILIA);
 
    if (json.containsKey("NivelActivo")) 
      {
      if((int)json["NivelActivo"]==0) nivelActivo=LOW;
      else nivelActivo=HIGH;
      }
    
    Traza.mensaje("Configuracion leida:\nNombre dispositivo: %s\nNivelActivo: %i\n",nombre_dispositivo.c_str(),nivelActivo);
//************************************************************************************************
    return true;
    }
  return false;  
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
    return "{\"nombre_dispositivo\": \"Nombre dispositivo\", \"NivelActivo\": \"" + String(nivelAct) + "\"}";
    }
    
  DynamicJsonBuffer jsonBuffer;
  JsonObject& json = jsonBuffer.parseObject(configActual.c_str());
  json.printTo(salida);//pinto el json que he creado
  Traza.mensaje("json leido:\n#%s#\n",salida.c_str());
  if (json.success()) 
    {
    Traza.mensaje("parsed json\n");          

//******************************Parte especifica del json a modificar*****************************
    json["NivelActivo"]=nivelAct;
//************************************************************************************************

    json.printTo(salida);//pinto el json que he creado
    Traza.mensaje("json creado:\n#%s#\n",salida.c_str());
    }//la de parsear el json

  return salida;  
  } 

/**********************************************************************/
/*       Vuelca a un json informacion general del dispositivo         */
/**********************************************************************/  
String generaJsonInfo(void)
  {
  String salida="";

  const size_t capacity = 1*JSON_ARRAY_SIZE(15) + JSON_OBJECT_SIZE(25);
  DynamicJsonBuffer jsonBuffer(capacity);

  JsonObject& json = jsonBuffer.createObject();
    
  JsonObject& basica = json.createNestedObject("basica");
  //uptime
  char tempcad[20]="";
  sprintf(tempcad,"%lu", (unsigned long)(esp_timer_get_time()/(unsigned long)1000000)); //la funcion esp_timer_get_time() devuelve el contador de microsegundos desde el arranque. rota cada 292.000 años  
  basica["uptime"] = String(tempcad);      
  basica["hora"] = getHora();
  basica["fecha"] = getFecha();
  basica["nivelActivo"] = String(nivelActivo);

  JsonArray& listaSalidas = json.createNestedArray("infoSalidas");
 
  for(int8_t i=0;i<salidas.getNumSalidas();i++)
    {
    JsonObject& salidaNueva = listaSalidas.createNestedObject();

    salidaNueva["id"] = String(i);
    salidaNueva["nombre"] = salidas.getSalida(i).getNombre();
    salidaNueva["estado"] = salidas.getSalida(i).getEstado();
    }
  
  JsonObject& wifi = json.createNestedObject("wifi");  
  wifi["ssid"] = nombreSSID();     
  wifi["ip"] =  WiFi.localIP().toString();
  wifi["potencia"] = String(WiFi.RSSI());

  JsonObject& mqtt = json.createNestedObject("MQTT");  
  mqtt["ipBRoker"] = getIPBroker().toString();     
  mqtt["puertoBroker"] =  getPuertoBroker();
  mqtt["usuario"] = getUsuarioMQTT();
  mqtt["pasword"] = getPasswordMQTT();
  mqtt["topicRoot"] = getTopicRoot();

  JsonObject& hardware = json.createNestedObject("hardware");  
  hardware["FreeHeap"] = String(ESP.getFreeHeap());    
  hardware["ChipId"] =  String(ESP.getChipRevision());
  hardware["SdkVersion"] = String(ESP.getSdkVersion());
  hardware["CpuFreqMHz"] = String(ESP.getCpuFreqMHz());
  hardware["FlashChipSize"] = String(ESP.getFlashChipSize());
  hardware["FlashChipSpeed"] = String(ESP.getFlashChipSpeed());

  json.printTo(salida);
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