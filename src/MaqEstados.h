/*******************************************/
/*                                         */
/*  Definicion de la maquina de estados    */
/*  configurable.                          */
/*                                         */
/*  Hay un mapeo de E/S del dispositivo y  */
/*  de la maquina de estados. La entrada 1 */
/*  de la maquina puede ser la 4 del dis-  */
/*  positivo. Igual con las salidas.       */
/*                                         */
/*  el estado 0 es el de error             */
/*  el estado 1 es el de inicio            */
/*  el resto configurables                 */
/*                                         */
/*******************************************/

/***************************** Defines *****************************/
#ifndef _MAQUINA_ESTADOS_
#define _MAQUINA_ESTADOS_

#define MAX_ESTADOS      10
#define MAX_TRANSICIONES 50//3*MAX_ESTADOS
#define ESTADO_ERROR      0 //Estado de error de la logica de la maquina. Imposible evolucionar del ese estado con las estradas actuales
#define ESTADO_INICIAL    1 //Estado inicio
#define ESTADO_NEUTRO_ME -1 //Estado neutro en la comparacion de las entradas y la definicion de estados
/***************************** Defines *****************************/

/***************************** Includes *****************************/
#include <Global.h>
/***************************** Includes *****************************/

class Estado{
    private:
        uint8_t id;
        String nombre;
        int8_t valorSalida[MAX_SALIDAS];
    public:
        //constructor
        Estado(void);
        //set
        void setId(uint8_t _id);
        void setNombre(String _nombre);
        void setValorSalida(uint8_t _salida, int8_t _valor);

        //get
        uint8_t getId(void); //uint8_t getIdEstados(uint8_t estado);
        String getNombre(void); //String getNombreEstado(uint8_t estado);
        int8_t getValorSalida(uint8_t salida); //uint8_t getValorSalidaEstados(uint8_t estado, uint8_t salida);
};

class Transicion{
    private:        
        uint8_t estadoInicial;
        uint8_t estadoFinal;
        int8_t valorEntrada[MAX_ENTRADAS];//Puede ser -1, significa que no importa el valor
    public:
        //constructor
        Transicion(void);
        //set
        void setEstadoInicial(uint8_t estado);
        void setEstadoFinal(uint8_t estado);
        void setValorEntrada(uint8_t entrada, int8_t valor);

        //get
        uint8_t getEstadoInicial(void);//uint8_t getEstadoInicialTransicion(int8_t transicion);
        uint8_t getEstadoFinal(void);//uint8_t getEstadoFinalTransicion(int8_t transicion);
        int8_t getValorEntrada(int8_t entrada);//int8_t getValorEntradaTransicion(int8_t transicion, int8_t entrada);
};

class MaquinaEstados{
    private:
        uint8_t numeroEntradas;
        uint8_t mapeoEntradas[MAX_ENTRADAS]; //posicion es la entrada de la maquina de estados, el valor es el id de la entrada del dispositivo

        uint8_t numeroSalidas;
        uint8_t mapeoSalidas[MAX_SALIDAS]; //posicion es la salida de la maquina de estados, el valor es el id de la salida del dispositivo

        uint8_t numeroEstados;
        uint8_t numeroTransiciones; //numero de lazos de la maquina de estados. A cada transicion se asocia un estado inicial, unos valores de las entradas y un estado final

        uint8_t estadoActual;
        int8_t entradasActual[MAX_ENTRADAS]; //VAlor leido de las entradas
        int8_t salidasActual[MAX_SALIDAS];
        boolean debugMaquinaEstados;

        boolean recuperaDatos(int debug);// Lee el fichero de configuracion de la maquina de estados o genera conf por defecto
        boolean parseaConfiguracion(String contenido);// Parsea el json leido del fichero de configuracio de la maquina de estados

    public:
        Estado* estados;
        Transicion* transiciones;

        //contructor
        MaquinaEstados(void);
        /***************** Funciones de configuracion *****************/
        void inicializa(void);// Inicializa los valores de la maquina de estados
        /***************** Fin funciones de configuracion *****************/
        /***************** Funciones de ejecucion *****************/
        void actualiza(int debug); // Analiza el estado de la maquina y evoluciona los estados y las salidas asociadas
        void actualiza(void);
        uint8_t mueveMaquina(uint8_t estado, int8_t entradasActual[], boolean debug);//busco en las transiciones a que estado debe evolucionar la maquina
        uint8_t mueveMaquina(uint8_t estado, int8_t entradasActual[]);
        int8_t actualizaSalidasMaquinaEstados(uint8_t estado);// Actualizo las salidas segun el estado actual
        /***************** Funciones de ejecucion *****************/
        /***************** Funciones de consulta de datos *****************/
        String getNombreEstadoActual(void);

        uint8_t getNumEstados(void);
        uint8_t getNumTransiciones(void);
        uint8_t getNumEntradas(void);
        uint8_t getNumSalidas(void);

        uint8_t getMapeoEntrada(uint8_t id);
        uint8_t getMapeoSalida(uint8_t id); 
        uint8_t getEntradasActual(uint8_t id);

        void setDebugMaquinaEstados(boolean debug);
        boolean getDebugMAquinaEstados(void);

        String generaJsonEstado(void);
        /***************** Fin funciones de consulta de datos *****************/
};

extern MaquinaEstados maquinaEstados;

#endif