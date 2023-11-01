/**********************************************/
/*                                            */
/*  Gestion de la conexion MQTT               */
/*  Incluye la conexion al bus y la           */
/*  definicion del callback de suscripcion    */
/*                                            */
/* Librria de sooprte del protocolo MQTT      */
/* para arduino/ESP8266/ESP32                 */
/*                                            */
/* https://pubsubclient.knolleary.net/api.html*/
/**********************************************/

/***************************** Defines *****************************/
#define MI_BUFFER_MQTT_SIZE 512
#define MI_MQTT_KEEPALIVE   20
//definicion de los comodines del MQTT
#define WILDCARD_ALL      "#"
#define WILDCARD_ONELEVEL "+"

//definicion de constantes para WILL
#define WILL_TOPIC  "will"
#define WILL_QOS    1
#define WILL_RETAIN false

//definicion del clean session
#define CLEAN_SESSION true

//definicion del topic ping
#define TOPIC_PING "ping"
#define TOPIC_PING_RESPUESTA "ping/respuesta"

#define DIR_CA_CERT "/ca.crt"
/***************************** Defines *****************************/

/***************************** Includes *****************************/
#include <MQTT.h>
#include <configNVS.h>
#include <ClienteHTTP.h>
#include <Sensores.h>
#include <Variables.h>
#include <Entradas.h>
#include <Salidas.h>
#include <RedWifi.h>
#include <Ficheros.h>
#include <MaqEstados.h>
#include <Secuenciador.h>

#include <acciones.h>
#include <PubSubClient.h>
#include <WiFiClientSecure.h>
/***************************** Includes *****************************/
/***************************** Prototipos *****************************/
/************************************************/
/* Recupera los datos de configuracion          */
/* del archivo de MQTT                          */
/************************************************/
boolean recuperaDatosMQTT(boolean debug);

/*********************************************/
/* Parsea el json leido del fichero de       */
/* configuracio MQTT                         */
/*********************************************/
boolean parseaConfiguracionMQTT(String contenido);

/***********************************************Funciones de gestion de mensajes MQTT**************************************************************/
/***************************************************/
/* Funcion que recibe el mensaje cuando se publica */
/* en el bus un topic al que esta subscrito        */
/***************************************************/
void callbackMQTT(char* topic, byte* payload, unsigned int length);

/****************************************************/
/* Funcion que gestiona la respuesta a los mensajes */ 
/* MQTT que no tienen gestion especifica            */
/****************************************************/
void respondeGenericoMQTT(char* topic, byte* payload, unsigned int length);
  
/****************************************************/
/* Funcion que gestiona la respuesta a los mensajes */ 
/* MQTT que llegan al buzon                         */
/****************************************************/
void respondeBuzonMQTT(char* topic, byte* payload, unsigned int length);

/***************************************************/
/* Funcion que gestiona la respuesta al ping MQTT  */
/***************************************************/
void respondePingMQTT(char* topic, byte* payload, unsigned int length);

/***************************************************/
/*    Genera el JSON de respuesta al Ping MQTT     */
/***************************************************/
String generaJSONPing(boolean debug);  

/********************************************/
/* Funcion que gestiona la conexion al bus  */
/* MQTT del broker                          */
/********************************************/
boolean conectaMQTT(void);  
/***************************** Prototipos *****************************/

//Definicion de variables globales
String modoMQTT="";
IPAddress IPBroker; //IP del bus MQTT
String BrokerDir; //IP o URL del broker
uint16_t puertoBroker; //Puerto del bus MQTT
uint16_t timeReconnectMQTT; //Tiempo de espera en la reconexion al bus
String usuarioMQTT; //usuario par ala conxion al broker
String passwordMQTT; //password parala conexion al broker
String topicRoot; //raiz del topic a publicar. Util para separar mensajes de produccion y prepropduccion
String topicCompleto; //topicRoot mas usuario (CID) mas nombreServicio (SID)
String ID_MQTT; //ID del modulo en su conexion al broker
int8_t publicarMedidas; //Flag para determinar si se envia el json con los valores de las medidas
int8_t publicarEntradas; //Flag para determinar si se envia el json con los valores de las entradas
int8_t publicarSalidas; //Flag para determinar si se envia el json con los valores de las salidas
int8_t publicarSecuenciador; //Flag para determinar si se envia el json con los valores del secuenciador
int8_t publicarMaquinaEstados; //Flag para determinar si se envia el json con los valores del estado de la maquina de estados

