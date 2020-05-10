/************************************************************************************************
Servicio                       URL                     Formato entrada Formato salida                                            Comentario                                            Ejemplo peticion              Ejemplo respuesta
Estado de los reles            http://IP/estado        N/A             <id_0>#<nombre_0>#<estado_0>|<id_1>#<nombre_1>#<estado_1> Devuelve el id de cada rele y su estado               http://IP/estado              1#1|2#0
Activa rele                    http://IP/activaRele    id=<id>         <id>#<estado>                                             Activa el rele indicado y devuelve el estado leido    http://IP/activaRele?id=1     1|1
Desactivarele                  http://IP/desactivaRele id=<id>         <id>#<estado>                                             Desactiva el rele indicado y devuelve el estado leido http://IP/desactivaRele?id=0  0|0
Test                           http://IP/test          N/A             HTML                                                      Verifica el estado del Actuadot   
Reinicia el actuador           http://IP/restart
Informacion del Hw del sistema http://IP/info
************************************************************************************************/

//Configuracion de los servicios web
#define PUERTO_WEBSERVER 80
#define ACTIVO    String("#EEEE00")
#define DESACTIVO String("#AAAAAA")
//#define ACTIVO    String("#EEEE00")
//#define DESACTIVO String("#AAAAAA")
#define FONDO     String("#DDDDDD")
#define TEXTO     String("#000000")
#define ACTIVO    String("#FFFF00")
#define DESACTIVO String("#DDDDDD")

//Includes
#ifdef ESP32
  #include <WiFi.h>
  #include <AsyncTCP.h>
#elif defined(ESP8266)
  #include <ESP8266WiFi.h>
  #include <ESPAsyncTCP.h>
#endif

#include "ESPAsyncWebServer.h"

//Variables globales
//WebServer server(PUERTO_WEBSERVER);
AsyncWebServer serverX(PUERTO_WEBSERVER);

//Cadenas HTML precargadas
String cabeceraHTML="";
String enlaces="<TABLE>\n<CAPTION>Enlaces</CAPTION>\n<TR><TD><a href=\"info\" target=\"_self\">Info</a></TD></TR>\n<TR><TD><a href=\"test\" target=\"_self\">Test</a></TD></TR>\n<TR><TD><a href=\"restart\" target=\"_self\">Restart</a></TD></TR>\n<TR><TD><a href=\"listaFicheros\" target=\"_self\">Lista ficheros</a></TD></TR>\n<TR><TD><a href=\"estado\" target=\"_self\">Estado</a></TD></TR>\n<TR><TD><a href=\"estadoSalidas\" target=\"_self\">Estado salidas</a></TD></TR>\n<TR><TD><a href=\"estadoEntradas\" target=\"_self\">Estado entradas</a></TD></TR>\n<TR><TD><a href=\"planes\" target=\"_self\">Planes del secuenciador</a></TD></TR>\n<TR><TD><a href=\"maquinaEstados\" target=\"_self\">Maquina de estados</a></TD></TR></TABLE>\n"; 
String pieHTML="</BODY></HTML>";

void handleNotFound(AsyncWebServerRequest *request);
bool handleFileRead(AsyncWebServerRequest *request);
void handleSpeechPath(AsyncWebServerRequest *request);
void handleFileUpload(AsyncWebServerRequest *request);
void handleManageFichero(AsyncWebServerRequest *request);
void handleLeeFichero(AsyncWebServerRequest *request);
void handleBorraFichero(AsyncWebServerRequest *request);
void handleCreaFichero(AsyncWebServerRequest *request);
void handleListaFicheros(AsyncWebServerRequest *request);
void handleInfo(AsyncWebServerRequest *request);
void handleRestart(AsyncWebServerRequest *request);
void handleTest(AsyncWebServerRequest *request);
void handleMaquinaEstados(AsyncWebServerRequest *request);
void handleDesactivaSecuenciador(AsyncWebServerRequest *request);
void handleActivaSecuenciador(AsyncWebServerRequest *request);
void handlePlanes(AsyncWebServerRequest *request);
void handlePulsoRele(AsyncWebServerRequest *request);
void handleRecuperaManual(AsyncWebServerRequest *request);
void handleFuerzaManual(AsyncWebServerRequest *request);
void handleDesactivaRele(AsyncWebServerRequest *request);
void handleActivaRele(AsyncWebServerRequest *request);
void handleEstadoEntradas(AsyncWebServerRequest *request);
void handleEstadoSalidas(AsyncWebServerRequest *request);
void handleEstado(AsyncWebServerRequest *request);
void handleRoot(AsyncWebServerRequest *request);

