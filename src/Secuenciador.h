/*****************************************/
/*                                       */
/*  Secuenciador de fases de entradas    */
/*                                       */
/*****************************************/

/***************************** Defines *****************************/
#ifndef _SECUENCIADOR_
#define _SECUENCIADOR_

#define HORAS_EN_DIA 24
/***************************** Defines *****************************/

/***************************** Includes *****************************/
#include <Arduino.h>
/***************************** Includes *****************************/

class Plan{
    private:
        int8_t id;
        String nombre;
        int8_t salidaAsociada;                     //salida a la que se asocia la secuencia
        int    horas[HORAS_EN_DIA];      //el valor es un campo de bit. los primeros 12 son los intervalos de 5 min de cada hora

    public:
        //contructor
        Plan(void);

        //get
        int8_t getSalida(void);//Devuelve el nuemro de salida asociada a un plan
        int getHoras(int8_t hora);
        String getNombre(void){return nombre;}
        int getEstado(uint8_t hora, uint8_t minuto);
        int getEstado(void);

        //set
        void configura(int8_t _id, String _nombre, int8_t _salida, int _horas[HORAS_EN_DIA]);
        void setNombre(String _nombre){nombre=_nombre;}

        //estado
        String pintaPlanHTML(void);//Genera codigo HTML para representar el plan        
};

class Secuenciador{
    private:
        Plan* planes;
        boolean activado; //plag para activar o desactivar el secuenciador
        uint8_t numeroPlanes;

        boolean recuperaDatos(boolean debug);
        boolean parseaConfiguracion(String contenido);

    public:
        //constructor
        Secuenciador(void);

        //configuracion
        void inicializa();// Lee la configuracion del SPIFFS y configura los planes del secuenciador 

        //get
        int planConfigurado(uint8_t id);//Devuelve si el plan esta configurados
        int8_t getNumPlanes(void);//Devuelve el nuemro de planes definido
        int8_t getSalida(uint8_t plan);
        boolean estado(void);//Devuelve el estado del secuenciador
        String getNombrePlan(uint8_t plan){return planes[plan].getNombre();}
        int getEstadoPlan(uint8_t plan, uint8_t hora, uint8_t minuto);
        int getEstadoPlan(uint8_t plan);

        //set
        void activar(void);//Activa el secuenciador
        void desactivar(void);//Desactiva el secuenciador        
        /*************************************************/
        /*Logica del secuenciador                        */
        /*Comporueba como debe estar en ese peridodo de  */
        /*cinco minutos parea esa hora y actualiza la    */
        /*salida correspondiente                         */
        /*************************************************/
        void actualiza(bool debug);

        //estado
        String generaJsonEstado(void);
        String pintaPlanHTML(uint8_t plan);//Genera codigo HTML para representar el plan
};

extern Secuenciador secuenciador;
#endif