/*******************************************/
/*                                         */
/*  Definicion de la maquina de estados    */
/*  configurable.                          */
/*                                         */
/*  Hay un mapeo de E/S del dispositivo y  */
/*  de la maquina de estados. La entrada 1 */
/*  de la maquina puede ser la 4 del dis-  */
/*  positivo. Igual con las salidas.       */
/*                                         */
/*  el estado 0 es el de error             */
/*  el estado 1 es el de inicio            */
/*  el resto configurables                 */
/*                                         */
/*******************************************/

/***************************** Defines *****************************/
/***************************** Defines *****************************/

/***************************** Includes *****************************/
#include <Global.h>
#include <MaqEstados.h>
#include <Entradas.h>
#include <Salidas.h>
#include <Ficheros.h>
/***************************** Includes *****************************/


/*********************************************************MAQUINA DE ESTADOS******************************************************************/    
MaquinaEstados maquinaEstados;

/************************************** Constructor ****************************************/
MaquinaEstados::MaquinaEstados(void){
  //Estado de la maquina
  debugMaquinaEstados=false;
  estadoActual=ESTADO_INICIAL;
  for(uint8_t i=0;i<entradas.getNumEntradas();i++) entradasActual[i]=NO_CONFIGURADO;
  for(uint8_t i=0;i<entradas.getNumEntradas();i++) salidasActual[i]=NO_CONFIGURADO;
  
  //Variables de configuracion
  numeroEstados=0;
  numeroTransiciones=0;
  numeroEntradas=0;
  numeroSalidas=0;

  //Entradas
  for(uint8_t i=0;i<entradas.getNumEntradas();i++) mapeoEntradas[i]=NO_CONFIGURADO;
    
  //Salidas
  for(uint8_t i=0;i<salidas.getNumSalidas();i++) mapeoSalidas[i]=NO_CONFIGURADO;
}
/************************************** Fin constructor ****************************************/

/************************************** Funciones de configuracion ****************************************/
/*********************************************/
/* Inicializa los valores de la maquina      */
/* de estados                                */
/*********************************************/
void MaquinaEstados::inicializa(void) {         
  //leo la configuracion del fichero
  if(!recuperaDatos(debugGlobal)) Traza.mensaje("Configuracion de la maquina de estados por defecto.\n");
  else {}  
}

/************************************************/
/* Lee el fichero de configuracion de la        */
/* maquina de estados o genera conf por defecto */
/************************************************/
boolean MaquinaEstados::recuperaDatos(int debug){
  String cad="";

  if (debug) Traza.mensaje("Recupero configuracion de archivo...\n");

  if(!leeFichero(MAQUINAESTADOS_CONFIG_FILE, cad)){
    //Confgiguracion por defecto
    Traza.mensaje("No existe fichero de configuracion de la maquina de estados\n");    
    cad="{\"Estados\":[],\"Transiciones\":[] }";
    //salvo la config por defecto
    //if(salvaFichero(MAQUINAESTADOS_CONFIG_FILE, MAQUINAESTADOS_CONFIG_BAK_FILE, cad)) Traza.mensaje("Fichero de configuracion de la maquina de estados creado por defecto\n");
  }
  return parseaConfiguracion(cad);
}

