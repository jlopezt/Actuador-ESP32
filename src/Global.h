/*******************************************/
/*                                         */
/*  Include general para todo el proyecto  */
/*                                         */
/*******************************************/

/***************************** Defines *****************************/
#ifndef _GLOBAL_
#define _GLOBAL_

//Defines generales
#define NOMBRE_FAMILIA   "Actuador/Secuenciador (E/S)"
#define VERSION          "esp32 - ver 6.4.0"//(ESP32 1.0.4|OTA|MQTT|Logic++|Secuenciador|FicherosWeb|Eventos SNTP|Maquina de estados)"

#define TRUE              1
#define FALSE             0
#define NO_CONFIGURADO   -1
#define CONFIGURADO       1

#define SEPARADOR        '|'
#define SUBSEPARADOR     '#'

#define MAX_VUELTAS      UINT16_MAX// 32767 

//Ficheros de configuracion
#define FICHERO_ERRORES                  "/Errores.log"
#define GLOBAL_CONFIG_FILE               "/Config.json"
#define GLOBAL_CONFIG_BAK_FILE           "/Config.json.bak"
#define ENTRADAS_CONFIG_FILE             "/EntradasConfig.json"
#define ENTRADAS_CONFIG_BAK_FILE         "/EntradasConfig.json.bak"
#define SALIDAS_CONFIG_FILE              "/SalidasConfig.json"
#define SALIDAS_CONFIG_BAK_FILE          "/SalidasConfig.json.bak"
#define WIFI_CONFIG_FILE                 "/WiFiConfig.json"
#define WIFI_CONFIG_BAK_FILE             "/WiFiConfig.json.bak"
#define MQTT_CONFIG_FILE                 "/MQTTConfig.json"
#define MQTT_CONFIG_BAK_FILE             "/MQTTConfig.json.bak"
#define SECUENCIADOR_CONFIG_FILE         "/SecuenciadorConfig.json"
#define SECUENCIADOR_CONFIG_BAK_FILE     "/SecuenciadorConfig.json.bak"
#define GHN_CONFIG_FILE                  "/GHNConfig.json"
#define GHN_CONFIG_BAK_FILE              "/GHNConfig.json.bak"
#define MAQUINAESTADOS_CONFIG_FILE       "/MaqEstadosConfig.json"
#define MAQUINAESTADOS_CONFIG_BAK_FILE   "/MaqEstadosConfig.json.bak"
#define FTP_CONFIG_FILE                  "/FTPConfig.json"
#define FTP_CONFIG_BAK_FILE              "/FTPConfig.json.bak"

//Entradas y Salidas
#define MAX_PINES         20 //numero de pines disponibles para entradas y salidas
#define MAX_ENTRADAS      10 //numero maximo de reles soportado
#define MAX_SALIDAS       MAX_PINES-MAX_ENTRADAS //numero maximo de salidas

//Secuenciador
#define MAX_PLANES 2

//Funciones de led
#define LED_BUILTIN                2 //GPIO del led de la placa en los ESP32   
/***************************** Defines *****************************/

/***************************** Includes *****************************/
#include <Arduino.h>
#include <Traza.h>
//#include <Ficheros.h>
#include <ArduinoJson.h>
/***************************** Includes *****************************/

/***************************** Variables *****************************/
extern String nombre_dispositivo;//(NOMBRE_FAMILIA);//Nombre del dispositivo, por defecto el de la familia
extern int debugGlobal; //por defecto desabilitado
extern int nivelActivo;
/***************************** Variables *****************************/

/************************* Utilidades ***************************/
void configuraLed(void);
void enciendeLed(void);
void apagaLed(void);
void parpadeaLed(uint8_t veces, uint16_t espera=100);
void parpadeaLed(uint8_t veces, uint16_t espera);

String generaJsonConfiguracionNivelActivo(String configActual, int nivelAct);
/************************* Utilidades ***************************/

#endif