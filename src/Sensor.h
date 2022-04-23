/*****************************************/
/*                                       */
/*        Sistema de Sensores            */
/*                                       */
/*****************************************/

/***************************** Defines *****************************/
#ifndef _SENSOR_
#define _SENSOR_
/***************************** Defines *****************************/

/***************************** Includes *****************************/
#include <arduino.h>

#include <OneWire.h>
#include <DallasTemperature.h>
#include <DHT.h>
#include <ClosedCube_HDC1080.h>
#include <Adafruit_BME280.h>
#include <Adafruit_BMP280.h>
#include <Adafruit_Sensor.h>
#include <BH1750.h>

#include <Global.h>
/***************************** Includes *****************************/
uint8_t convierteDireccionI2C(String s);
void convierteDireccionOneWire(String s,uint8_t* direccion);

extern OneWire oneWire;

/***********************************SENSOR*********************************************/
class Sensor{
    private:
        //valores de configuracion
        String nombre;
        uint8_t tipoSensor;
        Sensor* siguiente;

    public:
        Sensor(void){siguiente=NULL;tipoSensor=TIPO_NULO;nombre="";};
        ~Sensor(){};
        virtual void inicializa(String _nombre, uint8_t _tipo, String parametros){Serial.printf("Clase padre\n");};
        //virtual boolean parseaConfiguracion(String contenido);

        void setTipo(uint8_t tipo){tipoSensor=tipo;};
        void setNombre(String _nombre) {nombre=_nombre;};
        void setSiguiente(Sensor* _siguiente){siguiente=_siguiente;};
        uint8_t getTipo(void){return tipoSensor;};
        String getNombre(void) {return nombre;};
        Sensor* getSiguiente(void){return siguiente;};

        virtual String generaJsonEstado(JsonObject& medida){return "";};
        virtual String generaJsonConfiguracion(JsonObject& config){return "";};
        virtual void lee(void){Serial.printf("Clase padre\n");};
};
/***********************************FIN SENSOR*********************************************/

/***********************************DS18B20*********************************************/
class SensorDS18B20 : public Sensor{
    private:
        static DallasTemperature* ds18B20;
        uint8_t direccion[8];
        float temperatura;

        //void convierteDireccion(String s,uint8_t* direccion);
        boolean parseaConfiguracion(String contenido);
        
    public:
        SensorDS18B20();
        void inicializa(String _nombre, uint8_t _tipo, String parametros);
        uint8_t* getDireccion(void){return direccion;};
        String getDirecciontoString(void);

        String generaJsonEstado(JsonObject& medida);
        String generaJsonConfiguracion(JsonObject& config);
        void lee(void);        
};   
/***********************************FIN DS18B20*********************************************/

/***********************************DHT*********************************************/
class SensorDHT : public Sensor{
    private:
        DHT* dht;
        uint8_t pin;

        float temperatura;
        float humedad;

        void setPin(uint8_t _pin){pin=_pin;}
        boolean parseaConfiguracion(String contenido);

    public:
        SensorDHT();
        void inicializa(String _nombre, uint8_t _tipo, String parametros);
        String generaJsonEstado(JsonObject& medida);
        String generaJsonConfiguracion(JsonObject& config);
        void lee(void);     
};
/***********************************FIN DHT*********************************************/

/***********************************HDC1080*********************************************/
class SensorHDC1080 : public Sensor{
    private:
        ClosedCube_HDC1080* hdc1080;
        
        uint8_t direccionI2C;

        float temperatura;
        float humedad;

        //uint8_t convierteDireccion(String s);
        boolean parseaConfiguracion(String contenido);
        void setDireccionI2C(uint8_t _direccionI2C){direccionI2C=_direccionI2C;};

    public:
        SensorHDC1080();
        void inicializa(String _nombre, uint8_t _tipo, String parametros);
        String generaJsonEstado(JsonObject& medida);
        String generaJsonConfiguracion(JsonObject& config);
        void lee(void);     
};
/***********************************FIN HDC1080*********************************************/

/***********************************BMP280*********************************************/
class SensorBMP280 : public Sensor{
    private:
        Adafruit_BMP280* bmp280; // I2C Tº y presion
        
        uint8_t direccionI2C;

        float temperatura;
        float presion;

        boolean parseaConfiguracion(String contenido);
        void setDireccionI2C(uint8_t _direccionI2C){direccionI2C=_direccionI2C;};

    public:
        SensorBMP280();
        void inicializa(String _nombre, uint8_t _tipo, String parametros);
        String generaJsonEstado(JsonObject& medida);
        String generaJsonConfiguracion(JsonObject& config);
        void lee(void);     
};
/***********************************FIN BMP280*********************************************/

/***********************************BME280*********************************************/
class SensorBME280 : public Sensor{
    private:
        Adafruit_BME280* bme280; // I2C Tº, humedad y presion
        
        uint8_t direccionI2C;

        float temperatura;
        float humedad;
        float presion;
        
        boolean parseaConfiguracion(String contenido);
        void setDireccionI2C(uint8_t _direccionI2C){direccionI2C=_direccionI2C;};

    public:
        SensorBME280();
        void inicializa(String _nombre, uint8_t _tipo, String parametros);
        String generaJsonEstado(JsonObject& medida);
        String generaJsonConfiguracion(JsonObject& config);
        void lee(void);     
};
/***********************************FIN HDC1080*********************************************/

/***********************************BH1750*********************************************/
class SensorBH1750 : public Sensor{
    private:
        BH1750* bh1750; //I2C direccion por defecto 0x23

        uint8_t direccionI2C;

        float luz;

        boolean parseaConfiguracion(String contenido);
        void setDireccionI2C(uint8_t _direccionI2C){direccionI2C=_direccionI2C;};
        
    public:
        SensorBH1750();
        void inicializa(String _nombre, uint8_t _tipo, String parametros);
        String generaJsonEstado(JsonObject& medida);
        String generaJsonConfiguracion(JsonObject& config);
        void lee(void);     
};

/***********************************FIN BH1750*********************************************/

/***********************************GL5539*********************************************/
class SensorGL5539 : public Sensor{
    private:       
        float luz;

        boolean parseaConfiguracion(String contenido);

    public:
        SensorGL5539();
        void inicializa(String _nombre, uint8_t _tipo, String parametros);
        String generaJsonEstado(JsonObject& medida);
        String generaJsonConfiguracion(JsonObject& config);
        void lee(void);     
};
/***********************************FIN GL5539*********************************************/

/***********************************HumedadSuelo*********************************************/
class SensorHumedadSuelo : public Sensor{
    private:       
        float humedad;

        boolean parseaConfiguracion(String contenido);

    public:
        SensorHumedadSuelo();
        void inicializa(String _nombre, uint8_t _tipo, String parametros);
        String generaJsonEstado(JsonObject& medida);
        String generaJsonConfiguracion(JsonObject& config);
        void lee(void);     
};
/***********************************FIN HumedadSuelo*********************************************/
        
#endif