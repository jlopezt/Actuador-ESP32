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
#ifndef _MQTT_
#define _MQTT_
/***************************** Defines *****************************/

/***************************** Includes *****************************/
#include <Global.h>
/***************************** Includes *****************************/

/************************************************/
/* Inicializa valiables y estado del bus MQTT   */
/************************************************/
void inicializaMQTT(void);

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
/* MQTT menos al Ping                               */
/****************************************************/
void respondeGenericoMQTT(char* topic, byte* payload, unsigned int length);
  
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

/********************************************/
/* Funcion que envia un mensaje al bus      */
/* MQTT del broker                          */
/* Eliminado limite del buffer de envio     */
/********************************************/
boolean enviarMQTT(String topic, String payload);

/********************************************/
/* Funcion que revisa el estado del bus y   */
/* si se ha recibido un mensaje             */
/********************************************/
void atiendeMQTT(void);

/********************************************/
/*                                          */
/* Funcion que envia datos de estado del    */
/* controlador al broker                    */
/*                                          */
/********************************************/
void enviaDatos(boolean debug);
/******************************* UTILIDADES *************************************/
IPAddress getIPBroker(void);
uint16_t getPuertoBroker(void);
String getUsuarioMQTT(void);
String getPasswordMQTT(void);
String getTopicRoot(void);

String getIDMQTT(void);
int8_t getPublicarEntradas(void);
int8_t getPublicarSalidas(void);

String getWillTopic(void);
String getWillMsg(void);
boolean getCleanSession(void);

/********************************************/
/* Funcion que devuleve el estado           */
/* de conexion MQTT al bus                  */
/********************************************/
String stateTexto(void);  

#endif