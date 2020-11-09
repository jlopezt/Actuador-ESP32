/*****************************************/
/*                                       */
/*  Control de entradas y salidas        */
/*                                       */
/*****************************************/

/***************************** Defines *****************************/
#ifndef _ENTRADAS_SALIDAS_
#define _ENTRADAS_SALIDAS_

/* Incluido en Global.h
//Definicion de pines
#define MAX_PINES         20 //numero de pines disponibles para entradas y salidas
#define MAX_ENTRADAS      10 //numero maximo de reles soportado
#define MAX_SALIDAS       MAX_PINES-MAX_ENTRADAS //numero maximo de salidas

#ifndef NO_CONFIGURADO 
#define NO_CONFIGURADO    -1
#endif

#ifndef CONFIGURADO 
#define CONFIGURADO       1
#endif
*/

//modos de las salidas
#define MODO_MANUAL       0
#define MODO_SECUENCIADOR 1
#define MODO_SEGUIMIENTO  2
#define MODO_MAQUINA      3

//estados de las salidas
#define ESTADO_DESACTIVO  0
#define ESTADO_ACTIVO     1
#define ESTADO_PULSO      2

//tipo de salidas
#define TIPO_DIGITAL      0
#define TIPO_PWM          1
#define TIPO_LED          1

//Valores por defecto para PWM
#define RESOLUCION_PWM    10    //en bits
#define FRECUENCIA_PWM    1000  //en herzios

#define TOPIC_MENSAJES    "mensajes"
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
/* Inicializa los valores de los registros de*/
/* las salidas y recupera la configuracion   */
/*********************************************/
void inicializaSalidas(void);

/*********************************************/
/* Lee el fichero de configuracion de las    */
/* entradas o genera conf por defecto        */
/*********************************************/
boolean recuperaDatosEntradas(int debug);

/*********************************************/
/* Lee el fichero de configuracion de las    */
/* salidas o genera conf por defecto         */
/*********************************************/
boolean recuperaDatosSalidas(int debug);

/*********************************************/
/* Parsea el json leido del fichero de       */
/* configuracio de las entradas              */
/*********************************************/
boolean parseaConfiguracionEntradas(String contenido);

/*********************************************/
/* Parsea el json leido del fichero de       */
/* configuracio de las salidas               */
/*********************************************/
boolean parseaConfiguracionSalidas(String contenido);
  
/******************************************************/
/* Evalua el estado de cada salida y la actualiza     */
/* segun el modo de operacion                         */
/* estado=0 o 1 encendido o apagado segun nivelActivo */
/* estado=2 modo secuenciador                         */
/* estado=3 modo seguimiento de entrada (anchoPulso)  */
/******************************************************/
void actualizaSalida(int8_t salida);

/*************************************************/
/*Logica de los reles:                           */
/*Si esta activo para ese intervalo de tiempo(1) */
/*Si esta por debajo de la tMin cierro rele      */
/*si no abro rele                                */
/*************************************************/
void actualizaSalidas(bool debug);

/*************************************************/
/*                                               */
/*  Devuelve el estado  del rele indicado en id  */
/*  puede ser 0 apagado, 1 encendido, 2 pulsando */
/*                                               */
/*************************************************/
int8_t estadoSalida(int8_t id);

/********************************************************/
/*                                                      */
/*  Devuelve el tipo de salida digital/PWM              */
/*                                                      */
/********************************************************/
int8_t getTipo(int8_t id);

/********************************************************/
/*                                                      */
/*  Devuelve el nombre del tipo de salida digital/PWM   */
/*                                                      */
/********************************************************/
String getTipoNombre(int8_t id);

/********************************************************/
/*                                                      */
/*  Devuelve el valor de PWM,                           */
/*  si la salida es de ese tipo                         */
/*                                                      */
/********************************************************/
int16_t getValorPWM(int8_t id);

/********************************************************/
/*                                                      */
/*  establece el valor de la salida PWM                 */
/*                                                      */
/********************************************************/
int8_t setValorPWM(int8_t id, int16_t valor); 

/********************************************************/
/*                                                      */
/*  Devuelve el nombre del rele con el id especificado  */
/*                                                      */
/********************************************************/
String nombreSalida(int8_t id);

/*************************************************/
/*conmuta el rele indicado en id                 */
/*devuelve 1 si ok, -1 si ko                     */
/* LA ENTRADA ES EL ESTADO LOGICO. ADAPTO EL     */
/*  ESTADO FISICO SEGUN nivelActivo              */
/*************************************************/
int8_t conmutaSalida(int8_t id, int8_t estado_final, int debug);

/****************************************************/
/*   Genera un pulso en rele indicado en id         */
/*   devuelve 1 si ok, -1 si ko                     */
/****************************************************/
int8_t pulsoSalida(int8_t id);

/********************************************************/
/*                                                      */
/*     Recubre las dos funciones anteriores para        */
/*     actuar sobre un rele                             */
/*                                                      */
/********************************************************/ 
int8_t actuaSalida(int8_t id, int8_t estado);