/*********************************** Inicializacion y configuracion *****************************************************************/
void inicializaWebServer(void)
  {
  //lo inicializo aqui, despues de leer el nombre del dispositivo en la configuracion del cacharro
  cabeceraHTML="<HTML><HEAD><TITLE>" + nombre_dispositivo + " </TITLE></HEAD><BODY><h1><a href=\"../\" target=\"_self\">" + nombre_dispositivo + "</a><br></h1>";
    
  /*******Configuracion del Servicio Web***********/  
  //Inicializo los serivcios  
  //decalra las URIs a las que va a responder
  serverX.on("/", HTTP_ANY, handleRoot); //Responde con la iodentificacion del modulo
  serverX.on("/estado", HTTP_ANY, handleEstado); //Servicio de estdo de reles
  serverX.on("/estadoSalidas", HTTP_ANY, handleEstadoSalidas); //Servicio de estdo de reles
  serverX.on("/estadoEntradas", HTTP_ANY, handleEstadoEntradas); //Servicio de estdo de reles    
  serverX.on("/activaRele", HTTP_ANY, handleActivaRele); //Servicio de activacion de rele
  serverX.on("/desactivaRele", HTTP_ANY, handleDesactivaRele);  //Servicio de desactivacion de rele
  serverX.on("/fuerzaManualSalida", HTTP_ANY, handleFuerzaManual);  //Servicio para formar ua salida a modo manual
  serverX.on("/recuperaManualSalida", HTTP_ANY, handleRecuperaManual);  //Servicio para formar ua salida a modo manual  

  serverX.on("/pulsoRele", HTTP_ANY, handlePulsoRele);  //Servicio de pulso de rele
  serverX.on("/planes", HTTP_ANY, handlePlanes);  //Servicio de representacion del plan del secuenciador
  serverX.on("/activaSecuenciador", HTTP_ANY, handleActivaSecuenciador);  //Servicio para activar el secuenciador
  serverX.on("/desactivaSecuenciador", HTTP_ANY, handleDesactivaSecuenciador);  //Servicio para desactivar el secuenciador
  serverX.on("/maquinaEstados", HTTP_ANY, handleMaquinaEstados);  //Servicio de representacion de las transiciones d ela maquina de estados
      
  serverX.on("/test", HTTP_ANY, handleTest);  //URI de test
  serverX.on("/restart", HTTP_ANY, handleRestart);  //URI de test
  serverX.on("/info", HTTP_ANY, handleInfo);  //URI de test
  
  serverX.on("/listaFicheros", HTTP_ANY, handleListaFicheros);  //URI de leer fichero
  serverX.on("/creaFichero", HTTP_ANY, handleCreaFichero);  //URI de crear fichero
  serverX.on("/borraFichero", HTTP_ANY, handleBorraFichero);  //URI de borrar fichero
  serverX.on("/leeFichero", HTTP_ANY, handleLeeFichero);  //URI de leer fichero
  serverX.on("/manageFichero", HTTP_ANY, handleManageFichero);  //URI de leer fichero  
#ifdef ESP8266
  server.on("/infoFS", handleInfoFS);  //URI de info del FS
#endif
/*
  serverX.on("/edit.html",  HTTP_POST, [](AsyncWebServerRequest *request) {  // If a POST request is sent to the /edit.html address,
                                           request->send(200, "text/plain", ""); 
                                           }, handleFileUpload);                       // go to 'handleFileUpload'
  */
#ifdef ESP32  
  serverX.on("/speech", HTTP_ANY, handleSpeechPath);
#endif  
  
  serverX.onNotFound(handleNotFound);//pagina no encontrada

  serverX.begin();

  Serial.printf("Servicio web iniciado en puerto %i\n", PUERTO_WEBSERVER);
  }
/********************************* FIn inicializacion y configuracion ***************************************************************/

