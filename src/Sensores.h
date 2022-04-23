/*****************************************/
/*                                       */
/*        Sistema de Sensores             */
/*                                       */
/*****************************************/

/***************************** Defines *****************************/
#ifndef _SENSORES_
#define _SENSORES_

//Tipos de sensores
#define TIPO_NULO                      0 //"NULO"
#define TIPO_DS18B20                   1 //"DS18B20"  //Temperatura
#define TIPO_HDC1080                   2 //"HDC1080"  //Temperatura, Humedad
#define TIPO_DHT22                     3 //"DHT22"    //Temperatura, Humedad
#define TIPO_BME280                    4 //"BME280"   //Temperatura, Humedad y presion
#define TIPO_BMP280                    5 //"BMP280"   //Temperatura y presion
#define TIPO_GL5539                    6 //"GL5539"   //Luz
#define TIPO_BH1750                    7 //"BH1750"   //Luz
#define TIPO_SOILMOISTURECAPACITIVEV2  8 //"SOILMOISTUREV2" //Humedad del suelo 

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