/****************************************************/
/*   Genera un pulso en rele indicado en id         */
/*   devuelve 1 si ok, -1 si ko                     */
/****************************************************/
int8_t salidaMaquinaEstados(int8_t id, int8_t estado);

/********************************************************/
/*                                                      */
/*     Devuelve si el reles esta configurados           */
/*                                                      */
/********************************************************/ 
int releConfigurado(uint8_t id);
  
/********************************************************/
/*                                                      */
/*     Devuelve el numero de reles configurados         */
/*                                                      */
/********************************************************/ 
int relesConfigurados(void);

/********************************************************/
/*                                                      */
/*     Asocia la salida a un plan de secuenciador       */
/*                                                      */
/********************************************************/ 
void asociarSecuenciador(int8_t id, int8_t plan);

/********************************************************/
/*                                                      */
/*     Fuerza el modo manual en una salida que esta en  */
/*     en otro modo                                     */
/*                                                      */
/********************************************************/ 
int8_t forzarModoManualSalida(int8_t id);

/********************************************************/
/*                                                      */
/*     Fuerza el modo manual en una salida que esta en  */
/*     en otro modo                                     */
/*                                                      */
/********************************************************/ 
int8_t recuperarModoSalida(int8_t id);

/********************************************************/
/*                                                      */
/*     Devuelve el estado incial de la salida           */
/*                                                      */
/********************************************************/ 
int8_t inicioSalida(int8_t id);

/********************************************************/
/*                                                      */
/*     Devuelve el nombre de la salida                  */
/*                                                      */
/********************************************************/ 
String nombreSalida(uint8_t id);

/********************************************************/
/*                                                      */
/*     Devuelve el modo de la salida                    */
/*                                                      */
/********************************************************/ 
uint8_t pinSalida(uint8_t id);

/********************************************************/
/*                                                      */
/*     Devuelve el ancho del pulso de la salida         */
/*                                                      */
/********************************************************/ 
uint16_t anchoPulsoSalida(uint8_t id);

/********************************************************/
/*                                                      */
/*     Devuelve el fin del pulso de la salida           */
/*                                                      */
/********************************************************/ 
unsigned long finPulsoSalida(uint8_t id);

/********************************************************/
/*                                                      */
/*     Devuelve el valor de inicio de la salida         */
/*                                                      */
/********************************************************/ 
int8_t inicioSalida(uint8_t id);

/********************************************************/
/*                                                      */
/*  Devuelve el nombre del estado de una salida         */
/*                                                      */
/********************************************************/ 
String nombreEstadoSalida(uint8_t id, uint8_t estado);

/********************************************************/
/*                                                      */
/*  Devuelve el nombre del estado actual de una salida  */
/*                                                      */
/********************************************************/ 
String nombreEstadoSalidaActual(uint8_t id);

/********************************************************/
/*                                                      */
/*  Devuelve el mensaje de una salida en un estado      */
/*                                                      */
/********************************************************/ 
String mensajeEstadoSalida(uint8_t id, uint8_t estado);

/********************************************************/
/*                                                      */
/*  Devuelve el mensaje del estado actual una salida    */
/*                                                      */
/********************************************************/ 
String mensajeEstadoSalidaActual(uint8_t id);

/********************************************************/
/*                                                      */
/*     Devuelve el controlador de la salida si esta     */
/*     asociada a un plan de secuenciador               */
/*                                                      */
/********************************************************/ 
int8_t controladorSalida(int8_t id);

/********************************************************/
/*                                                      */
/*     Devuelve el modo de la salida                    */
/*                                                      */
/********************************************************/ 
uint8_t modoSalida(uint8_t id);

/********************************************************/
/*                                                      */
/*     Devuelve el modo de la salida                    */
/*                                                      */
/********************************************************/ 
String modoSalidaNombre(uint8_t id);

/********************************************************/
/*                                                      */
/*     Devuelve el modo inicial de la salida            */
/*                                                      */
/********************************************************/ 
uint8_t modoInicalSalida(uint8_t id);

/********************************************************** Fin salidas ******************************************************************/  
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
/********************************************************/
/*                                                      */
/*   Devuelve el estado de los reles en formato json    */
/*   devuelve un json con el formato:                   */
/* {
    "Salidas": [  
      {"id":  "0", "nombre": "Pulsador", "valor": "1" },
      {"id":  "1", "nombre": "Auxiliar", "valor": "0" }
      ]
   }
                                                        */
/********************************************************/   
String generaJsonEstadoSalidas(void);

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
String generaJsonEstadoEntradas(void);

/********************************************************/
/*                                                      */
/*   Devuelve el estado de los reles en formato json    */
/*   devuelve un json con el formato:                   */
/* {
  "Entradas": [ 
    {"id":  "0", "nombre": "P. abierta", "valor": "1" },
    {"id":  "1", "nombre": "P. cerrada", "valor": "0" }
  ],
  "Salidas": [  
    {"id":  "0", "nombre": "P. abierta", "valor": "1" },
    {"id":  "1", "nombre": "P. cerrada", "valor": "0" }
  ]
}
                                                        */
/********************************************************/   
String generaJsonEstado(void);

int8_t setSalidaActiva(int8_t id);
int8_t getSalidaActiva(void);

#endif