/************************* Gestores de las diferentes URL coniguradas ******************************/
/*************************************************/
/*                                               */
/*  Pagina principal. informacion de E/S         */
/*  Enlaces a las principales funciones          */
/*                                               */
/*************************************************/  
void handleRoot(AsyncWebServerRequest *request) 
  {
  String cad="";

  //genero la respuesta por defecto
  cad += cabeceraHTML;

  //Entradas  
  cad += "<TABLE style=\"border: 2px solid black\">\n";
  cad += "<CAPTION>Entradas</CAPTION>\n";  
  for(int8_t i=0;i<MAX_ENTRADAS;i++)
    {    
    if(entradaConfigurada(i)==CONFIGURADO) 
      {
      //cad += "<TR><TD>" + nombreEntrada(i) + "-></TD><TD>" + String(nombreEstadoEntrada(i,estadoEntrada(i))) + "</TD></TR>\n";    
      cad += "<TR>";
      cad += "<TD STYLE=\"color: " + TEXTO + "; text-align: center; background-color: " + FONDO + "\">" + nombreEntrada(i) + "</TD>";
      cad += "<TD STYLE=\"color: " + TEXTO + "; text-align: center; background-color: " + String((estadoEntrada(i)==ESTADO_DESACTIVO?DESACTIVO:ACTIVO)) + "; width: 100px\">" + String(nombreEstadoEntrada(i,estadoEntrada(i))) + "</TD>";
      cad += "</TR>";
      }
    }
  cad += "</TABLE>\n";
  cad += "<BR>";
  
  //Salidas
  cad += "\n<TABLE style=\"border: 2px solid blue\">\n";
  cad += "<CAPTION>Salidas</CAPTION>\n";  
  for(int8_t i=0;i<MAX_SALIDAS;i++)
    {
    if(releConfigurado(i)==CONFIGURADO)
      {
      String orden="";
      if (estadoRele(i)!=ESTADO_DESACTIVO) orden="desactiva"; 
      else orden="activa";

      cad += "<TR>\n";
      cad += "<TD STYLE=\"color: " + TEXTO + "; text-align: center; background-color: " + FONDO + "; width: 100px\">" + nombreRele(i) + "</TD>\n";
      cad += "<TD STYLE=\"color: " + TEXTO + "; text-align: center; background-color: " + String((estadoRele(i)==ESTADO_DESACTIVO?DESACTIVO:ACTIVO)) + "; width: 100px\">" + nombreEstadoSalidaActual(i) + "</TD>\n";

      //acciones en funcion del modo
      switch (modoSalida(i))          
        {
        case MODO_MANUAL:
          //Enlace para activar o desactivar y para generar un pulso
          cad += "<TD>Manual</TD>\n";
          cad += "<td>\n";
          cad += "<form action=\"" + orden + "Rele\">\n";
          cad += "<input  type=\"hidden\" id=\"id\" name=\"id\" value=\"" + String(i) + "\" >\n";
          cad += "<input STYLE=\"color: " + TEXTO + "; text-align: center; background-color: " + String((estadoRele(i)==ESTADO_DESACTIVO?ACTIVO:DESACTIVO)) + "; width: 80px\" type=\"submit\" value=\"" + orden + "r\">\n";
          cad += "</form>\n";
          cad += "<form action=\"pulsoRele\">\n";
          cad += "<input  type=\"hidden\" id=\"id\" name=\"id\" value=\"" + String(i) + "\" >\n";
          cad += "<input STYLE=\"color: " + TEXTO + "; text-align: center; background-color: " + ACTIVO + "; width: 80px\" type=\"submit\" value=\"pulso\">\n";
          cad += "</form>\n";
          if(modoInicalSalida(i)!=MODO_MANUAL)
            {
            cad += "<form action=\"recuperaManualSalida\">\n";
            cad += "<input  type=\"hidden\" id=\"id\" name=\"id\" value=\"" + String(i) + "\" >\n";
            cad += "<input STYLE=\"color: " + TEXTO + "; text-align: center; background-color: " + ACTIVO + "; width: 160px\" type=\"submit\" value=\"recupera manual\">\n";
            cad += "</form>\n";
            }          
          cad += "</td>\n";    
          break;
        case MODO_SECUENCIADOR:
          cad += "<TD colspan=2> | Secuenciador " + String(controladorSalida(i)) + "</TD>\n";
          break;
        case MODO_SEGUIMIENTO:
          cad += "<TD>Siguiendo a entrada " + String(controladorSalida(i)) + "</TD>\n";
          cad += "<td>\n";
          cad += "<form action=\"fuerzaManualSalida\">\n";
          cad += "<input  type=\"hidden\" id=\"id\" name=\"id\" value=\"" + String(i) + "\" >\n";
          cad += "<input STYLE=\"color: " + TEXTO + "; text-align: center; background-color: " + ACTIVO + "; width: 160px\" type=\"submit\" value=\"fuerza manual\">\n";
          cad += "</form>\n";
          cad += "</td>\n";    
          break;
        case MODO_MAQUINA:
          cad += "<TD>Controlado por la logica de la maquina de estados. Estado actual: " + getNombreEstadoActual() + "</TD>\n";
          break;            
        default://Salida no configurada
          cad += "<TD>No configurada</TD>\n";
        }
      cad += "</TR>\n";        
      }
    }
  cad += "</TABLE>\n";
  
  //Secuenciadores
  cad += "<BR><BR>\n";
  cad += "\n<TABLE style=\"border: 2px solid blue\">\n";
  cad += "<CAPTION>Secuenciadores</CAPTION>\n";  
  for(int8_t i=0;i<MAX_PLANES;i++)
    {
    if(planConfigurado(i)==CONFIGURADO)
      {      
      cad += "<TR>\n";
      if (estadoSecuenciador()) cad += "<TD><a href=\"desactivaSecuenciador\" \" target=\"_self\">Desactiva secuenciador</TD>";            
      else cad += "<TD><a href=\"activaSecuenciador\" \" target=\"_self\">Activa secuenciador</TD>";            
      cad += "</TR>\n";  
      }
    }
  cad += "</TABLE>\n";
  
  //Enlaces
  cad += "<BR><BR>\n";
  cad += enlaces;

  cad += "<BR><BR>" + nombre_dispositivo + " . Version " + String(VERSION) + ".";

  cad += pieHTML;
  request->send(200, "text/html", cad);
  }

