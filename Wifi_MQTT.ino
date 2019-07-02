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
//Includes MQTT
//#define MQTT_KEEPALIVE 60
#include <PubSubClient.h>
#define CLEAN_SESSION TRUE

//definicion de los comodines del MQTT
#define WILDCARD_ALL      "#"
#define WILDCARD_ONELEVEL "+"

//definicion de constantes para WILL
#define WILL_TOPIC  "will"
#define WILL_QOS    1
#define WILL_RETAIN false
#define WILL_MSG    ("¡"+ID_MQTT+" caido!").c_str()

//Definicion de variables globales
IPAddress IPBroker; //IP del bus MQTT
uint16_t puertoBroker; //Puerto del bus MQTT
String usuarioMQTT; //usuario par ala conxion al broker
String passwordMQTT; //password parala conexion al broker
String topicRoot; //raiz del topic a publicar. Util para separar mensajes de produccion y prepropduccion
String ID_MQTT; //ID del modulo en su conexion al broker
String topicKeepAlive; //Topic para los mensajes de keepAlive
int8_t enviarKeepAlive; //flag para determinar si se envia o no keepalive MQTT
int8_t publicarEntradas; //Flag para determinar si se envia el json con los valores de las entradas
int8_t publicarSalidas; //Flag para determinar si se envia el json con los valores de las salidas

WiFiClient espClient;
PubSubClient clienteMQTT(espClient);

/************************************************/
/* Inicializa valiables y estado del bus MQTT   */
/************************************************/
void inicializaMQTT(void)
  {
  //recupero datos del fichero de configuracion
  if (!recuperaDatosMQTT(false)) Serial.printf("error al recuperar config MQTT.\nConfiguracion por defecto.\n");

	//Si va bien inicializo con los valores correstoc, si no con valores por defecto
  //confituro el servidor y el puerto
  clienteMQTT.setServer(IPBroker, puertoBroker);
  //configuro el callback, si lo hay
  clienteMQTT.setCallback(callbackMQTT);

  if (conectaMQTT()) Serial.println("connectado al broker");  
  else Serial.printf("error al conectar al broker con estado %i\n",clienteMQTT.state());

  //Variables adicionales
  //topicKeepAlive=ID_MQTT + "/keepalive";
  topicKeepAlive="keepalive";
  }

