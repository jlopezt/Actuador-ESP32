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
String miniCabecera=" <html><head></head><body><link rel='stylesheet' type='text/css' href='css.css'>\n";
String miniPie="</body></html>";

void handleNombre(AsyncWebServerRequest *request);
void handleMain(AsyncWebServerRequest *request);
void handleRoot(AsyncWebServerRequest *request);

void handleEntradas(AsyncWebServerRequest *request);
void handleEstadoEntradas(AsyncWebServerRequest *request);
void handleConfigEntradas(AsyncWebServerRequest *request);

void handleSalidas(AsyncWebServerRequest *request);
void handleEstadoSalidas(AsyncWebServerRequest *request);
void handleConfigSalidas(AsyncWebServerRequest *request);

void handlePulsoRele(AsyncWebServerRequest *request);
void handleRecuperaManual(AsyncWebServerRequest *request);
void handleFuerzaManual(AsyncWebServerRequest *request);
void handleDesactivaRele(AsyncWebServerRequest *request);
void handleActivaRele(AsyncWebServerRequest *request);

void handleMaquinaEstados(AsyncWebServerRequest *request);
void handleEstadoMaquinaEstados(AsyncWebServerRequest *request);

void handleDesactivaSecuenciador(AsyncWebServerRequest *request);
void handleActivaSecuenciador(AsyncWebServerRequest *request);
void handlePlanes(AsyncWebServerRequest *request);

void handleParticiones(AsyncWebServerRequest *request);
void handleSetNextBoot(AsyncWebServerRequest *request);
void handleInfo(AsyncWebServerRequest *request);
void handleRestart(AsyncWebServerRequest *request);

void handleFicheros(AsyncWebServerRequest *request);
void handleListaFicheros(AsyncWebServerRequest *request);
void handleManageFichero(AsyncWebServerRequest *request);
void handleLeeFichero(AsyncWebServerRequest *request);
void handleBorraFichero(AsyncWebServerRequest *request);
void handleCreaFichero(AsyncWebServerRequest *request);

void handleNotFound(AsyncWebServerRequest *request);
void handleUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final);
bool handleFileRead(AsyncWebServerRequest *request);

void handleSpeechPath(AsyncWebServerRequest *request);

/*********************************** Inicializacion y configuracion *****************************************************************/
void inicializaWebServer(void)
  {
  /*******Configuracion del Servicio Web***********/  
  //Inicializo los serivcios, decalra las URIs a las que va a responder

  serverX.on("/", HTTP_GET, handleMain); //Responde con la identificacion del modulo    
  serverX.on("/root", HTTP_GET, handleRoot); //devuelve un JSON con las medidas, reles y modo para actualizar la pagina de datos
  serverX.on("/estadoEntradas", HTTP_GET, handleEstadoEntradas); //Responde con la identificacion del modulo
  serverX.on("/estadoSalidas", HTTP_GET, handleEstadoSalidas); //Responde con la identificacion del modulo
  serverX.on("/nombre", HTTP_GET, handleNombre); //devuelve un JSON con las medidas, reles y modo para actualizar la pagina de datos
  serverX.on("/salidas", HTTP_GET, handleSalidas); //Servicio de estdo de reles
  serverX.on("/configSalidas", HTTP_GET, handleConfigSalidas); //Servicio de estdo de reles
  serverX.on("/entradas", HTTP_GET, handleEntradas); //Servicio de estdo de reles    
  serverX.on("/configEntradas", HTTP_GET, handleConfigEntradas); //Servicio de estdo de reles
  
  serverX.on("/activaSalida", HTTP_ANY, handleActivaRele); //Servicio de activacion de rele
  serverX.on("/desactivaSalida", HTTP_ANY, handleDesactivaRele);  //Servicio de desactivacion de rele
  serverX.on("/fuerzaSalidaManual", HTTP_ANY, handleFuerzaManual);  //Servicio para formar ua salida a modo manual
  serverX.on("/recuperaSalidaManual", HTTP_ANY, handleRecuperaManual);  //Servicio para formar ua salida a modo manual  
  serverX.on("/pulsoSalida", HTTP_ANY, handlePulsoRele);  //Servicio de pulso de rele

  serverX.on("/planes", HTTP_ANY, handlePlanes);  //Servicio de representacion del plan del secuenciador
  serverX.on("/activaSecuenciador", HTTP_ANY, handleActivaSecuenciador);  //Servicio para activar el secuenciador
  serverX.on("/desactivaSecuenciador", HTTP_ANY, handleDesactivaSecuenciador);  //Servicio para desactivar el secuenciador
  
  serverX.on("/maquinaEstados", HTTP_ANY, handleMaquinaEstados);  //Servicio de representacion de las transiciones d ela maquina de estados
  serverX.on("/estadoMaquinaEstados", HTTP_ANY, handleEstadoMaquinaEstados);  //Servicio de representacion de las transiciones d ela maquina de estados

  serverX.on("/ficheros", HTTP_ANY, handleFicheros);  //URI de leer fichero    
  serverX.on("/listaFicheros", HTTP_ANY, handleListaFicheros);  //URI de leer fichero  
  serverX.on("/manageFichero", HTTP_ANY, handleManageFichero);  //URI de leer fichero  
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

  //Arranco el servidor
  serverX.begin();

  Traza.mensaje("Servicio web iniciado en puerto %i\n", PUERTO_WEBSERVER);
  }