/*********************************************/
/* Parsea el json leido del fichero de       */
/* configuracio de la maquina de estados     */
/*********************************************/
boolean MaquinaEstados::parseaConfiguracion(String contenido)
  {
  DynamicJsonBuffer jsonBuffer;
  JsonObject& json = jsonBuffer.parseObject(contenido.c_str());

  String salida;
  json.printTo(salida);//pinto el json que he leido
  Traza.mensaje("json cargado:\n#%s#\n",salida.c_str());
  
  if (!json.success()) return false;

  Traza.mensaje("\nparsed json\n");
//******************************Parte especifica del json a leer********************************
  if(!json.containsKey("Estados"))  return false; 
  if(!json.containsKey("Transiciones"))  return false; 
  if(!json.containsKey("Entradas"))  return false; 
  if(!json.containsKey("Salidas"))  return false; 

  /********************Entradas******************************/
  JsonArray& E = json["Entradas"];
  numeroEntradas=(E.size()<entradas.getNumEntradas()?E.size():entradas.getNumEntradas());
  for(uint8_t i=0;i<numeroEntradas;i++) mapeoEntradas[i]=E[i];   

  Traza.mensaje("Entradas asociadas a la maquina de estados: %i\n",numeroEntradas);
  for(uint8_t i=0;i<numeroEntradas;i++) Traza.mensaje("orden %i | id general %i\n", i,mapeoEntradas[i]);
  
  /********************Salidas******************************/
  JsonArray& S = json["Salidas"];
  numeroSalidas=(S.size()<salidas.getNumSalidas()?S.size():salidas.getNumSalidas());
  for(uint8_t i=0;i<numeroSalidas;i++) mapeoSalidas[i]=S[i];   

  Traza.mensaje("Salidas asociadas a la maquina de estados: %i\n",numeroSalidas);
  for(uint8_t i=0;i<numeroSalidas;i++) Traza.mensaje("orden %i | id general %i\n", i,mapeoSalidas[i]);
  
  /********************Estados******************************/
  JsonArray& Estados = json["Estados"];

  numeroEstados=(Estados.size()<MAX_ESTADOS?Estados.size():MAX_ESTADOS);

  estados=new Estado[numeroEstados];

  for(int8_t i=0;i<numeroEstados;i++)
    { 
    JsonObject& est = Estados[i];   
    estados[i].setId(est["id"]); 
    estados[i].setNombre(String((const char *)est["nombre"]));

    JsonArray& Salidas = est["salidas"];
    int8_t num_salidas;
    num_salidas=(Salidas.size()<salidas.getNumSalidas()?Salidas.size():salidas.getNumSalidas());
    if(num_salidas!=numeroSalidas) 
      {
      Traza.mensaje("Numero de salidas incorrecto en estado %i. definidas %i, esperadas %i\n",i,num_salidas,numeroSalidas);
      return false;
      }
    for(int8_t s=0;s<num_salidas;s++) estados[i].setValorSalida(s,Salidas.get<int>(s));
    }

  Traza.mensaje("*************************\nEstados:\n"); 
  Traza.mensaje("Se han definido %i estados\n",numeroEstados);
  for(int8_t i=0;i<numeroEstados;i++) 
    {
    Traza.mensaje("%01i: id= %i| nombre: %s\n",i,estados[i].getId(),estados[i].getNombre().c_str());
    Traza.mensaje("salidas:\n");
    for(int8_t s=0;s<numeroSalidas;s++) 
      {
      Traza.mensaje("salida[%02i]: valor: %i\n",s,estados[i].getValorSalida(s));
      }
    }
  Traza.mensaje("*************************\n");  
  
  /********************Transiciones******************************/
  JsonArray& Transiciones = json["Transiciones"];

  numeroTransiciones=(Transiciones.size()<MAX_TRANSICIONES?Transiciones.size():MAX_TRANSICIONES);

  transiciones=new Transicion[numeroTransiciones];

  for(int8_t i=0;i<numeroTransiciones;i++)
    { 
    JsonObject& trans = Transiciones[i];   
    transiciones[i].setEstadoInicial(trans["inicial"]); 
    transiciones[i].setEstadoFinal(trans["final"]);
    
    JsonArray& Entradas = trans["entradas"];   
    int8_t num_entradas;
    num_entradas=(Entradas.size()<entradas.getNumEntradas()?Entradas.size():entradas.getNumEntradas());
    if(num_entradas!=numeroEntradas) 
      {
      Traza.mensaje("Numero de entradas incorrecto en estado %i. definidas %i, esperadas %i\n",i,num_entradas,numeroEntradas);
      return false;
      }
    
    for(int8_t e=0;e<num_entradas;e++) transiciones[i].setValorEntrada(e,Entradas.get<int>(e));//Puede ser -1, significa que no importa el valor
    }

  Traza.mensaje("*************************\nTransiciones:\n"); 
  Traza.mensaje("Se han definido %i transiciones\n",numeroTransiciones);
  for(int8_t i=0;i<numeroTransiciones;i++) 
    {
    Traza.mensaje("%01i: estado inicial= %i| estado final: %i\n",i,transiciones[i].getEstadoInicial(),transiciones[i].getEstadoFinal());
    Traza.mensaje("entradas:\n");
    for(int8_t e=0;e<numeroEntradas;e++) 
      {
      Traza.mensaje("entradas[%02i]: valor: %i\n",e,transiciones[i].getValorEntrada(e));
      }
    }
  Traza.mensaje("*************************\n");  
//************************************************************************************************
  return true; 
  }

