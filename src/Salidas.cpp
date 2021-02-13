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
Salidas salidas;
/***************************** Fin variables locales *****************************/

/************************************** Constructor ****************************************/
Salidas::Salidas(void){
  numeroSalidas=0;
  salidaActiva=-1;
}
/************************************** Fin constructor ****************************************/

/************************************** Funciones de configuracion ****************************************/
/*********************************************/
/* Inicializa los valores de los registros de*/
/* las salidas y recupera la configuracion   */
/*********************************************/
void Salidas::inicializa(void){  
  //leo la configuracion del fichero
  if(!recuperaDatos(debugGlobal)) Traza.mensaje("Configuracion de los reles por defecto\n");
  else{  
    //Salidas
    for(int8_t i=0;i<salidas.getNumSalidas();i++){    
      //parte logica        
      salida[i].conmuta(salida[i].getEstadoInicial());//salida[i].estado=salida[i].inicio;  
      
      //parte fisica
      switch (salida[i].getTipo()){
        case TIPO_DIGITAL:
          pinMode(salida[i].getPin(), OUTPUT); //es salida
          break;  
        case TIPO_LED:
          ledcSetup(salida[i].getCanal(),salida[i].getFrecuencia(),salida[i].getResolucion());
          ledcAttachPin(salida[i].getPin(),salida[i].getCanal());
          break;
      }

      //Lo inicializo segun lo configurado               
      if(salida[i].getEstadoInicial()==ESTADO_ACTIVO){
        switch (salida[i].getTipo()){
          case TIPO_DIGITAL:
            digitalWrite(salida[i].getPin(), nivelActivo);  
            break;
          case TIPO_LED:
            if(nivelActivo==1) ledcWrite(salida[i].getCanal(),salida[i].getValorPWM());
            else ledcWrite(salida[i].getCanal(),(1<<RESOLUCION_PWM)-salida[i].getValorPWM());
            break;
        }
      }
      else{
        switch (salida[i].getTipo()){
          case TIPO_DIGITAL:
            digitalWrite(salida[i].getPin(), !nivelActivo); 
            break;
          case TIPO_LED:
            if(nivelActivo==1) ledcWrite(salida[i].getCanal(),0);
            else ledcWrite(salida[i].getCanal(),(1<<RESOLUCION_PWM));            
            break;
        }
      }
      
      Traza.mensaje("Nombre salida[%i]=%s | pin salida: %i | tipo: %i | estado= %i | estado inicial: %i | modo: %i\n",i,salida[i].getNombre().c_str(),salida[i].getPin(),salida[i].getTipo(),salida[i].getEstado(),salida[i].getEstadoInicial(), salida[i].getModo());
      Traza.mensaje("\tEstados y  mensajes:\n");
      for(uint8_t j=0;j<2;j++) Traza.mensaje("\t\tEstado %i: %s | mensaje: %s\n",j,salida[i].getNombreEstado(j).c_str(),salida[i].getMensajeEstado(j).c_str());
    }
  }
}

/*********************************************/
/* Lee el fichero de configuracion de las    */
/* salidas o genera conf por defecto         */
/*********************************************/
boolean Salidas::recuperaDatos(int debug){
  String cad="";

  if (debug) Traza.mensaje("Recupero configuracion de archivo...\n");
  
  if(!leeFichero(SALIDAS_CONFIG_FILE, cad)){
    //Confgiguracion por defecto
    Traza.mensaje("No existe fichero de configuracion de Salidas\n");    
    cad="{\"Salidas\": []}";
    //salvo la config por defecto
    //if(salvaFichero(SALIDAS_CONFIG_FILE, SALIDAS_CONFIG_BAK_FILE, cad)) Traza.mensaje("Fichero de configuracion de Salidas creado por defecto\n");
  }
    
  return parseaConfiguracion(cad);
}

