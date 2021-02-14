/*****************************************/
/*                                       */
/*  Control de salidas                   */
/*                                       */
/*****************************************/

/***************************** Defines *****************************/
#ifndef _SALIDA_
#define _SALIDA_

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

//modos de las salidas
#define MODO_MANUAL       0
#define MODO_SECUENCIADOR 1
#define MODO_SEGUIMIENTO  2
#define MODO_MAQUINA      3

//tipo de salidas
#define TIPO_DIGITAL      0
#define TIPO_PWM          1
#define TIPO_LED          2

//Valores por defecto para PWM
#define RESOLUCION_PWM    10    //en bits
#define FRECUENCIA_PWM    1000  //en herzios


/***************************** Defines *****************************/

/***************************** Includes *****************************/
#include <arduino.h>
#include <Global.h>
/***************************** Includes *****************************/

class Salida{
    private:
        String nombre;            //nombre configurado para el rele
        int8_t tipo;              //Tipo dgital normal o LED(PWM)
        int8_t pin;               // Pin al que esta conectado el rele
        int8_t inicio;            // modo inicial del rele "on"-->1/"off"-->0
        int8_t modo;              //0: manual, 1: secuanciador, 2: seguimiento
        int8_t modo_inicial;      //Modo incial, cuando se fuerza a manual este no cambia y cuando vuelve, se recupera de aqui  

        int16_t anchoPulso;       // Ancho en milisegundos del pulso para esa salida
        int8_t controlador;       //1 si esta asociado a un secuenciador que controla la salida, 0 si no esta asociado

        int16_t valorPWM;         //Valor de la salida en PWM
        int8_t canal;             //Canal de PWM al que se va a a asociar el pin
        int16_t frecuencia;       //Frecuencia de trabajo del PWM para ese canal
        int8_t resolucion;        //Resolucion en bits del PWM  

        int8_t estado;            //1 activo, 0 no activo (respecto a nivelActivo), modo pulso y modo maquina
        unsigned long finPulso;   //fin en millis del pulso para la activacion de ancho definido

        String nombreEstados[2];  //Son salidas binarias, solo puede haber 2 mensajes. El 0 nombre del estado en valor 0 y el 1 nombre del estado en valor 1
        String mensajes[2];       //Son salidas binarias, solo puede haber 2 mensajes. El 0 cuando pasa a 0 y el 1 cuando pasa a 1

    public:
        //constructor
        Salida(void);

        //Configuracion
        void configura(String _nombre, int8_t _tipo, int8_t _pin, int8_t _inicio, int16_t _valorPWM, int16_t _anchoPulso, int8_t _modo, int8_t _canal, int16_t _frecuencia, int8_t _resolucion, int8_t _controlador, String _nombres[2], String _mensajes[2]);

        /******************************************************/
        /* Evalua el estado de cada salida y la actualiza     */
        /* segun el modo de operacion                         */
        /* estado=0 o 1 encendido o apagado segun nivelActivo */
        /* estado=2 modo secuenciador                         */
        /* estado=3 modo seguimiento de entrada (anchoPulso)  */
        /******************************************************/
        void actualiza(void);//void actualizaSalida(int8_t salida);

        //get configurar
        String getNombre(void);//Devuelve el nombre de la salida
        uint8_t getPin(void);//Devuelve el modo de la salida
        int8_t getTipo(void);//Devuelve el tipo de salida digital/PWM
        String getTipoNombre(void);//Devuelve el nombre del tipo de salida digital/PWM
        int8_t getControlador(void);//Devuelve el controlador de la salida si esta asociada a un plan de secuenciador
        int8_t getEstadoInicial(void);//Devuelve el estado incial de la salida
        int8_t getCanal(void);
        int8_t getFrecuencia(void);
        int16_t getResolucion(void);

        //get consultar
        uint16_t getAnchoPulso(void);//uint16_t anchoPulsoSalida(uint8_t id); //   Devuelve el ancho del pulso de la salida
        unsigned long getFinPulso(void); //unsigned long finPulsoSalida(uint8_t id); //   Devuelve el fin del pulso de la salida

        int8_t getEstado(void);//Devuelve el estado de la salida, puede ser 0 apagado, 1 encendido, 2 pulsando
        String getNombreEstado(uint8_t estado);//Devuelve el nombre del estado de una salida
        String getNombreEstadoActual();//Devuelve el nombre del estado actual de una salida
        String getMensajeEstado(uint8_t estado);//Devuelve el mensaje de una salida en un estado
        String getMensajeEstadoActual(void);//Devuelve el mensaje del estado actual una salida

        uint8_t getModo(void);//Devuelve el modo actualde la salida
        String getModoNombre(void);//Devuelve el modo de la salida
        uint8_t getModoInical(void);//Devuelve el modo inicial de la salida

        int16_t getValorPWM(void);//Devuelve el valor de PWM, si la salida es de ese tipo

        //set
        int8_t setValorPWM(int16_t valor);//establece el valor de la salida PWM

        int8_t conmuta(int8_t estado_final);//conmuta la salida, devuelve 1 si ok, -1 si ko. LA ENTRADA ES EL ESTADO LOGICO. ADAPTO EL ESTADO FISICO SEGUN nivelActivo
        int8_t setPulso();// Genera un pulso en rele indicado en id devuelve 1 si ok, -1 si ko
        int8_t setSalida(int8_t estado);//Recubre las dos funciones anteriores para actuar sobre un rele
        int8_t actualizaPulso(void);
        int8_t salidaMaquinaEstados(int8_t estado);//Genera un pulso en rele indicado en id devuelve 1 si ok, -1 si ko
        void setModoManual(void);//Fuerza el modo manual en una salida que esta en otro modo
        void setModoInicial(void);//Fuerza el modo manual en una salida que esta en otro modo

        //configura
        void asociarSecuenciador(int8_t plan);//Asocia la salida a un plan de secuenciador
};

#endif