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

#include <WiFi.h>
#include <AsyncTCP.h>
#include "ESPAsyncWebServer.h"
#include <FS.h>     //this needs to be first, or it all crashes and burns...
#include <SPIFFS.h>
/***************************** Includes *****************************/

//Variables globales
AsyncWebServer serverX(PUERTO_WEBSERVER);

//Cadenas HTML precargadas
String miniCabecera="<html><head></head><body><link rel='stylesheet' type='text/css' href='css.css'>\n";
String miniPie="</body></html>";

//void handleMain(AsyncWebServerRequest *request);
//void handleRoot(AsyncWebServerRequest *request);
void handleNombre(AsyncWebServerRequest *request);

//void handleEntradas(AsyncWebServerRequest *request);
void handleEstadoEntradas(AsyncWebServerRequest *request);
void handleConfigEntradas(AsyncWebServerRequest *request);

//void handleSalidas(AsyncWebServerRequest *request);
void handleEstadoSalidas(AsyncWebServerRequest *request);
void handleConfigSalidas(AsyncWebServerRequest *request);

void handlePulsoRele(AsyncWebServerRequest *request);
void handleRecuperaManual(AsyncWebServerRequest *request);
void handleFuerzaManual(AsyncWebServerRequest *request);
void handleDesactivaRele(AsyncWebServerRequest *request);
void handleActivaRele(AsyncWebServerRequest *request);

//void handleMaquinaEstados(AsyncWebServerRequest *request);
void handleEstadoMaquinaEstados(AsyncWebServerRequest *request);

//void handleSecuenciador(AsyncWebServerRequest *request);
void handleDesactivaSecuenciador(AsyncWebServerRequest *request);
void handleActivaSecuenciador(AsyncWebServerRequest *request);
void handleEstadoSecuenciador(AsyncWebServerRequest *request);
void handleConfigSecuenciador(AsyncWebServerRequest *request);

void handleParticiones(AsyncWebServerRequest *request);
void handleSetNextBoot(AsyncWebServerRequest *request);
void handleInfo(AsyncWebServerRequest *request);
void handleRestart(AsyncWebServerRequest *request);

//void handleFicheros(AsyncWebServerRequest *request);
void handleListaFicheros(AsyncWebServerRequest *request);
//void handleEditaFichero(AsyncWebServerRequest *request);
void handleLeeFichero(AsyncWebServerRequest *request);
void handleBorraFichero(AsyncWebServerRequest *request);
void handleCreaFichero(AsyncWebServerRequest *request);

void handleNotFound(AsyncWebServerRequest *request);
void handleUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final);
bool handleFileRead(AsyncWebServerRequest *request);
bool handleFileReadChunked(AsyncWebServerRequest *request);
size_t loadData(File f, uint8_t *buffer, size_t maxLen, size_t index);
String getContentType(String filename);

void handleSpeechPath(AsyncWebServerRequest *request);