/********************************* FIn inicializacion y configuracion ***************************************************************/

/************************* Gestores de las diferentes URL coniguradas ******************************/
void handleMain(AsyncWebServerRequest *request) {request->redirect("main.html");}//Redirect to our html web page 
void handleRoot(AsyncWebServerRequest *request) {request->redirect("root.html");}//Redirect to our html web page 

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
  root["nombre"] = nombre_dispositivo;
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
  String cad=generaJsonEstadoEntradas();
  
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
  String cad=generaJsonEstadoSalidas();
  
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
  request->redirect("./");
  delay(500);
  ESP.restart();
  }

/*********************************************/
/*                                           */
/*  Pinta la lista de particiones en         */
/*  memoria                                  */ 
/*                                           */
/*********************************************/  
void handleParticiones(AsyncWebServerRequest *request)
  {
  String cad=miniCabecera;

  //cad += "<link rel='stylesheet' type='text/css' href='css.css'>";
  cad += pintaParticionHTML();
  cad += miniPie;

  request->send(200, "text/html", cad);
  }

/*********************************************/
/*                                           */
/*    Cambia la particion de arranque        */
/*                                           */
/*********************************************/  
void handleSetNextBoot(AsyncWebServerRequest *request)
  {
  if(request->hasArg("p")) {setParticionProximoArranque(request->arg("p"));}//si existen esos argumentos

  request->redirect("particiones");
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
  String cad=generaJsonEstadoSalidas(false);
  request->send(200, "text/json", cad);
  }
void handleSalidas(AsyncWebServerRequest *request) {request->redirect("salidas.html");}
  
/*****************************************************/
/*                                                   */
/*  Servicio de consulta de estado de las entradas   */
/*  devuelve un formato json                         */
/*                                                   */
/*****************************************************/  
void handleConfigEntradas(AsyncWebServerRequest *request)
  {
  String cad=generaJsonEstadoEntradas(false);
  request->send(200, "text/json", cad); 
  }
void handleEntradas(AsyncWebServerRequest *request) {request->redirect("entradas.html");}

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
    salidas[id].conmuta(ESTADO_ACTIVO);

    request->redirect("root");
    }
    else request->send(404, "text/plain", "");  
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

    salidas[id].conmuta(ESTADO_DESACTIVO);
    
    request->redirect("root");
    }
  else request->send(404, "text/plain", ""); 
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

    salidas[id].setPulso();
    
    request->redirect("root");
    }
  else request->send(404, "text/plain", ""); 
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

    salidas[id].setModoManual();
    
    request->redirect("root");
    }
  else request->send(404, "text/plain", ""); 
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

    salidas[id].setModoInicial();
    
    request->redirect("root");
    }
  else request->send(404, "text/plain", ""); 
  }
