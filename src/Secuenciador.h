/*****************************************/
/*                                       */
/*  Secuenciador de fases de entradas    */
/*                                       */
/*****************************************/

/***************************** Defines *****************************/
#ifndef _SECUENCIADOR_
#define _SECUENCIADOR_
/***************************** Defines *****************************/

/***************************** Includes *****************************/
#include <Arduino.h>
/***************************** Includes *****************************/

/************************************** Funciones de configuracion ****************************************/
/*********************************************/
/* Lee la configuracion del SPIFFS y         */
/* configura los planes del secuenciador     */
/*********************************************/
void inicializaSecuenciador();
/**********************************************************Fin configuracion******************************************************************/  

/**********************************************************SALIDAS******************************************************************/    
/*************************************************/
/*Logica del secuenciador                        */
/*Comporueba como debe estar en ese peridodo de  */
/*cinco minutos parea esa hora y actualiza el    */
/*rele correspondiente                           */
/*************************************************/
void actualizaSecuenciador(bool debug);

/**************************************************/
/*                                                */
/* Devuelve el nuemro de planes definido          */
/*                                                */
/**************************************************/
int8_t getNumPlanes();

/**************************************************/
/*                                                */
/* Devuelve el nuemro de salida asociada a un plan*/
/*                                                */
/**************************************************/
int8_t getSalidaPlan(uint8_t plan);

/********************************************************/
/*                                                      */
/*     Devuelve si el plan esta configurados            */
/*                                                      */
/********************************************************/ 
int planConfigurado(uint8_t id);

/********************************************************/
/*                                                      */
/*             Activa el secuenciador                   */
/*                                                      */
/********************************************************/ 
void activarSecuenciador(void);

/********************************************************/
/*                                                      */
/*             Desactiva el secuenciador                */
/*                                                      */
/********************************************************/ 
void desactivarSecuenciador(void);
  
/********************************************************/
/*                                                      */
/*     Devuelve el estado del secuenciador              */
/*                                                      */
/********************************************************/ 
boolean estadoSecuenciador(void);

/********************************************************/
/*                                                      */
/*     Genera codigo HTML para representar el plan      */
/*                                                      */
/********************************************************/ 
String pintaPlanHTML(int8_t plan);

#endif