String willMsg = String("¡"+ID_MQTT+" caido!");

WiFiClientSecure espClientSSL;
WiFiClient espClient;
PubSubClient clienteMQTT;
String caCert;

/************************************************/
/* Inicializa valiables y estado del bus MQTT   */
/************************************************/
void inicializaMQTT(void)
  {
  //recupero datos del fichero de configuracion
  if (!recuperaDatosMQTT(false)) Traza.mensaje("error al recuperar config MQTT.\nConfiguracion por defecto.\n");

  //Si va bien inicializo con los valores correctos, si no con valores por defecto
  
  if(modoMQTT=="TLS")
    {
    Traza.mensaje("Modo de conexion: %s\n", modoMQTT.c_str());
    
    //Leo el fichero con el certificado de CA
    if(!leeFichero(DIR_CA_CERT,caCert)) Traza.mensaje("No se pudo leer el certificado CA\n");
    //else Traza.mensaje("Certificado CA:\n%s\n",caCert.c_str());

    /* set SSL/TLS certificate */
    espClientSSL.setCACert(caCert.c_str());
    clienteMQTT.setClient(espClientSSL);
    }
  else
    {
    clienteMQTT.setClient(espClient);
    }
  Traza.mensaje("Cliente MQTT configurado\n");

  //configuro el servidor y el puerto
  if (BrokerDir==String("")) clienteMQTT.setServer(IPBroker, puertoBroker);
  else clienteMQTT.setServer(BrokerDir.c_str(), puertoBroker);

  //Configuro el tamaño del buffer
  clienteMQTT.setBufferSize(MI_BUFFER_MQTT_SIZE);
  clienteMQTT.setKeepAlive(MI_MQTT_KEEPALIVE);
  
  //configuro el callback, si lo hay
  clienteMQTT.setCallback(callbackMQTT);

  if (conectaMQTT()) Traza.mensaje("connectado al broker\n");
  else Traza.mensaje("error al conectar al broker con estado %i\n",clienteMQTT.state());
  }

/************************************************/
/* Recupera los datos de configuracion          */
/* del archivo de MQTT                          */
/************************************************/
boolean recuperaDatosMQTT(boolean debug)
  {
  String cad="";
  if (debug) Traza.mensaje("Recupero configuracion de archivo...\n");

  //cargo el valores por defecto
  modoMQTT=""; //Sin encriptacion SSL/TLS
  IPBroker.fromString("0.0.0.0");
  BrokerDir="";
  puertoBroker=0;
  timeReconnectMQTT=100;
  ID_MQTT=String(NOMBRE_FAMILIA); //ID del modulo en su conexion al broker
  usuarioMQTT="";
  passwordMQTT="";
  topicRoot="";
  publicarMedidas=1;
  publicarEntradas=1; 
  publicarSalidas=1;    
  publicarMaquinaEstados=1;

  if(!leeFichero(MQTT_CONFIG_FILE, cad))
    {
    //Algo salio mal, Confgiguracion por defecto
    Traza.mensaje("No existe fichero de configuracion MQTT o esta corrupto\n");
    cad="{\"IPBroker\": \"0.0.0.0\", \"BrokerDir\": \"\"; \"puerto\": 1883, \"timeReconnectMQTT\": 500, \"usuarioMQTT\": \"usuario\", \"passwordMQTT\": \"password\",  \"ID_MQTT\": \"" + String(NOMBRE_FAMILIA) + "\",  \"topicRoot\":  \"" + NOMBRE_FAMILIA + "\", \"publicarMedidas\": 0, \"publicarEntradas\": 0, \"publicarSalidas\": 0}";
    //if (salvaFichero(MQTT_CONFIG_FILE, MQTT_CONFIG_BAK_FILE, cad)) Traza.mensaje("Fichero de configuracion MQTT creado por defecto\n");    
    }

  return parseaConfiguracionMQTT(cad);
  }  

