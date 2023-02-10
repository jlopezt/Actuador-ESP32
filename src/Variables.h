/*****************************************/
/*                                       */
/*        Sistema de Variables            */
/*                                       */
/*****************************************/

/***************************** Defines *****************************/
#ifndef _VARIABLE_
#define _VARIABLE_

//Tipos de variables
#define TIPO_NULO                   0 //"NULO"
#define TIPO_TEMPERATURA            1 //Temperatura
#define TIPO_HUMEDAD                2 //Humedad
#define TIPO_PRESION                3 //Presion
#define TIPO_LUZ                    4 //Luz

//Tipos de humbrales
#define TIPO_VALOR_ES_INFERIOR -1
#define TIPO_VALOR_ES_SUPERIOR 1
/***************************** Defines *****************************/

/***************************** Includes *****************************/
#include <arduino.h>

#include <Global.h>
#include <sensor.h>
/***************************** Includes *****************************/

/***********************************Humbral*********************************************/
class Humbral{
    private:
        String nombre;
        int8_t tipo;
        float valor;
        String mensaje;
        Humbral* siguiente;

    public:
        Humbral(){siguiente=NULL;};
        Humbral(JsonObject& _config);

        void setTipo(uint8_t tipo){tipo=tipo;};
        void setNombre(String _nombre) {nombre=_nombre;};
        void setValor(float _valor) {valor=_valor;};
        void setMensaje(String _mensaje) {mensaje=_mensaje;}
        void setSiguiente(Humbral* _siguiente){siguiente=_siguiente;};

        float getValor(void) {return valor;};
        int8_t getTipo(void){return tipo;};
        String getNombre(void) {return nombre;};
        String getMensaje(void) {return mensaje;};
        Humbral* getSiguiente(void){return siguiente;};

        String generaJsonEstado(JsonObject& root);
        String generaJsonConfiguracion(JsonObject& root){return generaJsonEstado(root);};
};
/***********************************FIN VARIABLE******************************************/


/***********************************VARIABLE*********************************************/
class Variable{
    private:
        String nombre;
        String unidades;
        String descripcion;
        Sensor* sensor;
        uint8_t tipo;
        float valor;
        Humbral* pHumbral;
        uint8_t numeroHumbrales;
        Variable* siguiente;


    public:     
        Variable(void);
        Variable(JsonObject& _config);

        void setNombre(String _nombre) {nombre=_nombre;};
        void setUnidades(String _unidades){unidades=_unidades;};
        void setDescripcion(String _descripcion) {descripcion=_descripcion;};
        void setSensor(Sensor* _sensor) {sensor=_sensor;};
        void setTipo(String _tipo){tipo=convierteTipo(_tipo);};
        void setValor(float _valor){valor=_valor;};
        void setHumbral(Humbral* _humbral){pHumbral=_humbral;};
        void setSiguiente(Variable* _siguiente){siguiente=_siguiente;};
        
        String getNombre(void) {return nombre;};
        String getDescripcion(void) {return descripcion;};
        String getUnidades(void) {return unidades;};
        Sensor* getSensor(void){return sensor;};
        uint8_t getTipo(void){return tipo;};
        float getValor(void) {return valor;};
        Humbral* getHumbral(void){return pHumbral;}
        Variable* getSiguiente(void){return siguiente;};
        void comparaHumbrales(void);

        uint8_t convierteTipo(String _tipo){
            if(_tipo=="TEMPERATURA") return TIPO_TEMPERATURA;
            if(_tipo=="HUMEDAD") return TIPO_HUMEDAD;
            if(_tipo=="PRESION") return TIPO_PRESION;
            if(_tipo=="LUZ") return TIPO_LUZ;

            return TIPO_NULO;
        };

        String generaJsonEstado(JsonObject& medida);
        String generaJsonConfiguracion(JsonObject& medida){return generaJsonEstado(medida);};
};
/***********************************FIN VARIABLE******************************************/

/***********************************VARIABLES*********************************************/
class Variables{
    private:
        //valores de configuracion
        Variable *pCabeza,*pVariable;
        uint8_t numeroVariables;
            
        boolean recuperaDatos(boolean debug);// Lee el fichero de configuracion o genera conf por defecto
        boolean parseaConfiguracion(String contenido);//* Parsea el json leido del fichero de configuracion 
        void comparaHumbrales(void);

    public:
        //Constructor
        Variables(void);

        //Configuracion
        void inicializa(void);// Inicializa los valores de los registros de los sensores y recupera la configuracion

        //get
        int getNumVariables(void){return numeroVariables;}//Devuelve el numero de sensores configurados

        //set
        void lee(boolean debug=false);//Lee el estado de los sensores

        //estado en json
        String generaJsonEstado(boolean debug=false);
        String generaJsonConfiguracion(boolean debug=false){return generaJsonEstado();};
};

extern Variables variables;
/***********************************FIN VARIABLEs******************************************/
#endif