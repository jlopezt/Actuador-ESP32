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
//definicion de los comodines del MQTT
#define WILDCARD_ALL      "#"
#define WILDCARD_ONELEVEL "+"

//definicion de constantes para WILL
#define WILL_TOPIC  "will"
#define WILL_QOS    1
#define WILL_RETAIN false
#define WILL_MSG    String("¡"+ID_MQTT+" caido!").c_str()

//definicion del clean session
#define CLEAN_SESSION true

//definicion del topic ping
#define TOPIC_PING "ping"
#define TOPIC_PING_RESPUESTA "ping/respuesta"

#define DIR_CA_CERT "/ca.crt"
/***************************** Defines *****************************/

/***************************** Includes *****************************/
#include <MQTT.h>
#include <Entradas.h>
#include <Salidas.h>
#include <RedWifi.h>
#include <Ficheros.h>

#include <PubSubClient.h>
#include <WiFiClientSecure.h>
/***************************** Includes *****************************/

//Definicion de variables globales
String modoMQTT="";
IPAddress IPBroker; //IP del bus MQTT
String BrokerDir; //IP o URL del broker
uint16_t puertoBroker; //Puerto del bus MQTT
uint16_t timeReconnectMQTT; //Tiempo de espera en la reconexion al bus
String usuarioMQTT; //usuario par ala conxion al broker
String passwordMQTT; //password parala conexion al broker
String topicRoot; //raiz del topic a publicar. Util para separar mensajes de produccion y prepropduccion
String ID_MQTT; //ID del modulo en su conexion al broker
int8_t publicarEntradas; //Flag para determinar si se envia el json con los valores de las entradas
int8_t publicarSalidas; //Flag para determinar si se envia el json con los valores de las salidas

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
  Traza.mensaje("Cliente MQTT configurado");

  //configuro el servidor y el puerto
  if (BrokerDir==String("")) clienteMQTT.setServer(IPBroker, puertoBroker);
  else clienteMQTT.setServer(BrokerDir.c_str(), puertoBroker);
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
  publicarEntradas=1; 
  publicarSalidas=1;    

  if(!leeFichero(MQTT_CONFIG_FILE, cad))
    {
    //Algo salio mal, Confgiguracion por defecto
    Traza.mensaje("No existe fichero de configuracion MQTT o esta corrupto\n");
    cad="{\"IPBroker\": \"0.0.0.0\", \"BrokerDir\": \"\"; \"puerto\": 1883, \"timeReconnectMQTT\": 500, \"usuarioMQTT\": \"usuario\", \"passwordMQTT\": \"password\",  \"ID_MQTT\": \"" + String(NOMBRE_FAMILIA) + "\",  \"topicRoot\":  \"" + NOMBRE_FAMILIA + "\", \"publicarEntradas\": 0, \"publicarSalidas\": 0}";
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
  DynamicJsonBuffer jsonBuffer;
  JsonObject& json = jsonBuffer.parseObject(contenido.c_str());

  if (json.success()) 
    {
    Traza.mensaje("\nparsed json\n");
//******************************Parte especifica del json a leer********************************
    if (json.containsKey("modoMQTT"))  modoMQTT=json.get<String>("modoMQTT");
    ID_MQTT=json.get<String>("ID_MQTT");
    if (json.containsKey("IPBroker")) IPBroker.fromString(json.get<String>("IPBroker"));
    if (json.containsKey("BrokerDir")) BrokerDir=json.get<String>("BrokerDir");
    puertoBroker=json.get<uint16_t>("puerto");
    timeReconnectMQTT=json.get<uint16_t>("timeReconnectMQTT");
    usuarioMQTT=json.get<String>("usuarioMQTT");
    passwordMQTT=json.get<String>("passwordMQTT");
    topicRoot=json.get<String>("topicRoot");
    publicarEntradas=json.get<int8_t>("publicarEntradas"); 
    publicarSalidas=json.get<int8_t>("publicarSalidas"); 
    
    Traza.mensaje("Configuracion leida:\nID MQTT: %s\nIP broker: %s\nBrokerDir: %s\nIP Puerto del broker: %i\ntimeReconnectMQTT: %i\nUsuario: %s\nPassword: %s\nTopic root: %s\nPublicar entradas: %i\nPublicar salidas: %i\n",ID_MQTT.c_str(),IPBroker.toString().c_str(),BrokerDir.c_str(),puertoBroker,timeReconnectMQTT,usuarioMQTT.c_str(),passwordMQTT.c_str(),topicRoot.c_str(),publicarEntradas,publicarSalidas);
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
  if(debugGlobal) Traza.mensaje("Entrando en callback: \n Topic: %s\nPayload %s\nLongitud %i\n", topic, payload, length);
  
  /**********compruebo el topic*****************/
  //Identifica si el topic del mensaje es uno de los suscritos (deberia ser siempre que si)
  //Compara el topic recibido con los que tiene suscritos para redirigir a la funcion de gestion correspondiente  
  String cad=String(topic);

  //topics descartados (los genera este dispositivo)
  if(cad==String(topicRoot + "/" + ID_MQTT + "/entradas")) return;
  if(cad==String(topicRoot + "/" + ID_MQTT + "/salidas")) return;

  //Para cada topic suscrito...
  //if(cad.equalsIgnoreCase(topicRoot + <topicSuscrito>)) <funcion de gestion>(topic,payload,length);  
  if(cad.equalsIgnoreCase(TOPIC_PING)) respondePingMQTT(topic,payload,length);    
  //Si no empieza por <topicRoot + "/" + ID_MQTT> lo descarto
  else if(cad.substring(0,String(topicRoot + "/" + ID_MQTT).length())!=String(topicRoot + "/" + ID_MQTT)) //no deberia, solo se suscribe a los suyos
    {
    Traza.mensaje("Valor de String(topicRoot + ID_MQTT).length()\n topicRoot: #%s#\nID_MQTT: #%s#\nlongitud: %i\n",topicRoot .c_str(),ID_MQTT.c_str(),String(topicRoot + ID_MQTT).length());
    Traza.mensaje("Subcadena cad.substring(0,String(topicRoot + ID_MQTT).length()): %s\n",cad.substring(0,String(topicRoot + ID_MQTT).length()).c_str());
  

    Traza.mensaje("topic no reconocido: \ntopic: %s\nroot: %s\n", cad.c_str(),cad.substring(0,cad.indexOf("/")).c_str());  
    return;
    }  
  else respondeGenericoMQTT(topic,payload,length); 
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
  const size_t bufferSize = JSON_OBJECT_SIZE(3) + 50;
  DynamicJsonBuffer jsonBuffer(bufferSize);     
  JsonObject& root = jsonBuffer.parseObject(mensaje);
  if (!root.success()) 
    {
    Traza.mensaje("No se pudo parsear el JSON\n");
    return; //si el mensaje es incorrecto sale  
    }

  //Leo el rele y el valor a setear
  if(root.containsKey("id") && root.containsKey("estado"))
    {
    int id=root.get<int>("id"); 
    int estado=0;
    if(root.get<String>("estado")=="off") estado=0;       
    else if(root.get<String>("estado")=="on") estado=1;           
    else if(root.get<String>("estado")=="pulso") estado=2;     
     
    if(salidas[id].setSalida(estado)==-1) Traza.mensaje("Se intento actuar sobre una salida que no esta en modo manual\n");
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
  DynamicJsonBuffer jsonBuffer(bufferSize);     
  JsonObject& root = jsonBuffer.parseObject(mensaje);
  if (!root.success()) 
    {
    Traza.mensaje("No se pudo parsear el JSON\n");
    return; //si el mensaje es incorrecto sale  
    }

  //Si tiene IP se pregunta por un elemento en concreto. Compruebo si soy yo.
  if (root.containsKey("IP")) 
    {
    if (String(root["IP"].as<char*>())!=getIP(false)) return;
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
    Traza.mensaje("Parametros MQTT:\nID_MQTT: %s\nusuarioMQTT: %s\npasswordMQTT: %s\nWILL_TOPIC: %s\nWILL_QOS: %i\nWILL_RETAIN: %i\nWILL_MSG: %s\nCLEAN_SESSION: %i\n",ID_MQTT.c_str(),usuarioMQTT.c_str(),passwordMQTT.c_str(),(topicRoot+"/"+String(WILL_TOPIC)).c_str(), WILL_QOS, WILL_RETAIN,String(WILL_MSG).c_str(),CLEAN_SESSION);
   
    //boolean connect(const char* id, const char* user, const char* pass, const char* willTopic, uint8_t willQos, boolean willRetain, const char* willMessage, boolean cleanSession);
    if (clienteMQTT.connect(ID_MQTT.c_str(), usuarioMQTT.c_str(), passwordMQTT.c_str(), (topicRoot+"/"+String(WILL_TOPIC)).c_str(), WILL_QOS, WILL_RETAIN, String(WILL_MSG).c_str(), CLEAN_SESSION))
      {
      if(debugGlobal) Traza.mensaje("conectado\n");
      
      //Inicio la subscripcion al topic de las medidas boolean subscribe(const char* topic);
      topic = topicRoot + "/" + ID_MQTT + "/" + WILDCARD_ALL; //uso el + como comodin para culaquier habitacion
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
    String topicCompleto=topicRoot+"/"+topic;  
    
    if(clienteMQTT.beginPublish(topicCompleto.c_str(), payload.length(), false))
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

  //**************************************ENTRADAS******************************************
  if(publicarEntradas==1)
    {
    payload=generaJsonEstadoEntradas();//genero el json de las entradas
    //Lo envio al bus    
    if(enviarMQTT(ID_MQTT+"/"+"entradas", payload)) {if(debug)Traza.mensaje("Enviado json al broker con exito.\n");}
    else if(debug)Traza.mensaje("¡¡Error al enviar json al broker!!\n");
    }
  else if(debug)Traza.mensaje("No publico entradas. Publicar entradas es %i\n",publicarEntradas);
  //******************************************SALIDAS******************************************
  if(publicarSalidas==1)
    {
    payload=generaJsonEstadoSalidas();//genero el json de las salidas
    //Lo envio al bus    
    if(enviarMQTT(ID_MQTT+"/"+"salidas", payload)) {if(debug)Traza.mensaje("Enviado json al broker con exito.\n");}
    else if(debug)Traza.mensaje("¡¡Error al enviar json al broker!!\n");
    }  
  else if(debug)Traza.mensaje("No publico salidas. Publicar salidas es %i\n",publicarSalidas);  
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
int8_t getPublicarEntradas(void){return publicarEntradas;}
int8_t getPublicarSalidas(void){return publicarSalidas;}
String getWillTopic(void){return WILL_TOPIC;}
String getWillMsg(void){return WILL_MSG;}
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