/*********************************************/
/* Parsea el json leido del fichero de       */
/* configuracio MQTT                         */
/*********************************************/
boolean parseaConfiguracionMQTT(String contenido)
  {  
  DynamicJsonDocument doc(1024);
  DeserializationError err = deserializeJson(doc,contenido);

  if (err) {
    Serial.printf("Error deserializando el json %s\n",err.c_str());
    return false;
  }
  else
    {
    Traza.mensaje("\nparsed json\n");
//******************************Parte especifica del json a leer********************************
    if (doc.containsKey("modoMQTT"))  modoMQTT=doc["modoMQTT"].as<String>();
    ID_MQTT=doc["ID_MQTT"].as<String>();
    if (doc.containsKey("IPBroker")) IPBroker.fromString(doc["IPBroker"].as<String>());
    if (doc.containsKey("BrokerDir")) BrokerDir=doc["BrokerDir"].as<String>();
    puertoBroker=doc["puerto"].as<uint16_t>();
    timeReconnectMQTT=doc["timeReconnectMQTT"].as<uint16_t>();
    usuarioMQTT=doc["usuarioMQTT"].as<String>();
    passwordMQTT=doc["passwordMQTT"].as<String>();
    topicRoot=doc["topicRoot"].as<String>();
    topicCompleto=topicRoot + "/" + configNVS.usuario + "/" + configNVS.nombreServicio;
    publicarMedidas=doc["publicarMedidas"].as<int8_t>();
    publicarEntradas=doc["publicarEntradas"].as<int8_t>();
    publicarSalidas=doc["publicarSalidas"].as<int8_t>();
    publicarSecuenciador=doc["publicarSecuenciador"].as<int8_t>();
    publicarMaquinaEstados=doc["publicarMaquinaEstados"].as<int8_t>();
    
    Traza.mensaje("Configuracion leida:\nID MQTT: %s\nIP broker: %s\nBrokerDir: %s\nIP Puerto del broker: %i\ntimeReconnectMQTT: %i\nUsuario: %s\nPassword: %s\nTopic root: %s\nTopic completo: %s\nPublicar Medidas: %i\nPublicar entradas: %i\nPublicar salidas: %i\nPublicar secuenciador: %i\nPublicar maquina estados: %i\n",ID_MQTT.c_str(),IPBroker.toString().c_str(),BrokerDir.c_str(),puertoBroker,timeReconnectMQTT,usuarioMQTT.c_str(),passwordMQTT.c_str(),topicRoot.c_str(),topicCompleto.c_str(),publicarMedidas,publicarEntradas,publicarSalidas,publicarSecuenciador,publicarMaquinaEstados);
//************************************************************************************************
    return true;
    }
  return false;
  }
