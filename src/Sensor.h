/*****************************************/
/*                                       */
/*        Sistema de Sensores            */
/*                                       */
/*****************************************/

/***************************** Defines *****************************/
#ifndef _SENSOR_
#define _SENSOR_

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
        virtual uint8_t compararHumbral(void){Serial.printf("Clase padre\n"); return 0;};
};
/***********************************FIN SENSOR*********************************************/

/***********************************DS18B20*********************************************/
class SensorDS18B20 : public Sensor{
    private:
        static DallasTemperature* ds18B20;
        uint8_t direccion[8];
        float temperatura;
        int8_t comparaHumbralTemperatura; //-1 significa comparar si el valor es inferior al humbral. 0 no comparar. 1 comparar si es superior al humbral
        float humbralTemperatura;

        //void convierteDireccion(String s,uint8_t* direccion);
        boolean parseaConfiguracion(String contenido);
        
    public:
        SensorDS18B20();
        void inicializa(String _nombre, uint8_t _tipo, String parametros);
        uint8_t* getDireccion(void){return direccion;};
        String getDirecciontoString(void);
        void setHumbralTemperatura(uint8_t _humbral){humbralTemperatura=_humbral;};
        float getHumbralTemperatura(void){return humbralTemperatura;};

        String generaJsonEstado(JsonObject& medida);
        String generaJsonConfiguracion(JsonObject& config);
        void lee(void);        
        uint8_t compararHumbral(void);
};   
/***********************************FIN DS18B20*********************************************/

/***********************************DHT*********************************************/
class SensorDHT : public Sensor{
    private:
        DHT* dht;
        uint8_t pin;

        float temperatura;
        float humedad;
        int8_t comparaHumbralTemperatura; //-1 significa comparar si el valor es inferior al humbral. 0 no comparar. 1 comparar si es superior al humbral
        int8_t comparaHumbralHumedad; //-1 significa comparar si el valor es inferior al humbral. 0 no comparar. 1 comparar si es superior al humbral
        float humbralTemperatura;
        float humbralHumedad;

        void setPin(uint8_t _pin){pin=_pin;}
        boolean parseaConfiguracion(String contenido);

    public:
        SensorDHT();
        void inicializa(String _nombre, uint8_t _tipo, String parametros);
        void setHumbralTemperatura(uint8_t _humbral){humbralTemperatura=_humbral;};
        void setHumbralHumedad(uint8_t _humbral){humbralHumedad=_humbral;};
        float getHumbralTemperatura(void){return humbralTemperatura;};
        float getHumbralHumedad(void){return humbralHumedad;};
                                
        String generaJsonEstado(JsonObject& medida);
        String generaJsonConfiguracion(JsonObject& config);
        void lee(void);     
        uint8_t compararHumbral(void);
};
/***********************************FIN DHT*********************************************/

/***********************************HDC1080*********************************************/
class SensorHDC1080 : public Sensor{
    private:
        ClosedCube_HDC1080* hdc1080;
        
        uint8_t direccionI2C;

        float temperatura;
        float humedad;
        int8_t comparaHumbralTemperatura; //-1 significa comparar si el valor es inferior al humbral. 0 no comparar. 1 comparar si es superior al humbral
        int8_t comparaHumbralHumedad; //-1 significa comparar si el valor es inferior al humbral. 0 no comparar. 1 comparar si es superior al humbral
        float humbralTemperatura;
        float humbralHumedad;        

        //uint8_t convierteDireccion(String s);
        boolean parseaConfiguracion(String contenido);
        void setDireccionI2C(uint8_t _direccionI2C){direccionI2C=_direccionI2C;};

    public:
        SensorHDC1080();
        void inicializa(String _nombre, uint8_t _tipo, String parametros);
        void setHumbralTemperatura(uint8_t _humbral){humbralTemperatura=_humbral;};
        void setHumbralHumedad(uint8_t _humbral){humbralHumedad=_humbral;};
        float getHumbralTemperatura(void){return humbralTemperatura;};
        float getHumbralHumedad(void){return humbralHumedad;};

        String generaJsonEstado(JsonObject& medida);
        String generaJsonConfiguracion(JsonObject& config);
        void lee(void);     
        uint8_t compararHumbral(void);
};
/***********************************FIN HDC1080*********************************************/

/***********************************BMP280*********************************************/
class SensorBMP280 : public Sensor{
    private:
        Adafruit_BMP280* bmp280; // I2C Tº y presion
        
        uint8_t direccionI2C;

        float temperatura;
        float presion;
        int8_t comparaHumbralTemperatura; //-1 significa comparar si el valor es inferior al humbral. 0 no comparar. 1 comparar si es superior al humbral
        int8_t comparaHumbralPresion; //-1 significa comparar si el valor es inferior al humbral. 0 no comparar. 1 comparar si es superior al humbral
        float humbralTemperatura;
        float humbralPresion;        

