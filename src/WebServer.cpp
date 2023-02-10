/************************************************************************************************
************************************************************************************************/

/***************************** Defines *****************************/
//Configuracion de los servicios web
#define PUERTO_WEBSERVER 80
#define FONDO     String("#DDDDDD")
#define TEXTO     String("#000000")
#define ACTIVO    String("#FFFF00")
#define DESACTIVO String("#DDDDDD")
/***************************** Defines *****************************/

/***************************** Includes *****************************/
#include <Global.h>
#include <WebServer.h>
#include <Sensores.h>
#include <Variables.h>
#include <Entradas.h>
#include <Salidas.h>
#include <Secuenciador.h>
#include <MaqEstados.h>
#include <OTA.h>
#include <SNTP.h>
#include <RedWifi.h>
#include <MQTT.h>
#include <Ficheros.h>
#include <GoogleHomeNotifier.h>

#include <configNVS.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include "ESPAsyncWebServer.h"
#include <FS.h>     //this needs to be first, or it all crashes and burns...
#include <SPIFFS.h>
/***************************** Includes *****************************/

/***************************** Variables globales *****************************/
AsyncWebServer serverX(PUERTO_WEBSERVER);

//Cadenas HTML precargadas
//String miniCabecera="<html><head></head><body><link rel='stylesheet' type='text/css' href='css.css'>\n";
String miniCabecera="<html><head></head><body>\n";
String miniPie="</body></html>";

void handleNombre(AsyncWebServerRequest *request);
void handleServicios(AsyncWebServerRequest *request);

void handleEstadoVariables(AsyncWebServerRequest *request);
void handleConfigVariables(AsyncWebServerRequest *request);
void handleEstadoSensores(AsyncWebServerRequest *request);
void handleConfigSensores(AsyncWebServerRequest *request);

void handleEstadoEntradas(AsyncWebServerRequest *request);
void handleConfigEntradas(AsyncWebServerRequest *request);

void handleEstadoSalidas(AsyncWebServerRequest *request);
void handleConfigSalidas(AsyncWebServerRequest *request);

void handlePulsoRele(AsyncWebServerRequest *request);
void handleRecuperaManual(AsyncWebServerRequest *request);
void handleFuerzaManual(AsyncWebServerRequest *request);
void handleDesactivaRele(AsyncWebServerRequest *request);
void handleActivaRele(AsyncWebServerRequest *request);

void handleEstadoMaquinaEstados(AsyncWebServerRequest *request);

void handleDesactivaSecuenciador(AsyncWebServerRequest *request);
void handleActivaSecuenciador(AsyncWebServerRequest *request);
void handleEstadoSecuenciador(AsyncWebServerRequest *request);
void handleConfigSecuenciador(AsyncWebServerRequest *request);

void handleParticiones(AsyncWebServerRequest *request);
void handleSetNextBoot(AsyncWebServerRequest *request);
void handleInfo(AsyncWebServerRequest *request);
void handleRestart(AsyncWebServerRequest *request);
void handleResetWiFi(AsyncWebServerRequest *request);
void handleResetUser(AsyncWebServerRequest *request);
void handleResetID(AsyncWebServerRequest *request);
void handleResetTotal(AsyncWebServerRequest *request);
void handleInicializaNVS(AsyncWebServerRequest *request);

void handleListaFicheros(AsyncWebServerRequest *request);
void handleLeeFichero(AsyncWebServerRequest *request);
void handleBorraFichero(AsyncWebServerRequest *request);
void handleCreaFichero(AsyncWebServerRequest *request);

void handleNotFound(AsyncWebServerRequest *request);
void handleUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final);
bool handleFileRead(AsyncWebServerRequest *request);
bool handleFileReadChunked(AsyncWebServerRequest *request);

size_t loadData(File f, uint8_t *buffer, size_t maxLen, size_t index);
String getContentType(String filename);
String generaJsonServicios(boolean debug);

void handleSpeechPath(AsyncWebServerRequest *request);
/*************************** Fin variables globales ***************************/