/***********************************************Funciones de gestion de mensajes MQTT**************************************************************/
/***************************************************/
/* Funcion que recibe el mensaje cuando se publica */
/* en el bus un topic al que esta subscrito        */
/***************************************************/
void callbackMQTT(char* topic, byte* payload, unsigned int length)
  {
  char payload_char[length]="";
  uint8_t i=0;
  for(i=0;i<length;i++) payload_char[i]=payload[i];
  payload_char[i]=0;
  if(debugGlobal || 1) Traza.mensaje("Entrando en callback: \n Topic: %s\nPayload %s\nLongitud %i\n", topic, payload_char, length);
  
  /**********compruebo el topic*****************/
  //Identifica si el topic del mensaje es uno de los suscritos (deberia ser siempre que si)
  //Compara el topic recibido con los que tiene suscritos para redirigir a la funcion de gestion correspondiente  
  String cad=String(topic);

  //Descarto los mensajes generados por el propio dispositivo
  if(cad==String(topicCompleto + "/medidas")) return;
  if(cad==String(topicCompleto + "/entradas")) return;
  if(cad==String(topicCompleto + "/salidas")) return;
  if(cad==String(topicCompleto + "/variables")) return;
  if(cad==String(topicCompleto + "/sensores")) return;
  if(cad==String(topicCompleto + "/secuenciador")) return;
  if(cad==String(topicCompleto + "/maquinaEstados")) return;
  if(cad==String(topicCompleto + "/humbrales")) return;
  if(cad==String(topicCompleto + "/mensajes")) return;

  //Para cada topic suscrito...
  //if(cad.equalsIgnoreCase(topicCompleto + <topicSuscrito>)) <funcion de gestion>(topic,payload,length);  
  if(cad.equalsIgnoreCase(TOPIC_PING)) respondePingMQTT(topic,payload,length);    
  else if(cad.equalsIgnoreCase(topicCompleto + "/buzon")) respondeBuzonMQTT(topic,payload,length);
  else if(cad.substring(0,String(topicCompleto).length())!=String(topicCompleto)) //no deberia, solo se suscribe a los suyos
    {
    Traza.mensaje("topic no reconocido: \ntopic: %s\nroot: %s\n", cad.c_str(),cad.substring(0,cad.indexOf("/")).c_str());  
    return;
    }  
  else respondeGenericoMQTT(topic,payload,length); 
  }

/****************************************************/
/* Funcion que gestiona la respuesta a los mensajes */ 
/* MQTT que llegan al buzon                         */
/****************************************************/
void respondeBuzonMQTT(char* topic, byte* payload, unsigned int length){
  char mensaje[length+1];

  //copio el payload en la cadena mensaje
  for(int8_t i=0;i<length;i++) mensaje[i]=payload[i];
  mensaje[length]=0;//añado el final de cadena 
  String cad=String(mensaje);
  Traza.mensaje("Recibido mensaje:\ntopic: %s\npayload: %s\nlength: %i\n\n",topic,mensaje,length);

  /**********************Leo el JSON***********************/
  DynamicJsonDocument doc(512);///(bufferSize);     
  DeserializationError err = deserializeJson(doc,mensaje);
  if (err) {
    Serial.printf("Error deserializando el json %s\n",err.c_str());
    return;
  }

  //si el json es valido
  enrutador(cad);
}

/****************************************************/
/* Funcion que gestiona la respuesta a los mensajes */ 
/* MQTT menos al Ping                               */
/****************************************************/
void respondeGenericoMQTT(char* topic, byte* payload, unsigned int length)
  {  
  char mensaje[length+1];

  //copio el payload en la cadena mensaje
  for(int8_t i=0;i<length;i++) mensaje[i]=payload[i];
  mensaje[length]=0;//añado el final de cadena 
  Traza.mensaje("Recibido mensaje:\ntopic: %s\npayload: %s\nlength: %i\n\n",topic,mensaje,length);

  /**********************Leo el JSON***********************/
  DynamicJsonDocument doc(512);     
  DeserializationError err = deserializeJson(doc,mensaje);
  if (err) {
    Serial.printf("Error deserializando el json %s\n",err.c_str());
    return;
  }

  //Leo el rele y el valor a setear
  if(doc.containsKey("id") && doc.containsKey("estado"))
    {
    int id=doc["id"].as<int>(); 
    int estado=0;
    if(doc["estado"].as<String>()==String("off")) estado=0;       
    else if(doc["estado"].as<String>()==String("on")) estado=1;           
    else if(doc["estado"].as<String>()==String("pulso")) estado=2;     
     
    if(salidas.setSalida(id,estado)==-1) Traza.mensaje("Se intento actuar sobre una salida que no esta en modo manual\n");
    }
  else Traza.mensaje("Mensaje no esperado: %s\n",mensaje);
  /**********************Fin JSON***********************/    
  }
  