/************************************** Funciones de configuracion ****************************************/

/************************************** Maquina ************************************************************/  
/************************************************************************************/
/* Analiza el estado de la maquina y evoluciona los estados y las salidas asociadas */
/************************************************************************************/
void MaquinaEstados::actualiza(int debug)
  {
  boolean localDebug=debug || debugMaquinaEstados;
    
  //Actualizo el vaor de las entradas
  for(uint8_t i=0;i<numeroEntradas;i++) entradasActual[i]=entradas.getEntrada(mapeoEntradas[i]).getEstado();//  estadoEntrada(mapeoEntradas[i]);

  if(localDebug) 
    {
    Traza.mensaje("Estado inicial: (%i) %s\n",estadoActual,estados[estadoActual].getNombre().c_str());
    Traza.mensaje("Estado de las entradas:\n");
    for(uint8_t i=0;i<numeroEntradas;i++) Traza.mensaje("Entrada %i (dispositivo %i: %s)=> valor %i\n",i, mapeoEntradas[i],entradas.getEntrada(mapeoEntradas[i]).getNombre().c_str(),entradasActual[i]);
    }
    
  //busco en las transiciones a que estado debe evolucionar la maquina
  estadoActual=mueveMaquina(estadoActual, entradasActual, localDebug);

  //Actualizo las salidas segun el estado actual
  if(actualizaSalidasMaquinaEstados(estadoActual)!=1) Traza.mensaje("Error al actualizar las salidas\n");

  if(localDebug) Traza.mensaje("Estado actual: (%i) %s\n",estadoActual,estados[estadoActual].getNombre().c_str());
  }
void MaquinaEstados::actualiza(void){actualiza(false);}

/**********************************************************************/
/* busco en las transiciones a que estado debe evolucionar la maquina */
/**********************************************************************/
uint8_t MaquinaEstados::mueveMaquina(uint8_t estado, int8_t entradasActual[], boolean debug)
  {
  for(uint8_t regla=0;regla<numeroTransiciones;regla++) //las reglas se evaluan por orden
    {
    if(transiciones[regla].getEstadoInicial()==estado)//Solo analizo las que tienen como estado inicial el indicado
      {
      if(debug) Traza.mensaje("Revisando regla %i\n",regla);
  
      boolean coinciden=true;  
      for(uint8_t entrada=0;entrada<numeroEntradas;entrada++) 
        {
        if (transiciones[regla].getValorEntrada(entrada)!=NO_CONFIGURADO) coinciden=coinciden &&(entradasActual[entrada]==transiciones[regla].getValorEntrada(entrada));
        if(debug) Traza.mensaje("Revisando entradas %i de regla %i (valor actual: %i vs valor regla: %i). Resultado %i\n",entrada,regla,entradasActual[entrada],transiciones[regla].getValorEntrada(entrada),coinciden);
        }

      if(coinciden) return transiciones[regla].getEstadoFinal();
      }
    }
  return ESTADO_ERROR;  //Si no coincide ninguna regla, pasa a estado error
  }
