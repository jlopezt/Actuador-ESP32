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

//Includes
#include <WebServer.h>

//Variables globales
WebServer server(PUERTO_WEBSERVER);

//Cadenas HTML precargadas
String cabeceraHTML="";
String enlaces="<TABLE>\n<CAPTION>Enlaces</CAPTION>\n<TR><TD><a href=\"info\" target=\"_self\">Info</a></TD></TR>\n<TR><TD><a href=\"test\" target=\"_self\">Test</a></TD></TR>\n<TR><TD><a href=\"restart\" target=\"_self\">Restart</a></TD></TR>\n<TR><TD><a href=\"listaFicheros\" target=\"_self\">Lista ficheros</a></TD></TR>\n<TR><TD><a href=\"estado\" target=\"_self\">Estado</a></TD></TR>\n<TR><TD><a href=\"estadoSalidas\" target=\"_self\">Estado salidas</a></TD></TR>\n<TR><TD><a href=\"estadoEntradas\" target=\"_self\">Estado entradas</a></TD></TR>\n<TR><TD><a href=\"planes\" target=\"_self\">Planes del secuenciador</a></TD></TR>\n<TR><TD><a href=\"maquinaEstados\" target=\"_self\">Maquina de estados</a></TD></TR></TABLE>\n"; 
String pieHTML="</BODY></HTML>";

/*********************************** Inicializacion y configuracion *****************************************************************/
void inicializaWebServer(void)
  {
  //lo inicializo aqui, despues de leer el nombre del dispositivo en la configuracion del cacharro
  cabeceraHTML="<HTML><HEAD><TITLE>" + nombre_dispositivo + " </TITLE></HEAD><BODY><h1><a href=\"../\" target=\"_self\">" + nombre_dispositivo + "</a><br></h1>";
    
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
  server.on("/maquinaEstados", handleMaquinaEstados);  //Servicio de representacion de las transiciones d ela maquina de estados
      
  server.on("/test", handleTest);  //URI de test
  server.on("/restart", handleRestart);  //URI de test
  server.on("/info", handleInfo);  //URI de test
  
  server.on("/listaFicheros", handleListaFicheros);  //URI de leer fichero
  server.on("/creaFichero", handleCreaFichero);  //URI de crear fichero
  server.on("/borraFichero", handleBorraFichero);  //URI de borrar fichero
  server.on("/leeFichero", handleLeeFichero);  //URI de leer fichero
  server.on("/manageFichero", handleManageFichero);  //URI de leer fichero  

  server.on("/edit.html",  HTTP_POST, []() {  // If a POST request is sent to the /edit.html address,
                                           server.send(200, "text/plain", ""); 
                                           }, handleFileUpload);                       // go to 'handleFileUpload'
  server.on("/speech", handleSpeechPath);
  server.on("/habla", handleHablaPath);
  
  server.onNotFound(handleNotFound);//pagina no encontrada

  server.begin();

  Serial.printf("Servicio web iniciado en puerto %i\n", PUERTO_WEBSERVER);
  }

/*******************************************************/
/*                                                     */ 
/* Invocado desde Loop, gestiona las peticiones web    */
/*                                                     */
/*******************************************************/
void webServer(int debug)
  {
  server.handleClient();
  }
/********************************* FIn inicializacion y configuracion ***************************************************************/

