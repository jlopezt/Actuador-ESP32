/*****************************************/
/*                                       */
/*  Control de salidas                   */
/*                                       */
/*****************************************/

/***************************** Defines *****************************/
/***************************** Defines *****************************/

/***************************** Includes *****************************/
#include <Salidas.h>
#include <salida.h>

#include <MQTT.h>
#include <Ficheros.h>
/***************************** Includes *****************************/

/***************************** Variables locales *****************************/
int8_t salidaActiva=-1;
//definicion de los tipos de dataos para las salidas
salida* salidas=new salida[MAX_SALIDAS];
/***************************** Fin variables locales *****************************/

/************************************** Funciones de configuracion ****************************************/
/*********************************************/
/* Inicializa los valores de los registros de*/
/* las salidas y recupera la configuracion   */
/*********************************************/
void inicializaSalidas(void){  
  //Salidas
  for(int8_t i=0;i<MAX_SALIDAS;i++) {salidas[i]=salida();}
         
  //leo la configuracion del fichero
  if(!recuperaDatosSalidas(debugGlobal)) Traza.mensaje("Configuracion de los reles por defecto\n");
  else{  
    //Salidas
    for(int8_t i=0;i<MAX_SALIDAS;i++){    
      if(salidas[i].getConfigurada()){   
        //parte logica        
        salidas[i].conmuta(salidas[i].getEstadoInicial());//salidas[i].estado=salidas[i].inicio;  
        
        //parte fisica
        switch (salidas[i].getTipo()){
          case TIPO_DIGITAL:
            pinMode(salidas[i].getPin(), OUTPUT); //es salida
            break;  
          case TIPO_LED:
            ledcSetup(salidas[i].getCanal(),salidas[i].getFrecuencia(),salidas[i].getResolucion());
            ledcAttachPin(salidas[i].getPin(),salidas[i].getCanal());
            break;
        }

        //Lo inicializo segun lo configurado               
        if(salidas[i].getEstadoInicial()==ESTADO_ACTIVO){
          switch (salidas[i].getTipo()){
            case TIPO_DIGITAL:
              digitalWrite(salidas[i].getPin(), nivelActivo);  
              break;
            case TIPO_LED:
              if(nivelActivo==1) ledcWrite(salidas[i].getCanal(),salidas[i].getValorPWM());
              else ledcWrite(salidas[i].getCanal(),(1<<RESOLUCION_PWM)-salidas[i].getValorPWM());
              break;
          }
        }
        else{
          switch (salidas[i].getTipo()){
            case TIPO_DIGITAL:
              digitalWrite(salidas[i].getPin(), !nivelActivo); 
              break;
            case TIPO_LED:
              if(nivelActivo==1) ledcWrite(salidas[i].getCanal(),0);
              else ledcWrite(salidas[i].getCanal(),(1<<RESOLUCION_PWM));            
              break;
          }
        }
        
        Traza.mensaje("Nombre salida[%i]=%s | pin salida: %i | tipo: %i | estado= %i | estado inicial: %i | modo: %i\n",i,salidas[i].getNombre().c_str(),salidas[i].getPin(),salidas[i].getTipo(),salidas[i].getEstado(),salidas[i].getEstadoInicial(), salidas[i].getModo());
        Traza.mensaje("\tEstados y  mensajes:\n");
        for(uint8_t j=0;j<2;j++) Traza.mensaje("\t\tEstado %i: %s | mensaje: %s\n",j,salidas[i].getNombreEstado(j).c_str(),salidas[i].getMensajeEstado(j).c_str());
      }
    }
  }  
}

/*********************************************/
/* Lee el fichero de configuracion de las    */
/* salidas o genera conf por defecto         */
/*********************************************/
boolean recuperaDatosSalidas(int debug){
  String cad="";

  if (debug) Traza.mensaje("Recupero configuracion de archivo...\n");
  
  if(!leeFichero(SALIDAS_CONFIG_FILE, cad)){
    //Confgiguracion por defecto
    Traza.mensaje("No existe fichero de configuracion de Salidas\n");    
    cad="{\"Salidas\": []}";
    //salvo la config por defecto
    //if(salvaFichero(SALIDAS_CONFIG_FILE, SALIDAS_CONFIG_BAK_FILE, cad)) Traza.mensaje("Fichero de configuracion de Salidas creado por defecto\n");
  }
    
  return parseaConfiguracionSalidas(cad);
}