/*********************************************/
/* Parsea el json leido del fichero de       */
/* configuracio de las salidas               */
/*********************************************/
boolean Salidas::parseaConfiguracion(String contenido){ 
  DynamicJsonBuffer jsonBuffer;
  JsonObject& json = jsonBuffer.parseObject(contenido.c_str());
  
  String cad;
  json.printTo(cad);//pinto el json que he creado
  Traza.mensaje("json creado:\n#%s#\n",cad.c_str());
  
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
  
  numeroSalidas=(Salidas.size()<MAX_SALIDAS?Salidas.size():MAX_SALIDAS);
  Traza.mensaje("Se configurarán %i salidas\n",numeroSalidas);

  salidas.salida=new Salida[numeroSalidas];

  for(int8_t i=0;i<numeroSalidas;i++){ 
    JsonObject& _salida = json["Salidas"][i];

    if(_salida.containsKey("nombre")) nombre=_salida.get<String>("nombre");
    if(_salida.containsKey("tipo")) tipo=_salida.get<int>("tipo");
    if(_salida.containsKey("GPIO")) pin=_salida.get<int>("GPIO"); else return false;
    if(_salida.containsKey("inicio")){
      if(_salida.get<String>("inicio")=="on") inicio=1; //Si de inicio debe estar activado o desactivado
      else inicio=0;
    }
    if(_salida.containsKey("modo")) modo=_salida.get<int>("modo");
    if(_salida.containsKey("anchoPulso")) anchoPulso=_salida.get<int>("anchoPulso");
    if(_salida.containsKey("controlador")) controlador=_salida.get<int>("controlador");
    if(_salida.containsKey("valorPWM")) valorPWM=_salida.get<int>("valorPWM");
    if(_salida.containsKey("canal")) canal=_salida.get<int>("canal");
    if(_salida.containsKey("frecuencia")) frecuencia=_salida.get<int>("frecuencia");
    if(_salida.containsKey("resolucion")) resolucion=_salida.get<int>("resolucion");   
   
    if(_salida.containsKey("Estados")){
      int8_t est_max=_salida["Estados"].size();//maximo de mensajes en el JSON
      if (est_max>2) est_max=2;               //Si hay mas de 2; solo leo 2
      for(int8_t e=0;e<est_max;e++){
        if (_salida["Estados"][e]["valor"]==e) nombres[e]=String((const char *)_salida["Estados"][e]["texto"]);
      }
    }
    if(_salida.containsKey("Mensajes")){
      int8_t men_max=_salida["Mensajes"].size();//maximo de mensajes en el JSON
      if (men_max>2) men_max=2;                //Si hay mas de 2, solo leo 2
      for(int8_t m=0;m<men_max;m++){
        if (_salida["Mensajes"][m]["valor"]==m) mensajes[m]=String((const char *)_salida["Mensajes"][m]["texto"]);
      }
    }
    Traza.mensaje("Salida %i: nombre: %s, tipo: %i; pin: %i, inicio: %i, valorPWM: %i, anchoPulso: %i; modo: %i, canal: %i, frecuencia: %i, resolucion: %i, controlador: %i, nombre[0]: %s, nombre[1]: %s, mensaje[0]: %s, mensaje[1]: %s\n",i,nombre, tipo, pin, inicio, valorPWM, anchoPulso, modo, canal, frecuencia, resolucion, controlador, nombres[0], nombres[1], mensajes[0], mensajes[1]);
    configura(i,nombre, tipo, pin, inicio, valorPWM, anchoPulso, modo, canal, frecuencia, resolucion, controlador, nombres, mensajes);
  }

  if(debugGlobal || true) {
    Traza.mensaje("*************************\nSalidas:\n"); 
    for(int8_t i=0;i<numeroSalidas;i++){
      Traza.mensaje("%01i: %s |  pin: %i | tipo: %i | modo: %i | controlador: %i | ancho del pulso: %i\n",i,salida[i].getNombre().c_str(),salida[i].getPin(),salida[i].getTipo(), salida[i].getModo(),salida[i].getControlador(), salida[i].getAnchoPulso()); 
      Traza.mensaje("Estados:\n");
      for(int8_t e=0;e<2;e++){Traza.mensaje("Estado[%02i]: %s\n",e,salida[i].getNombreEstado(e).c_str());}
      Traza.mensaje("Mensajes:\n");
      for(int8_t m=0;m<2;m++) {Traza.mensaje("Mensaje[%02i]: %s\n",m,salida[i].getMensajeEstado(m).c_str());}    
    }
    Traza.mensaje("*************************\n");  
  }
//************************************************************************************************
  return true; 
}

