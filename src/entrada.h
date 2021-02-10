/*****************************************/
/*                                       */
/*  Control de entradas                  */
/*                                       */
/*****************************************/

/***************************** Defines *****************************/
#ifndef _ENTRADA_
#define _ENTRADA_

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

/********************************************* entrada *******************************************************************/
class entrada{
    private:
        //valores de configuracion
        boolean configurada;      //Si la entrada esta configurada o no
        String nombre;            //Nombre de la entrada
        String tipo;              //Puede ser INPUT, INPUT_PULLUP, No valido!!-->INPUT_PULLDOWN
        int8_t pin;               //Pin fisico alque esta conectada
        int8_t estadoActivo;      //Valor logico de referencia, se lee de la configuracion
        String nombreEstados[2];  //Son entradas binarias, solo puede haber 2 mensajes. El 0 nombre del estado en valor 0 y el 1 nombre del estado en valor 1
        String mensajes[2];       //Son entradas binarias, solo puede haber 2 mensajes. El 0 cuando pasa a 0 y el 1 cuando pasa a 1
        //valores de estado
        int8_t estadoFisico;      //leido ESTADO_ACTIVO o ESTADO_DESACTIVO
        int8_t estado;            //Estado logico asignado despues de leer el valo fisico

    public:
        //constructor
        entrada(void);

        //Configuracion
        void configuraEntrada(String _nombre, String _tipo, int8_t _pin, int8_t _estadoActivo, String _nombres[2], String _mensajes[2]);

        //funciones GET
        boolean getConfigurada(void); //boolean entradaConfigurada(void); //Devuelve si la entrada esta configurada          
        String getNombre(void);//String nombreEntrada(void); //Devuelve el nombre del rele con el id especificado  
        String getTipo(void);//String tipoEntrada(void); //Devuelve el tipo de la entrada                   
        uint8_t getPin(void);//uint8_t pinEntrada(void); //Devuelve el pin de la entrada                    
        int8_t getEstadoActivo(void);//int8_t estadoActivoEntrada(void); //Devuelve el estado activo 0|1 de la entrada   
        String getNombreEstado(uint8_t estado);//String nombreEstadoEntrada(uint8_t estado); //Devuelve el nombre del estado de una entrada        
        String getMensajeEstado(uint8_t estado);//String mensajeEstadoEntrada(uint8_t estado); //Devuelve el mensaje de una entrada en un estado     

        int8_t getEstado(void);//int8_t estadoEntrada(void); //Devuelve el estado 0|1 del rele indicado en id 
        int8_t getEstadoFisico(void);//int8_t estadoFisicoEntrada(void); //Devuelve el estado 0|1 de la entrada tal cual se ha leido, sin cruzar con nada              

        //funciones SET
        void setEstadoFisico(void); //Almacena el estado 0|1 de la entrada tal cual se ha leido, sin cruzar con nada                

        //otras
        void enviaMensaje(int8_t estado);// //Envia un mensaje MQTT para que se publique un audio en un GHN                               

};
/********************************************* Fin entrada *******************************************************************/
#endif