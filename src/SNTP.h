/***********************************************************
 *    Modulo de gestion de SNTP
 * ESP32:
 *    https://github.com/espressif/arduino-esp32/issues/680
 * ESP8266
 *    https://github.com/gmag11/NtpClient 
 ***********************************************************/

/***************************** Defines *****************************/
#ifndef _SNTP_
#define _SNTP_
/***************************** Defines *****************************/

/***************************** Includes *****************************/
#include <Arduino.h>
/***************************** Includes *****************************/

/****************************************************/
/*                                                  */
/*    Inicializa el modulo de SNTP                  */ 
/*                                                  */
/****************************************************/
void inicializaReloj(void);
  
/*******************************************************************/
/* Cambio horario:                                                 */
/* En España, esta medida se lleva adoptando desde 1974, aunque    */
/* la última regulación a la que nos hemos adaptado ha llegado     */
/* de la mano de la directiva Europea 2000/84, que entre otras     */
/* cosas unifica los días en los que se producen los cambios de    */
/* hora en todos los países de la Unión Europea, siendo estos      */
/* el último Domingo de Marzo y Octubre, respectivamente           */
/*                                                                 */
/* La funcion devuelve las horas que hay que sumar al reloj oficial*/
/*******************************************************************/
int8_t cambioHorario(void); 
int8_t diaSemana(void);

/***********************************/
/*        Solo para ordenes        */
/***********************************/
void imprimeDatosReloj(void);

/***************************************************************/
/*                                                             */
/*  Retorna si es por la tarde en formato booleano             */
/*                                                             */
/***************************************************************/
bool relojPM(); 

/***************************************************************/
/*                                                             */
/*  Devuleve la hora del sistema                               */
/*                                                             */
/***************************************************************/
int hora(); 

/***************************************************************/
/*                                                             */
/*  Devuleve el minuto del sistema                             */
/*                                                             */
/***************************************************************/
int minuto(); 

/***************************************************************/
/*                                                             */
/*  Devuleve el segundo del sistema                            */
/*                                                             */
/***************************************************************/
int segundo(); 
  
/***************************************************************/
/*                                                             */
/*  Devuleve el dia del sistema                                */
/*                                                             */
/***************************************************************/
int dia(); 

/***************************************************************/
/*                                                             */
/*  Devuleve el mes del sistema                                */
/*                                                             */
/***************************************************************/
int mes(); 

/***************************************************************/
/*                                                             */
/*  Devuleve el año del sistema                                */
/*                                                             */
/***************************************************************/
int anno(); 
  
/***************************************************************/
/*                                                             */
/*  Genera una cadena con la hora en formato HH:MM             */
/*                                                             */
/***************************************************************/
String getHora(void);

/***************************************************************/
/*                                                             */
/*  Genera un acadena con la fecha en formato DD/MM/AAAA       */
/*                                                             */
/***************************************************************/
String getFecha(void);

/***************************************************************/
/*                                                             */
/*  Genera una cadena con la hora en formato dd-mm-yy HH:MM:SS */
/*  a partir de la estrucutura time_t que se le pasa           */
/*                                                             */
/***************************************************************/
String horaYfecha(time_t hora);

#endif