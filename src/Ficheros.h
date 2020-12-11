/************************************************/
/*                                              */
/* Funciones para la gestion de ficheros en     */
/* memoria del modulo esp6288                   */
/*                                              */
/************************************************/

/***************************** Defines *****************************/
#ifndef _FICHEROS_
#define _FICHEROS_
/***************************** Defines *****************************/

/***************************** Includes *****************************/
#include <Global.h>
/***************************** Includes *****************************/

/************************************************/
/* Inicializa el sistema de ficheros del modulo */
/************************************************/
boolean inicializaFicheros(int debug);

/************************************************/
/* Recupera los datos de                        */
/* de un archivo cualquiera                     */
/************************************************/
boolean leeFichero(String nombre, String &contenido);

/**********************************************************************/
/* Salva la cadena pasada al fichero especificado                     */
/* Si ya existe lo sobreescribe                                       */
/**********************************************************************/  
boolean salvaFichero(String nombreFichero, String nombreFicheroBak, String contenidoFichero);

/**********************************************************************/
/* Salva la cadena pasada al fichero especificado                     */
/* Si ya existe añade                                                 */
/**********************************************************************/  
boolean anadeFichero(String nombreFichero, String contenidoFichero,int debug=0);
boolean anadeFichero(String nombreFichero, String contenidoFichero,int debug);

/****************************************************/
/* Borra el fichero especificado                    */
/****************************************************/  
boolean borraFichero(String nombreFichero);

/************************************************/
/* Recupera los ficheros almacenados en el      */
/* dispositivo. Devuelve un JSON                */
/************************************************/
String listadoFicheros(String prefix);

/************************************************/
/* Devuelve si existe o no un fichero en el     */
/* dispositivo                                  */
/************************************************/
boolean existeFichero(String nombre);

/************************************************/
/* Formatea el sistemas de ficheros del         */
/* dispositivo                                  */
/************************************************/
boolean formatearFS(void);

/************************************************/
/* Devuelve el nombre del direcotrio del        */
/* fichro que se pasa como parametro            */
/************************************************/
String directorioFichero(String nombreFichero);

#endif