uint8_t MaquinaEstados::mueveMaquina(uint8_t estado, int8_t entradasActual[]){return mueveMaquina(estado,entradasActual,false);}

/****************************************************/
/* Actualizo las salidas segun el estado actual     */
/****************************************************/
int8_t MaquinaEstados::actualizaSalidasMaquinaEstados(uint8_t estado)
  {
  int8_t retorno=1; //si todo va bien salidaMaquinaEstados devuelve 1, si hay error -1 

  for(uint8_t i=0;i<numeroSalidas;i++) 
    {
    int8_t valor=estados[estado].getValorSalida(i);
    uint8_t _salida=mapeoSalidas[i];
    //Serial.printf("Salida ME: %i | salida sistema: %i | valor a configurar: %i\n",i,_salida,valor);
    
    if(salidas.salidaMaquinaEstados(_salida,valor)==NO_CONFIGURADO) retorno=-1;
    }

  return retorno;
  }
/************************************** Fin mueve maquina ************************************************************/  

/************************************** Consulta ************************************************************/  
/****************************************************/
/* Funciones de consulta de datos (encapsulan)     */
/****************************************************/
uint8_t MaquinaEstados::getNumEstados(void){return numeroEstados;}
uint8_t MaquinaEstados::getNumTransiciones(void){return numeroTransiciones;}
uint8_t MaquinaEstados::getNumEntradas(void){return numeroEntradas;}
uint8_t MaquinaEstados::getNumSalidas(void){return numeroSalidas;}

String MaquinaEstados::getNombreEstadoActual(void){
  if(numeroEstados<=0) return "";//Si no se han definido estados, no hay nada que hacer

  return estados[estadoActual].getNombre();
}

uint8_t MaquinaEstados::getMapeoEntrada(uint8_t id) 
  {
  if (id<0 || id>entradas.getNumEntradas()) return NO_CONFIGURADO;
  return mapeoEntradas[id];
  }

uint8_t MaquinaEstados::getMapeoSalida(uint8_t id) 
  {
  if (id<0 || id>salidas.getNumSalidas()) return NO_CONFIGURADO;
  return mapeoSalidas[id];
  }

uint8_t MaquinaEstados::getEntradasActual(uint8_t id) 
  {
  if (id<0 || id>entradas.getNumEntradas()) return NO_CONFIGURADO;
  return entradasActual[id];
  }

void MaquinaEstados::setDebugMaquinaEstados(boolean debug){debugMaquinaEstados=debug;}
boolean MaquinaEstados::getDebugMAquinaEstados(void){return debugMaquinaEstados;}
/************************************** Consulta ************************************************************/  
/************************************** Funciones de estado *************************************************/
/***********************************************************/
/*                                                         */
/*   Devuelve el estado de las entradas en formato json    */
/*   devuelve un json con el formato:                      */
/* {
    "Entradas": [  
      {"id":  "0", "nombre": "P. abierta", "valor": "1" },
      {"id":  "1", "nombre": "P. cerrada", "valor": "0" }
    ]
  }
                                                           */