/***************************************************/
/* Funcion que gestiona la respuesta al ping MQTT  */
/***************************************************/
void respondePingMQTT(char* topic, byte* payload, unsigned int length)
  {  
  char mensaje[length];    

  Traza.mensaje("Recibido mensaje Ping:\ntopic: %s\npayload: %s\nlength: %i\n",topic,payload,length);
  
  //copio el payload en la cadena mensaje
  for(int8_t i=0;i<length;i++) mensaje[i]=payload[i];
  mensaje[length]=0;//acabo la cadena

  /**********************Leo el JSON***********************/
  const size_t bufferSize = JSON_OBJECT_SIZE(3) + 50;
  DynamicJsonDocument doc(512);     
  DeserializationError err = deserializeJson(doc,mensaje);
  
  if (err) {
    Serial.printf("Error deserializando el json %s\n",err.c_str());
    return;
  }

  //Si tiene IP se pregunta por un elemento en concreto. Compruebo si soy yo.
  if (doc.containsKey("IP")) 
    {
    if (String(doc["IP"].as<const char*>())!=getIP(false)) return;
    }

  //SI no tenia IP o si tenia la mia, respondo
  String T=TOPIC_PING_RESPUESTA;
  String P= generaJSONPing(false).c_str();
  Traza.mensaje("Topic: %s\nPayload: %s\n",T.c_str(),P.c_str());
  Traza.mensaje("Resultado: %i\n", clienteMQTT.publish(T.c_str(),P.c_str()));   
  /**********************Fin JSON***********************/    
  }

/***************************************************/
/*    Genera el JSON de respuesta al Ping MQTT     */
/***************************************************/
String generaJSONPing(boolean debug)  
  {
  String cad="";

  cad += "{";
  cad += "\"myIP\": \"" + getIP(false) + "\",";
  cad += "\"ID_MQTT\": \"" + ID_MQTT + "\",";
  cad += "\"IPBbroker\": \"" + IPBroker.toString() + "\",";
  cad += "\"BrokerDir\": \"" + BrokerDir + "\",";
  cad += "\"IPPuertoBroker\":" + String(puertoBroker) + "";
  cad += "}";

  if (debug) Traza.mensaje("Respuesta al ping MQTT: \n%s\n",cad.c_str());
  return cad;
  }

