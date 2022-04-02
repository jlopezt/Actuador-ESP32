/*********************************************************************/
/*                                                                   */
/*Funciones de gestion de la actualizacion OTA                       */
/*                                                                   */
/*********************************************************************/

/***************************** Defines *****************************/
#ifndef _OTA_
#define _OTA_
/***************************** Defines *****************************/

/***************************** Includes *****************************/
#include <Arduino.h>
/***************************** Includes *****************************/

void gestionaOTA(void);
void inicializaOTA(boolean debug);

String listaParticiones(void);
String getParticionProximoUpdate(void);
String getParticionEjecucion(void);
String getParticionProximoArranque(void);
boolean setParticionProximoArranque(String nombre);

#endif