/***********************************************************/   
String MaquinaEstados::generaJsonEstado(void)
  {
  String cad="";

  const size_t bufferSize = 2*JSON_ARRAY_SIZE(4) + 9*JSON_OBJECT_SIZE(3) + 42*JSON_ARRAY_SIZE(4) + JSON_ARRAY_SIZE(42) + JSON_OBJECT_SIZE(1) + 42*JSON_OBJECT_SIZE(3);
  DynamicJsonBuffer jsonBuffer(bufferSize);
  
  JsonObject& root = jsonBuffer.createObject();
  
  if(numeroEstados<=0 || numeroEntradas<=0 || numeroSalidas<=0 || numeroTransiciones<=0){//Si no se han definido la maquina, no hay nada que hacer
    root.printTo(cad);
    return cad;
  }

  root["estado"] = maquinaEstados.getNombreEstadoActual();
  root["numeroEntradas"] = maquinaEstados.getNumEntradas();
  root["numeroSalidas"] = maquinaEstados.getNumSalidas();

  JsonArray& Entradas = root.createNestedArray("entradas");
  for(int8_t id=0;id<numeroEntradas;id++){
    JsonObject& Entradas_0 = Entradas.createNestedObject(); 
    Entradas_0["id"] = id;
    Entradas_0["nombre"] = entradas.getEntrada(mapeoEntradas[id]).getNombre();
    Entradas_0["estado"] = entradas.getEntrada(mapeoEntradas[id]).getEstado();
    Entradas_0["entradaGlobal"] = mapeoEntradas[id];
  }

  JsonArray& Salidas = root.createNestedArray("salidas");
  for(int8_t id=0;id<numeroSalidas;id++){
    JsonObject& Salidas_0 = Salidas.createNestedObject(); 
    Salidas_0["id"] = id;
    Salidas_0["nombre"] = salidas.getSalida(mapeoSalidas[id]).getNombre();
    Salidas_0["estado"] = salidas.getSalida(mapeoSalidas[id]).getEstado();
    Salidas_0["salidaGlobal"] = mapeoSalidas[id];
    }

  JsonArray& listaTransiciones = root.createNestedArray("transiciones");
  for(uint8_t regla=0;regla<numeroTransiciones;regla++) //las reglas se evaluan por orden
    {
    JsonObject& transicionNueva = listaTransiciones.createNestedObject();
   
    transicionNueva["inicial"] = maquinaEstados.estados[maquinaEstados.transiciones[regla].getEstadoInicial()].getNombre();
    transicionNueva["final"] = maquinaEstados.estados[maquinaEstados.transiciones[regla].getEstadoFinal()].getNombre();
    JsonArray& valorEntradas = transicionNueva.createNestedArray("entradas");

    for(uint8_t entradaME=0;entradaME<numeroEntradas;entradaME++) valorEntradas.add(transiciones[regla].getValorEntrada(entradaME));
  }

  root.printTo(cad);
  return cad;  
  }
/************************************** Funciones de estado *************************************************/  
/*********************************************************FIN MAQUINA DE ESTADOS******************************************************************/    

/*********************************************************ESTADO******************************************************************/    
Estado::Estado(void){  
  id=0;
  nombre="Estado_" + String(id);
  for(uint8_t j=0;j<MAX_SALIDAS;j++) valorSalida[j]=NO_CONFIGURADO;
}

void Estado::setId(uint8_t _id) {id=_id;}
void Estado::setNombre(String _nombre) {nombre=_nombre;}
void Estado::setValorSalida(uint8_t _salida, int8_t _valor) {valorSalida[_salida]=_valor;}

uint8_t Estado::getId() {return id;}
String Estado::getNombre(void) {return nombre;}
int8_t Estado::getValorSalida(uint8_t salida) {return valorSalida[salida];}
/*********************************************************ESTADO******************************************************************/    

/*********************************************************TRANSICION******************************************************************/    
Transicion::Transicion(void){
  estadoInicial=NO_CONFIGURADO;
  estadoFinal=NO_CONFIGURADO;
  for(uint8_t j=0;j<MAX_ENTRADAS;j++) valorEntrada[j]=NO_CONFIGURADO;
}

void Transicion::setEstadoInicial(uint8_t _estado) {estadoInicial=_estado;}
void Transicion::setEstadoFinal(uint8_t _estado) {estadoFinal=_estado;}
void Transicion::setValorEntrada(uint8_t _entrada, int8_t _valor) {valorEntrada[_entrada]=_valor;}

uint8_t Transicion::getEstadoInicial(void) {return estadoInicial;}
uint8_t Transicion::getEstadoFinal(void) {return estadoFinal;}
int8_t Transicion::getValorEntrada(int8_t entrada) {return valorEntrada[entrada];}
/*********************************************************FIN TRANSICION******************************************************************/    