/*********************************** Inicializacion y configuracion *****************************************************************/
void inicializaWebServer(void)
  {
  /*******Configuracion del Servicio Web***********/  
  //Inicializo los serivcios, decalra las URIs a las que va a responder
  serverX.on("/nombre", HTTP_GET, handleNombre); //devuelve un JSON con las medidas, reles y modo para actualizar la pagina de datos
  serverX.on("/servicios", HTTP_GET, handleServicios); //devuelve un JSON con las medidas, reles y modo para actualizar la pagina de datos  

  serverX.on("/estadoVariables", HTTP_GET, handleEstadoVariables); //Responde con la identificacion del modulo
  serverX.on("/configVariables", HTTP_GET, handleConfigVariables); //Responde con la identificacion del modulo
  serverX.on("/estadoSensores", HTTP_GET, handleEstadoSensores); //Responde con la identificacion del modulo
  serverX.on("/configSensores", HTTP_GET, handleConfigSensores); //Servicio de estdo de reles

  serverX.on("/estadoEntradas", HTTP_GET, handleEstadoEntradas); //Responde con la identificacion del modulo  
  serverX.on("/configEntradas", HTTP_GET, handleConfigEntradas); //Servicio de estdo de reles

  serverX.on("/estadoSalidas", HTTP_GET, handleEstadoSalidas); //Responde con la identificacion del modulo
  serverX.on("/configSalidas", HTTP_GET, handleConfigSalidas); //Servicio de estdo de reles 
  serverX.on("/activaSalida", HTTP_ANY, handleActivaRele); //Servicio de activacion de rele
  serverX.on("/desactivaSalida", HTTP_ANY, handleDesactivaRele);  //Servicio de desactivacion de rele
  serverX.on("/fuerzaSalidaManual", HTTP_ANY, handleFuerzaManual);  //Servicio para formar ua salida a modo manual
  serverX.on("/recuperaSalidaManual", HTTP_ANY, handleRecuperaManual);  //Servicio para formar ua salida a modo manual  
  serverX.on("/pulsoSalida", HTTP_ANY, handlePulsoRele);  //Servicio de pulso de rele

  serverX.on("/configSecuenciador", HTTP_GET, handleConfigSecuenciador); //Servicio de estdo de reles
  serverX.on("/estadoSecuenciador", HTTP_ANY, handleEstadoSecuenciador);  //Serivico de estado del secuenciador
  serverX.on("/activaSecuenciador", HTTP_ANY, handleActivaSecuenciador);  //Servicio para activar el secuenciador
  serverX.on("/desactivaSecuenciador", HTTP_ANY, handleDesactivaSecuenciador);  //Servicio para desactivar el secuenciador

  serverX.on("/estadoMaquinaEstados", HTTP_ANY, handleEstadoMaquinaEstados);  //Servicio de representacion del estado de la maquina de estados

  serverX.on("/listaFicheros", HTTP_ANY, handleListaFicheros);  //URI de leer fichero  
  serverX.on("/creaFichero", HTTP_ANY, handleCreaFichero);  //URI de crear fichero
  serverX.on("/borraFichero", HTTP_ANY, handleBorraFichero);  //URI de borrar fichero
  serverX.on("/leeFichero", HTTP_ANY, handleLeeFichero);  //URI de leer fichero

  serverX.on("/particiones", HTTP_ANY, handleParticiones);  
  serverX.on("/setNextBoot", HTTP_ANY, handleSetNextBoot);

  serverX.on("/restart", HTTP_ANY, handleRestart);  //URI de test
  serverX.on("/resetWiFi", HTTP_ANY, handleResetWiFi);  //URI de test
  serverX.on("/resetUser", HTTP_ANY, handleResetUser);  //URI de test
  serverX.on("/resetID", HTTP_ANY, handleResetID);  //URI de test
  serverX.on("/resetTotal", HTTP_ANY, handleResetTotal);  //URI de test
  serverX.on("/inicializaNVS", HTTP_ANY, handleInicializaNVS);  //URI de test

  serverX.on("/info", HTTP_ANY, handleInfo);  //URI de test
 
  //upload de ficheros
  serverX.on("/upload", HTTP_GET, [](AsyncWebServerRequest *request) {request->redirect("/www/upload.html");});
  serverX.on("/upload", HTTP_POST, [](AsyncWebServerRequest *request) {
        request->send(200);
      }, handleUpload);

  serverX.on("/speech", HTTP_ANY, handleSpeechPath);
  
  serverX.onNotFound(handleNotFound);//pagina no encontrada

  //Evita errores de CORS
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");

  //Arranco el servidor
  serverX.begin();

  Traza.mensaje("Servicio web iniciado en puerto %i\n", PUERTO_WEBSERVER);
  }
