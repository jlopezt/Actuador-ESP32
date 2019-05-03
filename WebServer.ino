/************************************************************************************************
Servicio                       URL                             Formato entrada Formato salida                                            Comentario                                            Ejemplo peticion                      Ejemplo respuesta
Estado de los reles            http://IPActuador/estado        N/A             <id_0>#<nombre_0>#<estado_0>|<id_1>#<nombre_1>#<estado_1> Devuelve el id de cada rele y su estado               http://IPActuador/estado              1#1|2#0
Activa rele                    http://IPActuador/activaRele    id=<id>         <id>#<estado>                                             Activa el rele indicado y devuelve el estado leido    http://IPActuador/activaRele?id=1     1|1
Desactivarele                  http://IPActuador/desactivaRele id=<id>         <id>#<estado>                                             Desactiva el rele indicado y devuelve el estado leido http://IPActuador/desactivaRele?id=0  0|0
Test                           http://IPActuador/test          N/A             HTML                                                      Verifica el estado del Actuadot   
Reinicia el actuador           http://IPActuador/restart
Informacion del Hw del sistema http://IPActuador/info
************************************************************************************************/

//Configuracion de los servicios web
#define PUERTO_WEBSERVER  80
#define IDENTIFICACION  "<h1>Modulo Actuador (entradas/salidas)</h1>"

//Includes
#include <WebServer.h> //#include <ESP8266WebServer.h>
// Tapado en la migracion a ESP32#include <ESP8266mDNS.h>

//Variables globales
WebServer server(PUERTO_WEBSERVER); //ESP8266WebServer server[MAX_WEB_SERVERS];

//Cadenas HTML precargadas
String cabeceraHTML="<HTML><HEAD><TITLE>" + nombre_dispoisitivo + "</TITLE></HEAD><BODY>";
String pieHTML="</BODY></HTML>";
String enlaces="<TABLE>\n<CAPTION>Enlaces</CAPTION>\n<TR><TD><a href=\"info\" target=\"_blank\">Info</a></TD></TR>\n<TR><TD><a href=\"test\" target=\"_blank\">Test</a></TD></TR>\n<TR><TD><a href=\"restart\" target=\"_blank\">Restart</a></TD></TR>\n<TR><TD><a href=\"estado\" target=\"_blank\">Estado</a></TD></TR>\n<TR><TD><a href=\"estadoSalidas\" target=\"_blank\">Estado salidas</a></TD></TR>\n<TR><TD><a href=\"estadoEntradas\" target=\"_blank\">Estado entradas</a></TD></TR>\n<TR><TD><a href=\"planes\" target=\"_blank\">Planes del secuenciador</a></TD></TR></TABLE>\n"; 

/*******************************************************/
/*                                                     */ 
/* Invocado desde Loop, gestiona las peticiones web    */
/*                                                     */
/*******************************************************/
void webServer(int debug)
  {
  server.handleClient();  
  }