/*******************************FIN ACTIVACION/DESACITVACION ENTRADAS/SALIDAS*****************************************************/
/*******************************MAQUINA DE ESADOS*****************************************************/
/*************************************************/
/*                                               */
/*  Servicio de consulta de las transiciones de  */
/*  la maquina de estados                        */
/*                                               */
/*************************************************/  
void handleMaquinaEstados(AsyncWebServerRequest *request)
  {
  String cad=miniCabecera;
  String orden="";

  //Estado actual
  cad += "<TABLE border=\"0\" width=\"50%\" cellpadding=\"0\" cellspacing=\"0\" width=\"300\" class=\"tabla\">\n";
  cad += "<CAPTION>Estado actual</CAPTION>\n"; 
  cad += "<TR>\n";
  cad += "<TH width=\"100\">Estado: </TH>";
  cad += "<TD width=\"200\" class=\"modo2\">" + maquinaEstados.getNombreEstadoActual() + "</TD>";
  cad += "</TR>\n";
  cad += "</TABLE>\n";
  cad += "<BR>";

  //Entradas
  cad += "<TABLE border=\"0\" width=\"50%\" cellpadding=\"0\" cellspacing=\"0\" width=\"300\" class=\"tabla\">\n";
  cad += "<CAPTION>Entradas actual</CAPTION>\n";  
  cad += "<TR>"; 
  cad += "<TH>id</TH>";
  cad += "<TH>Nombre</TH>";
  cad += "<TH>Ultimo estado leido</TH>";
  cad += "</TR>";    
  for(uint8_t i=0;i<maquinaEstados.getNumEntradas();i++)
    {
    cad += "<TR class=\"modo2\">";  
    cad += "<TD>" + String(i) + ":" + String(maquinaEstados.getMapeoEntrada(i)) + "</TD>";  
    cad += "<TD>" + entradas[maquinaEstados.getMapeoEntrada(i)].getNombre()+ "</TD>";
    cad += "<TD>" + String(maquinaEstados.getEntradasActual(i)) + ":" + String(entradas[maquinaEstados.getMapeoEntrada(i)].getEstado()) + "</TD>";
    cad += "</TR>";
    }
  cad += "</TABLE>";
  cad += "<BR>";
  
  //Salidas
  cad += "<TABLE border=\"0\" width=\"50%\" cellpadding=\"0\" cellspacing=\"0\" width=\"300\" class=\"tabla\">\n";
  cad += "<CAPTION>Salidas actual</CAPTION>\n";  
  cad += "<TR>"; 
  cad += "<TH>id</TH>";  
  cad += "<TH>Nombre</TH>";
  cad += "<TH>Estado actual</TH>";
  cad += "</TR>";    
  for(uint8_t i=0;i<maquinaEstados.getNumSalidas();i++)
    {
    cad += "<TR class=\"modo2\">";  
    cad += "<TD>" + String(i) + ":" + String(maquinaEstados.getMapeoSalida(i)) + "</TD>";  
    cad += "<TD>" + String(salidas[maquinaEstados.getMapeoSalida(i)].getNombre()) + "</TD>";
    cad += "<TD>" + String(salidas[maquinaEstados.getMapeoSalida(i)].getEstado()) + "</TD>";
    cad += "</TR>";
    }
  cad += "</TABLE>";
  cad += "<BR>";
  
  //Estados  
  cad += "<TABLE border=\"0\" width=\"50%\" cellpadding=\"0\" cellspacing=\"0\" width=\"300\" class=\"tabla\">\n";
  cad += "<CAPTION>ESTADOS</CAPTION>\n";  
  cad += "<TR>"; 
  cad += "<TH>id</TH>";  
  cad += "<TH>Nombre</TH>";
  for(uint8_t i=0;i<maquinaEstados.getNumSalidas();i++) cad += "<TH>" + salidas[maquinaEstados.getMapeoSalida(i)].getNombre() + "</TH>";
  cad += "</TR>"; 
    
  for(uint8_t i=0;i<maquinaEstados.getNumEstados();i++)
    {
    cad += "<TR class=\"modo2\">";  
    cad += "<TD>" + String(i) + "</TD>";  
    cad += "<TD>" + maquinaEstados.estados[i].getNombre() + "</TD>";
    for(uint8_t j=0;j<maquinaEstados.getNumSalidas();j++) cad += "<TD>" + String(maquinaEstados.estados[i].getValorSalida(j)) + "</TD>";
    cad += "</TR>";
    }
  cad += "</TABLE>";
  cad += "<BR>";
  
  //Transiciones
  cad += "<TABLE border=\"0\" width=\"50%\" cellpadding=\"0\" cellspacing=\"0\" width=\"300\" class=\"tabla\">\n";
  cad += "<CAPTION>TRANSICIONES</CAPTION>\n";  

  cad += "<TR>";
  cad += "<TH ROWSPAN=2>Estado inicial</TH>";
  cad += "<TH COLSPAN=" + String(maquinaEstados.getNumEntradas()) + ">Entradas</TH>";
  cad += "<TH ROWSPAN=2>Estado final</TH>";
  cad += "</TR>";
  cad += "<TR>";
  for(uint8_t i=0;i<maquinaEstados.getNumEntradas();i++) cad += "<TH>" + String(entradas[i].getNombre()) + "</TH>";
  cad += "</TR>";

  for(uint8_t i=0;i<maquinaEstados.getNumTransiciones();i++)
    {
    cad += "<TR class=\"modo2\">";
    cad += "<TD>" + String(maquinaEstados.estados[maquinaEstados.transiciones[i].getEstadoInicial()].getNombre()) + "</TD>";
    for(uint8_t j=0;j<maquinaEstados.getNumEntradas();j++) cad += "<TD>" + String(maquinaEstados.transiciones[i].getValorEntrada(j)) + "</TD>";
    cad += "<TD>" + String(maquinaEstados.estados[maquinaEstados.transiciones[i].getEstadoFinal()].getNombre()) + "</TD>";    
    cad += "</TR>";
    }
  cad += "</TABLE>";
  cad += "<BR>";
  
  cad += miniPie;
  request->send(200, "text/html", cad);
  }

