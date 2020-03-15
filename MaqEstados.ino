/*******************************************/
/*                                         */
/*  Definicion de la maquina de estados    */
/*  configurable.                          */ 
/*
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

/********************definiciones**************************/
#define MAX_ESTADOS 10
#define MAX_TRANSICIONES 3*MAX_ESTADOS
#define ESTADO_ERROR   0 //Estado de error de la logica de la maquina. Imposible evolucionar del ese estado con las estradas actuales
#define ESTADO_INICIAL 1 //Estado inicio

//Estados de la maquina. Tienen un nombre, un id y una lista de salidas asociadas, son el valor de las salidas en ese estado
typedef struct {
  uint8_t id;
  String nombre;
  uint8_t salidas;
  int8_t salidasAsociadas[MAX_SALIDAS];
  }estados_t;

//Los lazos del grafo. Estado inicial, estado final y los valores de las entradas que gobiernan la transicion
typedef struct {
  int8_t estadoInicial;
  int8_t estadoFinal;
  uint8_t entradas;
  int8_t valorEntradas[MAX_ENTRADAS];
  }transicionEstados_t;
  
/********************includes**************************/

/********************variables locales**************************/
uint8_t numeroEstados;
uint8_t numeroTransiciones;
uint8_t numeroEntradas;
uint8_t numeroSalidas;

estados_t estados[MAX_ESTADOS];
transicionEstados_t transiciones[MAX_TRANSICIONES];
uint8_t mapeoEntradas[MAX_ENTRADAS]; //posicion es la entrada de la maquina de estados, el valor es el id de la entrada del dispositivo
uint8_t mapeoSalidas[MAX_SALIDAS]; //posicion es la salida de la maquina de estados, el valor es el id de la salida del dispositivo

uint8_t estadoActual;
int8_t entradasActual[MAX_ENTRADAS];
int8_t salidasActual[MAX_SALIDAS];
boolean debugMaquinaEstados;
/************************************** Funciones de configuracion ****************************************/
/*********************************************/
/* Inicializa los valores de la maquina      */
/* de estados                                */
/*********************************************/
void inicializaMaquinaEstados(void)
  {      
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
    
  //Estados
  for(int8_t i=0;i<MAX_ESTADOS;i++)
    {
    //inicializo la parte logica
    estados[i].id=i;
    estados[i].nombre="Estado_" + String(i);
    estados[i].salidas=0;
    for(uint8_t j=0;j<MAX_SALIDAS;j++) estados[i].salidasAsociadas[j]=NO_CONFIGURADO;
    }

  //Transiciones
  for(int8_t i=0;i<MAX_TRANSICIONES;i++)
    {
    //inicializo la parte logica
    transiciones[i].estadoInicial=NO_CONFIGURADO;
    transiciones[i].estadoFinal=NO_CONFIGURADO;
    transiciones[i].entradas=0;
    for(uint8_t j=0;j<MAX_ENTRADAS;j++) transiciones[i].valorEntradas[j]=NO_CONFIGURADO;
    }
    
  //leo la configuracion del fichero
  if(!recuperaDatosMaquinaEstados(debugGlobal)) Serial.println("Configuracion de la maquina de estados por defecto");
  else
    { 
      
    }  
  }

/************************************************/
/* Lee el fichero de configuracion de la        */
/* maquina de estados o genera conf por defecto */
/************************************************/
boolean recuperaDatosMaquinaEstados(int debug)
  {
  String cad="";

  if (debug) Serial.println("Recupero configuracion de archivo...");

  if(!leeFicheroConfig(MAQUINAESTADOS_CONFIG_FILE, cad)) 
    {
    //Confgiguracion por defecto
    Serial.printf("No existe fichero de configuracion de la maquina de estados\n");    
    cad="{\"Estados\":[],\"Transiciones\":[] }";
    //salvo la config por defecto
    //if(salvaFicheroConfig(MAQUINAESTADOS_CONFIG_FILE, MAQUINAESTADOS_CONFIG_BAK_FILE, cad)) Serial.printf("Fichero de configuracion de la maquina de estados creado por defecto\n");
    }      
  return parseaConfiguracionMaqEstados(cad);
  }