/************************* Gestores de las diferentes URL coniguradas ******************************/
/*************************************************/
/*                                               */
/*  Pagina principal. informacion de E/S         */
/*  Enlaces a las principales funciones          */
/*                                               */
/*************************************************/  
void handleRoot() 
  {
  String cad=cabeceraHTML;
  String orden="";

  //genero la respuesta por defecto  
  cad += IDENTIFICACION;
  cad += "<BR>\n";

  //Entradas  
  cad += "<TABLE style=\"border: 2px solid black\">\n";
  cad += "<CAPTION>Entradas</CAPTION>\n";  
  for(int8_t i=0;i<MAX_ENTRADAS;i++)
    {
    if(entradas[i].configurada==CONFIGURADO) cad += "<TR><TD>" + entradas[i].nombre + "</TD><TD>" + String(entradas[i].estado) + "</TD></TR>\n";
    }
  cad += "</TABLE>\n";
  cad += "<BR>";
  
  //Salidas
  cad += "\n<TABLE style=\"border: 2px solid blue\">\n";
  cad += "<CAPTION>Salidas</CAPTION>\n";  
  for(int8_t i=0;i<MAX_RELES;i++)
    {
    if(releConfigurado(i)==CONFIGURADO)
      {      
      cad += "<TR>\n";
      cad += "<TD>" + nombreRele(i) + "</TD><TD>" + String(estadoRele(i)) + "</TD>";            

      //compruebo si esta asociada a un plan de secuenciador
      if(asociadaASecuenciador(i)!=NO_CONFIGURADO && estadoSecuenciador())//Si esa asociada a un secuenciador o el secuenciador esta on
        {
        cad += "<TD colspan=2>Secuenciador " + String(asociadaASecuenciador(i)) + "</TD>";
        }
      else      
        {
        //Enlace para activar o desactivar
        if (estadoRele(i)==1) orden="desactiva"; else orden="activa";//para 0 y 2 (apagado y en pulso) activa
        cad += "<TD><a href=\"" + orden + "Rele\?id=" + String(i) + "\" target=\"_self\">" + orden + " rele</a></TD>\n";  
        //Enlace para generar un pulso
        cad += "<TD><a href=\"pulsoRele\?id=" + String(i) + "\" target=\"_self\">Pulso</a></TD>\n";
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
  cad += "<BR><BR>" + nombre_dispoisitivo + " . Version " + String(VERSION) + ".";

  server.send(200, "text/html", cad);
  }

/*************************************************/
/*                                               */
/*  Servicio de consulta de estado de            */
/*  las Salidas y las entradas                   */
/*  devuelve un formato json                     */
/*                                               */
/*************************************************/  
void handleEstado(void)
  {
  String cad=generaJsonEstado();
  
  server.send(200, "text/json", cad); 
  }  

/***************************************************/
/*                                                 */
/*  Servicio de consulta de estado de las salidas  */ 
/*  devuelve un formato json                       */
/*                                                 */
/***************************************************/  
void handleEstadoSalidas(void)
  {
  String cad=generaJsonEstadoSalidas();
  
  server.send(200, "text/json", cad); 
  }
  
/*****************************************************/
/*                                                   */
/*  Servicio de consulta de estado de las entradas   */
/*  devuelve un formato json                         */
/*                                                   */
/*****************************************************/  
void handleEstadoEntradas(void)
  {
  String cad=generaJsonEstadoEntradas();
  
  server.send(200, "text/json", cad); 
  }
  
/*********************************************/
/*                                           */
/*  Servicio de actuacion de rele            */
/*                                           */
/*********************************************/  
void handleActivaRele(void)
  {
  int8_t id=0;

  if(server.hasArg("id") ) 
    {
    int8_t id=server.arg("id").toInt();

    //activaRele(id);
    conmutaRele(id, nivelActivo, debugGlobal);

    handleRoot();
    }
    else server.send(404, "text/plain", "");  
  }

/*********************************************/
/*                                           */
/*  Servicio de desactivacion de rele        */
/*                                           */
/*********************************************/  
void handleDesactivaRele(void)
  {
  int8_t id=0;

  if(server.hasArg("id") ) 
    {
    int8_t id=server.arg("id").toInt();

    //desactivaRele(id);
    conmutaRele(id, !nivelActivo, debugGlobal);
    
    handleRoot();
    }
  else server.send(404, "text/plain", ""); 
  }

/*********************************************/
/*                                           */
/*  Servicio de pulso de rele                */
/*                                           */
/*********************************************/  
void handlePulsoRele(void)
  {
  int8_t id=0;

  if(server.hasArg("id") ) 
    {
    int8_t id=server.arg("id").toInt();

    //desactivaRele(id);
    pulsoRele(id);
    
    handleRoot();
    }
  else server.send(404, "text/plain", ""); 
  }

/*********************************************/
/*                                           */
/*  Servicio de representacion de los        */
/*  planes del secuenciador                  */
/*                                           */
/*********************************************/  
void handlePlanes(void)
  {
  String cad=cabeceraHTML;
  cad += IDENTIFICACION; //"Modulo " + String(direccion) + " Habitacion= " + nombres[direccion];

  for(int8_t i=0;i<getNumPlanes();i++)
    {
    cad += pintaPlanHTML(i);
    cad += "<BR><BR>";
    }
  
  cad += pieHTML;
    
  server.send(200, "text/html", cad); 
  
  }

/*********************************************/
/*                                           */
/*  Servicio para activar el secuenciador    */
/*                                           */
/*********************************************/  
void handleActivaSecuenciador(void)
  {
  activarSecuenciador();
  handleRoot();
  }

/*********************************************/
/*                                           */
/*  Servicio para desactivar el secuenciador */
/*                                           */
/*********************************************/  
void handleDesactivaSecuenciador(void)
  {
  desactivarSecuenciador();
  handleRoot();
  }

/*********************************************/
/*                                           */
/*  Servicio de test                         */
/*                                           */
/*********************************************/  
void handleTest(void)
  {
  String cad=cabeceraHTML;
  cad += IDENTIFICACION; //"Modulo " + String(direccion) + " Habitacion= " + nombres[direccion];
  
  cad += "Test OK<br>";
  cad += pieHTML;
    
  server.send(200, "text/html", cad); 
  }
  
/*********************************************/
/*                                           */
/*  Reinicia el dispositivo mediante         */
/*  peticion HTTP                            */ 
/*                                           */
/*********************************************/  
void handleRestart(void)
  {
  String cad=cabeceraHTML;
  cad += IDENTIFICACION;
  
  cad += "Reiniciando...<br>";
  cad += pieHTML;
    
  server.send(200, "text/html", cad);     
  delay(100);
  ESP.restart();
  }

/*********************************************/
/*                                           */
/*  Lee info del chipset mediante            */
/*  peticion HTTP                            */ 
/*                                           */
/*********************************************/  
void handleInfo(void)
  {
  String cad=cabeceraHTML;
  cad += IDENTIFICACION; //"Modulo " + String(direccion) + " Habitacion= " + nombres[direccion];

  cad+= "<BR>-----------------info logica-----------------<BR>";
  cad += "IP: " + String(getIP(debugGlobal));
  cad += "<BR>";  
  cad += "nivelActivo: " + String(nivelActivo);
  cad += "<BR>";  
  for(int8_t i=0;i<MAX_RELES;i++)
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
  cad += "ChipId: " + String(ESP.getChipRevision());
  cad += "<BR>";  
  cad += "SdkVersion: " + String(ESP.getSdkVersion());
  cad += "<BR>";  
  cad += "CpuFreqMHz: " + String(ESP.getCpuFreqMHz());
  cad += "<BR>";  
  cad += "FlashChipSize: " + String(ESP.getFlashChipSize());
  cad += "<BR>";  
  cad += "FlashChipSpeed: " + String(ESP.getFlashChipSpeed());
  cad += "<BR>";    
  cad += "-----------------------------------------------<BR>";  
  
  cad += pieHTML;
  server.send(200, "text/html", cad);     
  }

/*********************************************/
/*                                           */
/*  Crea un fichero a traves de una          */
/*  peticion HTTP                            */ 
/*                                           */
/*********************************************/  
void handleCreaFichero(void)
  {
  String cad=cabeceraHTML;
  String nombreFichero="";
  String contenidoFichero="";
  boolean salvado=false;

  cad += IDENTIFICACION;//"<h1>" + String(NOMBRE_FAMILIA) + "<br></h1>";

  if(server.hasArg("nombre") && server.hasArg("contenido")) //si existen esos argumentos
    {
    nombreFichero=server.arg("nombre");
    contenidoFichero=server.arg("contenido");

    if(salvaFichero( nombreFichero, nombreFichero+".bak", contenidoFichero)) cad += "Fichero salvado con exito<br>";
    else cad += "No se pudo salvar el fichero<br>"; 
    }
  else cad += "Falta el argumento <nombre de fichero>"; 

  cad += pieHTML;
  server.send(200, "text/html", cad); 
  }

/*********************************************/
/*                                           */
/*  Borra un fichero a traves de una         */
/*  peticion HTTP                            */ 
/*                                           */
/*********************************************/  
void handleBorraFichero(void)
  {
  String cad=cabeceraHTML;
  String nombreFichero="";
  String contenidoFichero="";

  cad += IDENTIFICACION;//"<h1>" + String(NOMBRE_FAMILIA) + "<br></h1>";
  
  if(server.hasArg("nombre") ) //si existen esos argumentos
    {
    nombreFichero=server.arg("nombre");

    if(borraFichero(nombreFichero)) cad += "El fichero " + nombreFichero + " ha sido borrado.\n";
    else cad += "No sepudo borrar el fichero " + nombreFichero + ".\n"; 
    }
  else cad += "Falta el argumento <nombre de fichero>"; 

  cad += pieHTML;
  server.send(200, "text/html", cad); 
  }

/*********************************************/
/*                                           */
/*  Lee un fichero a traves de una           */
/*  peticion HTTP                            */ 
/*                                           */
/*********************************************/  
void handleLeeFichero(void)
  {
  String cad=cabeceraHTML;
  String nombreFichero="";

  cad += IDENTIFICACION;//"<h1>" + String(NOMBRE_FAMILIA) + "<br></h1>";
  
  if(server.hasArg("nombre") ) //si existen esos argumentos
    {
    nombreFichero=server.arg("nombre");

    //inicializo el sistema de ficheros
    if (SPIFFS.begin()) 
      {
      Serial.println("---------------------------------------------------------------\nmounted file system");  
      //file exists, reading and loading
      if(!SPIFFS.exists(nombreFichero)) cad += "El fichero " + nombreFichero + " no existe.\n";
      else
        {
         File f = SPIFFS.open(nombreFichero, "r");
         if (f) 
           {
           Serial.println("Fichero abierto");
           size_t tamano_fichero=f.size();
           Serial.printf("El fichero tiene un tamaño de %i bytes.\n",tamano_fichero);
           cad += "El fichero tiene un tamaño de ";
           cad += tamano_fichero;
           cad += " bytes.<BR>";
           char buff[tamano_fichero+1];
           f.readBytes(buff,tamano_fichero);
           buff[tamano_fichero+1]=0;
           Serial.printf("El contenido del fichero es:\n******************************************\n%s\n******************************************\n",buff);
           cad += "El contenido del fichero es:<BR>";
           cad += buff;
           cad += "<BR>";
           f.close();
           }
         else cad += "Error al abrir el fichero " + nombreFichero + "<BR>";
        }  
      Serial.println("unmounted file system\n---------------------------------------------------------------");
      }//La de abrir el sistema de ficheros
    }
  else cad += "Falta el argumento <nombre de fichero>"; 

  cad += pieHTML;
  server.send(200, "text/html", cad); 
  }

/*********************************************/
/*                                           */
/*  Pagina no encontrada                     */
/*                                           */
/*********************************************/
void handleNotFound()
  {
  String message = "";//"<h1>" + String(NOMBRE_FAMILIA) + "<br></h1>";

  message = "<h1>" + String(NOMBRE_FAMILIA) + "<br></h1>";
  message += "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";

  for (uint8_t i=0; i<server.args(); i++)
    {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
    }
    
  server.send(404, "text/html", message);
  }

/*********************************** Inicializacion y configuracion *****************************************************************/
void inicializaWebServer(void)
  {
  /*******Configuracion del Servicio Web***********/  
  //Inicializo los serivcios  
  //decalra las URIs a las que va a responder
  server.on("/", handleRoot); //Responde con la iodentificacion del modulo
  server.on("/estado", handleEstado); //Servicio de estdo de reles
  server.on("/estadoSalidas", handleEstadoSalidas); //Servicio de estdo de reles
  server.on("/estadoEntradas", handleEstadoEntradas); //Servicio de estdo de reles    
  server.on("/activaRele", handleActivaRele); //Servicio de activacion de rele
  server.on("/desactivaRele", handleDesactivaRele);  //Servicio de desactivacion de rele
  server.on("/pulsoRele", handlePulsoRele);  //Servicio de pulso de rele
  server.on("/planes", handlePlanes);  //Servicio de representacion del plan del secuenciador
	server.on("/activaSecuenciador", handleActivaSecuenciador);  //Servicio para activar el secuenciador
  server.on("/desactivaSecuenciador", handleDesactivaSecuenciador);  //Servicio para desactivar el secuenciador
  
  server.on("/test", handleTest);  //URI de test

  server.on("/restart", handleRestart);  //URI de test
  server.on("/info", handleInfo);  //URI de test
  
  server.on("/creaFichero", handleCreaFichero);  //URI de crear fichero
  server.on("/borraFichero", handleBorraFichero);  //URI de borrar fichero
  server.on("/leeFichero", handleLeeFichero);  //URI de leer fichero

  server.onNotFound(handleNotFound);//pagina no encontrada

  server.begin();

  Serial.printf("Servicio web iniciado en puerto %i\n", PUERTO_WEBSERVER);
  }