/*************************************************/
/*                                               */
/*  Servicio de consulta de las transiciones de  */
/*  la maquina de estados                        */
/*                                               */
/*************************************************/  
void handleMaquinaEstados(AsyncWebServerRequest *request)
  {
  String cad="";
  String orden="";

  //genero la respuesta por defecto
  cad += cabeceraHTML;

  //Estado actual
  cad += "Estado actual: " + getNombreEstadoActual();
  cad += "<BR><BR>";
  
  //Entradas
  cad += "<TABLE style=\"border: 2px solid black\">\n";
  cad += "<CAPTION>Entradas actual</CAPTION>\n";  
  cad += "<TR>"; 
  cad += "<TD>id</TD>";  
  cad += "<TD>Nombre</TD>";
  cad += "<TD>Ultimo estado leido</TD>";
  cad += "</TR>";    
  for(uint8_t i=0;i<getNumEntradasME();i++)
    {
    cad += "<TR>";  
    cad += "<TD>" + String(i) + ":" + String(mapeoEntradas[i]) + "</TD>";  
    cad += "<TD>" + nombreEntrada(mapeoEntradas[i])+ "</TD>";
    cad += "<TD>" + String(entradasActual[i]) + ":" + String(estadoEntrada(mapeoEntradas[i])) + "</TD>";
    cad += "</TR>";
    }
  cad += "</TABLE>";
  cad += "<BR><BR>";
  
  //Salidas
  cad += "<TABLE style=\"border: 2px solid black\">\n";
  cad += "<CAPTION>Salidas actual</CAPTION>\n";  
  cad += "<TR>"; 
  cad += "<TD>id</TD>";  
  cad += "<TD>Nombre</TD>";
  cad += "<TD>Estado actual</TD>";
  cad += "</TR>";    
  for(uint8_t i=0;i<getNumSalidasME();i++)
    {
    cad += "<TR>";  
    cad += "<TD>" + String(i) + ":" + String(mapeoSalidas[i]) + "</TD>";  
    cad += "<TD>" + String(nombreSalida(mapeoSalidas[i])) + "</TD>";
    cad += "<TD>" + String(estadoRele(mapeoSalidas[i])) + "</TD>";
    cad += "</TR>";
    }
  cad += "</TABLE>";
  cad += "<BR><BR>";
  
  //Estados  
  cad += "<TABLE style=\"border: 2px solid black\">\n";
  cad += "<CAPTION>ESTADOS</CAPTION>\n";  
  cad += "<TR>"; 
  cad += "<TD>id</TD>";  
  cad += "<TD>Nombre</TD>";
  for(uint8_t i=0;i<getNumSalidasME();i++) cad += "<TD>Salida[" + String(i) + "] salida asociada(" + mapeoSalidas[i] + ")</TD>";
  cad += "</TR>"; 
    
  for(uint8_t i=0;i<getNumEstados();i++)
    {
    cad += "<TR>";  
    cad += "<TD>" + String(i) + "</TD>";  
    cad += "<TD>" + estados[i].nombre + "</TD>";
    for(uint8_t j=0;j<getNumSalidasME();j++) cad += "<TD>" + String(estados[i].valorSalidas[j]) + "</TD>";
    cad += "</TR>";
    }
  cad += "</TABLE>";
  cad += "<BR><BR>";
  
  //Transiciones
  cad += "<TABLE style=\"border: 2px solid black\">\n";
  cad += "<CAPTION>TRANSICIONES</CAPTION>\n";  

  cad += "<TR>";
  cad += "<TD>Inicial</TD>";
  for(uint8_t i=0;i<getNumEntradasME();i++) cad += "<TD>Entrada " + String(i) + "</TD>";
  cad += "<TD>Final</TD>";  
  cad += "</TR>";
  
  for(uint8_t i=0;i<getNumTransiciones();i++)
    {
    cad += "<TR>";
    cad += "<TD>" + String(transiciones[i].estadoInicial) + "</TD>";
    for(uint8_t j=0;j<getNumEntradasME();j++) cad += "<TD>" + String(transiciones[i].valorEntradas[j]) + "</TD>";
    cad += "<TD>" + String(transiciones[i].estadoFinal) + "</TD>";    
    cad += "</TR>";
    }
  cad += "</TABLE>";
  cad += "<BR><BR>";
  
  cad += pieHTML;
  request->send(200, "text/html", cad);
  }

/*************************************************/
/*                                               */
/*  Servicio de consulta de estado de            */
/*  las Salidas y las entradas                   */
/*  devuelve un formato json                     */
/*                                               */
/*************************************************/  
void handleEstado(AsyncWebServerRequest *request)
  {
  String cad=generaJsonEstado();
  
  request->send(200, "text/json", cad); 
  }  

