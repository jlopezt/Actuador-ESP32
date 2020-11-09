/*******************************************/
/*                                         */
/*  Definicion de la maquina de estados    */
/*  configurable.                          */
/*                                         */
/*  Hay un mapeo de E/S del dispositivo y  */
/*  de la maquina de estados. La entrada 1 */
/*  de la maquina puede ser la 4 del dis-  */
/*  positivo. Igual con las salidas.       */
/*                                         */
/*  el estado 0 es el de error             */
/*  el estado 1 es el de inicio            */
/*  el resto configurables                 */
/*                                         */
/*******************************************/

/***************************** Defines *****************************/
#ifndef _MAQUINA_ESTADOS_
#define _MAQUINA_ESTADOS_
/***************************** Defines *****************************/

/***************************** Includes *****************************/
#include <Global.h>
/***************************** Includes *****************************/

/************************************** Funciones de configuracion ****************************************/
/*********************************************/
/* Inicializa los valores de la maquina      */
/* de estados                                */
/*********************************************/
void inicializaMaquinaEstados(void);

/************************************************/
/* Lee el fichero de configuracion de la        */
/* maquina de estados o genera conf por defecto */
/************************************************/
boolean recuperaDatosMaquinaEstados(int debug);

/*********************************************/
/* Parsea el json leido del fichero de       */
/* configuracio de la maquina de estados     */
/*********************************************/
boolean parseaConfiguracionMaqEstados(String contenido);
/**********************************************************Fin configuracion******************************************************************/  

/*********************************************************MAQUINA DE ESTADOS******************************************************************/    
/****************************************************/
/* Analiza el estado de la maquina y evoluciona     */
/* los estados y las salidas asociadas              */
/****************************************************/
void actualizaMaquinaEstados(int debug=false);
void actualizaMaquinaEstados(int debug);

/****************************************************/
/* busco en las transiciones a que estado debe      */
/* evolucionar la maquina                           */
/****************************************************/
uint8_t mueveMaquina(uint8_t estado, int8_t entradasActual[], boolean debug=false);
uint8_t mueveMaquina(uint8_t estado, int8_t entradasActual[], boolean debug);
  
/****************************************************/
/* Actualizo las salidas segun el estado actual     */
/****************************************************/
int8_t actualizaSalidasMaquinaEstados(uint8_t estado);

/****************************************************/
/* Funciones de consulta de dataos (encapsulan)     */
/****************************************************/
boolean setDebugMaquinaEstados(boolean debug);
boolean getDebugMAquinaEstados(void);

uint8_t getNumEstados(void);
uint8_t getNumTransiciones(void);
uint8_t getNumEntradasME(void);
uint8_t getNumSalidasME(void);

uint8_t getNumEntradaME(uint8_t entrada);

uint8_t getNumSalidaME(uint8_t salida);

uint8_t getEstadoInicialTransicion(int8_t transicion);
uint8_t getEstadoFinalTransicion(int8_t transicion);
int8_t getValorEntradaTransicion(int8_t transicion, int8_t entrada);

String getNombreEstado(uint8_t estado);
String getNombreEstadoActual(void);

uint8_t getMapeoEntradas(uint8_t id);
uint8_t getMapeoSalidas(uint8_t id); 
uint8_t getEntradasActual(uint8_t id);
uint8_t getIdEstados(uint8_t estado);
String getNombreEstados(uint8_t estado);
uint8_t getValorSalidaEstados(uint8_t estado, uint8_t salida);

#endif