void Salidas::configura(uint8_t id, String _nombre, int8_t _tipo, int8_t _pin, int8_t _inicio, int16_t _valorPWM, int16_t _anchoPulso, int8_t _modo, int8_t _canal, int16_t _frecuencia, int8_t _resolucion, int8_t _controlador, String _nombres[2], String _mensajes[2]){
  salida[id].configuraSalida(_nombre, _tipo, _pin, _inicio, _valorPWM, _anchoPulso, _modo, _canal, _frecuencia, _resolucion, _controlador, _nombres, _mensajes);
}

/**********************************************************Fin configuracion******************************************************************/  

/**********************************************************SALIDAS******************************************************************/    
/*************************************************/
/*Logica de las salidas:                         */
/*Si esta activo para ese intervalo de tiempo(1) */
/*Si esta por debajo de la tMin cierro rele      */
/*si no abro rele                                */
/*************************************************/
void Salidas::actualiza(bool debug){
  for(int8_t id=0;id<numeroSalidas;id++) salida[id].actualiza();
}

int8_t Salidas::conmuta(uint8_t id, int8_t estado_final){
  return salida[id].conmuta(estado_final);
}

int8_t Salidas::setValorPWM(uint8_t id, int16_t valor){
  return salida[id].setValorPWM(valor);
}

int8_t Salidas::setPulso(uint8_t id){
  return salida[id].setPulso();
}
int8_t Salidas::setSalida(uint8_t id, int8_t estado){
  return salida[id].setSalida(estado);
}
int8_t Salidas::salidaMaquinaEstados(uint8_t id, int8_t estado){
  return salida[id].salidaMaquinaEstados(estado);
}
void Salidas::setModoManual(uint8_t id){
  return salida[id].setModoManual();
}
void Salidas::setModoInicial(uint8_t id){
  return salida[id].setModoInicial();
}

/********************************************************/
/*                                                      */
/*     Devuelve el numero de reles configurados         */
/*                                                      */
/********************************************************/ 
int Salidas::salidasConfiguradas(void){return numeroSalidas;}

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
String Salidas::generaJsonEstado(boolean filtro){
  String cad="";

  //const size_t bufferSize = JSON_ARRAY_SIZE(2) + JSON_OBJECT_SIZE(1) + 2*JSON_OBJECT_SIZE(8);
  const size_t bufferSize = JSON_ARRAY_SIZE(1) + JSON_OBJECT_SIZE(1) + JSON_OBJECT_SIZE(15);

  DynamicJsonBuffer jsonBuffer(bufferSize);
  
  JsonObject& root = jsonBuffer.createObject();
  
  JsonArray& Salidas = root.createNestedArray("salidas");
  for(int8_t id=0;id<numeroSalidas;id++){

    JsonObject& Salidas_0 = Salidas.createNestedObject();
    Salidas_0["id"] = id;
    Salidas_0["nombre"] = salida[id].getNombre();
    Salidas_0["pin"] = salida[id].getPin();
    Salidas_0["modo"] = salida[id].getModoNombre();
    Salidas_0["controlador"] = salida[id].getControlador();
    Salidas_0["estado"] = salida[id].getEstado();
    Salidas_0["nombreEstado"] = salida[id].getNombreEstadoActual();
    Salidas_0["anchoPulso"] = salida[id].getAnchoPulso();
    Salidas_0["finPulso"] = salida[id].getFinPulso();

    if(!filtro){
      Salidas_0["tipo"] = salida[id].getTipoNombre();
      Salidas_0["valorPWM"] = salida[id].getValorPWM();
      Salidas_0["mensajeEstado"] = salida[id].getMensajeEstadoActual();
      Salidas_0["inicio"] = salida[id].getEstadoInicial();
    }
  }
    
  root.printTo(cad);
  return cad;  
}  
String Salidas::generaJsonEstado(void) {return generaJsonEstado(true);}

int8_t Salidas::setSalidaActiva(int8_t id){
  if(id <0 || id>=salidas.getNumSalidas()) return NO_CONFIGURADO;
  
  salidaActiva=id;

  return CONFIGURADO;
}