/************************************************/
/* Recupera los datos de configuracion          */
/* del archivo de MQTT                          */
/************************************************/
boolean recuperaDatosMQTT(boolean debug)
  {
  String cad="";
  if (debug) Serial.println("Recupero configuracion de archivo...");

  //cargo el valores por defecto
  IPBroker.fromString("0.0.0.0");
  ID_MQTT=String(NOMBRE_FAMILIA); //ID del modulo en su conexion al broker
  puertoBroker=0;
  usuarioMQTT="";
  passwordMQTT="";
  topicRoot=""; 
  enviarKeepAlive=0; 
  publicarEntradas=1; 
  publicarSalidas=1;    

  if(!leeFicheroConfig(MQTT_CONFIG_FILE, cad)) 
    {
    //Confgiguracion por defecto
    Serial.printf("No existe fichero de configuracion MQTT\n");
    //cad="{\"IPBroker\": \"10.68.1.100\", \"puerto\": 1883, \"usuarioMQTT\": \"usuario\", \"passwordMQTT\": \"password\",  \"ID_MQTT\": \"garaje32/puerta\",  \"topicRoot\":  \"casa\",  \"keepAlive\": 0, \"publicarEntradas\": 1, \"publicarSalidas\": 0}";
    cad="{\"IPBroker\": \"0.0.0.0\", \"puerto\": 1883, \"usuarioMQTT\": \"usuario\", \"passwordMQTT\": \"password\",  \"ID_MQTT\": \"" + String(NOMBRE_FAMILIA) + "\",  \"topicRoot\":  \"" + NOMBRE_FAMILIA + "\",  \"keepAlive\": 0, \"publicarEntradas\": 0, \"publicarSalidas\": 0}";
    if(salvaFicheroConfig(MQTT_CONFIG_FILE, MQTT_CONFIG_BAK_FILE, cad)) Serial.printf("Fichero de configuracion MQTT creado por defecto\n");    
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
  //json.printTo(Serial);
  if (json.success()) 
    {
    Serial.println("parsed json");
//******************************Parte especifica del json a leer********************************
    IPBroker.fromString((const char *)json["IPBroker"]);
    ID_MQTT=((const char *)json["ID_MQTT"]);
    puertoBroker = atoi(json["puerto"]); 
    usuarioMQTT=((const char *)json["usuarioMQTT"]);
    passwordMQTT=((const char *)json["passwordMQTT"]);
    topicRoot=((const char *)json["topicRoot"]);
    enviarKeepAlive=atoi(json["keepAlive"]); 
    publicarEntradas=atoi(json["publicarEntradas"]); 
    publicarSalidas=atoi(json["publicarSalidas"]); 
    Serial.printf("Configuracion leida:\nID MQTT: %s\nIP broker: %s\nIP Puerto del broker: %i\nUsuario: %s\nPassword: %s\nTopic root: %s\nEnviar KeepAlive: %i\nPublicar entradas: %i\nPublicar salidas: %i\n",ID_MQTT.c_str(),IPBroker.toString().c_str(),puertoBroker,usuarioMQTT.c_str(),passwordMQTT.c_str(),topicRoot.c_str(),enviarKeepAlive,publicarEntradas,publicarSalidas);
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
  //char mensaje[length+1];  

  //Serial.printf("Entrando en callback: \n Topic: %s\nPayload %s\nLongitud %i\n", topic, payload, length);
  
  /**********compruebo el topic*****************/
  //Sirve para solo atender a los topic de medidas. Si se suscribe a otro habira que gestionarlo aqui
  String cad=String(topic);
  //topics descartados
  if(cad==String(topicRoot + "/" + ID_MQTT + "/keepalive")) return;
  if(cad==String(topicRoot + "/" + ID_MQTT + "/entradas")) return;
  if(cad==String(topicRoot + "/" + ID_MQTT + "/salidas")) return;
  
  //si es del tipo "casa/#"
  //copio el topic a la cadena cad

  if(cad.substring(0,String(topicRoot + "/" + ID_MQTT).length())!=String(topicRoot + "/" + ID_MQTT)) //no deberia, solo se suscribe a los suyos
    {
  Serial.printf("Valor de String(topicRoot + ID_MQTT).length()\n topicRoot: #%s#\nID_MQTT: #%s#\nlongitud: %i\n",topicRoot .c_str(),ID_MQTT.c_str(),String(topicRoot + ID_MQTT).length());
  Serial.printf("Subcadena cad.substring(0,String(topicRoot + ID_MQTT).length()): %s\n",cad.substring(0,String(topicRoot + ID_MQTT).length()).c_str());
  

    Serial.printf("topic no reconocido: \ntopic: %s\nroot: %s\n", cad.c_str(),cad.substring(0,cad.indexOf("/")).c_str());  
    return;
    }  
  else//topic correcto
    {  
    //copio el payload en la cadena mensaje
    char mensaje[length+1];
    for(int8_t i=0;i<length;i++) mensaje[i]=payload[i];
    mensaje[length]=0;//añado el final de cadena 
    Serial.printf("MQTT mensaje recibido: %s\nmensaje copiado %s\n",payload,mensaje);
  
    /**********************Leo el JSON***********************/
    const size_t bufferSize = JSON_OBJECT_SIZE(3) + 50;
    DynamicJsonBuffer jsonBuffer(bufferSize);     
    JsonObject& root = jsonBuffer.parseObject(mensaje);
    if (root.success()) 
      {  
      //Registro el satelite y copio sobre la habitacion correspondiente del array los datos recibidos
      int id=atoi(root["id"]);      
      int estado;
      if(root["estado"]=="off") estado=0;       
      else if(root["estado"]=="on") estado=1;           
      else if(root["estado"]=="pulso") estado=2;     
       
      actuaRele(id, estado);
      }
    }
  }

/********************************************/
/* Funcion que gestiona la conexion al bus  */
/* MQTT del broker                          */
/********************************************/
boolean conectaMQTT(void)  
  {
  int8_t intentos=0;

  if(IPBroker==IPAddress(0,0,0,0)) return (false);
    
  while (!clienteMQTT.connected()) 
    {    
    if(debugGlobal) Serial.println("No conectado, intentando conectar.");
  
    // Attempt to connect
    //boolean connect(const char* id, const char* user, const char* pass, const char* willTopic, uint8_t willQos, boolean willRetain, const char* willMessage, boolean cleanSession);
    if (clienteMQTT.connect(ID_MQTT.c_str(), usuarioMQTT.c_str(), passwordMQTT.c_str(), (topicRoot+"/"+String(WILL_TOPIC)).c_str(), WILL_QOS, WILL_RETAIN, ("¡"+ID_MQTT+" caido!").c_str(), CLEAN_SESSION))
      {
      if(debugGlobal) Serial.println("conectado");
      //Inicio la subscripcion al topic de las medidas boolean subscribe(const char* topic);
      String topic = topicRoot + "/" + ID_MQTT + "/" + WILDCARD_ALL; //uso el + como comodin para culaquier habitacion
      if (clienteMQTT.subscribe(topic.c_str())) Serial.printf("Subscrito al topic %s\n", topic.c_str());
      else Serial.printf("Error al subscribirse al topic %s\n", topic.c_str());       
      return(true);
      }

    if(intentos++>3) return (false);
    
    if(debugGlobal) Serial.printf("Error al conectar al broker. Estado: %s\n",stateTexto().c_str());
    delay(500);      
    }
  return true;
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
    String topicCompleto=topicRoot+"/"+ID_MQTT+"/"+topic;  
    //Serial.printf("Enviando:\ntopic:  %s | payload: %s\n",topicCompleto.c_str(),payload.c_str());
  
    if(clienteMQTT.beginPublish(topicCompleto.c_str(), payload.length(), false))//boolean beginPublish(const char* topic, unsigned int plength, boolean retained)
      {
      for(uint8_t i=0;i<payload.length();i++) clienteMQTT.write((uint8_t)payload.charAt(i));//virtual size_t write(uint8_t);
      return(clienteMQTT.endPublish()); //int endPublish();
      }
    }
  else return (false);
  }

/********************************************/
/* Funcion que revisa el estado del bus y   */
/* si se ha recibido un mensaje             */
/********************************************/
void atiendeMQTT(boolean debug)
  {
  if(enviarKeepAlive)
    {  
    String topic=topicKeepAlive;  
    String payload=String(millis());
  
    if(enviarMQTT(topic, payload)) if(debug)Serial.println("Enviado json al broker con exito.");
    else if(debug)Serial.println("¡¡Error al enviar json al broker!!");
    }
    
  clienteMQTT.loop();
  }  

void enviaDatos(boolean debug)
  {
  String payload;

  //**************************************ENTRADAS******************************************
  if(publicarEntradas==1)
    {
    payload=generaJsonEstadoEntradas();//genero el json de las entradas
    //Lo envio al bus    
    if(enviarMQTT("entradas", payload)) if(debug)Serial.println("Enviado json al broker con exito.");
    else if(debug)Serial.println("¡¡Error al enviar json al broker!!");
    }
  else if(debug)Serial.printf("No publico entradas. Publicar entradas es %i\n",publicarEntradas);
  //******************************************SALIDAS******************************************
  if(publicarSalidas==1)
    {
    payload=generaJsonEstadoSalidas();//genero el json de las salidas
    //Lo envio al bus    
    if(enviarMQTT("salidas", payload)) if(debug)Serial.println("Enviado json al broker con exito.");
    else if(debug)Serial.println("¡¡Error al enviar json al broker!!");  
    }  
  else if(debug)Serial.printf("No publico salidas. Publicar salidas es %i\n",publicarSalidas);  
  }

/******************************* UTILIDADES *************************************/
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
  