/********************************* FIn inicializacion y configuracion ***************************************************************/

/************************* Gestores de las diferentes URL coniguradas ******************************/
/*******************************UTILIDADES*****************************************************/
/*************************************************/
/*                                               */
/*  Servicio de consulta del nombre del          */
/*  dispositivo y devuelve un formato json       */
/*                                               */
/*************************************************/  
void handleNombre(AsyncWebServerRequest *request)
  {
  const size_t capacity = JSON_OBJECT_SIZE(2);
  DynamicJsonBuffer jsonBuffer(capacity);
  
  JsonObject& root = jsonBuffer.createObject();
  root["nombreFamilia"] = NOMBRE_FAMILIA;
  root["nombreDispositivo"] = configNVS.nombreServicio;
  root["version"] = VERSION;
  
  String cad="";
  root.printTo(cad);
  request->send(200,"text/json",cad);
  }

/*********************************************/
/*                                           */
/*  Lista de servicios proporcionados por    */
/*  el actuador                              */ 
/*                                           */
/*********************************************/  
void handleServicios(AsyncWebServerRequest *request) {
  String cad=generaJsonServicios(false);
  
  request->send(200, "text/json", cad); 
  }  

/*********************************************/
/*                                           */
/*  Reinicia el dispositivo mediante         */
/*  peticion HTTP                            */ 
/*                                           */
/*********************************************/  
void handleRestart(AsyncWebServerRequest *request)
  {
  delay(1000);
  ESP.restart();
  }

/*********************************************/
/*                                           */
/*  Resetea la memoria NVS donde se almacena */
/*  el ssid y la password WiFi               */ 
/*                                           */
/*********************************************/  
void handleResetWiFi(AsyncWebServerRequest *request){
  resetNVS_WiFi();
  
  request->send(200, "text/json", "{}");
}

/*********************************************/
/*                                           */
/*  Resetea la memoria NVS donde se almacena */
/*  el usuario y password de la plataforma   */ 
/*                                           */
/*********************************************/  
void handleResetUser(AsyncWebServerRequest *request){
  resetNVS_user();
  
  request->send(200, "text/json", "{}");
}

/*********************************************/
/*                                           */
/*  Resetea la memoria NVS donde se almacena */
/*  el deviceID                              */ 
/*                                           */
/*********************************************/  
void handleResetID(AsyncWebServerRequest *request){
  resetNVS_ID();

  request->send(200, "text/json", "{}");
}

/*********************************************/
/*                                           */
/*  Resetea la memoria NVS y el SPIFFS       */
/*                                           */ 
/*********************************************/  
void handleResetTotal(AsyncWebServerRequest *request){
  resetNVS_Total();  

  request->send(200, "text/json", "{}");
}

/*********************************************/
/*                                           */
/*  Resetea la memoria NVS con los valores   */
/*  por defecto                              */
/*                                           */ 
/*********************************************/  
void handleInicializaNVS(AsyncWebServerRequest *request){
  String cad="/inicializaNVS?DeviceID=<value>&mDNS=<value>&SSID=<value>&pass=<value>";

  //No hay argumentos, valores por defecto
  if(request->args()==0) escribeConfigNVSDefecto();
  else{
    for (uint8_t i=0; i<request->args(); i++){
      Serial.printf("parametro: %s | valor: %s\n",request->argName(i),request->arg(i));

      if(request->argName(i)=="DeviceID"){ 
        configNVS.deviceID=request->arg(i).toInt();
        Serial.printf("Valor de Device ID: %016llx\n",configNVS.deviceID);
      }
      else if(request->argName(i)=="nombreServicio") configNVS.nombreServicio=request->arg(i);
      else if(request->argName(i)=="mDNS") configNVS.nombremDNS=request->arg(i);
      else if(request->argName(i)=="SSID") configNVS.SSID=request->arg(i);
      else if(request->argName(i)=="pass") configNVS.pass=request->arg(i);
      else if(request->argName(i)=="usuario") configNVS.usuario=request->arg(i);
      //else if(request->argName(i)=="contrasena") configNVS.contrasena=request->arg(i);      
      else Serial.printf("argumento %s no valido\n",request->argName(i).c_str());
    }

    escribeConfigNVS(configNVS);
  }

  request->send(200, "text/json", "{\"formato\": \"" + cad + "\"}");
}