/***********************************************/
/* Genera el JSON de estado de la Maq. Estados */
/***********************************************/
void handleEstadoMaquinaEstados(AsyncWebServerRequest *request){
    String cad=maquinaEstados.generaJsonEstadoMaquinaEstados();

    request->send(200, "text/json", cad);
}
/****************************FIN MAQUINA ESTADOS******************************************/
/****************************SECUENCIADOR******************************************/
/*********************************************/
/*                                           */
/*  Servicio de representacion de los        */
/*  planes del secuenciador                  */
/*                                           */
/*********************************************/  
void handlePlanes(AsyncWebServerRequest *request)
  {
  int8_t numPlanes=getNumPlanes();  
  String cad="";

  cad += "<h3>hay " + String(numPlanes) + " plan(es) activo(s).</h3>";
  
  for(int8_t i=0;i<numPlanes;i++)
    {
    cad += pintaPlanHTML(i);
    cad += "<BR>";
    }
  
  request->send(200, "text/html", cad); 
  
  }

/*********************************************/
/*                                           */
/*  Servicio para activar el secuenciador    */
/*                                           */
/*********************************************/  
void handleActivaSecuenciador(AsyncWebServerRequest *request)
  {
  activarSecuenciador();
  handleRoot(request);
  }

/*********************************************/
/*                                           */
/*  Servicio para desactivar el secuenciador */
/*                                           */
/*********************************************/  
void handleDesactivaSecuenciador(AsyncWebServerRequest *request)
  {
  desactivarSecuenciador();
  handleRoot(request);
  }

