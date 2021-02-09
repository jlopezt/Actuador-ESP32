/*****************************************/
/*                                       */
/*  Control de entradas y salidas        */
/*                                       */
/*****************************************/

/***************************** Defines *****************************/
#ifndef _ENTRADAS_
#define _ENTRADAS_

#define TOPIC_MENSAJES    "mensajes"

//estados de las entradas y salidas
#ifndef ESTADO_DESACTIVO
#define ESTADO_DESACTIVO  0
#endif
#ifndef ESTADO_ACTIVO
#define ESTADO_ACTIVO  1
#endif
#ifndef ESTADO_PULSO
#define ESTADO_PULSO  2
#endif
/***************************** Defines *****************************/

/***************************** Includes *****************************/
#include <arduino.h>
#include <Global.h>
/***************************** Includes *****************************/

/************************************** Funciones de configuracion ****************************************/
/*********************************************/
/* Inicializa los valores de los registros de*/
/* las entradas y recupera la configuracion  */
/*********************************************/
void inicializaEntradas(void);

/*********************************************/
/* Lee el fichero de configuracion de las    */
/* entradas o genera conf por defecto        */
/*********************************************/
boolean recuperaDatosEntradas(int debug);

/*********************************************/
/* Parsea el json leido del fichero de       */
/* configuracio de las entradas              */
/*********************************************/
boolean parseaConfiguracionEntradas(String contenido);

/**********************************************************ENTRADAS******************************************************************/  
/*************************************************/
/*                                               */
/*       Lee el estado de las entradas           */
/*                                               */
/*************************************************/
void consultaEntradas(bool debug);

/*************************************************/
/*                                               */
/*Devuelve el estado 0|1 del rele indicado en id */
/*                                               */
/*************************************************/
int8_t estadoEntrada(int8_t id);

/*************************************************/
/*                                               */
/* Devuelve el estado 0|1 de la entrada tal cual */
/* se ha leido, sin cruzar con nada              */
/*                                               */
/*************************************************/
int8_t estadoFisicoEntrada(int8_t id);

/*************************************************/
/*                                               */
/* Devuelve el estado activo 0|1 de la entrada   */
/*                                               */
/*************************************************/
int8_t estadoActivoEntrada(int8_t id);

/*************************************************/
/*                                               */
/* Envia un mensaje MQTT para que se publique un */
/* audio en un GHN                               */
/*                                               */
/*************************************************/
void enviaMensajeEntrada(int8_t id_entrada, int8_t estado);

/********************************************************/
/*                                                      */
/*     Devuelve si la entrada esta configurada          */
/*                                                      */
/********************************************************/ 
boolean entradaConfigurada(uint8_t id);

/********************************************************/
/*                                                      */
/*  Devuelve el nombre del rele con el id especificado  */
/*                                                      */
/********************************************************/
String nombreEntrada(int8_t id);

/********************************************************/
/*                                                      */
/*     Devuelve el pin de la entrada                    */
/*                                                      */
/********************************************************/ 
uint8_t pinEntrada(uint8_t id);

/********************************************************/
/*                                                      */
/*     Devuelve el tipo de la entrada                   */
/*                                                      */
/********************************************************/ 
String tipoEntrada(uint8_t id);

/********************************************************/
/*                                                      */
/*  Devuelve el nombre del estado de una entrada        */
/*                                                      */
/********************************************************/ 
String nombreEstadoEntrada(uint8_t id, uint8_t estado);

/********************************************************/
/*                                                      */
/*  Devuelve el mensaje de una entrada en un estado     */
/*                                                      */
/********************************************************/ 
String mensajeEstadoEntrada(uint8_t id, uint8_t estado);

/********************************************************/
/*                                                      */
/*  Devuelve el numero de entradas configuradas         */
/*                                                      */
/********************************************************/ 
int entradasConfiguradas(void);
/********************************************* Fin entradas *******************************************************************/
  
/****************************************** Funciones de estado ***************************************************************/
/***********************************************************/
/*                                                         */
/*   Devuelve el estado de las entradas en formato json    */
/*   devuelve un json con el formato:                      */
/* {
    "Entradas": [  
      {"id":  "0", "nombre": "P. abierta", "valor": "1" },
      {"id":  "1", "nombre": "P. cerrada", "valor": "0" }
    ]
  }
                                                           */
/***********************************************************/   
String generaJsonEstadoEntradas(boolean filtro);
String generaJsonEstadoEntradas(void);

#endif