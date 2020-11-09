/************************************************************************************************
Servicio                       URL                     Formato entrada Formato salida                                            Comentario                                            Ejemplo peticion              Ejemplo respuesta
Estado de los reles            http://IP/estado        N/A             <id_0>#<nombre_0>#<estado_0>|<id_1>#<nombre_1>#<estado_1> Devuelve el id de cada rele y su estado               http://IP/estado              1#1|2#0
Activa rele                    http://IP/activaRele    id=<id>         <id>#<estado>                                             Activa el rele indicado y devuelve el estado leido    http://IP/activaRele?id=1     1|1
Desactivarele                  http://IP/desactivaRele id=<id>         <id>#<estado>                                             Desactiva el rele indicado y devuelve el estado leido http://IP/desactivaRele?id=0  0|0
Test                           http://IP/test          N/A             HTML                                                      Verifica el estado del Actuadot   
Reinicia el actuador           http://IP/restart
Informacion del Hw del sistema http://IP/info
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
#include <EntradasSalidas.h>
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
#include <SPIFFS.h>
/***************************** Includes *****************************/

//Variables globales
AsyncWebServer serverX(PUERTO_WEBSERVER);

//Cadenas HTML precargadas
String cabeceraHTML="";

//version de la web propia del cacharro
String pagina_a = "<!DOCTYPE html>\n<html lang=\"es\">\n <head>\n <meta charset=\"UTF-8\">\n <TITLE>Domoticae</TITLE>\n <link rel=\"stylesheet\" type=\"text/css\" href=\"css.css\">\n </HEAD>\n <BODY>\n <table style=\"width:100%;\" cellpadding=\"10\" cellspacing=\"0\">\n  <tr style=\"height:20%; background-color:black\">\n  <th align=\"left\">\n   <span style=\"font-family:verdana;font-size:30px;color:white\">DOMOTI</span><span style=\"font-family:verdana;font-size:30px;color:red\">C</span><span style=\"font-family:verdana;font-size:30px;color:white\">AE - ";
//en medio va el nombre_dispositivo
String pagina_b = "</span>   \n  </th>\n  </tr>\n  <tr style=\"height:10%;\">\n  	<td>";
String enlaces = "<table class=\"tabla\"><tr class=\"modo1\"><td><a href=\"..\" target=\"_self\" style=\"text-decoration:none; color: black;\">Home</a></td><td><a href=\"configEntradas\" target=\"_self\" style=\"text-decoration:none; color: black;\">Entradas</a></td><td><a href=\"configSalidas\" target=\"_self\" style=\"text-decoration:none; color: black;\">Salidas</a></td><td><a href=\"planes\" target=\"_self\" style=\"text-decoration:none; color: black;\">Secuenciador</a></td><td><a href=\"maquinaEstados\" target=\"_self\" style=\"text-decoration:none; color: black;\">Maquina de estados</a></td><td><a href=\"listaFicheros\" target=\"_self\" style=\"text-decoration:none; color: black;\">Ficheros</a></td><td><a href=\"info\" target=\"_self\" style=\"text-decoration:none; color: black;\">Info</a></td><td><a href=\"particiones\" target=\"_self\" style=\"text-decoration:none; color: black;\">Particiones</a></td><td><a href=\"restart\" target=\"_self\" style=\"text-decoration:none; color: black;\">Restart</a></td></tr></table>";
String pagina_c = "</td></tr><TR style=\"height:60%\"><TD>";
//En medio va el cuerpo de la pagina
String pieHTML = "</TD>\n</TR>\n<TR>\n<TD style=\"color:white; background-color:black\"><a href=\"https://domoticae.lopeztola.com\" target=\"_self\" style=\"text-decoration:none; color:white;\">domoticae-2020</a></TD>\n</TR>\n</table>\n</BODY>\n</HTML>";