        boolean parseaConfiguracion(String contenido);
        void setDireccionI2C(uint8_t _direccionI2C){direccionI2C=_direccionI2C;};

    public:
        SensorBMP280();
        void inicializa(String _nombre, uint8_t _tipo, String parametros);
        void setHumbralTemperatura(uint8_t _humbral){humbralTemperatura=_humbral;};
        void setHumbralPresion(uint8_t _humbral){humbralPresion=_humbral;};
        float getHumbralTemperatura(void){return humbralTemperatura;};
        float getHumbralPresion(void){return humbralPresion;};


        String generaJsonEstado(JsonObject& medida);
        String generaJsonConfiguracion(JsonObject& config);
        void lee(void);
        uint8_t compararHumbral(void);
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
        int8_t comparaHumbralTemperatura; //-1 significa comparar si el valor es inferior al humbral. 0 no comparar. 1 comparar si es superior al humbral
        int8_t comparaHumbralHumedad; //-1 significa comparar si el valor es inferior al humbral. 0 no comparar. 1 comparar si es superior al humbral
        int8_t comparaHumbralPresion; //-1 significa comparar si el valor es inferior al humbral. 0 no comparar. 1 comparar si es superior al humbral
        float humbralTemperatura;
        float humbralHumedad;
        float humbralPresion;        
        
        boolean parseaConfiguracion(String contenido);
        void setDireccionI2C(uint8_t _direccionI2C){direccionI2C=_direccionI2C;};

    public:
        SensorBME280();
        void inicializa(String _nombre, uint8_t _tipo, String parametros);
        void setHumbralTemperatura(uint8_t _humbral){humbralTemperatura=_humbral;};
        void setHumbralHumedad(uint8_t _humbral){humbralHumedad=_humbral;};
        void setHumbralPresion(uint8_t _humbral){humbralPresion=_humbral;};
        float getHumbralTemperatura(void){return humbralTemperatura;};
        float getHumbralHumedad(void){return humbralHumedad;};
        float getHumbralPresion(void){return humbralPresion;};

        String generaJsonEstado(JsonObject& medida);
        String generaJsonConfiguracion(JsonObject& config);
        void lee(void);     
        uint8_t compararHumbral(void);
};
/***********************************FIN HDC1080*********************************************/

/***********************************BH1750*********************************************/
class SensorBH1750 : public Sensor{
    private:
        BH1750* bh1750; //I2C direccion por defecto 0x23

        uint8_t direccionI2C;

        float luz;
        int8_t comparaHumbralLuz; //-1 significa comparar si el valor es inferior al humbral. 0 no comparar. 1 comparar si es superior al humbral
        float humbralLuz;

        boolean parseaConfiguracion(String contenido);
        void setDireccionI2C(uint8_t _direccionI2C){direccionI2C=_direccionI2C;};
        
    public:
        SensorBH1750();
        void inicializa(String _nombre, uint8_t _tipo, String parametros);
        void setHumbralLuz(uint8_t _humbral){humbralLuz=_humbral;};
        float getHumbralLuz(void){return humbralLuz;};

        String generaJsonEstado(JsonObject& medida);
        String generaJsonConfiguracion(JsonObject& config);
        void lee(void);     
        uint8_t compararHumbral(void);
};

/***********************************FIN BH1750*********************************************/

/***********************************GL5539*********************************************/
class SensorGL5539 : public Sensor{
    private:       
        float luz;
        int8_t comparaHumbralLuz; //-1 significa comparar si el valor es inferior al humbral. 0 no comparar. 1 comparar si es superior al humbral
        float humbralLuz;
        
        boolean parseaConfiguracion(String contenido);

    public:
        SensorGL5539();
        void inicializa(String _nombre, uint8_t _tipo, String parametros);
        void setHumbralLuz(uint8_t _humbral){humbralLuz=_humbral;};
        float getHumbralLuz(void){return humbralLuz;};

        String generaJsonEstado(JsonObject& medida);
        String generaJsonConfiguracion(JsonObject& config);
        void lee(void);     
        uint8_t compararHumbral(void);
};
/***********************************FIN GL5539*********************************************/

/***********************************HumedadSuelo*********************************************/
class SensorHumedadSuelo : public Sensor{
    private:       
        float humedad;
        int8_t comparaHumbralHumedad; //-1 significa comparar si el valor es inferior al humbral. 0 no comparar. 1 comparar si es superior al humbral
        float humbralHumedad;

        boolean parseaConfiguracion(String contenido);

    public:
        SensorHumedadSuelo();
        void inicializa(String _nombre, uint8_t _tipo, String parametros);
        void setHumbralHumedad(uint8_t _humbral){humbralHumedad=_humbral;};
        float getHumbralHumedad(void){return humbralHumedad;};

        String generaJsonEstado(JsonObject& medida);
        String generaJsonConfiguracion(JsonObject& config);
        void lee(void);     
        uint8_t compararHumbral(void);
};
/***********************************FIN HumedadSuelo*********************************************/
        
#endif