/****************************FIN SECUENCIADOR******************************************/
/*********************************************/
/*                                           */
/*  Lee info del chipset mediante            */
/*  peticion HTTP                            */ 
/*                                           */
/*********************************************/  
void handleInfo(AsyncWebServerRequest *request)
  {
  String cad="";

  cad+="<link rel='stylesheet' type='text/css' href='css.css'>\n";
  cad+= "<p style=\"color:black;\">";
  cad+= "-----------------Uptime---------------------<BR>";
  char tempcad[20]="";
  sprintf(tempcad,"%lu", (unsigned long)(esp_timer_get_time()/(unsigned long)1000000)); //la funcion esp_timer_get_time() devuelve el contador de microsegundos desde el arranque. rota cada 292.000 años  
  cad += "Uptime: " + String(tempcad) + " segundos"; //la funcion esp_timer_get_time() devuelve el contador de microsegundos desde el arranque. rota cada 292.000 años
  cad += "<BR>-----------------------------------------------<BR>";  

  cad+= "<BR>-----------------info logica-----------------<BR>";
  cad += "Hora actual: " + getHora(); 
  cad += "<BR>";
  cad += "Fecha actual: " + getFecha(); 
  cad += "<BR>";
  cad += "IP: " + String(getIP(debugGlobal));
  cad += "<BR>";  
  cad += "nivelActivo: " + String(nivelActivo);
  cad += "<BR>";  
  for(int8_t i=0;i<MAX_SALIDAS;i++)
    {
    cad += "Rele " + String(i) + " nombre: " + salidas[i].getNombre() + "| estado: " + salidas[i].getEstado();    
    cad += "<BR>";   
    }
  cad += "-----------------------------------------------<BR>"; 
  
  cad += "<BR>-----------------WiFi info-----------------<BR>";
  cad += "SSID: " + nombreSSID();
  cad += "<BR>";    
  cad += "IP: " + WiFi.localIP().toString();
  cad += "<BR>";    
  cad += "Potencia: " + String(WiFi.RSSI());
  cad += "<BR>";    
  cad += "-----------------------------------------------<BR>";  

  cad += "<BR>-----------------MQTT info-----------------<BR>";
  cad += "IP broker: " + getIPBroker().toString();
  cad += "<BR>";
  cad += "Puerto broker: " +   getPuertoBroker();
  cad += "<BR>";  
  cad += "Usuario: " + getUsuarioMQTT();
  cad += "<BR>";  
  cad += "Password: " + getPasswordMQTT();
  cad += "<BR>";  
  cad += "Topic root: " + getTopicRoot();
  cad += "<BR>";  
  cad += "-----------------------------------------------<BR>";  

  cad += "<BR>-----------------Hardware info-----------------<BR>";
  cad += "FreeHeap: " + String(ESP.getFreeHeap());
  cad += "<BR>";
  cad += "ChipId: " + String(ESP.getChipRevision());
  cad += "<BR>";  
  cad += "SdkVersion: " + String(ESP.getSdkVersion());
  cad += "<BR>";  
  cad += "CpuFreqMHz: " + String(ESP.getCpuFreqMHz());
  cad += "<BR>";  
     //gets the size of the flash as set by the compiler
  cad += "FlashChipSize: " + String(ESP.getFlashChipSize());
  cad += "<BR>";  
  cad += "FlashChipSpeed: " + String(ESP.getFlashChipSpeed());
  cad += "<BR>";  
  cad += "-----------------------------------------------<BR>";  
  cad += "</p>";
 
  request->send(200, "text/html", cad);     
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
  String cad="";
  String nombreFichero="";
  String contenidoFichero="";

  if(request->hasArg("nombre") && request->hasArg("contenido")) //si existen esos argumentos
    {
    nombreFichero=request->arg("nombre");
    contenidoFichero=request->arg("contenido");

    if(salvaFichero( nombreFichero, nombreFichero+".bak", contenidoFichero)) 
	    {
      String cad=directorioFichero(nombreFichero);
      request->redirect("ficheros?dir=" + cad);
      return;
	    }  
    else cad += "No se pudo salvar el fichero<br>"; 
    }
  else cad += "Falta el argumento <nombre de fichero>"; 

  request->send(200, "text/html", cad); 
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
  String contenidoFichero="";
  String cad="";

  if(request->hasArg("nombre") ) //si existen esos argumentos
    {
    nombreFichero=request->arg("nombre");

    if(borraFichero(nombreFichero)) 
      {
      String cad=directorioFichero(nombreFichero);
      request->redirect("ficheros?dir=" + cad);
      return;
      }
    else cad += "No sepudo borrar el fichero " + nombreFichero + ".\n"; 
    }
  else cad += "Falta el argumento <nombre de fichero>"; 

  request->send(200, "text/html", cad); 
  }