/*********************************************/
/* Parsea el json leido del fichero de       */
/* configuracio de la maquina de estados     */
/*
/*********************************************/
boolean parseaConfiguracionMaqEstados(String contenido)
  {
  DynamicJsonBuffer jsonBuffer;
  JsonObject& json = jsonBuffer.parseObject(contenido.c_str());

  json.printTo(Serial);
  if (!json.success()) return false;

  Serial.println("\nparsed json");
//******************************Parte especifica del json a leer********************************
  if(!json.containsKey("Estados"))  return false; 
  if(!json.containsKey("Transiciones"))  return false; 
  if(!json.containsKey("Entradas"))  return false; 
  if(!json.containsKey("Salidas"))  return false; 

  /********************Entradas******************************/
  JsonArray& E = json["Entradas"];
  numeroEntradas=(E.size()<MAX_ENTRADAS?E.size():MAX_ENTRADAS);
  for(uint8_t i=0;i<numeroEntradas;i++) mapeoEntradas[i]=E[i];   

  Serial.printf("Entradas asociadas a la maquina de estados: %i\n",numeroEntradas);
  for(uint8_t i=0;i<numeroEntradas;i++) Serial.printf("orden %i | id general %i\n", i,mapeoEntradas[i]);
  
  /********************Salidas******************************/
  JsonArray& S = json["Salidas"];
  numeroSalidas=(S.size()<MAX_SALIDAS?S.size():MAX_SALIDAS);
  for(uint8_t i=0;i<numeroSalidas;i++) mapeoSalidas[i]=S[i];   

  Serial.printf("Salidas asociadas a la maquina de estados: %i\n",numeroSalidas);
  for(uint8_t i=0;i<numeroSalidas;i++) Serial.printf("orden %i | id general %i\n", i,mapeoSalidas[i]);
  
  /********************Estados******************************/
  JsonArray& Estados = json["Estados"];

  numeroEstados=(Estados.size()<MAX_ESTADOS?Estados.size():MAX_ESTADOS);
  for(int8_t i=0;i<numeroEstados;i++)
    { 
    JsonObject& est = Estados[i];   
    estados[i].id=est["id"]; 
    estados[i].nombre=String((const char *)est["nombre"]);

    JsonArray& Salidas = est["salidas"];
    int8_t num_salidas;
    num_salidas=(Salidas.size()<MAX_SALIDAS?Salidas.size():MAX_SALIDAS);
    if(num_salidas!=numeroSalidas) 
      {
      Serial.printf("Numero de salidas incorrecto en estado %i. definidas %i, esperadas %i\n",i,num_salidas,numeroSalidas);
      return false;
      }
    
    for(int8_t s=0;s<num_salidas;s++) estados[i].salidasAsociadas[s]=Salidas[s]["valor"];
    }

  Serial.printf("*************************\nEstados:\n"); 
  Serial.printf("Se han definido %i estados\n",numeroEstados);
  for(int8_t i=0;i<numeroEstados;i++) 
    {
    Serial.printf("%01i: id= %i| nombre: %s\n",i,estados[i].id,estados[i].nombre.c_str());
    Serial.printf("salidas:\n");
    for(int8_t s=0;s<numeroSalidas;s++) 
      {
      Serial.printf("salida[%02i]: valor: %i\n",s,estados[i].salidasAsociadas[s]);
      }
    }
  Serial.printf("*************************\n");  
  
  /********************Transiciones******************************/
  JsonArray& Transiciones = json["Transiciones"];

  numeroTransiciones=(Transiciones.size()<MAX_TRANSICIONES?Transiciones.size():MAX_TRANSICIONES);
  for(int8_t i=0;i<numeroTransiciones;i++)
    { 
    JsonObject& trans = Transiciones[i];   
    transiciones[i].estadoInicial=trans["inicial"]; 
    transiciones[i].estadoFinal=trans["final"];
    
    JsonArray& Entradas = trans["entradas"];   
    int8_t num_entradas;
    num_entradas=(Entradas.size()<MAX_ENTRADAS?Entradas.size():MAX_ENTRADAS);
    if(num_entradas!=numeroEntradas) 
      {
      Serial.printf("Numero de entradas incorrecto en estado %i. definidas %i, esperadas %i\n",i,num_entradas,numeroEntradas);
      return false;
      }
    
    for(int8_t e=0;e<num_entradas;e++) transiciones[i].valorEntradas[e]=atoi(Entradas[e]["valor"]);//Puede ser -1, significa que no importa el valor
    }

  Serial.printf("*************************\nTransiciones:\n"); 
  Serial.printf("Se han definido %i transiciones\n",numeroTransiciones);
  for(int8_t i=0;i<numeroTransiciones;i++) 
    {
    Serial.printf("%01i: estado inicial= %i| estado final: %i\n",i,transiciones[i].estadoInicial,transiciones[i].estadoFinal);
    Serial.printf("entradas:\n");
    for(int8_t e=0;e<numeroEntradas;e++) 
      {
      Serial.printf("entradas[%02i]: valor: %i\n",e,transiciones[i].valorEntradas[e]);
      }
    }
  Serial.printf("*************************\n");  
//************************************************************************************************
  return true; 
  }