/*********************************************/
/*                                           */
/*  Pinta la lista de particiones en         */
/*  memoria                                  */ 
/*                                           */
/*********************************************/  
void handleParticiones(AsyncWebServerRequest *request){
  String cad=listaParticiones();
  request->send(200, "text/json", cad);
}

/*********************************************/
/*                                           */
/*    Cambia la particion de arranque        */
/*                                           */
/*********************************************/  
void handleSetNextBoot(AsyncWebServerRequest *request)
  {
  if(request->hasArg("p")) {
    setParticionProximoArranque(request->arg("p"));//si existen esos argumentos
    request->send(200,"text/plain", "particion activada");
    }

  request->send(501,"text/plain", "Error");
  }

/*********************************************/
/*                                           */
/*  Lee info del chipset mediante            */
/*  peticion HTTP                            */ 
/*                                           */
/*********************************************/  
void handleInfo(AsyncWebServerRequest *request){
  String cad=generaJsonInfo();
  request->send(200, "text/json", cad);
}
/*******************************FIN UTILIDADES*****************************************************/
/*******************************ACTIVACION/DESACITVACION MEDIDAS, ENTRADAS Y SALIDAS*****************************************************/
/*************************************************/
/*                                               */
/*  Servicio de consulta de estado de            */
/*  las entradas y devuelve un formato json      */
/*                                               */
/*************************************************/  
void handleEstadoSensores(AsyncWebServerRequest *request)
  {
  String cad=sensores.generaJsonEstado();

  request->send(200, "text/json", cad); 
  }  

/*************************************************/
/*                                               */
/*  Servicio de consulta de estado de            */
/*  las entradas y devuelve un formato json      */
/*                                               */
/*************************************************/  
void handleEstadoVariables(AsyncWebServerRequest *request)
  {
  String cad=variables.generaJsonEstado();

  request->send(200, "text/json", cad); 
  }  

/*************************************************/
/*                                               */
/*  Servicio de consulta de estado de            */
/*  las entradas y devuelve un formato json      */
/*                                               */
/*************************************************/  
void handleEstadoEntradas(AsyncWebServerRequest *request)
  {
  String cad=entradas.generaJsonEstado();
  
  request->send(200, "text/json", cad); 
  }  

/*************************************************/
/*                                               */
/*  Servicio de consulta de estado de            */
/*  las Salidas , devuelve un formato json       */
/*                                               */
/*************************************************/  
void handleEstadoSalidas(AsyncWebServerRequest *request)
  {
  String cad=salidas.generaJsonEstado();
  
  request->send(200, "text/json", cad); 
  }  

/*****************************************************/
/*                                                   */
/*  Servicio de consulta de estado de los sensores   */
/*  devuelve un formato json                         */
/*                                                   */
/*****************************************************/  
void handleConfigSensores(AsyncWebServerRequest *request){
  String cad=sensores.generaJsonConfiguracion(false);
  request->send(200, "text/json", cad);
  //request->redirect("Sensores.json");
  }

/*****************************************************/
/*                                                   */
/*  Servicio de consulta de estado de los sensores   */
/*  devuelve un formato json                         */
/*                                                   */
/*****************************************************/  
void handleConfigVariables(AsyncWebServerRequest *request){
  String cad=variables.generaJsonConfiguracion(false);
  request->send(200, "text/json", cad);
  //request->redirect("Sensores.json");
  }

