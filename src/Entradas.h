/*****************************************/
/*                                       */
/*  Control de entradas                  */
/*                                       */
/*****************************************/

/***************************** Defines *****************************/
#ifndef _ENTRADAS_
#define _ENTRADAS_
/***************************** Defines *****************************/

/***************************** Includes *****************************/
#include <arduino.h>
#include <Global.h>

#include <entrada.h>
/***************************** Includes *****************************/

class Entradas{
  private:    
    Entrada* entrada;//definicion de los tipos de datos para las entradas
    uint8_t numeroEntradas;

    boolean recuperaDatos(int debug);// Lee el fichero de configuracion de las entradas o genera conf por defecto
    boolean parseaConfiguracion(String contenido);//* Parsea el json leido del fichero de configuracio de las entradas
    void configura(uint8_t id, String _nombre, String _tipo, int8_t _pin, int8_t _estadoActivo, String _nombres[2], String _mensajes[2]);

  public:
    //Constructor
    Entradas(void);

    //Configuracion
    void inicializa(void);// Inicializa los valores de los registros de las entradas y recupera la configuracion
    //get
    int getNumEntradas(void){return numeroEntradas;}//Devuelve el numero de entradas configuradas
    Entrada getEntrada(uint8_t id){return entrada[id];}

    //set
    void actualiza(bool debug);//Lee el estado de las entradas

    //estado en json
    String generaJsonEstado(boolean filtro);//Devuelve el estado de las entradas en formato json
    String generaJsonEstado(void);
};

extern Entradas entradas;
#endif