/*********************************************/
/*                                           */
/*  Lee un fichero a traves de una           */
/*  peticion HTTP                            */ 
/*                                           */
/*********************************************/  
void handleLeeFichero(AsyncWebServerRequest *request)
  {
  String cad="";
  String nombreFichero="";
  String contenido="";
   
  if(request->hasArg("nombre") ) //si existen esos argumentos
    {
    nombreFichero=request->arg("nombre");

    if(leeFichero(nombreFichero, contenido))
      {
      cad += "El fichero tiene un tama&ntilde;o de ";
      cad += contenido.length();
      cad += " bytes.<BR>";           
      cad += "El contenido del fichero es:<BR>";
      cad += "<textarea readonly=true cols=75 rows=20 name=\"contenido\">";
      cad += contenido;
      cad += "</textarea>";
      cad += "<BR>";
      }
    else cad += "Error al abrir el fichero " + nombreFichero + "<BR>";   
    }
  else cad += "Falta el argumento <nombre de fichero>"; 

  request->send(200, "text/html", cad); 
  }

/*********************************************/
/*                                           */
/*  Habilita la edicion y borrado del        */
/*  fichero indicado, a traves de una        */
/*  peticion HTTP                            */ 
/*                                           */
/*********************************************/ 
void handleManageFichero(AsyncWebServerRequest *request)
  {
  String nombreFichero="";
  String contenido="";
  String cad=miniCabecera;
   
  if(request->hasArg("nombre") ) //si existen esos argumentos
    {
    nombreFichero=request->arg("nombre");

    if(leeFichero(nombreFichero, contenido))
      {           
      //cad += "<link rel='stylesheet' type='text/css' href='css.css'>";
      //cad += "<style> table{border-collapse: collapse;} th, td{border: 1px solid black; padding: 5px; text-align: left;}</style>";

      cad += "<form id=\"borrarFichero\" action=\"/borraFichero\">\n";
      cad += "  <input type=\"hidden\" name=\"nombre\" value=\"" + nombreFichero + "\">\n";
      cad += "</form>\n";

      cad += "<form id=\"salvarFichero\" action=\"creaFichero\" target=\"_self\">";
      cad += "  <input type=\"hidden\" name=\"nombre\" value=\"" + nombreFichero + "\">";
      cad += "</form>\n";

      cad += "<div id=\"contenedor\" style=\"width:900px;\">\n";
      cad += "  <p align=\"center\" style=\"margin-top: 0px;font-size: 16px; background-color: #83aec0; background-repeat: repeat-x; color: #FFFFFF; font-family: Trebuchet MS, Arial; text-transform: uppercase;\">Fichero: " + nombreFichero + "(" + contenido.length() + ")</p>\n";
      cad += "  <BR>\n";
      cad += "  <button form=\"salvarFichero\" style=\"float: left;\" type=\"submit\" value=\"Submit\">Salvar</button>\n";
      cad += "  <button form=\"borrarFichero\" style=\"float: right;\" type=\"submit\" value=\"Submit\">Borrar</button>\n";
      cad += "  <BR><BR>\n";
      cad += "  <textarea form=\"salvarFichero\" cols=120 rows=45 name=\"contenido\">" + contenido + "</textarea>\n";
      cad += "</div>\n";
      }
    else cad += "Error al abrir el fichero " + nombreFichero + "<BR>";
    }
  else cad += "Falta el argumento <nombre de fichero>"; 

  cad += miniPie;
  request->send(200, "text/html", cad); 
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

void handleFicheros(AsyncWebServerRequest *request)
  {
  String prefix="/";  

  if(request->hasArg("dir")) prefix=request->arg("dir");

  request->redirect("ficheros.html?dir=" + prefix);
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
  if(!handleFileRead(request))
    {
    Traza.mensaje("No se encontro el fichero en SPIFFS\n");  
    String message = miniCabecera;

    message = "<h1>" + nombre_dispositivo + "<br></h1>";
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