/*********************************** Inicializacion y configuracion *****************************************************************/
void inicializaWebServer(void)
  {
  /*******Configuracion del Servicio Web***********/  
  //Inicializo los serivcios, decalra las URIs a las que va a responder

  //serverX.on("/", HTTP_GET, handleMain); //Responde con la identificacion del modulo    
  //serverX.on("/root", HTTP_GET, handleRoot); //devuelve un JSON con las medidas, reles y modo para actualizar la pagina de datos
  serverX.on("/estadoEntradas", HTTP_GET, handleEstadoEntradas); //Responde con la identificacion del modulo
  serverX.on("/estadoSalidas", HTTP_GET, handleEstadoSalidas); //Responde con la identificacion del modulo
  serverX.on("/nombre", HTTP_GET, handleNombre); //devuelve un JSON con las medidas, reles y modo para actualizar la pagina de datos
  //serverX.on("/salidas", HTTP_GET, handleSalidas); //Servicio de estdo de reles
  serverX.on("/configSalidas", HTTP_GET, handleConfigSalidas); //Servicio de estdo de reles
  //serverX.on("/entradas", HTTP_GET, handleEntradas); //Servicio de estdo de reles    
  serverX.on("/configEntradas", HTTP_GET, handleConfigEntradas); //Servicio de estdo de reles
  
  serverX.on("/activaSalida", HTTP_ANY, handleActivaRele); //Servicio de activacion de rele
  serverX.on("/desactivaSalida", HTTP_ANY, handleDesactivaRele);  //Servicio de desactivacion de rele
  serverX.on("/fuerzaSalidaManual", HTTP_ANY, handleFuerzaManual);  //Servicio para formar ua salida a modo manual
  serverX.on("/recuperaSalidaManual", HTTP_ANY, handleRecuperaManual);  //Servicio para formar ua salida a modo manual  
  serverX.on("/pulsoSalida", HTTP_ANY, handlePulsoRele);  //Servicio de pulso de rele

  //serverX.on("/secuenciador", HTTP_GET, handleSecuenciador); //Servicio de estdo de reles
  serverX.on("/configSecuenciador", HTTP_GET, handleConfigSecuenciador); //Servicio de estdo de reles
  serverX.on("/estadoSecuenciador", HTTP_ANY, handleEstadoSecuenciador);  //Serivico de estado del secuenciador
  serverX.on("/activaSecuenciador", HTTP_ANY, handleActivaSecuenciador);  //Servicio para activar el secuenciador
  serverX.on("/desactivaSecuenciador", HTTP_ANY, handleDesactivaSecuenciador);  //Servicio para desactivar el secuenciador

  //serverX.on("/maquinaEstados", HTTP_ANY, handleMaquinaEstados);  //Servicio de representacion de las transiciones de la maquina de estados
  serverX.on("/estadoMaquinaEstados", HTTP_ANY, handleEstadoMaquinaEstados);  //Servicio de representacion del estado de la maquina de estados

  //serverX.on("/ficheros", HTTP_ANY, handleFicheros);  //URI de leer fichero    
  serverX.on("/listaFicheros", HTTP_ANY, handleListaFicheros);  //URI de leer fichero  
  //serverX.on("/editaFichero", HTTP_ANY, handleEditaFichero);  //Devuelve la pagina estatica editaFichero.html
  serverX.on("/creaFichero", HTTP_ANY, handleCreaFichero);  //URI de crear fichero
  serverX.on("/borraFichero", HTTP_ANY, handleBorraFichero);  //URI de borrar fichero
  serverX.on("/leeFichero", HTTP_ANY, handleLeeFichero);  //URI de leer fichero

  serverX.on("/particiones", HTTP_ANY, handleParticiones);  
  serverX.on("/setNextBoot", HTTP_ANY, handleSetNextBoot);

  serverX.on("/restart", HTTP_ANY, handleRestart);  //URI de test
  serverX.on("/info", HTTP_ANY, handleInfo);  //URI de test
 
  //upload de ficheros
  serverX.on("/upload", HTTP_GET, [](AsyncWebServerRequest *request) {request->redirect("upload.html");});
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
//void handleMain(AsyncWebServerRequest *request) {request->redirect("main.html");}//Redirect to our html web page 
//void handleRoot(AsyncWebServerRequest *request) {request->redirect("root.html");}//Redirect to our html web page 

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
  root["nombreDispositivo"] = nombre_dispositivo;
  root["version"] = VERSION;
  
  String cad="";
  root.printTo(cad);
  request->send(200,"text/json",cad);
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
/*******************************FIN UTILIDADES*****************************************************/
/*******************************ACTIVACION/DESACITVACION ENTRADAS/SALIDAS*****************************************************/
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
//void handleSalidas(AsyncWebServerRequest *request) {request->redirect("salidas.html");}
  
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
//void handleEntradas(AsyncWebServerRequest *request) {request->redirect("entradas.html");}

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
/*******************************FIN ACTIVACION/DESACITVACION ENTRADAS/SALIDAS*****************************************************/
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
/****************************************************************/
/*                                                              */
/*   Servicio de representacion de los datos del secuenciador   */
/*                                                              */
/****************************************************************/
//void handleSecuenciador(AsyncWebServerRequest *request){request->redirect("secuenciador.html");}

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
/*  Habilita la edicion y borrado del        */
/*  fichero indicado, a traves de una        */
/*  peticion HTTP                            */ 
/*                                           */
/*********************************************/ 
/*
void handleEditaFichero(AsyncWebServerRequest *request){
  if(request->hasArg("nombre") ) //si existen esos argumentos
    {
    request->redirect("editaFichero.html?nombre=" + request->arg("nombre"));
    return;
    }
  
  AsyncResponseStream *response = request->beginResponseStream("text/html");   
  response->printf(miniCabecera.c_str());
  response->printf("Falta el argumento <nombre de fichero>"); 
  response->printf(miniPie.c_str());
  request->send(response);     
}
*/
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

/*
void handleFicheros(AsyncWebServerRequest *request)
  {
  String prefix="/";  

  if(request->hasArg("dir")) prefix=request->arg("dir");

  request->redirect("ficheros.html?dir=" + prefix);
  }
*/  
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

    message += "<h1>" + nombre_dispositivo + "<br></h1>";
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
  path = "/www" + path; //busco los ficheros en el SPIFFS en la carpeta www
  
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
  path = "/www" + path; //busco los ficheros en el SPIFFS en la carpeta www
  
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
    request->redirect("resultadoUpload.html");
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
