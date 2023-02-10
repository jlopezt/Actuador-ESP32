/*****************************************/
/*                                       */
/*        Sistema de Sensores            */
/*                                       */
/*****************************************/

/***************************** Defines *****************************/
#ifndef _SENSORES_
#define _SENSORES_

#define MAX_SENSORES   10
/***************************** Defines *****************************/

/***************************** Includes *****************************/
#include <arduino.h>
#include <Sensor.h>
#include <Global.h>
/***************************** Includes *****************************/
class Sensores{
    private:
        //valores de configuracion
        Sensor *pCabeza,*pSensor;
        uint8_t numeroSensores;
            
        boolean recuperaDatos(boolean debug);// Lee el fichero de configuracion o genera conf por defecto
        boolean parseaConfiguracion(String contenido);//* Parsea el json leido del fichero de configuracion        

        uint8_t tipoSensor(String tipo);


    public:
        //Constructor
        Sensores(void);

        //Configuracion
        void inicializa(void);// Inicializa los valores de los registros de los sensores y recupera la configuracion

        //get
        int getNumSensores(void){return numeroSensores;}//Devuelve el numero de sensores configurados
        Sensor* getSensor(String nombre);

        //set
        void lee(boolean debug=false);//Lee el estado de los sensores

        void scannerI2C(void);
        void scannerOneWire(void);

        //estado en json
        String generaJsonEstado(boolean debug=false);
        String generaJsonConfiguracion(boolean debug=false);
};

extern Sensores sensores;
#endif

