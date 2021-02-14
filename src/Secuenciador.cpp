/*****************************************/
/*                                       */
/*  Secuenciador de fases de entradas    */
/*                                       */
/*****************************************/

/***************************** Defines *****************************/

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
    //cad="{ \"estadoInicial\": 0, \"Planes\":[ {\"id_plan\": 1, \"salida\": 1, \"intervalos\": [{\"id\":  0, \"valor\": 0},{\"id\":  1, \"valor\": 1}, {\"id\":  2, \"valor\": 0}, {\"id\":  3, \"valor\": 1}, {\"id\":  4, \"valor\": 0}, {\"id\":  5, \"valor\": 1}, {\"id\":  6, \"valor\": 0}, {\"id\":  7, \"valor\": 1}, {\"id\":  8, \"valor\": 0}, {\"id\":  9, \"valor\": 1}, {\"id\": 10, \"valor\": 0}, {\"id\": 11, \"valor\": 1},{\"id\":  12, \"valor\": 0},{\"id\":  13, \"valor\": 1}, {\"id\":  14, \"valor\": 0}, {\"id\":  15, \"valor\": 1}, {\"id\":  16, \"valor\": 0}, {\"id\":  17, \"valor\": 1}, {\"id\":  18, \"valor\": 0}, {\"id\":  19, \"valor\": 1}, {\"id\":  20, \"valor\": 0}, {\"id\":  21, \"valor\": 1}, {\"id\": 22, \"valor\": 0}, {\"id\": 23, \"valor\": 1} ] } ] }";
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
    int    _horas[HORAS_EN_DIA];      //el valor es un campo de bit. los primeros 12 son los intervalos de 5 min de cada hora
    for(uint8_t i=0;i<HORAS_EN_DIA;i++) _horas[i]=0;

    JsonObject& _plan = json["Planes"][i];
    //leo los valores del json
    _id=i;
    if(_plan.containsKey("nombre")) _nombre=_plan.get<String>("nombre");
    if(_plan.containsKey("salida")) _salidaAsociada=_plan.get<int>("salida");

    if(_salidaAsociada>=salidas.getNumSalidas()) {
      Traza.mensaje("Plan %i, La salida asociada (%i) no esta configurada\n",_id,_salidaAsociada);
      numeroPlanes=0;//Esto aborta la configuracion de los planes!!!!!!!
    }
    else{
      //Intevalos
      for(int8_t j=0;j<HORAS_EN_DIA;j++) {
        JsonObject& _intervalo = json["Planes"][i]["intervalos"][j];  
        if(_intervalo.containsKey("valor")) _horas[j]=_intervalo.get<int>("valor");        
      }
    
      //Traza.mensaje("Plan %i: id: %i, nombre: %s, salida: %i\n", i, _id, _nombre.c_str(), _salidaAsociada);
      //for(int8_t j=0;j<HORAS_EN_DIA;j++) Traza.mensaje("hora[%i]: %i\n", j, _horas[j]);
      planes[i].configura(_id, _nombre, _salidaAsociada, _horas);

      Traza.mensaje("Plan %s (%i):\n\tSalida: %i\n", planes[i].getNombre(), i, planes[i].getSalida()); 
      for(int8_t j=0;j<HORAS_EN_DIA;j++) Traza.mensaje("\thora %02i: valor: %01i\n",j,planes[i].getHoras(j));    
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
  if(!estado()) return;
    
  for(int8_t i=0;i<getNumPlanes();i++){
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
boolean Secuenciador::estado(void){return activado;}  

/*********************************************************/
/*      Genera codigo HTML para representar el plan      */
/*********************************************************/
String Secuenciador::pintaPlanHTML(uint8_t plan){return planes[plan].pintaPlanHTML();}

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

  JsonArray& _planes = root.createNestedArray("planes");
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
  for(int8_t j=0;j<12;j++) horas[j]=0;
}
/************************* Fin constructor ***********************/

/**************************************************/
/* Configura el plan con lo valores recibidos     */
/**************************************************/
void Plan::configura(int8_t _id, String _nombre, int8_t _salidaAsociada, int _horas[HORAS_EN_DIA]){
  id=_id,
  nombre=_nombre;
  salidaAsociada=_salidaAsociada;
  
  for(uint8_t j=0;j<HORAS_EN_DIA;j++) horas[j]=_horas[j];

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
int Plan::getHoras(int8_t hora){return horas[hora];}

/*********************************************************/
/* Devuelve el estado configurado para una hora y minuto */
/* valor devuelto: ESTADO_DESACTIVO o ESTADO_ACTIVO      */
/*********************************************************/
int Plan::getEstado(uint8_t hora, uint8_t minuto){
  int _valor=horas[hora];
  int _intervalo=minuto / 5; //calulo el bit que contiene el intervalo del minuto indicado

  int _mascara=pow(2,_intervalo);
  //Traza.mensaje("Hora: %i | minuto: %i | intervalo: %i | mascara: %i | retorno: %i\n",hora,minuto,_intervalo,_mascara,_valor & _mascara);

  if(_valor & _mascara) return ESTADO_ACTIVO;
  else return ESTADO_DESACTIVO;
}
int Plan::getEstado(void){return getEstado(hora(),minuto());}

/********************************************************/
/*     Genera codigo HTML para representar el plan      */
/********************************************************/ 
String Plan::pintaPlanHTML(void)
  {
  String cad="";

  cad += "<TABLE width=\"90%\" border=\"0\" cellpadding=\"0\" cellspacing=\"0\" class=\"tabla\">\n";
  cad += "<CAPTION>Plan " + nombre + "</CAPTION>\n";  

  //Cabecera
  cad += "<tr>";
  cad += "<th width=\"200px\">Hora</th>";
  for(int8_t i=0;i<HORAS_EN_DIA;i++) cad += "<th style=\"width:40px\">" + String(i) + "</th>";
  cad += "</tr>";

  //Cada fila es un intervalo, cada columna un hora
  int mascara=1;  
  
  for(int8_t intervalo=0;intervalo<12;intervalo++)
    {
    //Traza.mensaje("intervalo: %i | cad: %i\n",intervalo,cad.length());      
    cad += "<TR class=\"modo2\">";
    cad += "<th>" + String(intervalo) + ": (min " + String(intervalo*5) + " a " + String(intervalo*5+4) + ")</th>";    
    for(int8_t i=0;i<HORAS_EN_DIA;i++) cad += "<td style=\"text-align:center;\">" + (horas[i] & mascara?String(1):String(0)) + "</td>";
    cad += "</tr>";
    
    mascara<<=1;
    }  
    
  cad+="</table>";
    
  return cad;  
  }