/***************************************************/
/*                                                 */
/*  Servicio de consulta de estado de las salidas  */ 
/*  devuelve un formato json                       */
/*                                                 */
/***************************************************/  
void handleConfigSalidas(AsyncWebServerRequest *request)
  {
  String cad=salidas.generaJsonEstado(false);
  request->send(200, "text/json", cad);
  }
 
/*****************************************************/
/*                                                   */
/*  Servicio de consulta de estado de las entradas   */
/*  devuelve un formato json                         */
/*                                                   */
/*****************************************************/  
void handleConfigEntradas(AsyncWebServerRequest *request)
  {
  String cad=entradas.generaJsonEstado(false);
  request->send(200, "text/json", cad); 
  }

/*********************************************/
/*                                           */
/*  Servicio de actuacion de rele            */
/*                                           */
/*********************************************/  
void handleActivaRele(AsyncWebServerRequest *request)
  {
  if(request->hasArg("id") ) 
    {
    int8_t id=request->arg("id").toInt();

    //activaRele(id);
    salidas.conmuta(id,ESTADO_ACTIVO);

    request->send(200, "text/plain", "Salida activada");
    }
  else request->send(404, "text/plain", "Error");
  }

/*********************************************/
/*                                           */
/*  Servicio de desactivacion de rele        */
/*                                           */
/*********************************************/  
void handleDesactivaRele(AsyncWebServerRequest *request)
  {
  if(request->hasArg("id") ) 
    {
    int8_t id=request->arg("id").toInt();

    salidas.conmuta(id,ESTADO_DESACTIVO);
    
    request->send(200, "text/plain", "Salida desactivada");
    }
  else request->send(404, "text/plain", "Error"); 
  }

/*********************************************/
/*                                           */
/*  Servicio de pulso de rele                */
/*                                           */
/*********************************************/  
void handlePulsoRele(AsyncWebServerRequest *request)
  {
  if(request->hasArg("id") ) 
    {
    int8_t id=request->arg("id").toInt();

    salidas.setPulso(id);
    
    request->send(200, "text/plain", "Pulso generado");
    }
  else request->send(404, "text/plain", "Error"); 
  }

/*********************************************/
/*                                           */
/*  Servicio para forzar el modo manual      */
/*  del rele                                 */
/*                                           */
/*********************************************/  
void handleFuerzaManual(AsyncWebServerRequest *request)
  {
  if(request->hasArg("id")) 
    {
    int8_t id=request->arg("id").toInt();

    salidas.setModoManual(id);
    
    request->send(200, "text/plain", "Salida en modo manual");
    }
  else request->send(404, "text/plain", "error"); 
  }

/*********************************************/
/*                                           */
/*  Servicio para recuperar el modo inicial  */
/*  del rele                                 */
/*                                           */
/*********************************************/  
void handleRecuperaManual(AsyncWebServerRequest *request)
  {
  if(request->hasArg("id")) 
    {
    int8_t id=request->arg("id").toInt();

    salidas.setModoInicial(id);
    
    request->send(200, "text/plain", "Salida en modo automatico");
    }
  else request->send(404, "text/plain", "Error"); 
  }
/*******************************FIN ACTIVACION/DESACITVACION MEDIDAS, ENTRADAS Y SALIDAS*****************************************************/
/*******************************MAQUINA DE ESADOS*****************************************************/
/*************************************************/
/*                                               */
/*  Servicio de consulta del estado de           */
/*  la maquina de estados                        */
/*                                               */
/*************************************************/  
void handleEstadoMaquinaEstados(AsyncWebServerRequest *request){
    String cad=maquinaEstados.generaJsonEstado();

    request->send(200, "text/json", cad);
}
/****************************FIN MAQUINA ESTADOS******************************************/
/****************************SECUENCIADOR******************************************/
/*********************************************************/
/*                                                       */
/*  Servicio de informacion delestado del secuenciador   */
/*                                                       */
/*********************************************************/
//estadoSecuenciador
void handleEstadoSecuenciador(AsyncWebServerRequest *request){
    String cad=secuenciador.generaJsonEstado();

    request->send(200, "text/json", cad);
}

