/*****************************************/
/*                                       */
/*  Control de entrada                   */
/*                                       */
/*****************************************/

/***************************** Defines *****************************/
/***************************** Defines *****************************/

/***************************** Includes *****************************/
#include <entrada.h>

#include <MQTT.h>
#include <Ficheros.h>
/***************************** Includes *****************************/

/***************************** Contructores ****************************/
Entrada::Entrada(void){    
    //inicializo la parte logica
    nombre="No configurado";
    estado=NO_CONFIGURADO;   
    estadoFisico=ESTADO_DESACTIVO; 
    estadoActivo=ESTADO_ACTIVO;  //NO_CONFIGURADO;
    tipo="INPUT";
    pin=-1;
    nombreEstados[0]="0";
    nombreEstados[1]="1";
    mensajes[0]="";
    mensajes[1]="";
}
/***************************** Fin contructores ****************************/

/***************************** Configuracion ****************************/
void Entrada::configura(String _nombre, String _tipo, int8_t _pin, int8_t _estadoActivo, String _nombres[2], String _mensajes[2]){    
    //inicializo la parte logica
    nombre=_nombre;
    estado=CONFIGURADO;   
    estadoFisico=ESTADO_DESACTIVO; 
    estadoActivo=_estadoActivo;
    tipo=_tipo;
    pin=_pin;
    nombreEstados[0]=_nombres[0];
    nombreEstados[1]=_nombres[1];
    mensajes[0]=_mensajes[0];
    mensajes[1]=_mensajes[1];
}
/***************************** Fin configuracion ****************************/

/****************************** get estado *****************************/
int8_t Entrada::getEstado(void){return (estado);}//Devuelve el estado 0|1 del rele indicado en id 
int8_t Entrada::getEstadoFisico(void){return estadoFisico;}//Devuelve el estado 0|1 de la entrada tal cual se ha leido, sin cruzar con nada
/****************************** Fin get estado *****************************/

/****************************** set estado *****************************/
void Entrada::setEstado(void){
    int8_t valor_inicial = estado;

    estadoFisico=digitalRead(pin);
    if(estadoFisico==estadoActivo) estado=ESTADO_ACTIVO;
    else estado=ESTADO_DESACTIVO;
    //Traza.mensaje("Entrada %s en pin %i, valor fisico leido %i, valor logico %i\n",nombre.c_str(),pin,estadoFisico,estado);

    if(valor_inicial!=NO_CONFIGURADO && valor_inicial!=estado) enviaMensaje(estado);      
}
/****************************** Fin set estado *****************************/        

/****************************** get configuracion *****************************/
String Entrada::getNombre(void) {return nombre;} //Devuelve el nombre del rele con el id especificado
uint8_t Entrada::getPin(void) {return pin;}   //Devuelve el pin de la entrada
String Entrada::getTipo(void) {return tipo;}//Devuelve el tipo de la entrada
int8_t Entrada::getEstadoActivo(void){return estadoActivo;}//Devuelve el estado activo 0|1 de la entrada
/********************************************************/
/*  Devuelve el nombre del estado de una entrada        */
/********************************************************/ 
String Entrada::getNombreEstado(uint8_t estado){
  //validaciones previas
  if(estado>2) return "ERROR";
       
  return nombreEstados[estado];
}   

/********************************************************/
/*  Devuelve el mensaje de una entrada en un estado     */
/********************************************************/ 
String Entrada::getMensajeEstado(uint8_t estado){
  //validaciones previas
  if(estado>2) return "ERROR";
       
  return mensajes[estado];
}
/****************************** Fin get configuracion *****************************/

/****************************** Otras *****************************/
/***************************************************************/
/*Envia un mensaje MQTT para que se publique un audio en un GHN*/
/***************************************************************/
void Entrada::enviaMensaje(int8_t estado){
  String mensaje="";

  mensaje="{\"origen\": \"" + nombre + "\",\"mensaje\":\"" + mensajes[estado] + "\"}";
  Traza.mensaje("Envia mensaje para la entrada %s y por cambiar a estado %i. Mensaje: %s\n\n",nombre.c_str(),estado,mensajes[estado].c_str());
  Traza.mensaje("A enviar: topic %s\nmensaje %s\n", TOPIC_MENSAJES,mensaje.c_str());
  enviarMQTT(TOPIC_MENSAJES, mensaje);
}
/****************************** Fin otras *****************************/