/*********************************************/
/* Parsea el json leido del fichero de       */
/* configuracio de las salidas               */
/*********************************************/
boolean parseaConfiguracionSalidas(String contenido){ 
  DynamicJsonBuffer jsonBuffer;
  JsonObject& json = jsonBuffer.parseObject(contenido.c_str());
  
  String salida;
  json.printTo(salida);//pinto el json que he creado
  Traza.mensaje("json creado:\n#%s#\n",salida.c_str());
  
  if (!json.success()) return false;
        
  Traza.mensaje("\nparsed json\n");
//******************************Parte especifica del json a leer********************************
  JsonArray& Salidas = json["Salidas"];
  
  //variables teporales para almacenar los valores leidos del json
  String nombre="";
  int8_t tipo=-1;
  int8_t pin=-1;
  int8_t inicio=-1;
  int8_t modo=-1;

  int16_t anchoPulso=-1;
  int8_t controlador=-1;

  int16_t valorPWM=-1;
  int8_t canal=-1;
  int16_t frecuencia=-1;
  int8_t resolucion=-1;

  String nombres[2]={"",""};
  String mensajes[2]={"",""};
  
  int8_t max;
  max=(Salidas.size()<MAX_SALIDAS?Salidas.size():MAX_SALIDAS);
  Traza.mensaje("Se configurarán %i salidas\n",max);
  for(int8_t i=0;i<max;i++){ 
    JsonObject& salida = json["Salidas"][i];

    if(salida.containsKey("nombre")) nombre=salida.get<String>("nombre");
    if(salida.containsKey("tipo")) tipo=salida.get<int>("tipo");
    if(salida.containsKey("GPIO")) pin=salida.get<int>("GPIO"); else return false;
    if(salida.containsKey("inicio")){
      if(salida.get<String>("inicio")=="on") inicio=1; //Si de inicio debe estar activado o desactivado
      else inicio=0;
    }
    if(salida.containsKey("modo")) modo=salida.get<int>("modo");
    if(salida.containsKey("anchoPulso")) anchoPulso=salida.get<int>("anchoPulso");
    if(salida.containsKey("controlador")) controlador=salida.get<int>("controlador");
    if(salida.containsKey("valorPWM")) valorPWM=salida.get<int>("valorPWM");
    if(salida.containsKey("canal")) canal=salida.get<int>("canal");
    if(salida.containsKey("frecuencia")) frecuencia=salida.get<int>("frecuencia");
    if(salida.containsKey("resolucion")) resolucion=salida.get<int>("resolucion");   
   
    if(salida.containsKey("Estados")){
      int8_t est_max=salida["Estados"].size();//maximo de mensajes en el JSON
      if (est_max>2) est_max=2;               //Si hay mas de 2; solo leo 2
      for(int8_t e=0;e<est_max;e++){
        if (salida["Estados"][e]["valor"]==e) nombres[e]=String((const char *)salida["Estados"][e]["texto"]);
      }
    }
    if(salida.containsKey("Mensajes")){
      int8_t men_max=salida["Mensajes"].size();//maximo de mensajes en el JSON
      if (men_max>2) men_max=2;                //Si hay mas de 2, solo leo 2
      for(int8_t m=0;m<men_max;m++){
        if (salida["Mensajes"][m]["valor"]==m) mensajes[m]=String((const char *)salida["Mensajes"][m]["texto"]);
      }
    }
    salidas[i].configuraSalida(nombre, tipo, pin, inicio, valorPWM, anchoPulso, modo, canal, frecuencia, resolucion, controlador, nombres, mensajes);
  }

  if(debugGlobal) {
    Traza.mensaje("*************************\nSalidas:\n"); 
    for(int8_t i=0;i<MAX_SALIDAS;i++){
      Traza.mensaje("%01i: %s | configurado= %i | pin: %i | tipo: %i | modo: %i | controlador: %i | ancho del pulso: %i\n",i,salidas[i].getNombre().c_str(),salidas[i].getConfigurada(),salidas[i].getPin(),salidas[i].getTipo(), salidas[i].getModo(),salidas[i].getControlador(), salidas[i].getAnchoPulso()); 
      Traza.mensaje("Estados:\n");
      for(int8_t e=0;e<2;e++){Traza.mensaje("Estado[%02i]: %s\n",e,salidas[i].getNombreEstado(e).c_str());}
      Traza.mensaje("Mensajes:\n");
      for(int8_t m=0;m<2;m++) {Traza.mensaje("Mensaje[%02i]: %s\n",m,salidas[i].getMensajeEstado(m).c_str());}    
    }
    Traza.mensaje("*************************\n");  
  }
//************************************************************************************************
  return true; 
}
/**********************************************************Fin configuracion******************************************************************/  