/***************************************************/
/*                                                 */
/*  Servicio de consulta de estado de las salidas  */ 
/*  devuelve un formato json                       */
/*                                                 */
/***************************************************/  
void handleEstadoSalidas(AsyncWebServerRequest *request)
  {
  String cad="";

  //genero la respuesta por defecto
  cad += cabeceraHTML;

  //Estados
  cad += "<TABLE style=\"border: 2px solid black\">\n";
  cad += "<CAPTION>Salidas</CAPTION>\n";  

  cad += "<TR>"; 
  cad += "<TD>id</TD>";  
  cad += "<TD>Nombre</TD>";
  cad += "<TD>Configurada</TD>";
  cad += "<TD>Pin</TD>";  
  cad += "<TD>Controlador</TD>";  
  cad += "<TD>Modo</TD>";  
  cad += "<TD>Ancho del pulso</TD>";  
  cad += "<TD>Inicio del pulso</TD>";  
  cad += "<TD>Fin del pulso</TD>";  
  cad += "<TD>Estado</TD>";
  cad += "<TD>Nombre del estado</TD>";
  cad += "<TD>mensaje</TD>";
  cad += "</TR>"; 

  for(uint8_t salida=0;salida<MAX_SALIDAS;salida++)
    {
    cad += "<TR>"; 
    cad += "<TD>" + String(salida) + "</TD>";
    cad += "<TD>" + String(nombreSalida(salida)) + "</TD>";  
    cad += "<TD>" + String(releConfigurado(salida)) + "</TD>";
    cad += "<TD>" + String(pinSalida(salida)) + "</TD>";
    cad += "<TD>" + String(controladorSalida(salida)) + "</TD>";
    cad += "<TD>" + String(modoSalida(salida)) + "</TD>";
    cad += "<TD>" + String(anchoPulsoSalida(salida)) + "</TD>";
    cad += "<TD>" + String(inicioSalida(salida)) + "</TD>";
    cad += "<TD>" + String(finPulsoSalida(salida)) + "</TD>";
    cad += "<TD>" + String(estadoRele(salida)) + "</TD>";
    cad += "<TD>" + String(nombreEstadoSalida(salida,estadoRele(salida))) + "</TD>";
    cad += "<TD>" + String(mensajeEstadoSalida(salida,estadoRele(salida))) + "</TD>";
    cad += "</TR>";     
    }
  cad += "</TABLE>";

  cad += pieHTML;
  request->send(200, "text/HTML", cad);
  }
  
/*****************************************************/
/*                                                   */
/*  Servicio de consulta de estado de las entradas   */
/*  devuelve un formato json                         */
/*                                                   */
/*****************************************************/  
void handleEstadoEntradas(AsyncWebServerRequest *request)
  {
  String cad="";

  //genero la respuesta por defecto
  cad += cabeceraHTML;

  //Estados
  cad += "<TABLE style=\"border: 2px solid black\">\n";
  cad += "<CAPTION>Entradas</CAPTION>\n";  

  cad += "<TR>"; 
  cad += "<TD>id</TD>";  
  cad += "<TD>Nombre</TD>";
  cad += "<TD>Configurada</TD>";
  cad += "<TD>Tipo</TD>";
  cad += "<TD>Pin</TD>";
  cad += "<TD>Estado activo</TD>";
  cad += "<TD>Estado fisico</TD>";
  cad += "<TD>Estado</TD>";
  cad += "<TD>Nombre del estado</TD>";
  cad += "<TD>mensaje</TD>";
  cad += "</TR>"; 

  for(uint8_t entrada=0;entrada<MAX_ENTRADAS;entrada++)
    {
    cad += "<TR>"; 
    cad += "<TD>" + String(entrada) + "</TD>";
    cad += "<TD>" + String(nombreEntrada(entrada)) + "</TD>";  
    cad += "<TD>" + String(entradaConfigurada(entrada)) + "</TD>";
    cad += "<TD>" + String(tipoEntrada(entrada)) + "</TD>";
    cad += "<TD>" + String(pinEntrada(entrada)) + "</TD>";
    cad += "<TD>" + String(estadoActivoEntrada(entrada)) + "</TD>";
    cad += "<TD>" + String(estadoFisicoEntrada(entrada)) + "</TD>";    
    cad += "<TD>" + String(estadoEntrada(entrada)) + "</TD>";
    cad += "<TD>" + String(nombreEstadoEntrada(entrada,estadoEntrada(entrada))) + "</TD>";
    cad += "<TD>" + String(mensajeEstadoEntrada(entrada,estadoEntrada(entrada))) + "</TD>";
    cad += "</TR>";     
    }
  cad += "</TABLE>";

  cad += pieHTML;
  request->send(200, "text/HTML", cad); 
  }
  