/*********************************************************/
/*                                                       */
/*  Servicio de informacion de la configuracion          */
/*  del secuenciador                                     */
/*                                                       */
/*********************************************************/
void handleConfigSecuenciador(AsyncWebServerRequest *request){
     request->send(SPIFFS, "/SecuenciadorConfig.json", "text/json");
}
/*********************************************/
/*                                           */
/*  Servicio para activar el secuenciador    */
/*                                           */
/*********************************************/  
void handleActivaSecuenciador(AsyncWebServerRequest *request){
  secuenciador.activar();
  request->send(200, "text/plain", "Secuenciador activado");
}

/*********************************************/
/*                                           */
/*  Servicio para desactivar el secuenciador */
/*                                           */
/*********************************************/  
void handleDesactivaSecuenciador(AsyncWebServerRequest *request){
  secuenciador.desactivar();
  request->send(200, "text/plain", "Secuenciador desactivado");
}
/****************************FIN SECUENCIADOR******************************************/

/****************************GESTION DE FICHEROS******************************************/
/*********************************************/
/*                                           */
/*  Crea un fichero a traves de una          */
/*  peticion HTTP                            */ 
/*                                           */
/*********************************************/  
void handleCreaFichero(AsyncWebServerRequest *request)
  {
  String nombreFichero="";
  String contenidoFichero="";

  if(request->hasArg("nombre") && request->hasArg("contenido")) //si existen esos argumentos
    {
    nombreFichero=request->arg("nombre");
    contenidoFichero=request->arg("contenido");

    if(salvaFichero( nombreFichero, nombreFichero+".bak", contenidoFichero)) request->send(200, "text/plain", "Fichero creado"); 
    else request->send(501, "text/plain", "No se pudo salvar el fichero"); 
    }
  
  request->send(404, "text/plain", "Falta el argumento <nombre de fichero>"); 
  }
/*********************************************/
/*                                           */
/*  Borra un fichero a traves de una         */
/*  peticion HTTP                            */ 
/*                                           */
/*********************************************/  
void handleBorraFichero(AsyncWebServerRequest *request)
  {
  String nombreFichero="";

  if(request->hasArg("nombre") ) //si existen esos argumentos
    {
    nombreFichero=request->arg("nombre");

    if(borraFichero(nombreFichero)) request->send(200, "text/plain", "Fichero borrado"); 
    else request->send(501, "text/plain", "Error al borrar el fichero"); 
    }

  request->send(404, "text/plain", "Falta nombre de fichero"); 
  }

/*********************************************/
/*                                           */
/*  Lee un fichero a traves de una           */
/*  peticion HTTP                            */ 
/*  COPIADO DE ACTUADOR                      */
/*                                           */
/*********************************************/
void handleLeeFichero(AsyncWebServerRequest *request){
  if(request->hasArg("nombre") ) {
    String nombreFichero = request->arg("nombre");
    String contentType = getContentType(nombreFichero);

    if(!nombreFichero.startsWith("/"))  nombreFichero = "/" + nombreFichero;

    File miFichero=SPIFFS.open(nombreFichero,"r");
    if(!miFichero) request->send(404,"text/html", "Fichero no encontrado");
    else{
      //request->send(fichero,  nombreFichero, contentType,false);
      AsyncWebServerResponse *response=request->beginChunkedResponse(contentType, [miFichero](uint8_t *buffer, size_t maxlen, size_t index)->size_t{
        return loadData(miFichero,buffer,maxlen,index);      
      });

      request->send(response);
    }
  }
}

/*********************************************/
/*                                           */
/*  Lista los ficheros en el sistema a       */
/*  traves de una peticion HTTP              */ 
/*                                           */
/*********************************************/  
void handleListaFicheros(AsyncWebServerRequest *request)
  {
  String prefix="/";  

  if(request->hasArg("dir")) prefix=request->arg("dir");

  request->send(200,"text/json",listadoFicheros(prefix));
  }
