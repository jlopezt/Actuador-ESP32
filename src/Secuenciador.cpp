/*****************************************/
/*                                       */
/*  Secuenciador de fases de entradas    */
/*                                       */
/*****************************************/

/***************************** Defines *****************************/
#define MAX_PLANES 5
/***************************** Defines *****************************/

/***************************** Includes *****************************/
#include <Global.h>
#include <Secuenciador.h>
#include <Entradas.h>
#include <Salidas.h>
#include <SNTP.h>
#include <Ficheros.h>
/***************************** Includes *****************************/

Secuenciador secuenciador;

/************************* Constructor ***********************/
Secuenciador::Secuenciador(void){
  activado=false;
  numeroPlanes=0;
}
/************************* Fin constructor ***********************/

/************************************** Funciones de configuracion ****************************************/
void Secuenciador::inicializa(){
  //leo la configuracion del fichero
  if(!recuperaDatos(debugGlobal)) Traza.mensaje("Configuracion del secuenciador por defecto\n");
  else{ 
  }
}

boolean Secuenciador::recuperaDatos(boolean debug){
  String cad="";

  if (debug) Traza.mensaje("Recupero configuracion de archivo...\n");
  
  if(!leeFichero(SECUENCIADOR_CONFIG_FILE, cad)){
    //Confgiguracion por defecto
    Traza.mensaje("No existe fichero de configuracion del secuenciador\n");
    cad="{\"estadoInicial\": 0,\"Planes\":[]}";
    //if(salvaFichero(SECUENCIADOR_CONFIG_FILE, SECUENCIADOR_CONFIG_BAK_FILE, cad)) Traza.mensaje("Fichero de configuracion del secuenciador creado por defecto\n");
  }
    
  return parseaConfiguracion(cad);
}

/*********************************************/
/* Parsea el json leido del fichero de       */
/* configuracio de los reles                 */
/*********************************************/
boolean Secuenciador::parseaConfiguracion(String contenido){  
  DynamicJsonBuffer jsonBuffer;
  JsonObject& json = jsonBuffer.parseObject(contenido.c_str());
  
  String cad;
  json.printTo(cad);//pinto el json que he leido
  Traza.mensaje("json creado:\n#%s#\n",cad.c_str());
  
  if (!json.success()) return false;
        
  Traza.mensaje("\nparsed json\n");
//******************************Parte especifica del json a leer********************************  
  if(json.containsKey("estadoInicial")) activado=json.get<int>("estadoInicial");
  
  JsonArray& Planes = json["Planes"];  

  numeroPlanes=(Planes.size()<MAX_PLANES?Planes.size():MAX_PLANES);
  Traza.mensaje("Se configuraran %i planes\n", numeroPlanes);

  secuenciador.planes = new Plan[numeroPlanes];

  for(int8_t i=0;i<numeroPlanes;i++){ 
    int8_t _id;
    String _nombre="";
    int8_t _salidaAsociada=-1;        //salida a la que se asocia la secuencia
    uint32_t _intervalos[INTERVALOS_EN_HORA];      //el valor es un campo de bit. los primeros INTERVALOS_EN_HORA son los intervalos de cada hora
    for(uint8_t i=0;i<INTERVALOS_EN_HORA;i++) _intervalos[i]=0;

    JsonObject& _plan = Planes[i];//json["Planes"][i];
    //leo los valores del json
    _id=i;
    if(_plan.containsKey("nombre")) _nombre=_plan.get<String>("nombre");
    else {
      Traza.mensaje("Plan %i, no contiene campo nombre\n",_id);
      numeroPlanes=0;//Esto aborta la configuracion de los planes!!!!!!!
      return false;
    }

    if(_plan.containsKey("salida")) _salidaAsociada=_plan.get<int>("salida");
    else {
      Traza.mensaje("Plan %i, no contiene campo salida\n",_id);
      numeroPlanes=0;//Esto aborta la configuracion de los planes!!!!!!!
      return false;
    }

    if(_salidaAsociada>=salidas.getNumSalidas()) {
      Traza.mensaje("Plan %i, La salida asociada (%i) no esta configurada\n",_id,_salidaAsociada);
      numeroPlanes=0;//Esto aborta la configuracion de los planes!!!!!!!
    }
    else{
      //Intevalos
      for(int8_t j=0;j<INTERVALOS_EN_HORA;j++) {
        JsonObject& intervalo = _plan["intervalos"][j];//json["Planes"][i]["horas"][j];  
        if(intervalo.containsKey("valor")) _intervalos[j]=intervalo.get<int>("valor");        
      }
    
      //Traza.mensaje("Plan %i: id: %i, nombre: %s, salida: %i\n", i, _id, _nombre.c_str(), _salidaAsociada);
      //for(int8_t j=0;j<INTERVALOS_EN_HORA;j++) Traza.mensaje("intervalo[%i]: %i\n", j, _intervalos[j]);
      planes[i].configura(_id, _nombre, _salidaAsociada, _intervalos);

      Traza.mensaje("Plan %s (%i):\n\tSalida: %i\n", planes[i].getNombre(), i, planes[i].getSalida()); 
      for(int8_t j=0;j<INTERVALOS_EN_HORA;j++) Traza.mensaje("\tintervalo %02i: valor: %01i\n",j,planes[i].getIntervalo(j));    
    }
  }
//************************************************************************************************
  return true; 
}
/**********************************************************Fin configuracion******************************************************************/  