/********************************************/
/* Funcion que gestiona la conexion al bus  */
/* MQTT del broker                          */
/********************************************/
boolean conectaMQTT(void)  
  {
  int8_t intentos=0;
  String topic;

  if(IPBroker==IPAddress(0,0,0,0) && BrokerDir==String("")) 
    {
    if(debugGlobal) Traza.mensaje("IP del broker = 0.0.0.0 y BrokerDir="", no se intenta conectar.\n");
    return (false);//SI la IP del Broker es 0.0.0.0 (IP por defecto) no intentaq conectar y sale con error
    }
  
  if(WiFi.status()!=WL_CONNECTED) 
    {
    if(debugGlobal) Traza.mensaje("La conexion WiFi no se encuentra disponible.\n");
    return (false);
    }

  while (!clienteMQTT.connected()) 
    {    
    if(debugGlobal) Traza.mensaje("No conectado, intentando conectar.\n");
  
    // Attempt to connect
    Traza.mensaje("Parametros MQTT:\nID_MQTT: %s\nusuarioMQTT: %s\npasswordMQTT: %s\nWILL_TOPIC: %s\nWILL_QOS: %i\nWILL_RETAIN: %i\nWILL_MSG: %s\nCLEAN_SESSION: %i\n",ID_MQTT.c_str(),usuarioMQTT.c_str(),passwordMQTT.c_str(),(topicRoot+"/"+String(WILL_TOPIC)).c_str(), WILL_QOS, WILL_RETAIN,willMsg.c_str(),CLEAN_SESSION);
   
    //boolean connect(const char* id, const char* user, const char* pass, const char* willTopic, uint8_t willQos, boolean willRetain, const char* willMessage, boolean cleanSession);
    if (clienteMQTT.connect(ID_MQTT.c_str(), usuarioMQTT.c_str(), passwordMQTT.c_str(), (topicCompleto+"/"+String(WILL_TOPIC)).c_str(), WILL_QOS, WILL_RETAIN, willMsg.c_str(), CLEAN_SESSION))
      {
      if(debugGlobal) Traza.mensaje("conectado\n");
      
      //Inicio la subscripcion al topic de las medidas boolean subscribe(const char* topic);
      //topic = topicCompleto + "/" + WILDCARD_ALL; //uso el + como comodin para culaquier habitacion
      topic = topicCompleto + "/buzon"; //me subscribo al buzon
      if (clienteMQTT.subscribe(topic.c_str())) Traza.mensaje("Subscrito al topic %s\n", topic.c_str());
      else Traza.mensaje("Error al subscribirse al topic %s\n", topic.c_str());       

      //Suscripcion al topic de ping
      topic=TOPIC_PING;
      if (clienteMQTT.subscribe(topic.c_str())) Traza.mensaje("Subscrito al topic %s\n", topic.c_str());
      else Traza.mensaje("Error al subscribirse al topic %s\n", topic.c_str());

      return(true);
      }

    if(debugGlobal) Traza.mensaje("Error al conectar al broker. Estado: %s\n",stateTexto().c_str());
    if(intentos++>=2) return (false);
    delay(timeReconnectMQTT);      
    }
    
  return (false);  
  }

/********************************************/
/* Funcion que envia un mensaje al bus      */
/* MQTT del broker                          */
/* Eliminado limite del buffer de envio     */
/********************************************/
boolean enviarMQTT(String topic, String payload)
  {
  //si no esta conectado, conecto
  if (!clienteMQTT.connected()) conectaMQTT();

  //si y esta conectado envio, sino salgo con error
  if (clienteMQTT.connected()) 
    {    
    if(clienteMQTT.beginPublish((topicCompleto+"/"+topic).c_str(), payload.length(), false))
      {
      for(uint16_t i=0;i<payload.length();i++) clienteMQTT.write((uint8_t)payload.charAt(i));//virtual size_t write(uint8_t);
      return(clienteMQTT.endPublish()); //int endPublish();
      }
    }
  
  return (false);
  }

/********************************************/
/* Funcion que revisa el estado del bus y   */
/* si se ha recibido un mensaje             */
/********************************************/
void atiendeMQTT(void)
  {
  clienteMQTT.loop();
  }