/**********************************************************SALIDAS******************************************************************/    
/*************************************************/
/*Logica de las salidas:                         */
/*Si esta activo para ese intervalo de tiempo(1) */
/*Si esta por debajo de la tMin cierro rele      */
/*si no abro rele                                */
/*************************************************/
void actualizaSalidas(bool debug){
  for(int8_t id=0;id<MAX_SALIDAS;id++){if (salidas[id].getConfigurada()) salidas[id].actualiza();}
}

/********************************************************/
/*                                                      */
/*     Devuelve el numero de reles configurados         */
/*                                                      */
/********************************************************/ 
int salidasConfiguradas(void){
  int resultado=0;
  
  for(int8_t i=0;i<MAX_SALIDAS;i++){if(salidas[i].getConfigurada()) resultado++;}
  return resultado;
}

/********************************************************** Fin salidas ******************************************************************/  

/****************************************** Funciones de estado ***************************************************************/
/********************************************************/
/*                                                      */
/*   Devuelve el estado de los reles en formato json    */
/*   devuelve un json con el formato:                   */
/* {
    "Salidas": [  
      {"id":  "0", "nombre": "Pulsador", "valor": "1" },
      {"id":  "1", "nombre": "Auxiliar", "valor": "0" }
      ]
   }
                                                        */
/********************************************************/   
String generaJsonEstadoSalidas(boolean filtro){
  String salida="";

  //const size_t bufferSize = JSON_ARRAY_SIZE(2) + JSON_OBJECT_SIZE(1) + 2*JSON_OBJECT_SIZE(8);
  const size_t bufferSize = JSON_ARRAY_SIZE(1) + JSON_OBJECT_SIZE(1) + JSON_OBJECT_SIZE(15);

  DynamicJsonBuffer jsonBuffer(bufferSize);
  
  JsonObject& root = jsonBuffer.createObject();
  
  JsonArray& Salidas = root.createNestedArray("salidas");
  for(int8_t id=0;id<MAX_SALIDAS;id++){
    if(salidas[id].getConfigurada() || !filtro){       
      JsonObject& Salidas_0 = Salidas.createNestedObject();
      Salidas_0["id"] = id;
      Salidas_0["nombre"] = salidas[id].getNombre();
      Salidas_0["pin"] = salidas[id].getPin();
      Salidas_0["modo"] = salidas[id].getModoNombre();
      Salidas_0["controlador"] = salidas[id].getControlador();
      Salidas_0["estado"] = salidas[id].getEstado();
      Salidas_0["nombreEstado"] = salidas[id].getNombreEstadoActual();
      Salidas_0["anchoPulso"] = salidas[id].getAnchoPulso();
      Salidas_0["finPulso"] = salidas[id].getFinPulso();

      if(!filtro){
        Salidas_0["tipo"] = salidas[id].getTipoNombre();
        Salidas_0["valorPWM"] = salidas[id].getValorPWM();
        Salidas_0["mensajeEstado"] = salidas[id].getMensajeEstadoActual();
        Salidas_0["configurada"] = salidas[id].getConfigurada();
        Salidas_0["inicio"] = salidas[id].getEstadoInicial();
      }
    }
  }
    
  root.printTo(salida);
  return salida;  
}  
String generaJsonEstadoSalidas(void) {return generaJsonEstadoSalidas(true);}

int8_t setSalidaActiva(int8_t id){
  if(id <0 || id>=MAX_SALIDAS) return NO_CONFIGURADO;
  if(salidas[id].getConfigurada()) return -1; //El rele no esta configurado
  
  salidaActiva=id;

  return CONFIGURADO;
}
  
int8_t getSalidaActiva(void){return salidaActiva;}