//version para integrar en otras paginas
String cabeceraHTMLlight = "<!DOCTYPE html>\n<html lang=\"es\">\n<head>\n<meta charset=\"UTF-8\" />\n<HTML><HEAD><TITLE>Domoticae</TITLE><link rel=\"stylesheet\" type=\"text/css\" href=\"css.css\"></HEAD><BODY>\n"; 

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
void handleConfigEntradas(AsyncWebServerRequest *request);
void handleConfigSalidas(AsyncWebServerRequest *request);
void handleEstado(AsyncWebServerRequest *request);
void handleRoot(AsyncWebServerRequest *request);
void handleListaFicheros2(AsyncWebServerRequest *request);
void handleVersion(AsyncWebServerRequest *request); 
void handleDatos(AsyncWebServerRequest *request); 
void handleParticiones(AsyncWebServerRequest *request);
void handleSetNextBoot(AsyncWebServerRequest *request);

/*********************************** Inicializacion y configuracion *****************************************************************/
void inicializaWebServer(void)
  {
  //lo inicializo aqui, despues de leer el nombre del dispositivo en la configuracion del cacharro  
  cabeceraHTML = pagina_a + nombre_dispositivo + pagina_b + enlaces + pagina_c;
    
  /*******Configuracion del Servicio Web***********/  
  //Inicializo los serivcios, decalra las URIs a las que va a responder
  serverX.on("/", HTTP_ANY, handleRoot); //Responde con la iodentificacion del modulo
  serverX.on("/estado", HTTP_ANY, handleEstado); //Servicio de estdo de reles
  serverX.on("/configSalidas", HTTP_ANY, handleConfigSalidas); //Servicio de estdo de reles
  serverX.on("/configEntradas", HTTP_ANY, handleConfigEntradas); //Servicio de estdo de reles    
  serverX.on("/activaRele", HTTP_ANY, handleActivaRele); //Servicio de activacion de rele
  serverX.on("/desactivaRele", HTTP_ANY, handleDesactivaRele);  //Servicio de desactivacion de rele
  serverX.on("/fuerzaManualSalida", HTTP_ANY, handleFuerzaManual);  //Servicio para formar ua salida a modo manual
  serverX.on("/recuperaManualSalida", HTTP_ANY, handleRecuperaManual);  //Servicio para formar ua salida a modo manual  

  serverX.on("/pulsoSalida", HTTP_ANY, handlePulsoRele);  //Servicio de pulso de rele
  serverX.on("/planes", HTTP_ANY, handlePlanes);  //Servicio de representacion del plan del secuenciador
  serverX.on("/activaSecuenciador", HTTP_ANY, handleActivaSecuenciador);  //Servicio para activar el secuenciador
  serverX.on("/desactivaSecuenciador", HTTP_ANY, handleDesactivaSecuenciador);  //Servicio para desactivar el secuenciador
  serverX.on("/maquinaEstados", HTTP_ANY, handleMaquinaEstados);  //Servicio de representacion de las transiciones d ela maquina de estados
      
  serverX.on("/test", HTTP_ANY, handleTest);  //URI de test
  serverX.on("/restart", HTTP_ANY, handleRestart);  //URI de test
  serverX.on("/info", HTTP_ANY, handleInfo);  //URI de test

  serverX.on("/particiones", HTTP_ANY, handleParticiones);  
  serverX.on("/setNextBoot", HTTP_ANY, handleSetNextBoot);

  serverX.on("/listaFicheros", HTTP_ANY, handleListaFicheros);  //URI de leer fichero
  serverX.on("/creaFichero", HTTP_ANY, handleCreaFichero);  //URI de crear fichero
  serverX.on("/borraFichero", HTTP_ANY, handleBorraFichero);  //URI de borrar fichero
  serverX.on("/leeFichero", HTTP_ANY, handleLeeFichero);  //URI de leer fichero
  serverX.on("/manageFichero", HTTP_ANY, handleManageFichero);  //URI de leer fichero  

  serverX.on("/datos", HTTP_ANY, handleDatos);
  serverX.on("/version", HTTP_ANY, handleVersion);
  serverX.on("/listaFicheros2", HTTP_ANY, handleListaFicheros2);

  serverX.on("/speech", HTTP_ANY, handleSpeechPath);
  
  serverX.onNotFound(handleNotFound);//pagina no encontrada

  serverX.begin();

  Traza.mensaje("Servicio web iniciado en puerto %i\n", PUERTO_WEBSERVER);
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
  cad += "<TABLE border=\"0\" width=\"50%\" cellpadding=\"0\" cellspacing=\"0\" width=\"300\" class=\"tabla\">\n";
  cad += "<CAPTION>Entradas</CAPTION>\n";    
  for(int8_t i=0;i<MAX_ENTRADAS;i++)
    {    
    if(entradaConfigurada(i)==CONFIGURADO) 
      {
      cad += "<TR class=\"modo2\">";
      cad += "<TD STYLE=\"color: " + TEXTO + "; text-align: center; background-color: " + FONDO + "; width: 100px\">" + nombreEntrada(i) + "</TD>";
      cad += "<TD STYLE=\"color: " + TEXTO + "; text-align: center; background-color: " + String((estadoEntrada(i)==ESTADO_DESACTIVO?DESACTIVO:ACTIVO)) + "; width: 200px\">" + String(nombreEstadoEntrada(i,estadoEntrada(i))) + "</TD>";
      cad += "</TR>";
      }
    }
  cad += "</TABLE>\n";
  cad += "<BR>\n";
  
  //Salidas
  cad += "<TABLE border=\"0\" width=\"50%\" cellpadding=\"0\" cellspacing=\"0\" width=\"300\" class=\"tabla\">\n";
  cad += "<CAPTION>Salidas</CAPTION>\n";  
  for(int8_t i=0;i<MAX_SALIDAS;i++)
    {
    if(releConfigurado(i)==CONFIGURADO)
      {
      String orden="";
      if (estadoSalida(i)!=ESTADO_DESACTIVO) orden="desactiva"; 
      else orden="activa";

      cad += "<TR class=\"modo2\">\n";
      cad += "<TD STYLE=\"color: " + TEXTO + "; text-align: center; background-color: " + FONDO + "; width: 100px\">" + nombreSalida(i) + "</TD>\n";
      cad += "<TD STYLE=\"color: " + TEXTO + "; text-align: center; background-color: " + String((estadoSalida(i)==ESTADO_DESACTIVO?DESACTIVO:ACTIVO)) + "; width: 100px\">" + String(nombreEstadoSalida(i,estadoSalida(i))) + "</TD>";

      //acciones en funcion del modo
      switch (modoSalida(i))          
        {
        case MODO_MANUAL:
          //Enlace para activar o desactivar y para generar un pulso
          cad += "<TD width=\"100\">Manual</TD>\n";
          cad += "<td>\n";
          cad += "<form action=\"" + orden + "Rele\">\n";
          cad += "<input  type=\"hidden\" id=\"id\" name=\"id\" value=\"" + String(i) + "\" >\n";
          cad += "<input STYLE=\"color: " + TEXTO + "; text-align: center; background-color: " + String((estadoSalida(i)==ESTADO_DESACTIVO?ACTIVO:DESACTIVO)) + "; width: 80px\" type=\"submit\" value=\"" + orden + "r\">\n";
          cad += "</form>\n";
          cad += "<form action=\"pulsoSalida\">\n";
          cad += "<input  type=\"hidden\" id=\"id\" name=\"id\" value=\"" + String(i) + "\" >\n";
          cad += "<input STYLE=\"color: " + TEXTO + "; text-align: center; background-color: " + ACTIVO + "; width: 80px\" type=\"submit\" value=\"pulso\">\n";
          cad += "</form>\n";
          if(modoInicalSalida(i)!=MODO_MANUAL)
            {
            cad += "<form action=\"recuperaManualSalida\">\n";
            cad += "<input  type=\"hidden\" id=\"id\" name=\"id\" value=\"" + String(i) + "\" >\n";
            cad += "<input STYLE=\"color: " + TEXTO + "; text-align: center; background-color: " + ACTIVO + "; width: 160px\" type=\"submit\" value=\"recupera automatico\">\n";
            cad += "</form>\n";
            }          
          cad += "</td>\n";    
          break;
        case MODO_SECUENCIADOR:
          cad += "<TD colspan=2>Secuenciador " + String(controladorSalida(i)) + "</TD>\n";
          break;
        case MODO_SEGUIMIENTO:
          cad += "<TD>Siguiendo a la entrada " + nombreEntrada(controladorSalida(i)) + "</TD>\n";
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
  cad += "<BR><BR>\n";

  //Secuenciadores
  cad += "<TABLE border=\"0\" width=\"50%\" cellpadding=\"0\" cellspacing=\"0\" width=\"300\" class=\"tabla\">\n";
  cad += "<CAPTION>Secuenciadores</CAPTION>\n";  

  cad += "<TR>";  
  cad += "<TH>plan</TH>";  
  cad += "<TH>acción</TH>";  
  cad += "</TR>";  

  for(int8_t i=0;i<MAX_PLANES;i++)
    {
    if(planConfigurado(i)==CONFIGURADO)
      {      
      cad += "<TR class=\"modo2\">\n";
      cad += "<TD>Plan " + String(i) + "</TD>";
      cad += "<TD>";
      if (estadoSecuenciador()) cad += "<a href=\"desactivaSecuenciador\" \" target=\"_self\">Desactiva secuenciador";
      else cad += "<a href=\"activaSecuenciador\" \" target=\"_self\">Activa secuenciador";
      cad += "</TD>";
      cad += "</TR>\n";  
      }
    }
  cad += "</TABLE>\n";
  cad += "<BR><BR>\n";
  
  //Maquina de estados
  cad += "<TABLE border=\"0\" width=\"50%\" cellpadding=\"0\" cellspacing=\"0\" width=\"300\" class=\"tabla\">\n";
  cad += "<CAPTION>Máquina de estados</CAPTION>\n"; 
  cad += "<TR>\n";
  cad += "<TH width=\"100\">Estado: </TH>";
  cad += "<TD width=\"200\" class=\"modo2\">" + getNombreEstadoActual() + "</TD>";
  cad += "</TR>\n";
  cad += "</TABLE>\n";

  cad += "<BR>";
  
  //Informacion del dispositivo
  cad += "<p style=\"font-size: 12px;color:black;\">" + nombre_dispositivo + " - Version " + String(VERSION) + "</p>";

  cad += pieHTML;
  request->send(200, "text/html", cad);
  }

/*********************************************/
/*                                           */
/*  Pinta la lista de particiones en         */
/*  memoria                                  */ 
/*                                           */
/*********************************************/  
void handleParticiones(AsyncWebServerRequest *request)
  {
  String cad=cabeceraHTML;

  cad +=pintaParticionHTML();

  cad += pieHTML;
  request->send(200, "text/html", cad);
  }

/*********************************************/
/*                                           */
/*    Cambia la particion de arranque        */
/*                                           */
/*********************************************/  
void handleSetNextBoot(AsyncWebServerRequest *request)
  {
  String cad=cabeceraHTML;

  if(request->hasArg("p")) //si existen esos argumentos
    {
    if(setParticionProximoArranque(request->arg("p"))) cad += "EXITO";
    else cad += "FRACASO";
    }

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
  cad += "<TABLE border=\"0\" width=\"50%\" cellpadding=\"0\" cellspacing=\"0\" width=\"300\" class=\"tabla\">\n";
  cad += "<CAPTION>Estado actual</CAPTION>\n"; 
  cad += "<TR>\n";
  cad += "<TH width=\"100\">Estado: </TH>";
  cad += "<TD width=\"200\" class=\"modo2\">" + getNombreEstadoActual() + "</TD>";
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
  for(uint8_t i=0;i<getNumEntradasME();i++)
    {
    cad += "<TR class=\"modo2\">";  
    cad += "<TD>" + String(i) + ":" + String(getMapeoEntradas(i)) + "</TD>";  
    cad += "<TD>" + nombreEntrada(getMapeoEntradas(i))+ "</TD>";
    cad += "<TD>" + String(getEntradasActual(i)) + ":" + String(estadoEntrada(getMapeoEntradas(i))) + "</TD>";
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
  for(uint8_t i=0;i<getNumSalidasME();i++)
    {
    cad += "<TR class\"modo2\">";  
    cad += "<TD>" + String(i) + ":" + String(getMapeoSalidas(i)) + "</TD>";  
    cad += "<TD>" + String(nombreSalida(getMapeoSalidas(i))) + "</TD>";
    cad += "<TD>" + String(estadoSalida(getMapeoSalidas(i))) + "</TD>";
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
  for(uint8_t i=0;i<getNumSalidasME();i++) cad += "<TD>Salida[" + String(i) + "] salida asociada(" + getMapeoSalidas(i) + ")</TD>";
  cad += "</TR>"; 
    
  for(uint8_t i=0;i<getNumEstados();i++)
    {
    cad += "<TR class=\"modo2\">";  
    cad += "<TD>" + String(i) + "</TD>";  
    cad += "<TD>" + getNombreEstados(i) + "</TD>";
    for(uint8_t j=0;j<getNumSalidasME();j++) cad += "<TD>" + String(getValorSalidaEstados(i,j)) + "</TD>";
    cad += "</TR>";
    }
  cad += "</TABLE>";
  cad += "<BR>";
  
  //Transiciones
  cad += "<TABLE border=\"0\" width=\"50%\" cellpadding=\"0\" cellspacing=\"0\" width=\"300\" class=\"tabla\">\n";
  cad += "<CAPTION>TRANSICIONES</CAPTION>\n";  

  cad += "<TR>";
  cad += "<TH ROWSPAN=2>Estado inicial</TH>";
  cad += "<TH COLSPAN=" + String(getNumEntradasME()) + ">Entradas</TH>";
  cad += "<TH ROWSPAN=2>Estado final</TH>";
  cad += "</TR>";
  cad += "<TR>";
  for(uint8_t i=0;i<getNumEntradasME();i++) cad += "<TH>" + String(nombreEntrada(i)) + "</TH>";
  cad += "</TR>";

  for(uint8_t i=0;i<getNumTransiciones();i++)
    {
    cad += "<TR class=\"modo2\">";
    cad += "<TD>" + String(getNombreEstado(getEstadoInicialTransicion(i))) + "</TD>";
    for(uint8_t j=0;j<getNumEntradasME();j++) cad += "<TD>" + String(getValorEntradaTransicion(i,j)) + "</TD>";
    cad += "<TD>" + String(getNombreEstado(getEstadoFinalTransicion(i))) + "</TD>";    
    cad += "</TR>";
    }
  cad += "</TABLE>";
  cad += "<BR>";
  
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
void handleConfigSalidas(AsyncWebServerRequest *request)
  {
  String cad="";

  //genero la respuesta por defecto
  cad += cabeceraHTML;

  //Estados
  cad += "<TABLE border=\"0\" width=\"80%\" cellpadding=\"0\" cellspacing=\"0\" width=\"300\" class=\"tabla\">\n";
  cad += "<CAPTION>Salidas</CAPTION>\n";  

  cad += "<TR>"; 
  cad += "<TH>id</TH>";  
  cad += "<TH>Nombre</TH>";
  cad += "<TH>Configurada</TH>";
  cad += "<TH>Pin</TH>";  
  cad += "<TH>Controlador</TH>";  
  cad += "<TH>Inicio</TH>";    
  cad += "<TH>Modo</TH>";  
  cad += "<TH>Tipo</TH>";  
  cad += "<TH>valor PWM</TH>";      
  cad += "<TH>Ancho del pulso</TH>";  
  cad += "<TH>Inicio del pulso</TH>";  
  cad += "<TH>Fin del pulso</TH>";  
  cad += "<TH>Estado</TH>";
  cad += "<TH>Nombre del estado</TH>";
  cad += "<TH>mensaje</TH>";
  cad += "</TR>"; 

  for(uint8_t salida=0;salida<MAX_SALIDAS;salida++)
    {
    cad += "<TR class=\"modo2\">"; 
    cad += "<TD>" + String(salida) + "</TD>";
    cad += "<TD>" + String(nombreSalida(salida)) + "</TD>";  
    cad += "<TD>" + String(releConfigurado(salida)) + "</TD>";
    cad += "<TD>" + String(pinSalida(salida)) + "</TD>";
    cad += "<TD>" + String(controladorSalida(salida)) + "</TD>";
    cad += "<TD>" + String(inicioSalida(salida)) + "</TD>";
    cad += "<TD>" + String(modoSalidaNombre(salida)) + "</TD>";
    cad += "<TD>" + String(getTipoNombre(salida)) + "</TD>";
    cad += "<TD>" + String(getValorPWM(salida)) + "</TD>";
    cad += "<TD>" + String(anchoPulsoSalida(salida)) + "</TD>";
    cad += "<TD>" + String(inicioSalida(salida)) + "</TD>";
    cad += "<TD>" + String(finPulsoSalida(salida)) + "</TD>";
    cad += "<TD>" + String(estadoSalida(salida)) + "</TD>";
    cad += "<TD>" + String(nombreEstadoSalida(salida,estadoSalida(salida))) + "</TD>";
    cad += "<TD>" + String(mensajeEstadoSalida(salida,estadoSalida(salida))) + "</TD>";
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
void handleConfigEntradas(AsyncWebServerRequest *request)
  {
  String cad="";

  //genero la respuesta por defecto
  cad += cabeceraHTML;

  //Estados
  cad += "<TABLE border=\"0\" width=\"80%\" cellpadding=\"0\" cellspacing=\"0\" width=\"300\" class=\"tabla\">\n";
  cad += "<CAPTION>Entradas</CAPTION>\n";  

  cad += "<TR>"; 
  cad += "<TH>id</TH>";  
  cad += "<TH>Nombre</TH>";
  cad += "<TH>Configurada</TH>";
  cad += "<TH>Tipo</TH>";
  cad += "<TH>Pin</TH>";
  cad += "<TH>Estado activo</TH>";
  cad += "<TH>Estado fisico</TH>";
  cad += "<TH>Estado</TH>";
  cad += "<TH>Nombre del estado</TH>";
  cad += "<TH>mensaje</TH>";
  cad += "</TR>"; 

  for(uint8_t entrada=0;entrada<MAX_ENTRADAS;entrada++)
    {
    cad += "<TR class=\"modo2\">"; 
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
  if(request->hasArg("id") ) 
    {
    int8_t id=request->arg("id").toInt();

    //activaRele(id);
    conmutaSalida(id, ESTADO_ACTIVO, debugGlobal);

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
  if(request->hasArg("id") ) 
    {
    int8_t id=request->arg("id").toInt();

    conmutaSalida(id, ESTADO_DESACTIVO, debugGlobal);
    
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
  if(request->hasArg("id") ) 
    {
    int8_t id=request->arg("id").toInt();

    pulsoSalida(id);
    
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
  
  cad += "<h3>hay " + String(numPlanes) + " plan(es) activo(s).</h3>";
  
  for(int8_t i=0;i<numPlanes;i++)
    {
    cad += pintaPlanHTML(i);
    cad += "<BR>";
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
    cad += "Rele " + String(i) + " nombre: " + nombreSalida(i) + "| estado: " + estadoSalida(i);    
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

  cad += cabeceraHTML;

  if(request->hasArg("nombre") && request->hasArg("contenido")) //si existen esos argumentos
    {
    nombreFichero=request->arg("nombre");
    contenidoFichero=request->arg("contenido");

    if(salvaFichero( nombreFichero, nombreFichero+".bak", contenidoFichero)) 
	  {
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
  String message = "";

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
  String cad="";

  cad += cabeceraHTML;
  
  //Variables para manejar la lista de ficheros
  String contenido="";
  String fichero="";  
  int16_t to=0;
  
  if(listaFicheros(contenido)) 
    {
    Traza.mensaje("contenido inicial= %s\n",contenido.c_str());      
    //busco el primer separador
    to=contenido.indexOf(SEPARADOR); 

    cad +="<style> table{border-collapse: collapse;} th, td{border: 1px solid black; padding: 10px; text-align: left;}</style>";
    cad += "<TABLE border=\"0\" width=\"50%\" cellpadding=\"0\" cellspacing=\"0\" width=\"300\" class=\"tabla\">\n";
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
  Traza.mensaje("handleFileRead: %s\n", path.c_str());
  
  if (!path.startsWith("/")) path += "/";
  path = "/www" + path; //busco los ficheros en el SPIFFS en la carpeta www
  
  String contentType = getContentType(path);             // Get the MIME type
  String pathWithGz = path + ".gz";
  //if (SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)) 
  if (existeFichero(pathWithGz) || existeFichero(path)) 
    { // If the file exists, either as a compressed archive, or normal
    //if (SPIFFS.exists(pathWithGz)) path += ".gz"; // Si hay una version comprimida del fichero disponible, usa la version comprimida
    if (existeFichero(pathWithGz)) path += ".gz"; // Si hay una version comprimida del fichero disponible, usa la version comprimida
                                             
    request->send(SPIFFS, path, contentType);
    
    Traza.mensaje("\tfichero enviado: %s | contentType: %s\n",path.c_str(),contentType.c_str());
    return true;
    }
  Traza.mensaje("\tfichero no encontrado en SPIFFS: %s\n", path.c_str());   // If the file doesn't exist, return false
  return false;
  }
/****************************Google Home Notifier ******************************/
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

/**********************************************************************************************/
/*********************************************/
/*                                           */
/*  Datos del cacharro, entradas y salidas   */
/*                                           */
/*  version para incrustar                   */
/*                                           */
/*********************************************/  
void handleDatos(AsyncWebServerRequest *request) 
  {
  String cad="";

  //genero la respuesta por defecto
  cad += cabeceraHTMLlight;

  //Entradas  
  cad += "<TABLE border=\"0\" width=\"50%\" cellpadding=\"0\" cellspacing=\"0\" width=\"300\" class=\"tabla\">\n";
  cad += "<CAPTION>Entradas</CAPTION>\n";  
  for(int8_t i=0;i<MAX_ENTRADAS;i++)
    {
    if(entradaConfigurada(i)==CONFIGURADO) 
      {
      cad += "<TR>";
      cad += "<TD STYLE=\"color: " + TEXTO + "; text-align: center; background-color: " + FONDO + "\">" + nombreEntrada(i) + "</TD>";
      cad += "<TD STYLE=\"color: " + TEXTO + "; text-align: center; background-color: " + String((estadoEntrada(i)==ESTADO_DESACTIVO?DESACTIVO:ACTIVO)) + "; width: 100px\">" + String(nombreEstadoEntrada(i,estadoEntrada(i))) + "</TD>";
      cad += "</TR>";
      }
    }
  cad += "</TABLE>\n";
  cad += "<BR>";
  
  //Salidas
  cad += "<TABLE border=\"0\" width=\"50%\" cellpadding=\"0\" cellspacing=\"0\" width=\"300\" class=\"tabla\">\n";
  cad += "<CAPTION>Salidas</CAPTION>\n";  
  for(int8_t i=0;i<MAX_SALIDAS;i++)
    {
    if(releConfigurado(i)==CONFIGURADO)
      {      
      String orden="";
      if (estadoSalida(i)!=ESTADO_DESACTIVO) orden="desactiva"; 
      else orden="activa";

      cad += "<TR>\n";
      cad += "<TD STYLE=\"color: " + TEXTO + "; text-align: center; background-color: " + FONDO + "; width: 100px\">" + nombreSalida(i) + "</TD>\n";
      cad += "<TD STYLE=\"color: " + TEXTO + "; text-align: center; background-color: " + String((estadoSalida(i)==ESTADO_DESACTIVO?DESACTIVO:ACTIVO)) + "; width: 100px\">" + String(nombreEstadoSalida(i,estadoSalida(i))) + "</TD>";

      //acciones en funcion del modo
      switch (modoSalida(i))          
        {
        case MODO_MANUAL:
          //Enlace para activar o desactivar y para generar un pulso
          cad += "<TD>Manual</TD>\n";
          cad += "<td>\n";
          cad += "<form action=\"" + orden + "Rele\">\n";
          cad += "<input  type=\"hidden\" id=\"id\" name=\"id\" value=\"" + String(i) + "\" >\n";
          cad += "<input STYLE=\"color: " + TEXTO + "; text-align: center; background-color: " + String((estadoSalida(i)==ESTADO_DESACTIVO?ACTIVO:DESACTIVO)) + "; width: 80px\" type=\"submit\" value=\"" + orden + "r\">\n";
          cad += "</form>\n";
          cad += "<form action=\"pulsoSalida\">\n";
          cad += "<input  type=\"hidden\" id=\"id\" name=\"id\" value=\"" + String(i) + "\" >\n";
          cad += "<input STYLE=\"color: " + TEXTO + "; text-align: center; background-color: " + ACTIVO + "; width: 80px\" type=\"submit\" value=\"pulso\">\n";
          cad += "</form>\n";
          if(modoInicalSalida(i)!=MODO_MANUAL)
            {
            cad += "<form action=\"recuperaManualSalida\">\n";
            cad += "<input  type=\"hidden\" id=\"id\" name=\"id\" value=\"" + String(i) + "\" >\n";
            cad += "<input STYLE=\"color: " + TEXTO + "; text-align: center; background-color: " + ACTIVO + "; width: 160px\" type=\"submit\" value=\"recupera automatico\">\n";
            cad += "</form>\n";
            }
          cad += "</td>\n";    
          break;
        case MODO_SECUENCIADOR:
          cad += "<TD colspan=2> | Secuenciador " + String(controladorSalida(i)) + "</TD>\n";
          break;
        case MODO_SEGUIMIENTO:
          cad += "<TD>Siguiendo a la entrada " + nombreEntrada(controladorSalida(i)) + "</TD>\n"; //String(controladorSalida(i)) + "</TD>\n";
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
  cad += "<TABLE border=\"0\" width=\"50%\" cellpadding=\"0\" cellspacing=\"0\" width=\"300\" class=\"tabla\">\n";
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
  
  request->send(200, "text/html", cad);
  }

/*********************************************/
/*                                           */
/*  Version del firmware del cacharro        */
/*                                           */
/*  version para incrustar                   */
/*                                           */
/*********************************************/  
void handleVersion(AsyncWebServerRequest *request) 
  {
  String cad="";

  cad = cabeceraHTMLlight;
  cad += "<BR>" + nombre_dispositivo + ". Version " + String(VERSION) + ".";

  cad += pieHTML;
  
  request->send(200, "text/HTML", cad);  
  }

/*********************************************/
/*                                           */
/*  Lista los ficheros en el sistema a       */
/*  traves de una peticion HTTP              */ 
/*                                           */
/*  version para incrustar                   */
/*                                           */
/*********************************************/  
void handleListaFicheros2(AsyncWebServerRequest *request)
  {
  String nombreFichero="";
  String contenidoFichero="";
  String cad="";

  cad += "<h2>Lista de ficheros</h2>";
  
  //Variables para manejar la lista de ficheros
  String contenido="";
  String fichero="";  
  int16_t to=0;
  
  if(listaFicheros(contenido)) 
    {
    Traza.mensaje("contenido inicial= %s\n",contenido.c_str());      
    //busco el primer separador
    to=contenido.indexOf(SEPARADOR); 

    cad +="<style>";
    cad +="table{border-collapse: collapse;}";
    cad += "th, td{border: 1px solid black; padding: 10px; text-align: left;}";
    cad +="</style>";

    cad += "<table style=\"border: 0px; border-color: #FFFFFF;\"><tr style=\"border: 0px; border-color: #FFFFFF;\"><td style=\"border: 0px; border-color: #FFFFFF;\">";
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
    cad += "</td>";
    
    //Para crear un fichero nuevo
    cad += "<td style=\"border: 0px; border-color: #FFFFFF;\">";
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
    cad += "</td>";
    cad += "</tr></table>";
    }
  else cad += "<TR><TD>No se pudo recuperar la lista de ficheros</TD></TR>"; 

  cad += pieHTML;
  request->send(200, "text/html", cad); 
  }