/****************************FIN DE GESTION DE FICHEROS******************************************/
/****************************INICIO DE NOT FOUND Y RETORNO DESDE SPIFSS******************************************/
/*********************************************/
/*                                           */
/*  Pagina no encontrada                     */
/*                                           */
/*********************************************/
void handleNotFound(AsyncWebServerRequest *request)
  {
  if(!handleFileReadChunked(request))
    {
    Traza.mensaje("No se encontro el fichero en SPIFFS\n");  
    String message = miniCabecera;

    message += "<h1>" + configNVS.nombreServicio + "<br></h1>";
    message += "File Not Found\n\n";
    message += "URI: ";
    message += request->url();
    message += "\nMethod: ";
    message += request->methodToString();
    message += "\nArguments: ";
    message += request->args();
    message += "\n";

    for (uint8_t i=0; i<request->args(); i++)
      {
      message += " " + request->argName(i) + ": " + request->arg(i) + "\n";
      }
      
    message += miniPie;
    request->send(404, "text/html", message);
    }
  }

/*****************************************************/
/*                                                   */
/*  Determina el contentType de la respuesta en base */
/*  al fichero que se lee y se devuelve              */
/*                                                   */
/*****************************************************/
String getContentType(String filename) { // determine the filetype of a given filename, based on the extension
  //if (request->hasArg("download")) return "application/octet-stream";
   if (filename.endsWith(".htm")) return "text/html";
  else if (filename.endsWith(".html")) return "text/html";
  else if (filename.endsWith(".css")) return "text/css";
  else if (filename.endsWith(".js")) return "application/javascript";
  else if (filename.endsWith(".png")) return "image/png";
  else if (filename.endsWith(".gif")) return "image/gif";
  else if (filename.endsWith(".jpg")) return "image/jpeg";
  else if (filename.endsWith(".ico")) return "image/x-icon";
  else if (filename.endsWith(".xml")) return "text/xml";
  else if (filename.endsWith(".pdf")) return "application/x-pdf";
  else if (filename.endsWith(".zip")) return "application/x-zip";
  else if (filename.endsWith(".gz")) return "application/x-gzip";
  else if (filename.endsWith(".json")) return "application/json";
  return "text/plain";
}

/*****************************************************/
/*                                                   */
/*  Lee un fichero del SPIFFS y lo envia como        */
/*  respuest a la request                            */
/*                                                   */
/*****************************************************/
bool handleFileRead(AsyncWebServerRequest *request) 
  {
  String path=request->url();
  // send the right file to the client (if it exists)
  Traza.mensaje("handleFileRead: %s\n", path.c_str());
  
  if (!path.startsWith("/")) path += "/";
  //path = "/www" + path; //busco los ficheros en el SPIFFS en la carpeta www
  
  String contentType = getContentType(path);             // Get the MIME type
  String pathWithGz = path + ".gz";
  //if (SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)) 
  if (existeFichero(pathWithGz) || existeFichero(path)) 
    { // If the file exists, either as a compressed archive, or normal
    if (existeFichero(pathWithGz)) path += ".gz"; // Si hay una version comprimida del fichero disponible, usa la version comprimida
                                             
    request->send(SPIFFS, path, contentType);
    
    Traza.mensaje("\tfichero enviado: %s | contentType: %s\n",path.c_str(),contentType.c_str());
    return true;
    }
  Traza.mensaje("\tfichero no encontrado en SPIFFS: %s\n", path.c_str());   // If the file doesn't exist, return false
  return false;
  }

/*****************************************************/
/*                                                   */
/*  Gestiona los envios del chunked                  */
/*                                                   */
/*****************************************************/
size_t loadData(File f, uint8_t *buffer, size_t maxLen, size_t index){
  //if(!f.available()){Serial.printf("Chungo, salgo\n"); return 0;}

  Serial.printf("Enviado: %i | maxlen: %i | ",index, maxLen);
  size_t salida=f.read(buffer,maxLen);
      
  Serial.printf("salida: %i\n",salida);
  if(!salida) f.close();

  return salida;
}

