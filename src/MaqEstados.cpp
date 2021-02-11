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
  for(uint8_t i=0;i<MAX_ENTRADAS;i++) entradasActual[i]=NO_CONFIGURADO;
  for(uint8_t i=0;i<MAX_ENTRADAS;i++) salidasActual[i]=NO_CONFIGURADO;
  
  //Variables de configuracion
  numeroEstados=0;
  numeroTransiciones=0;
  numeroEntradas=0;
  numeroSalidas=0;

  //Entradas
  for(uint8_t i=0;i<MAX_ENTRADAS;i++) mapeoEntradas[i]=NO_CONFIGURADO;
    
  //Salidas
  for(uint8_t i=0;i<MAX_SALIDAS;i++) mapeoSalidas[i]=NO_CONFIGURADO;

  //inicializo estados
  for(int8_t i=0;i<MAX_ESTADOS;i++) estados[i]=Estado();//Inicializo todas a valores por defecto, solo las configuradas vienen en el fichero
  //inicializo transiciones
  for(int8_t i=0;i<MAX_TRANSICIONES;i++) transiciones[i]=Transicion();//Inicializo todas a valores por defecto, solo las configuradas vienen en el fichero
}
/************************************** Fin constructor ****************************************/

/************************************** Funciones de configuracion ****************************************/
/*********************************************/
/* Inicializa los valores de la maquina      */
/* de estados                                */
/*********************************************/
void MaquinaEstados::inicializaMaquinaEstados(void) {         
  //leo la configuracion del fichero
  if(!recuperaDatosMaquinaEstados(debugGlobal)) Traza.mensaje("Configuracion de la maquina de estados por defecto.\n");
  else {}  
}

/************************************************/
/* Lee el fichero de configuracion de la        */
/* maquina de estados o genera conf por defecto */
/************************************************/
boolean MaquinaEstados::recuperaDatosMaquinaEstados(int debug){
  String cad="";

  if (debug) Traza.mensaje("Recupero configuracion de archivo...\n");

  if(!leeFichero(MAQUINAESTADOS_CONFIG_FILE, cad)){
    //Confgiguracion por defecto
    Traza.mensaje("No existe fichero de configuracion de la maquina de estados\n");    
    cad="{\"Estados\":[],\"Transiciones\":[] }";
    //salvo la config por defecto
    //if(salvaFichero(MAQUINAESTADOS_CONFIG_FILE, MAQUINAESTADOS_CONFIG_BAK_FILE, cad)) Traza.mensaje("Fichero de configuracion de la maquina de estados creado por defecto\n");
  }
  return parseaConfiguracionMaqEstados(cad);
}

