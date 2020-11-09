/*********************************************************/
/*                                                       */
/*                                                       */
/*   Gestion de notificaciones a traves de Google Home   */
/*                                                       */
/*                                                       */
/*********************************************************/
/***************************** Defines *****************************/
#ifndef _GHN_
#define _GHN_
/***************************** Defines *****************************/

/***************************** Includes *****************************/
#include <Global.h>
/***************************** Includes *****************************/

/***************************************************/
/*                                                 */
/* Inicializa la comunicacion con los dispositivos */
/* google Home de la casa. Los descubre a traves   */
/* del nombre por mDNS                             */
/*                                                 */
/***************************************************/
void inicializaGHN(void); 

/***************************************************/
/*                                                 */
/* Envia la peticion a google para reproducir el   */
/* mensaje deseado en el google Home alque se      */
/* conecto                                         */
/*                                                 */
/***************************************************/
boolean enviaNotificacion(char *mensaje);

/***************************************************/
/*    Acceso a datos                               */
/***************************************************/
uint8_t getActivaGoogleHomeNotifier(void);
String getNombreEquipo(void);
String getIdioma(void);
#endif