/*****************************************************/
/*                                                   */
/*  Lee un fichero del SPIFFS y lo envia como        */
/*  respuest a la request, en modo chunked           */
/*                                                   */
/*****************************************************/
bool handleFileReadChunked(AsyncWebServerRequest *request) 
  {
  String path=request->url();

  // send the right file to the client (if it exists)
  Traza.mensaje("handleFileReadChunked: %s\n", path.c_str());
  
  if (!path.startsWith("/")) path += "/";
  //path = "/www" + path; //busco los ficheros en el SPIFFS en la carpeta www
  
  String contentType = getContentType(path);             // Get the MIME type
  String pathWithGz = path + ".gz";
  //if (SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)) 
  if (existeFichero(pathWithGz) || existeFichero(path)) 
    { // If the file exists, either as a compressed archive, or normal
    if (existeFichero(pathWithGz)) path += ".gz"; // Si hay una version comprimida del fichero disponible, usa la version comprimida

    File miFichero = SPIFFS.open(path,FILE_READ);

    if(!miFichero) request->send(500, "text/html", "Error mio...");
    else{
      Traza.mensaje("\tInicio de envio chunked. fichero a enviar: %s | contentType: %s | tamaño: %i\n",path.c_str(),contentType.c_str(),miFichero.size());
    
      AsyncWebServerResponse *response=request->beginChunkedResponse(contentType, [miFichero](uint8_t *buffer, size_t maxlen, size_t index)->size_t {
        return loadData(miFichero,buffer,maxlen,index);
        });

      request->send(response);
      
      return true;
      }
    }
  Traza.mensaje("\tfichero no encontrado en SPIFFS: %s\n", path.c_str());   // If the file doesn't exist, return false
  return false;
  }

/*****************************************************/
/*                                                   */
/*  Recive los ficheros en la subida fileUpload      */
/*  https://github.com/CelliesProjects/minimalUploadAuthESP32/blob/master/upload.htm*/
/*  https://github.com/espressif/arduino-esp32/blob/master/libraries/FS/src/FS.h*/
/*                                                   */
/*****************************************************/
void handleUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final){
  static File newFile;

  if(!index)
    {
    String nombre=filename;
    if (!nombre.startsWith("/")) nombre = "/" + nombre;  
    Serial.printf("UploadStart: %s\n", nombre.c_str());
    newFile = SPIFFS.open(nombre.c_str(), FILE_WRITE);//abro el fichero, si existe lo borra

    if (!newFile) request->send(200,"text/plain","Error al abrir fichero");
    }

  Serial.printf("Index %u\nlen: %i\n",index,len);
  for(size_t i=0; i<len; i++)
    {
    Serial.write(data[i]);    
    if(!newFile.write(data[i])) request->send(200,"text/plain","Error al guardar en el fichero");
    }

  if(final){
    Serial.printf("UploadEnd: %s, %u B\n", filename.c_str(), index+len);
    newFile.close();      
    request->redirect("/www/resultadoUpload.html");
  }
}
/****************************Google Home Notifier ******************************/
void handleSpeechPath(AsyncWebServerRequest *request) 
  {
  String phrase = request->arg("phrase");
  if (phrase == "") 
    {
    request->send(401, "text/plain", "query 'phrase' is not found");
    return;
    }

  if(enviaNotificacion((char*)phrase.c_str())) request->send(200, "text / plain", "OK");
  else request->send(404, "text / plain", "KO");  
  }

String generaJsonServicios(boolean debug){
  String cad="";

  const size_t capacity = JSON_ARRAY_SIZE(5) + JSON_OBJECT_SIZE(1);
  DynamicJsonBuffer jsonBuffer(capacity);

  JsonObject& root = jsonBuffer.createObject();

  JsonArray& Servicios = root.createNestedArray("datos");////Servicios
  if(entradas.getNumEntradas()>0) Servicios.add("Entradas");
  if(salidas.getNumSalidas()>0) Servicios.add("Salidas");
  if(maquinaEstados.getNumEstados()>0) Servicios.add("MaquinaEstados");
  if(secuenciador.getNumPlanes()>0) Servicios.add("Secuenciador");
  if(variables.getNumVariables()>0) Servicios.add("Variables");  

  root.printTo(cad);
  return cad;  
}