/*********************************************/
/* Parsea el json leido del fichero de       */
/* configuracio de la maquina de estados     */
/*********************************************/
boolean MaquinaEstados::parseaConfiguracionMaqEstados(String contenido)
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
  numeroEntradas=(E.size()<MAX_ENTRADAS?E.size():MAX_ENTRADAS);
  for(uint8_t i=0;i<numeroEntradas;i++) mapeoEntradas[i]=E[i];   

  Traza.mensaje("Entradas asociadas a la maquina de estados: %i\n",numeroEntradas);
  for(uint8_t i=0;i<numeroEntradas;i++) Traza.mensaje("orden %i | id general %i\n", i,mapeoEntradas[i]);
  
  /********************Salidas******************************/
  JsonArray& S = json["Salidas"];
  numeroSalidas=(S.size()<MAX_SALIDAS?S.size():MAX_SALIDAS);
  for(uint8_t i=0;i<numeroSalidas;i++) mapeoSalidas[i]=S[i];   

  Traza.mensaje("Salidas asociadas a la maquina de estados: %i\n",numeroSalidas);
  for(uint8_t i=0;i<numeroSalidas;i++) Traza.mensaje("orden %i | id general %i\n", i,mapeoSalidas[i]);
  
  /********************Estados******************************/
  JsonArray& Estados = json["Estados"];

  numeroEstados=(Estados.size()<MAX_ESTADOS?Estados.size():MAX_ESTADOS);
  for(int8_t i=0;i<numeroEstados;i++)
    { 
    JsonObject& est = Estados[i];   
    estados[i].setId(est["id"]); 
    estados[i].setNombre(String((const char *)est["nombre"]));

    JsonArray& Salidas = est["salidas"];
    int8_t num_salidas;
    num_salidas=(Salidas.size()<MAX_SALIDAS?Salidas.size():MAX_SALIDAS);
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
  for(int8_t i=0;i<numeroTransiciones;i++)
    { 
    JsonObject& trans = Transiciones[i];   
    transiciones[i].setEstadoInicial(trans["inicial"]); 
    transiciones[i].setEstadoFinal(trans["final"]);
    
    JsonArray& Entradas = trans["entradas"];   
    int8_t num_entradas;
    num_entradas=(Entradas.size()<MAX_ENTRADAS?Entradas.size():MAX_ENTRADAS);
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
void MaquinaEstados::actualizaMaquinaEstados(int debug)
  {
  boolean localDebug=debug || debugMaquinaEstados;
    
  //Actualizo el vaor de las entradas
  for(uint8_t i=0;i<numeroEntradas;i++) entradasActual[i]=entradas[mapeoEntradas[i]].getEstado();//  estadoEntrada(mapeoEntradas[i]);

  if(localDebug) 
    {
    Traza.mensaje("Estado inicial: (%i) %s\n",estadoActual,estados[estadoActual].getNombre().c_str());
    Traza.mensaje("Estado de las entradas:\n");
    for(uint8_t i=0;i<numeroEntradas;i++) Traza.mensaje("Entrada %i (dispositivo %i: %s)=> valor %i\n",i, mapeoEntradas[i],entradas[mapeoEntradas[i]].getNombre().c_str(),entradasActual[i]);
    }
    
  //busco en las transiciones a que estado debe evolucionar la maquina
  estadoActual=mueveMaquina(estadoActual, entradasActual, localDebug);

  //Actualizo las salidas segun el estado actual
  if(actualizaSalidasMaquinaEstados(estadoActual)!=1) Traza.mensaje("Error al actualizar las salidas\n");

  if(localDebug) Traza.mensaje("Estado actual: (%i) %s\n",estadoActual,estados[estadoActual].getNombre().c_str());
  }
void MaquinaEstados::actualizaMaquinaEstados(void){actualizaMaquinaEstados(false);}

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
    
    if(salidas[_salida].salidaMaquinaEstados(valor)==NO_CONFIGURADO) retorno=-1;
    //if(salidas[mapeoSalidas[i]].salidaMaquinaEstados(estados[estado].getValorSalida(i))==NO_CONFIGURADO) retorno=-1;
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

String MaquinaEstados::getNombreEstadoActual(void){return estados[estadoActual].getNombre();}

uint8_t MaquinaEstados::getMapeoEntrada(uint8_t id) 
  {
  if (id<0 || id>MAX_ENTRADAS) return NO_CONFIGURADO;
  return mapeoEntradas[id];
  }

uint8_t MaquinaEstados::getMapeoSalida(uint8_t id) 
  {
  if (id<0 || id>MAX_SALIDAS) return NO_CONFIGURADO;
  return mapeoSalidas[id];
  }

uint8_t MaquinaEstados::getEntradasActual(uint8_t id) 
  {
  if (id<0 || id>MAX_ENTRADAS) return NO_CONFIGURADO;
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
String MaquinaEstados::generaJsonEstadoMaquinaEstados(void)
  {
  String salida="";

  const size_t bufferSize = 2*JSON_ARRAY_SIZE(4) + 9*JSON_OBJECT_SIZE(3);
  DynamicJsonBuffer jsonBuffer(bufferSize);
  
  JsonObject& root = jsonBuffer.createObject();
  
  root["estado"] = maquinaEstados.getNombreEstadoActual();

  JsonArray& Entradas = root.createNestedArray("entradas");
  for(int8_t id=0;id<numeroEntradas;id++){
    JsonObject& Entradas_0 = Entradas.createNestedObject(); 
    Entradas_0["id"] = id;
    Entradas_0["nombre"] = entradas[mapeoEntradas[id]].getNombre();
    Entradas_0["estado"] = entradas[mapeoEntradas[id]].getEstado();
  }

  JsonArray& Salidas = root.createNestedArray("salidas");
  for(int8_t id=0;id<numeroSalidas;id++){
    JsonObject& Salidas_0 = Salidas.createNestedObject(); 
    Salidas_0["id"] = id;
    Salidas_0["nombre"] = salidas[mapeoSalidas[id]].getNombre();
    Salidas_0["estado"] = salidas[mapeoSalidas[id]].getEstado();
  }

  root.printTo(salida);
  return salida;  
  }
/************************************** Funciones de estado *************************************************/  
/*********************************************************FIN MAQUINA DE ESTADOS******************************************************************/    

/*********************************************************ESTADO******************************************************************/    
Estado::Estado(void){  
  for(int8_t i=0;i<MAX_ESTADOS;i++){
    //inicializo la parte logica
    id=i;
    nombre="Estado_" + String(i);
    for(uint8_t j=0;j<MAX_SALIDAS;j++) valorSalidas[j]=NO_CONFIGURADO;
  }
}

void Estado::setId(uint8_t _id) {id=_id;}
void Estado::setNombre(String _nombre) {nombre=_nombre;}
void Estado::setValorSalida(uint8_t _salida, int8_t _valor) {valorSalidas[_salida]=_valor;}

uint8_t Estado::getId() {return id;}
String Estado::getNombre(void) {return nombre;}
int8_t Estado::getValorSalida(uint8_t salida) {return valorSalidas[salida];}
/*********************************************************ESTADO******************************************************************/    

/*********************************************************TRANSICION******************************************************************/    
Transicion::Transicion(void){
  for(int8_t i=0;i<MAX_TRANSICIONES;i++){
    //inicializo la parte logica
    setEstadoInicial(NO_CONFIGURADO);
    setEstadoFinal(NO_CONFIGURADO);
    for(uint8_t j=0;j<MAX_ENTRADAS;j++) setValorEntrada(j,NO_CONFIGURADO);
  }
}

void Transicion::setEstadoInicial(uint8_t _estado) {estadoInicial=_estado;}
void Transicion::setEstadoFinal(uint8_t _estado) {estadoFinal=_estado;}
void Transicion::setValorEntrada(uint8_t _entrada, int8_t _valor) {valorEntradas[_entrada]=_valor;}

uint8_t Transicion::getEstadoInicial(void) {return estadoInicial;}
uint8_t Transicion::getEstadoFinal(void) {return estadoFinal;}
int8_t Transicion::getValorEntrada(int8_t entrada) {return valorEntradas[entrada];}
/*********************************************************FIN TRANSICION******************************************************************/    