/********************************************/
/*                                          */
/* Funcion que envia datos de estado del    */
/* controlador al broker                    */
/*                                          */
/********************************************/
void enviaDatos(boolean debug)
  {
  String payload;

  //**************************************MEDIDAS******************************************
  if(publicarMedidas==1)
    {
    payload=variables.generaJsonEstado();//genero el json de las entradas
    //Lo envio al bus    
    if(enviarMQTT("medidas", payload)) {if(debug)Traza.mensaje("Enviado json al broker con exito.\n");}
    else if(debug)Traza.mensaje("¡¡Error al enviar json al broker!!\n");
    }
  else if(debug)Traza.mensaje("No publico medidas. Publicar medidas es %i\n",publicarMedidas);
  //**************************************ENTRADAS******************************************
  if(publicarEntradas==1)
    {
    payload=entradas.generaJsonEstado();//genero el json de las entradas
    //Lo envio al bus    
    if(enviarMQTT("entradas", payload)) {if(debug)Traza.mensaje("Enviado json al broker con exito.\n");}
    else if(debug)Traza.mensaje("¡¡Error al enviar json al broker!!\n");
    }
  else if(debug)Traza.mensaje("No publico entradas. Publicar entradas es %i\n",publicarEntradas);
  //******************************************SALIDAS******************************************
  if(publicarSalidas==1)
    {
    payload=salidas.generaJsonEstado();//genero el json de las salidas
    //Lo envio al bus    
    if(enviarMQTT("salidas", payload)) {if(debug)Traza.mensaje("Enviado json al broker con exito.\n");}
    else if(debug)Traza.mensaje("¡¡Error al enviar json al broker!!\n");
    }  
  else if(debug)Traza.mensaje("No publico salidas. Publicar salidas es %i\n",publicarSalidas);  
  //******************************************Secuenciador******************************************
  if(publicarSecuenciador==1)
    {
    payload=secuenciador.generaJsonEstado();//genero el json de estado de la maquina d eestados
    //Lo envio al bus    
    if(enviarMQTT("secuenciador", payload)) {if(debug)Traza.mensaje("Enviado json al broker con exito.\n");}
    else if(debug)Traza.mensaje("¡¡Error al enviar json al broker!!\n");
    }  
  else if(debug)Traza.mensaje("No publico secuenciador. Publicar secuenciador es %i\n",publicarSecuenciador);      
  //******************************************Maquina de estados******************************************
  if(publicarMaquinaEstados==1)
    {
    payload=maquinaEstados.generaJsonEstado();//genero el json de estado de la maquina d eestados
    //Lo envio al bus    
    if(enviarMQTT("maquinaEstados", payload)) {if(debug)Traza.mensaje("Enviado json al broker con exito.\n");}
    else if(debug)Traza.mensaje("¡¡Error al enviar json al broker!!\n");
    }  
  else if(debug)Traza.mensaje("No publico maquina de estados. Publicar maquina de estados es %i\n",publicarMaquinaEstados);  
    
  }

/******************************* UTILIDADES *************************************/
IPAddress getIPBroker(void){return IPBroker;}
String getBrokerDir(void){return BrokerDir;}
String getBroker(void){
  if (BrokerDir!="") return BrokerDir;
  
  return IPBroker.toString();
}
uint16_t getPuertoBroker(void){return puertoBroker;}
String getUsuarioMQTT(void){return usuarioMQTT;}
String getPasswordMQTT(void){return passwordMQTT;}
String getTopicRoot(void){return topicRoot;}
String getIDMQTT(void){return ID_MQTT;}
int8_t getPublicarMedidas(void){return publicarMedidas;}
int8_t getPublicarEntradas(void){return publicarEntradas;}
int8_t getPublicarSalidas(void){return publicarSalidas;}
String getWillTopic(void){return WILL_TOPIC;}
String getWillMsg(void){return willMsg;}
boolean getCleanSession(void){return CLEAN_SESSION;}

/********************************************/
/* Funcion que devuleve el estado           */
/* de conexion MQTT al bus                  */
/********************************************/
String stateTexto(void)  
  {
  int r = clienteMQTT.state();

  String cad=String(r) + " : ";
  
  switch (r)
    {
    case -4:
      cad += "MQTT_CONNECTION_TIMEOUT";
      break;
    case -3:
      cad += "MQTT_CONNECTION_LOST";
      break;
    case -2:
      cad += "MQTT_CONNECT_FAILED";
      break;
    case -1:
      cad += "MQTT_DISCONNECTED";
      break;
    case  0:
      cad += "MQTT_CONNECTED";
      break;
    case  1:
      cad += "MQTT_CONNECT_BAD_PROTOCOL";
      break;
    case  2:
      cad += "MQTT_CONNECT_BAD_CLIENT_ID";
      break;
    case  3:
      cad += "MQTT_CONNECT_UNAVAILABLE";
      break;
    case  4:
      cad += "MQTT_CONNECT_BAD_CREDENTIALS";
      break;     
    case  5:
      cad += "MQTT_CONNECT_UNAUTHORIZED";
      break;
    default:
      cad += "????";
    }
      
  return (cad);
  }
