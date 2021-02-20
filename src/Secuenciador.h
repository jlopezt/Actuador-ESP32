/*****************************************/
/*                                       */
/*  Secuenciador de fases de entradas    */
/*                                       */
/*****************************************/

/***************************** Defines *****************************/
#ifndef _SECUENCIADOR_
#define _SECUENCIADOR_

#define HORAS_EN_DIA 24
#define MAX_INTERVALOS_EN_HORA 60
/***************************** Defines *****************************/

/***************************** Includes *****************************/
#include <Arduino.h>
/***************************** Includes *****************************/

class Plan{
    private:
        int8_t id;
        String nombre;
        int8_t salidaAsociada;        //salida a la que se asocia la secuencia
        uint32_t intervalos[MAX_INTERVALOS_EN_HORA]; //el valor es un campo de bit. los primeros MAX_INTERVALOS_EN_HORA son los intervalos de cada hora

    public:
        //contructor
        Plan(void);

        //get
        int8_t getSalida(void);//Devuelve el nuemro de salida asociada a un plan
        uint32_t getIntervalo(int8_t intervalo);
        String getNombre(void){return nombre;}
        int getEstado(uint8_t hora, uint8_t minuto);
        int getEstado(void);

        //set
        void configura(int8_t _id, String _nombre, int8_t _salida, uint32_t _intervalos[MAX_INTERVALOS_EN_HORA]);
        void setNombre(String _nombre){nombre=_nombre;}

        //estado
        String pintaPlanHTML(void);//Genera codigo HTML para representar el plan        
};

class Secuenciador{
    private:
        Plan* planes;
        boolean activado; //plag para activar o desactivar el secuenciador
        uint8_t numeroPlanes;
        uint8_t numeroIntervalos;  //numero de intervalos en los que se divide una hora. Es igual al numero maximo de intervalos definidos en el fichero (<= MAX_INTERVALOS_EN_HORA)
        uint16_t anchoIntervalo;

        boolean recuperaDatos(boolean debug);
        boolean parseaConfiguracion(String contenido);

    public:
        //constructor
        Secuenciador(void);

        //configuracion
        void inicializa();// Lee la configuracion del SPIFFS y configura los planes del secuenciador 

        //get
        int8_t getNumPlanes(void);//Devuelve el nuemro de planes definido
        uint8_t getNumeroIntervalos(void);
        uint16_t getAnchoIntervalo(void);
        int8_t getSalida(uint8_t plan);
        boolean getEstado(void);//Devuelve el estado del secuenciador
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