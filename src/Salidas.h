/*****************************************/
/*                                       */
/*  Control de salidas                   */
/*                                       */
/*****************************************/

/***************************** Defines *****************************/
#ifndef _SALIDAS_
#define _SALIDAS_
/*
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
*/

/***************************** Defines *****************************/

/***************************** Includes *****************************/
#include <arduino.h>
#include <Global.h>

#include <salida.h>
/***************************** Includes *****************************/

class Salidas{
  private:
    Salida* salida;
    uint8_t numeroSalidas;
    int8_t salidaActiva;

    boolean recuperaDatos(int debug);//Lee el fichero de configuracion de las salidas o genera conf por defecto
    boolean parseaConfiguracion(String contenido);//Parsea el json leido del fichero de configuracio de las salidas
    void configura(uint8_t id, String _nombre, int8_t _tipo, int8_t _pin, int8_t _inicio, int16_t _valorPWM, int16_t _anchoPulso, int8_t _modo, int8_t _canal, int16_t _frecuencia, int8_t _resolucion, int8_t _controlador, String _nombres[2], String _mensajes[2]);

  public:
    //constructor
    Salidas(void);

    //configuracion
    void inicializa(void);//Inicializa los valores de los registros de las salidas y recupera la configuracion

    //get
    int getNumSalidas(void){return numeroSalidas;}//Devuelve el numero de entradas configuradas
    Salida getSalida(uint8_t id){return salida[id];}    
    int salidasConfiguradas(void);//Devuelve el numero de reles configurados
    int8_t getSalidaActiva(void){return salidaActiva;}

    //set
    /*************************************************/
    /*Logica de los reles:                           */
    /*Si esta activo para ese intervalo de tiempo(1) */
    /*Si esta por debajo de la tMin cierro rele      */
    /*si no abro rele                                */
    /*************************************************/
    void actualiza(bool debug);
    int8_t conmuta(uint8_t id, int8_t estado_final);
    int8_t setValorPWM(uint8_t id, int16_t valor);
    int8_t setPulso(uint8_t id); // int8_t pulsoSalida(int8_t id); // Genera un pulso en rele indicado en id devuelve 1 si ok, -1 si ko
    int8_t setSalida(uint8_t id, int8_t estado); //int8_t actuaSalida(int8_t id, int8_t estado); //Recubre las dos funciones anteriores para actuar sobre un rele
    int8_t salidaMaquinaEstados(uint8_t id, int8_t estado); //int8_t salidaMaquinaEstados(int8_t id, int8_t estado); //Genera un pulso en rele indicado en id devuelve 1 si ok, -1 si ko
    void setModoManual(uint8_t id); //int8_t forzarModoManualSalida(int8_t id); //Fuerza el modo manual en una salida que esta en otro modo
    void setModoInicial(uint8_t id); //int8_t recuperarModoSalida(int8_t id); //Fuerza el modo manual en una salida que esta en otro modo
    void asociarSecuenciador(uint8_t salidaAsociada, uint8_t plan);
    int8_t setSalidaActiva(int8_t id);

    //estado en json
    String generaJsonEstado(boolean filtro);//devuelve un json con el formato
    String generaJsonEstado(void);

};

extern Salidas salidas;

#endif