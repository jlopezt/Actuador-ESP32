/**********************************************/
/*                                            */
/*  Servidor FTP para el intercambio de       */
/*  ficheros                                  */
/*                                            */
/*  Utiliza libreria externa                  */
/*                                            */
/**********************************************/

/***************************** Defines *****************************/
#ifndef _FTP_
#define _FTP_
/***************************** Defines *****************************/

/***************************** Includes *****************************/
#include <Global.h>
#include <FtpServer.h>
/***************************** Includes *****************************/

/************************************** Funciones de configuracion ****************************************/
/*********************************************/
/* Inicializa el servidor FTP, recupera los  */
/* valores del fichero deconfiguracion       */
/*********************************************/
boolean inicializaFTP(boolean debug);

/*********************************************/
/* Recupera los datos del fichero de         */
/* configuracion                             */
/*********************************************/
boolean recuperaDatosFTP(boolean debug);

/*********************************************/
/* Parsea el json leido del fichero de       */
/* configuracio de los reles                 */
/*********************************************/
boolean parseaConfiguracionFTP(String contenido);

/**********************************************************Fin configuracion******************************************************************/  
/*********************************************/
/* La funcion que gestiona el FTP.           */
/* La que se llam desde el loop              */
/*********************************************/
void gestionaFTP(void);

extern FtpServer ftpSrv;
#endif