/*********************************************/
/*                                           */
/*  Servicio de actuacion de rele            */
/*                                           */
/*********************************************/  
void handleActivaRele(AsyncWebServerRequest *request)
  {
  int8_t id=0;

  if(request->hasArg("id") ) 
    {
    int8_t id=request->arg("id").toInt();

    //activaRele(id);
    conmutaRele(id, ESTADO_ACTIVO, debugGlobal);

    handleRoot(request);
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
  int8_t id=0;

  if(request->hasArg("id") ) 
    {
    int8_t id=request->arg("id").toInt();

    //desactivaRele(id);
    conmutaRele(id, ESTADO_DESACTIVO, debugGlobal);
    
    handleRoot(request);
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
  int8_t id=0;

  if(request->hasArg("id") ) 
    {
    int8_t id=request->arg("id").toInt();

    //desactivaRele(id);
    pulsoRele(id);
    
    handleRoot(request);
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
  int8_t id=0;

  if(request->hasArg("id")) 
    {
    int8_t id=request->arg("id").toInt();

    forzarModoManualSalida(id);
    
    handleRoot(request);
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
  int8_t id=0;

  if(request->hasArg("id")) 
    {
    int8_t id=request->arg("id").toInt();

    recuperarModoSalida(id);
    
    handleRoot(request);
    }
  else request->send(404, "text/plain", ""); 
  }

/*********************************************/
/*                                           */
/*  Servicio de representacion de los        */
/*  planes del secuenciador                  */
/*                                           */
/*********************************************/  
void handlePlanes(AsyncWebServerRequest *request)
  {
  int8_t numPlanes=getNumPlanes();  
  String cad=cabeceraHTML;
  
  cad += "<h1>hay " + String(numPlanes) + " plan(es) activo(s).</h1><BR>";
  
  for(int8_t i=0;i<numPlanes;i++)
    {
    cad += pintaPlanHTML(i);
    cad += "<BR><BR>";
    }
  
  cad += pieHTML;
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

/*********************************************/
/*                                           */
/*  Servicio de test                         */
/*                                           */
/*********************************************/  
void handleTest(AsyncWebServerRequest *request)
  {
  String cad="";

  cad += cabeceraHTML;
  cad += "Test OK<br>";
  cad += pieHTML;
  request->send(200, "text/html", cad); 
  }
  
/*********************************************/
/*                                           */
/*  Reinicia el dispositivo mediante         */
/*  peticion HTTP                            */ 
/*                                           */
/*********************************************/  
void handleRestart(AsyncWebServerRequest *request)
  {
  String cad="";

  cad += cabeceraHTML;
  cad += "Reiniciando...<br>";
  cad += pieHTML;
  request->send(200, "text/html", cad);     
  delay(100);
  ESP.restart();
  }

/*********************************************/
/*                                           */
/*  Lee info del chipset mediante            */
/*  peticion HTTP                            */ 
/*                                           */
/*********************************************/  
void handleInfo(AsyncWebServerRequest *request)
  {
  String cad=cabeceraHTML;
  cad += "<h1>" + nombre_dispositivo + "</h1>";

  cad+= "<BR>-----------------Uptime---------------------<BR>";
  char tempcad[20]="";
#ifdef ESP32
  sprintf(tempcad,"%lu", (esp_timer_get_time()/(unsigned long)1000000)); //la funcion esp_timer_get_time() devuelve el contador de microsegundos desde el arranque. rota cada 292.000 años  
#else
  sprintf(tempcad,"%lu", (millis()/(unsigned long)1000000)); //la funcion esp_timer_get_time() devuelve el contador de microsegundos desde el arranque. rota cada 292.000 años  
#endif  
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
    cad += "Rele " + String(i) + " nombre: " + nombreRele(i) + "| estado: " + estadoRele(i);    
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
/*
  cad += "<BR>-----------------MQTT info-----------------<BR>";
  cad += "IP broker: " + IPBroker.toString();
  cad += "<BR>";
  cad += "Puerto broker: " +   puertoBroker=0;
  cad += "<BR>";  
  cad += "Usuario: " + usuarioMQTT="";
  cad += "<BR>";  
  cad += "Password: " + passwordMQTT="";
  cad += "<BR>";  
  cad += "Topic root: " + topicRoot="";
  cad += "<BR>";  
  cad += "-----------------------------------------------<BR>";  
*/    
  cad += "<BR>-----------------Hardware info-----------------<BR>";
  cad += "FreeHeap: " + String(ESP.getFreeHeap());
  cad += "<BR>";
#ifdef ESP32
  cad += "ChipId: " + String(ESP.getChipRevision());
#else
  cad += "ChipId: " + String(ESP.getChipId());
#endif  
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
#ifdef ESP8266  
  cad += "Vcc: " + String(ESP.getVcc());
  cad += "<BR>";  
  cad += "CoreVersion: " + ESP.getCoreVersion();
  cad += "<BR>";  
  cad += "FullVersion: " + ESP.getFullVersion();
  cad += "<BR>";  
  cad += "BootVersion: " + String(ESP.getBootVersion());
  cad += "<BR>";  
  cad += "BootMode: " + String(ESP.getBootMode());
  cad += "<BR>";  
  cad += "FlashChipId: " + String(ESP.getFlashChipId());
  cad += "<BR>";  
     //gets the actual chip size based on the flash id
  cad += "FlashChipRealSize: " + String(ESP.getFlashChipRealSize());
  cad += "<BR>";  
     //FlashMode_t ESP.getFlashChipMode());
  cad += "FlashChipSizeByChipId: " + String(ESP.getFlashChipSizeByChipId());  
  cad += "<BR>";  
#endif  
  cad += "-----------------------------------------------<BR>";  

#ifdef ESP8266
  cad += "<BR>-----------------info fileSystem-----------------<BR>";   
  FSInfo fs_info;
  if(SPIFFS.info(fs_info)) 
    {
    /*        
     struct FSInfo {
        size_t totalBytes;
        size_t usedBytes;
        size_t blockSize;
        size_t pageSize;
        size_t maxOpenFiles;
        size_t maxPathLength;
    };
     */
    cad += "totalBytes: ";
    cad += fs_info.totalBytes;
    cad += "<BR>usedBytes: ";
    cad += fs_info.usedBytes;
    cad += "<BR>blockSize: ";
    cad += fs_info.blockSize;
    cad += "<BR>pageSize: ";
    cad += fs_info.pageSize;    
    cad += "<BR>maxOpenFiles: ";
    cad += fs_info.maxOpenFiles;
    cad += "<BR>maxPathLength: ";
    cad += fs_info.maxPathLength;
    }
  else cad += "Error al leer info";
  cad += "<BR>-----------------------------------------------<BR>"; 
#endif
  
  cad += pieHTML;
  request->send(200, "text/html", cad);     
  }

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
  boolean salvado=false;

  cad += cabeceraHTML;

  if(request->hasArg("nombre") && request->hasArg("contenido")) //si existen esos argumentos
    {
    nombreFichero=request->arg("nombre");
    contenidoFichero=request->arg("contenido");

    if(salvaFichero( nombreFichero, nombreFichero+".bak", contenidoFichero)) 
	  {
	  //cad += "Fichero salvado con exito<br>";
	  handleListaFicheros(request);
	  return;
	  }  
    else cad += "No se pudo salvar el fichero<br>"; 
    }
  else cad += "Falta el argumento <nombre de fichero>"; 

  cad += pieHTML;
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

  cad += cabeceraHTML;
  
  if(request->hasArg("nombre") ) //si existen esos argumentos
    {
    nombreFichero=request->arg("nombre");

    if(borraFichero(nombreFichero)) 
      {
      //cad += "El fichero " + nombreFichero + " ha sido borrado.\n";
      handleListaFicheros(request);
      return;
      }
    else cad += "No sepudo borrar el fichero " + nombreFichero + ".\n"; 
    }
  else cad += "Falta el argumento <nombre de fichero>"; 

  cad += pieHTML;
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
  String cad=cabeceraHTML;
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

  cad += pieHTML;
  request->send(200, "text/html", cad); 
  }

#ifdef ESP8266
/*********************************************/
/*                                           */
/*  Lee info del FS                          */
/*  peticion HTTP                            */ 
/*                                           */
/*********************************************/  
void handleInfoFS(void)
  {
  String cad="";

  cad += cabeceraHTML;
  
  //inicializo el sistema de ficheros
  if (SPIFFS.begin()) 
    {
    Serial.println("---------------------------------------------------------------\nmounted file system");  
    FSInfo fs_info;
    if(SPIFFS.info(fs_info)) 
      {
      /*        
       struct FSInfo {
          size_t totalBytes;
          size_t usedBytes;
          size_t blockSize;
          size_t pageSize;
          size_t maxOpenFiles;
          size_t maxPathLength;
      };
       */
      cad += "totalBytes: ";
      cad += fs_info.totalBytes;
      cad += "<BR>usedBytes: ";
      cad += fs_info.usedBytes;
      cad += "<BR>blockSize: ";
      cad += fs_info.blockSize;
      cad += "<BR>pageSize: ";
      cad += fs_info.pageSize;    
      cad += "<BR>maxOpenFiles: ";
      cad += fs_info.maxOpenFiles;
      cad += "<BR>maxPathLength: ";
      cad += fs_info.maxPathLength;
      }
    else cad += "Error al leer info";

    Serial.println("unmounted file system\n---------------------------------------------------------------");
    }//La de abrir el sistema de ficheros

  cad += pieHTML;
  server.send(200, "text/html", cad); 
  }
#endif

/*********************************************/
/*                                           */
/*  Pagina no encontrada                     */
/*                                           */
/*********************************************/
void handleNotFound(AsyncWebServerRequest *request)
  {
  //Serial.printf("URL no encontrada: %s\n",request->url().c_str());  
  if(!handleFileRead(request))
    {
  Serial.printf("No se encontro el fichero en SPIFFS\n");  
  String message = "";

  message = "<h1>" + nombre_dispositivo + "<br></h1>";
  message += "File Not Found\n\n";
  message += "URI: ";
  message += request->url();
  message += "\nMethod: ";
  message += request->methodToString();// == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += request->args();
  message += "\n";

  for (uint8_t i=0; i<request->args(); i++)
    {
    message += " " + request->argName(i) + ": " + request->arg(i) + "\n";
    }
    
  request->send(404, "text/html", message);
    }
  }

/*********************************************/
/*                                           */
/*  Habilita la edicion y borrado de los     */
/*  ficheros en el sistema a traves de una   */
/*  peticion HTTP                            */ 
/*                                           */
/*********************************************/ 
void handleManageFichero(AsyncWebServerRequest *request)
  {
  String nombreFichero="";
  String contenido="";
  String cad="";

  cad += cabeceraHTML;
  cad += "<h1>" + nombre_dispositivo + "</h1>";
  
  if(request->hasArg("nombre") ) //si existen esos argumentos
    {
    nombreFichero=request->arg("nombre");
    cad += "<h2>Fichero: " + nombreFichero + "</h2><BR>";  

    if(leeFichero(nombreFichero, contenido))
      {
      cad += "El fichero tiene un tama&ntilde;o de ";
      cad += contenido.length();
      cad += " bytes.<BR>";
      cad += "El contenido del fichero es:<BR>";
      cad += "<textarea readonly=true cols=100 rows=20 name=\"contenido\">";
      cad += contenido;
      cad += "</textarea>";
      cad += "<BR>";

      cad += "<table><tr><td>";
      cad += "<form action=\"borraFichero\" target=\"_self\">";
      cad += "  <p>";
      cad += "    <input type=\"hidden\" name=\"nombre\" value=\"" + nombreFichero + "\">";
      cad += "    <input type=\"submit\" value=\"borrar\">";
      cad += "  </p>";
      cad += "</form>";
      cad += "</td></tr></table>";
      
      cad += "<table>Modificar fichero<tr><td>";      
      cad += "<form action=\"creaFichero\" target=\"_self\">";
      cad += "  <p>";
      cad += "    <input type=\"hidden\" name=\"nombre\" value=\"" + nombreFichero + "\">";
      cad += "    contenido del fichero: <br><textarea cols=100 rows=20 name=\"contenido\">" + contenido + "</textarea>";
      cad += "    <BR>";
      cad += "    <input type=\"submit\" value=\"salvar\">";
      cad += "  </p>";
      cad += "</td></tr></table>";
      }
    else cad += "Error al abrir el fichero " + nombreFichero + "<BR>";
    }
  else cad += "Falta el argumento <nombre de fichero>"; 

  cad += pieHTML;
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
  String nombreFichero="";
  String contenidoFichero="";
  boolean salvado=false;
  String cad="";

  cad += cabeceraHTML;
  
  //Variables para manejar la lista de ficheros
  String contenido="";
  String fichero="";  
  int16_t to=0;
  
  if(listaFicheros(contenido)) 
    {
    Serial.printf("contenido inicial= %s\n",contenido.c_str());      
    //busco el primer separador
    to=contenido.indexOf(SEPARADOR); 

    cad +="<style> table{border-collapse: collapse;} th, td{border: 1px solid black; padding: 10px; text-align: left;}</style>";
    cad += "<TABLE>";
    while(to!=-1)
      {
      fichero=contenido.substring(0, to);//cojo el principio como el fichero
      contenido=contenido.substring(to+1); //la cadena ahora es desde el separador al final del fichero anterior
      to=contenido.indexOf(SEPARADOR); //busco el siguiente separador

      cad += "<TR><TD>" + fichero + "</TD>";           
      cad += "<TD>";
      cad += "<form action=\"manageFichero\" target=\"_self\">";
      cad += "    <input type=\"hidden\" name=\"nombre\" value=\"" + fichero + "\">";
      cad += "    <input type=\"submit\" value=\"editar\">";
      cad += "</form>";
      cad += "</TD><TD>";
      cad += "<form action=\"borraFichero\" target=\"_self\">";
      cad += "    <input type=\"hidden\" name=\"nombre\" value=\"" + fichero + "\">";
      cad += "    <input type=\"submit\" value=\"borrar\">";
      cad += "</form>";
      cad += "</TD></TR>";
      }
    cad += "</TABLE>\n";
    cad += "<BR>";
    
    //Para crear un fichero nuevo
    cad += "<h2>Crear un fichero nuevo:</h2>";
    cad += "<table><tr><td>";      
    cad += "<form action=\"creaFichero\" target=\"_self\">";
    cad += "  <p>";
    cad += "    Nombre:<input type=\"text\" name=\"nombre\" value=\"\">";
    cad += "    <BR>";
    cad += "    Contenido:<br><textarea cols=75 rows=20 name=\"contenido\"></textarea>";
    cad += "    <BR>";
    cad += "    <input type=\"submit\" value=\"salvar\">";
    cad += "  </p>";
    cad += "</td></tr></table>";      
    }
  else cad += "<TR><TD>No se pudo recuperar la lista de ficheros</TD></TR>"; 

  cad += pieHTML;
  request->send(200, "text/html", cad); 
  }

/**********************************************************************/
String getContentType(String filename) { // determine the filetype of a given filename, based on the extension
  if (filename.endsWith(".html")) return "text/html";
  else if (filename.endsWith(".css")) return "text/css";
  else if (filename.endsWith(".js")) return "application/javascript";
  else if (filename.endsWith(".ico")) return "image/x-icon";
  else if (filename.endsWith(".gz")) return "application/x-gzip";
  return "text/plain";
}

bool handleFileRead(AsyncWebServerRequest *request) 
  {
  String path=request->url();
  // send the right file to the client (if it exists)
  Serial.println("handleFileRead: " + path);
  
  if (!path.startsWith("/")) path += "/";
  path = "/www" + path; //busco los ficheros en el SPIFFS en la carpeta www
  //if (!path.endsWith("/")) path += "/";
  
  String contentType = getContentType(path);             // Get the MIME type
  String pathWithGz = path + ".gz";
  if (SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)) 
    { // If the file exists, either as a compressed archive, or normal
    if (SPIFFS.exists(pathWithGz)) path += ".gz"; // Si hay una version comprimida del fichero disponible, usa la version comprimida
                                             
    request->send(SPIFFS, path, contentType);
    
    Serial.printf("\tfichero enviado: %s | contentType: %s\n",path.c_str(),contentType.c_str());
    return true;
    }
  Serial.println(String("\tfichero no encontrado en SPIFFS: ") + path);   // If the file doesn't exist, return false
  return false;
  }
/****************************Google Home Notifier ******************************/
#ifdef ESP32
void handleSpeechPath(AsyncWebServerRequest *request) 
  {
  String phrase = request->arg("phrase");
  if (phrase == "") 
    {
    request->send(401, "text / plain", "query 'phrase' is not found");
    return;
    }

  if(enviaNotificacion((char*)phrase.c_str())) request->send(200, "text / plain", "OK");
  else request->send(404, "text / plain", "KO");  
  }
#endif