/**********************************************************Fin configuracion******************************************************************/  

/*********************************************************MAQUINA DE ESTADOS******************************************************************/    
/****************************************************/
/* Analiza el estado de la maquina y evoluciona     */
/* los estados y las salidas asociadas              */
/****************************************************/
void actualizaMaquinaEstados(int debug=false);
void actualizaMaquinaEstados(int debug)
  {
  boolean localDebug=debug || debugMaquinaEstados;
    
  //Actualizo el vaor de las entradas
  for(uint8_t i=0;i<numeroEntradas;i++) entradasActual[i]=estadoLogicoEntrada(mapeoEntradas[i]);

  if(localDebug) 
    {
    Serial.printf("Estado inicial: (%i) %s\n",estadoActual,estados[estadoActual].nombre.c_str());
    Serial.printf("Estado de las entradas:\n");
    for(uint8_t i=0;i<numeroEntradas;i++) Serial.printf("Entrada %i (dispositivo %i)=> valor %i\n",i, mapeoEntradas[i],entradasActual[i]);
    }
    
  //busco en las transiciones a que estado debe evolucionar la maquina
  estadoActual=mueveMaquina(estadoActual, entradasActual, localDebug);

  //Actualizo las salidas segun el estado actual
  actualizaSalidasMaquinaEstados(estadoActual);

  if(localDebug) Serial.printf("Estado actual: (%i) %s\n",estadoActual,estados[estadoActual].nombre.c_str());
  }

/****************************************************/
/* busco en las transiciones a que estado debe      */
/* evolucionar la maquina                           */
/****************************************************/
uint8_t mueveMaquina(uint8_t estado, int8_t entradasActual[], boolean debug=false);
uint8_t mueveMaquina(uint8_t estado, int8_t entradasActual[], boolean debug)
  {
  for(uint8_t i=0;i<numeroTransiciones;i++) //las reglas se evaluan por orden
    {
    if(transiciones[i].estadoInicial==estado)//Solo analizo las que tienen como estado inicial el indicado
      {
      if(debug) Serial.printf("Revisando regla %i\n",i);
  
      boolean coinciden=true;  
      for(uint8_t j=0;j<numeroEntradas;j++) 
        {
        if (transiciones[i].valorEntradas[j]!=NO_CONFIGURADO) coinciden=coinciden &&(entradasActual[j]==transiciones[i].valorEntradas[j]);
        if(debug) Serial.printf("Revisando entradas %i de regla %i (actual: %i vs regla: %i). Resultado %i\n",i,j,entradasActual[j],transiciones[i].valorEntradas[j],coinciden);
        }

      if(coinciden) return transiciones[i].estadoFinal;
      }
    }
  return ESTADO_ERROR;  //Si no coincide ninguna regla, pasa a estado error
  }
  
/****************************************************/
/* Actualizo las salidas segun el estado actual     */
/****************************************************/
int8_t actualizaSalidasMaquinaEstados(uint8_t estado)
  {
  int8_t retorno=1; //si todo va bien salidaMaquinaEstados devuelve 1, si hay error -1 
  for(uint8_t i=0;i<numeroSalidas;i++) retorno *= salidaMaquinaEstados(mapeoSalidas[i], estados[estado].salidasAsociadas[i]);

  return retorno;
  }


/****************************************************/
/* Funciones de consulta de dataos (encapsulan)     */
/****************************************************/
uint8_t getNumEstados(void){return numeroEstados;}
uint8_t getNumTransiciones(void){return numeroTransiciones;}
uint8_t getNumEntradasME(void){return numeroEntradas;}
uint8_t getNumSalidasME(void){return numeroSalidas;}