/************************* Gestores de las diferentes URL coniguradas ******************************/
/*************************************************/
/*                                               */
/*  Pagina principal. informacion de E/S         */
/*  Enlaces a las principales funciones          */
/*                                               */
/*************************************************/  
void handleRoot() 
  {
  String cad="";
  String orden="";

  //genero la respuesta por defecto
  cad += cabeceraHTML;
  //Entradas  
  cad += "<TABLE style=\"border: 2px solid black\">\n";
  cad += "<CAPTION>Entradas</CAPTION>\n";  
  for(int8_t i=0;i<MAX_ENTRADAS;i++)
    {
    //if(entradas[i].configurada==CONFIGURADO) cad += "<TR><TD>" + entradas[i].nombre + "-></TD><TD>" + String(entradas[i].estado) + "</TD></TR>\n";
    if(entradas[i].configurada==CONFIGURADO) cad += "<TR><TD>" + entradas[i].nombre + "-></TD><TD>" + String(entradas[i].nombreEstados[entradas[i].estado]) + "</TD></TR>\n";
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
      cad += "<TR>\n";
      cad += "<TD>" + nombreRele(i) + "-></TD><TD>" + String(estadoRele(i)) + "</TD>";            
/*Se sustituye por el bloque siguiente
      //compruebo si esta asociada a un plan de secuenciador
      if(asociadaASecuenciador(i)!=NO_CONFIGURADO && estadoSecuenciador())//Si esa asociada a un secuenciador o el secuenciador esta on
        {
        cad += "<TD colspan=2> | Secuenciador " + String(asociadaASecuenciador(i)) + "</TD>";
        }
      else if(salidaSeguimiento(i)!=NO_CONFIGURADO)
        {
        cad += "<TD>Siguiendo a entrada " + String(salidaSeguimiento(i)) + "</TD>";
        }
      else      
        {
        //Enlace para activar o desactivar
        if (estadoRele(i)==1) orden="desactiva"; else orden="activa";//para 0 y 2 (apagado y en pulso) activa
        cad += "<TD><a href=\"" + orden + "Rele\?id=" + String(i) + "\" target=\"_self\">" + orden + " rele</a></TD>\n";  
        //Enlace para generar un pulso
        cad += "<TD><a href=\"pulsoRele\?id=" + String(i) + "\" target=\"_self\">Pulso</a></TD>\n";
        }
  */
        //acciones en funcion del modo
        switch (getModoSalida(i))          
          {
          case MODO_MANUAL:
            //Enlace para activar o desactivar
            if (estadoRele(i)==1) orden="desactiva"; else orden="activa";//para 0 y 2 (apagado y en pulso) activa
            cad += "<TD><a href=\"" + orden + "Rele\?id=" + String(i) + "\" target=\"_self\">" + orden + " rele</a></TD>\n";  
            //Enlace para generar un pulso
            cad += "<TD><a href=\"pulsoRele\?id=" + String(i) + "\" target=\"_self\">Pulso</a></TD>\n";
            break;
          case MODO_SECUENCIADOR:
            cad += "<TD colspan=2> | Secuenciador " + String(asociadaASecuenciador(i)) + "</TD>";
            break;
          case MODO_SEGUIMIENTO:
            cad += "<TD>Siguiendo a entrada " + String(salidaSeguimiento(i)) + "</TD>";
            break;
          default://Salida no configurada
            cad += "<TD>¿?>/TD>";  
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

  server.send(200, "text/html", cad);
  }

/*************************************************/
/*                                               */
/*  Servicio de consulta de las transiciones de  */
/*  la maquina de estados                        */
/*                                               */
/*************************************************/  
void handleMaquinaEstados(void)
  {
  String cad="";
  String orden="";

  //genero la respuesta por defecto
  cad += cabeceraHTML;

  //Estados
  cad += "<TABLE style=\"border: 2px solid black\">\n";
  cad += "<CAPTION>ESTADOS</CAPTION>\n";  

  cad += "<TR>"; 
  cad += "<TD>id</TD>";  
  cad += "<TD>Nombre</TD>";
  for(uint8_t i=0;i<getNumSalidasME();i++) cad += "<TD>Salida " + String(i) + "</TD>";
  cad += "</TR>"; 
    
  for(uint8_t i=0;i<getNumEstados();i++)
    {
    cad += "<TR>";  
    cad += "<TD>" + String(i) + "</TD>";  
    cad += "<TD>" + estados[i].nombre + "</TD>";
    for(uint8_t j=0;j<getNumSalidasME();j++) cad += "<TD>" + String(estados[i].salidasAsociadas[j]) + "</TD>";
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
  int8_t numPlanes=getNumPlanes();  
  String cad=cabeceraHTML;
  
  cad += "<h1>hay " + String(numPlanes) + " plan(es) activo(s).</h1><BR>";
  
  for(int8_t i=0;i<numPlanes;i++)
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
  String cad="";

  cad += cabeceraHTML;
  
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
  String cad="";

  cad += cabeceraHTML;
  
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
  cad += "<h1>" + nombre_dispositivo + "</h1>";

  cad+= "<BR>-----------------Uptime---------------------<BR>";
  char tempcad[20]="";
  sprintf(tempcad,"%lu", (esp_timer_get_time()/(unsigned long)1000000)); //la funcion esp_timer_get_time() devuelve el contador de microsegundos desde el arranque. rota cada 292.000 años  
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
  String cad="";
  String nombreFichero="";
  String contenidoFichero="";
  boolean salvado=false;

  cad += cabeceraHTML;

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
  String nombreFichero="";
  String contenidoFichero="";
  String cad="";

  cad += cabeceraHTML;
  
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
  String contenido="";
   
  if(server.hasArg("nombre") ) //si existen esos argumentos
    {
    nombreFichero=server.arg("nombre");

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
  server.send(200, "text/html", cad); 
  }

/*********************************************/
/*                                           */
/*  Pagina no encontrada                     */
/*                                           */
/*********************************************/
void handleNotFound()
  {
  if(handleFileRead(server.uri()))return;
    
  String message = "";

  message = "<h1>" + nombre_dispositivo + "<br></h1>";
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

/*********************************************/
/*                                           */
/*  Habilita la edicion y borrado de los     */
/*  ficheros en el sistema a traves de una   */
/*  peticion HTTP                            */ 
/*                                           */
/*********************************************/ 
void handleManageFichero(void)
  {
  String nombreFichero="";
  String contenido="";
  String cad="";

  cad += cabeceraHTML;
  cad += "<h1>" + nombre_dispositivo + "</h1>";
  
  if(server.hasArg("nombre") ) //si existen esos argumentos
    {
    nombreFichero=server.arg("nombre");
    cad += "<h2>Fichero: " + nombreFichero + "</h2><BR>";  

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
      cad += "    contenido del fichero: <br><textarea cols=75 rows=20 name=\"contenido\">" + contenido + "</textarea>";
      cad += "    <BR>";
      cad += "    <input type=\"submit\" value=\"salvar\">";
      cad += "  </p>";
      cad += "</td></tr></table>";
      }
    else cad += "Error al abrir el fichero " + nombreFichero + "<BR>";
    }
  else cad += "Falta el argumento <nombre de fichero>"; 

  cad += pieHTML;
  server.send(200, "text/html", cad); 
  }

/*********************************************/
/*                                           */
/*  Lista los ficheros en el sistema a       */
/*  traves de una peticion HTTP              */ 
/*                                           */
/*********************************************/  
void handleListaFicheros(void)
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
  server.send(200, "text/html", cad); 
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

bool handleFileRead(String path) 
  { // send the right file to the client (if it exists)
  Serial.println("handleFileRead: " + path);
  
  if (!path.startsWith("/")) path += "/";
  path = "/www" + path; //busco los ficheros en el SPIFFS en la carpeta www
  //if (!path.endsWith("/")) path += "/";
  
  String contentType = getContentType(path);             // Get the MIME type
  String pathWithGz = path + ".gz";
  if (SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)) 
    { // If the file exists, either as a compressed archive, or normal
    if (SPIFFS.exists(pathWithGz))                         // If there's a compressed version available
      path += ".gz";                                         // Use the compressed verion
    File file = SPIFFS.open(path, "r");                    // Open the file
    size_t sent = server.streamFile(file, contentType);    // Send it to the client
    file.close();                                          // Close the file again
    Serial.println(String("\tSent file: ") + path);
    return true;
    }
  Serial.println(String("\tFile Not Found: ") + path);   // If the file doesn't exist, return false
  return false;
  }

void handleFileUpload()
  {
  File fsUploadFile;
  HTTPUpload& upload = server.upload();
  String path;
 
  if(upload.status == UPLOAD_FILE_START)
    {
    path = upload.filename;
    if(!path.startsWith("/")) path = "/"+path;
    if(!path.startsWith("/www")) path = "/www"+path;
    if(!path.endsWith(".gz")) 
      {                          // The file server always prefers a compressed version of a file 
      String pathWithGz = path+".gz";                    // So if an uploaded file is not compressed, the existing compressed
      if(SPIFFS.exists(pathWithGz))                      // version of that file must be deleted (if it exists)
         SPIFFS.remove(pathWithGz);
      }
      
    Serial.print("handleFileUpload Name: "); Serial.println(path);
    fsUploadFile = SPIFFS.open(path, "w");            // Open the file for writing in SPIFFS (create if it doesn't exist)
    path = String();
    }
  else if(upload.status == UPLOAD_FILE_WRITE)
    {
    if(fsUploadFile) fsUploadFile.write(upload.buf, upload.currentSize); // Write the received bytes to the file
    } 
  else if(upload.status == UPLOAD_FILE_END)
    {
    if(fsUploadFile) 
      {                                    // If the file was successfully created
      fsUploadFile.close();                               // Close the file again
      Serial.print("handleFileUpload Size: "); Serial.println(upload.totalSize);
      server.sendHeader("Location","/success.html");      // Redirect the client to the success page
      server.send(303);
      }
    else 
      {
      server.send(500, "text/plain", "500: couldn't create file");
      }
    }
  }

/****************************Google Home Notifier ******************************/
void handleSpeechPath() {
  String phrase = server.arg("phrase");
  if (phrase == "") {
    server.send(401, "text / plain", "query 'phrase' is not found");
    return;
  }
  

  /*
  if (ghn.notify(phrase.c_str()) != true) {
    Serial.println(ghn.getLastError());
    server.send(500, "text / plain", ghn.getLastError());
    return;
  }*/
  if(enviaNotificacion((char*)phrase.c_str())) server.send(200, "text / plain", "OK");
  server.send(404, "text / plain", "KO");  
}

void handleHablaPath() {
  server.send(200, "text/html", "<html><head></head><body><input type=\"text\"><button>speech</button><script>var d = document;d.querySelector('button').addEventListener('click',function(){xhr = new XMLHttpRequest();xhr.open('GET','/speech?phrase='+encodeURIComponent(d.querySelector('input').value));xhr.send();});</script></body></html>");
}  