/**********************************************************SALIDAS******************************************************************/    
/******************************************************************/
/*Logica del secuenciador: pregunta por cada plan, como debe      */
/*estar en ese peridodo de y actualiza la salida correspondiente  */
/******************************************************************/
void Secuenciador::actualiza(bool debug){
  if(!activado) return;
  if(getNumPlanes()==0) return;

  for(int8_t i=0;i<getNumPlanes();i++){
    /*
    Traza.mensaje("Inicio plan %i------------------------------------------------\n",i);
    if(planes[i].getEstado()!=0) Traza.mensaje("Es 1\n");
    else Traza.mensaje("Es 0\n");
    Traza.mensaje("Fin----------------------------------------------------------\n");
    */
    if(planes[i].getEstado()!=salidas.getSalida(planes[i].getSalida()).getEstado()) salidas.conmuta(planes[i].getSalida(),planes[i].getEstado());
  }
}

/**************************************************/
/* Devuelve el nuemro de planes definido          */
/**************************************************/
int8_t Secuenciador::getNumPlanes(){return numeroPlanes;}  

/***************************************************/
/* Devuelve el numero de salida asociada a un plan */
/***************************************************/
int8_t Secuenciador::getSalida(uint8_t plan){return planes[plan].getSalida();}

/*********************************************************/
/* Devuelve el estado configurado para el plan indicado, */
/* en una hora y minuto concreto                         */
/* valor devuelto: ESTADO_DESACTIVO o ESTADO_ACTIVO      */
/*********************************************************/
int Secuenciador::getEstadoPlan(uint8_t plan, uint8_t hora, uint8_t minuto){return planes[plan].getEstado(hora, minuto);}
int Secuenciador::getEstadoPlan(uint8_t plan){return planes[plan].getEstado();}

/********************************************************/
/*             Activa el secuenciador                   */
/********************************************************/ 
void Secuenciador::activar(void){activado=true;}

/********************************************************/
/*             Desactiva el secuenciador                */
/********************************************************/ 
void Secuenciador::desactivar(void){activado=false;}
  
/*********************************************************/
/*          Devuelve el estado del secuenciador          */
/*********************************************************/
boolean Secuenciador::getEstado(void){return activado;}  

/***********************************************************/
/*   Devuelve el estado de las entradas en formato json    */
/***********************************************************/
String Secuenciador::generaJsonEstado(void){
  String cad="";

  const size_t bufferSize = JSON_ARRAY_SIZE(5) + JSON_OBJECT_SIZE(2) + 5*JSON_OBJECT_SIZE(4);
  DynamicJsonBuffer jsonBuffer(bufferSize);
  
  JsonObject& root = jsonBuffer.createObject();
  
  if(activado) root["estado"] = 1;
  else root["estado"]=0;

  JsonArray& _planes = root.createNestedArray("datos");////Planes
  for(int8_t id=0;id<numeroPlanes;id++){
    JsonObject& _plan = _planes.createNestedObject(); 
    _plan["id"] = id;
    _plan["nombre"] = planes[id].getNombre();
    _plan["salida"] = planes[id].getSalida();
    _plan["estado"] = planes[id].getEstado();
  }

  root.printTo(cad);
  return cad;  
}
/************************* Secuenciador ********************************/


/************************************** Plan *********************************/
/************************* Constructor ***********************/
Plan::Plan(void){
  id=-1;
  salidaAsociada=NO_CONFIGURADO;
  nombre="";
  for(int8_t j=0;j<INTERVALOS_EN_HORA;j++) intervalos[j]=0;
}
/************************* Fin constructor ***********************/

/**************************************************/
/* Configura el plan con lo valores recibidos     */
/**************************************************/
void Plan::configura(int8_t _id, String _nombre, int8_t _salidaAsociada, uint32_t _intervalos[INTERVALOS_EN_HORA]){
  id=_id,
  nombre=_nombre;
  salidaAsociada=_salidaAsociada;
  
  for(uint8_t j=0;j<INTERVALOS_EN_HORA;j++) intervalos[j]=_intervalos[j];

  //configuro la salida asociada en modo secuenciador y la asocio al plan
  salidas.asociarSecuenciador(salidaAsociada,id);
}

/**************************************************/
/* Devuelve el numero de salida asociada a un plan*/
/**************************************************/
int8_t Plan::getSalida(void){return salidaAsociada;}  

/*****************************************************/
/* Devuelve el valor de horas de ese plan a esa hora */
/*****************************************************/
uint32_t Plan::getIntervalo(int8_t intervalo){return intervalos[intervalo];}

/*********************************************************/
/* Devuelve el estado configurado para una hora y minuto */
/* valor devuelto: ESTADO_DESACTIVO o ESTADO_ACTIVO      */
/*********************************************************/
int Plan::getEstado(uint8_t hora, uint8_t intervalo){ 
  uint32_t _mascara=1;
  _mascara<<=hora;

  //**//Traza.mensaje("Plan %i | Hora: %i | intervalo: %i | mascara: %i | valor: %i | retorno: %i\n",id,hora,intervalo,_mascara,intervalos[intervalo],(intervalos[intervalo] & _mascara?1:0));
  
  if(intervalos[intervalo] & _mascara) return ESTADO_ACTIVO;
  else return ESTADO_DESACTIVO;
}
int Plan::getEstado(void){return getEstado(hora(),minuto());}