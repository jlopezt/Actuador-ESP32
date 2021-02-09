/*****************************************/
/*                                       */
/*  Control de entradas y salidas        */
/*                                       */
/*****************************************/

/***************************** Defines *****************************/
/***************************** Defines *****************************/

/***************************** Includes *****************************/
#include <Entradas.h>
//#include <Salidas.h>

#include <MQTT.h>
#include <Ficheros.h>
/***************************** Includes *****************************/

//definicion de los tipos de dataos para las entradas
//Entradas
typedef struct{
  int8_t configurada;       //Si la entrada esta configurada o no
  String nombre;            //Nombre de la entrada
  int8_t estadoFisico;      //leido ESTADO_ACTIVO o ESTADO_DESACTIVO
  int8_t estado;            //Estado logico asignado despues de leer el valo fisico
  int8_t estadoActivo;      //Valor logico de referencia, se lee de la configuracion
  int8_t pin;               //Pin fisico alque esta conectada
  String tipo;              //Puede ser INPUT, INPUT_PULLUP, No valido!!-->INPUT_PULLDOWN
  String nombreEstados[2];  //Son entradas binarias, solo puede haber 2 mensajes. El 0 nombre del estado en valor 0 y el 1 nombre del estado en valor 1
  String mensajes[2];       //Son entradas binarias, solo puede haber 2 mensajes. El 0 cuando pasa a 0 y el 1 cuando pasa a 1
  }entrada_t; 
entrada_t entradas[MAX_ENTRADAS];
/************************************** Funciones de configuracion ****************************************/
/*********************************************/
/* Inicializa los valores de los registros de*/
/* las entradas y recupera la configuracion  */
/*********************************************/
void inicializaEntradas(void)
  {  
  //Entradas
  for(int8_t i=0;i<MAX_ENTRADAS;i++)
    {
    //inicializo la parte logica
    entradas[i].configurada=NO_CONFIGURADO;  //la inicializo a no configurada
    entradas[i].nombre="No configurado";
    entradas[i].estado=NO_CONFIGURADO;   
    entradas[i].estadoFisico=ESTADO_DESACTIVO; 
    entradas[i].estadoActivo=ESTADO_ACTIVO;  //NO_CONFIGURADO;
    entradas[i].tipo="INPUT";
    entradas[i].pin=-1;
    entradas[i].nombreEstados[0]="0";
    entradas[i].nombreEstados[1]="1";
    entradas[i].mensajes[0]="";
    entradas[i].mensajes[1]="";
    }

  //leo la configuracion del fichero
  if(!recuperaDatosEntradas(debugGlobal)) Traza.mensaje("Configuracion de los reles por defecto\n");
  else
    { 
    //Entradas
    for(int8_t i=0;i<MAX_ENTRADAS;i++)
      {
      if(entradas[i].configurada==CONFIGURADO)
        {  
        if(entradas[i].tipo=="INPUT_PULLUP") pinMode(entradas[i].pin, INPUT_PULLUP);
        else if(entradas[i].tipo=="INPUT_PULLDOWN") pinMode(entradas[i].pin, INPUT_PULLDOWN);
        else pinMode(entradas[i].pin, INPUT); //PULLUP

        Traza.mensaje("Nombre entrada[%i]=%s | pin entrada: %i | tipo: %s | estado activo %i\n",i,entradas[i].nombre.c_str(),entradas[i].pin,entradas[i].tipo.c_str(),entradas[i].estadoActivo);
        Traza.mensaje("\tEstados y  mensajes:\n");
        for(uint8_t j=0;j<2;j++) Traza.mensaje("\t\tEstado %i: %s | mensaje: %s\n",j,entradas[i].nombreEstados[j].c_str(),entradas[i].mensajes[j].c_str());        
        }
      }
    }  
  }

/*********************************************/
/* Lee el fichero de configuracion de las    */
/* entradas o genera conf por defecto        */
/*********************************************/
boolean recuperaDatosEntradas(int debug)
  {
  String cad="";

  if (debug) Traza.mensaje("Recupero configuracion de archivo...\n");

  if(!leeFichero(ENTRADAS_CONFIG_FILE, cad)) 
    {
    //Confgiguracion por defecto
    Traza.mensaje("No existe fichero de configuracion de Entradas\n");    
    cad="{\"Entradas\": []}";
    //salvo la config por defecto
    //if(salvaFichero(ENTRADAS_CONFIG_FILE, ENTRADAS_CONFIG_BAK_FILE, cad)) Traza.mensaje("Fichero de configuracion de Entradas creado por defecto\n");
    }      
  return parseaConfiguracionEntradas(cad);
  }

/*********************************************/
/* Parsea el json leido del fichero de       */
/* configuracio de las entradas              */
/*********************************************/
boolean parseaConfiguracionEntradas(String contenido)
  {
  DynamicJsonBuffer jsonBuffer;
  JsonObject& json = jsonBuffer.parseObject(contenido.c_str());

  String salida;
  json.printTo(salida);//pinto el json que he creado
  Traza.mensaje("json leido:\n#%s#\n",salida.c_str());
  
  if (!json.success()) return false;

  Traza.mensaje("\nparsed json\n");
//******************************Parte especifica del json a leer********************************
  JsonArray& Entradas = json["Entradas"];

  int8_t max;
  max=(Entradas.size()<MAX_ENTRADAS?Entradas.size():MAX_ENTRADAS);
  for(int8_t i=0;i<max;i++)
    { 
    JsonObject& entrada = json["Entradas"][i];
      
    entradas[i].configurada=CONFIGURADO; //Cambio el valor para configurarla  
    if(entrada.containsKey("nombre")) entradas[i].nombre=entrada.get<String>("nombre"); 
    if(entrada.containsKey("tipo")) entradas[i].tipo=entrada.get<String>("tipo"); 
    if(entrada.containsKey("GPIO")) entradas[i].pin=entrada.get<int>("GPIO"); else return false;
    if(entrada.containsKey("estadoActivo")) entradas[i].estadoActivo=entrada.get<int>("estadoActivo");
    
    if(entrada.containsKey("Estados"))
      {
      int8_t est_max=entrada["Estados"].size();//maximo de mensajes en el JSON
      if (est_max>2) est_max=2;                     //Si hay mas de 2, solo leo 2
      for(int8_t e=0;e<est_max;e++)  
        {
        if (entrada["Estados"][e]["valor"]==e) entradas[i].nombreEstados[e]=String((const char *)entrada["Estados"][e]["texto"]);
        }
      }
    if(entrada.containsKey("Mensajes"))
      {
      int8_t men_max=entrada["Mensajes"].size();//maximo de mensajes en el JSON
      if (men_max>2) men_max=2;                     //Si hay mas de 2, solo leo 2
      for(int8_t m=0;m<men_max;m++)  
        {
        if (entrada["Mensajes"][m]["valor"]==m) entradas[i].mensajes[m]=String((const char *)entrada["Mensajes"][m]["texto"]);
        }
      }
    }

  Traza.mensaje("*************************\nEntradas:\n"); 
  for(int8_t i=0;i<MAX_ENTRADAS;i++) 
    {
  	Traza.mensaje("%01i: %s|pin: %i|configurado= %i|tipo=%s|estado activo: %i\n",i,entradas[i].nombre.c_str(),entradas[i].pin,entradas[i].configurada,entradas[i].tipo.c_str(),entradas[i].estadoActivo);
    Traza.mensaje("Mensajes:\n");
    for(int8_t m=0;m<2;m++) 
      {
      Traza.mensaje("Mensaje[%02i]: %s\n",m,entradas[i].mensajes[m].c_str());     
      }    
    }
  Traza.mensaje("*************************\n");  
//************************************************************************************************
  return true; 
  }

/**********************************************************ENTRADAS******************************************************************/  
/*************************************************/
/*                                               */
/*       Lee el estado de las entradas           */
/*                                               */
/*************************************************/
void consultaEntradas(bool debug)
  {
  //Actualizo las entradas  
  for(int8_t i=0;i<MAX_ENTRADAS;i++)
    {
    if(entradas[i].configurada==CONFIGURADO) 
      {
      int8_t valor_inicial = entradas[i].estado;

      entradas[i].estadoFisico=digitalRead(entradas[i].pin);     
      if(entradas[i].estadoFisico==entradas[i].estadoActivo) entradas[i].estado=ESTADO_ACTIVO;
      else entradas[i].estado=ESTADO_DESACTIVO;
      if (debug) Traza.mensaje("Entrada %i en pin %i leido %i, alor logico %i\n",i,entradas[i].pin,entradas[i].estadoFisico,entradas[i].estado);

      if(valor_inicial!=NO_CONFIGURADO && valor_inicial!=entradas[i].estado) enviaMensajeEntrada(i,entradas[i].estado);      
      }
    }
  }

/*************************************************/
/*                                               */
/*Devuelve el estado 0|1 del rele indicado en id */
/*                                               */
/*************************************************/
int8_t estadoEntrada(int8_t id)
  {
  if(id <0 || id>=MAX_ENTRADAS) return NO_CONFIGURADO; //Rele fuera de rango
  if(entradas[id].configurada!=CONFIGURADO) return NO_CONFIGURADO;
  
  return (entradas[id].estado);
  }

/*************************************************/
/*                                               */
/* Devuelve el estado 0|1 de la entrada tal cual */
/* se ha leido, sin cruzar con nada              */
/*                                               */
/*************************************************/
int8_t estadoFisicoEntrada(int8_t id)
  {
  if(id <0 || id>=MAX_ENTRADAS) return NO_CONFIGURADO; //Rele fuera de rango
  if(entradas[id].configurada!=CONFIGURADO) return NO_CONFIGURADO;
  
  return entradas[id].estadoFisico;
  }

/*************************************************/
/*                                               */
/* Devuelve el estado activo 0|1 de la entrada   */
/*                                               */
/*************************************************/
int8_t estadoActivoEntrada(int8_t id)
  {
  if(id <0 || id>=MAX_ENTRADAS) return NO_CONFIGURADO; //Rele fuera de rango
  if(entradas[id].configurada!=CONFIGURADO) return NO_CONFIGURADO;
  
  return entradas[id].estadoActivo;
  }

/*************************************************/
/*                                               */
/* Envia un mensaje MQTT para que se publique un */
/* audio en un GHN                               */
/*                                               */
/*************************************************/
void enviaMensajeEntrada(int8_t id_entrada, int8_t estado)
  {
  String mensaje="";

  mensaje="{\"origen\": \"" + entradas[id_entrada].nombre + "\",\"mensaje\":\"" + entradas[id_entrada].mensajes[estado] + "\"}";
  Traza.mensaje("Envia mensaje para la entrada con id %i y por cambiar a estado %i. Mensaje: %s\n\n",id_entrada,estado,entradas[id_entrada].mensajes[estado].c_str());
  Traza.mensaje("A enviar: topic %s\nmensaje %s\n", TOPIC_MENSAJES,mensaje.c_str());
  enviarMQTT(TOPIC_MENSAJES, mensaje);
  }

/********************************************************/
/*                                                      */
/*     Devuelve si la entrada esta configurada          */
/*                                                      */
/********************************************************/ 
boolean entradaConfigurada(uint8_t id)
  {
  //validaciones previas
  if(id <0 || id>=MAX_ENTRADAS) return NO_CONFIGURADO;
       
  return (entradas[id].configurada==CONFIGURADO);
  }   

/********************************************************/
/*                                                      */
/*  Devuelve el nombre del rele con el id especificado  */
/*                                                      */
/********************************************************/
String nombreEntrada(int8_t id)
  { 
  if(id <0 || id>=MAX_ENTRADAS) return "ERROR"; //Rele fuera de rango    
  return entradas[id].nombre;
  } 

/********************************************************/
/*                                                      */
/*     Devuelve el pin de la entrada                    */
/*                                                      */
/********************************************************/ 
uint8_t pinEntrada(uint8_t id)
  {
  //validaciones previas
  if(id <0 || id>=MAX_ENTRADAS) return NO_CONFIGURADO;
       
  return entradas[id].pin;
  }   

/********************************************************/
/*                                                      */
/*     Devuelve el tipo de la entrada                   */
/*                                                      */
/********************************************************/ 
String tipoEntrada(uint8_t id)
  {
  //validaciones previas
  if(id <0 || id>=MAX_ENTRADAS) return "ERROR";
       
  return entradas[id].tipo;
  }   

/********************************************************/
/*                                                      */
/*  Devuelve el nombre del estado de una entrada        */
/*                                                      */
/********************************************************/ 
String nombreEstadoEntrada(uint8_t id, uint8_t estado)
  {
  //validaciones previas
  if(id <0 || id>=MAX_ENTRADAS) return "ERROR";
  if(estado>2) return "ERROR";
       
  return entradas[id].nombreEstados[estado];
  }   

/********************************************************/
/*                                                      */
/*  Devuelve el mensaje de una entrada en un estado     */
/*                                                      */
/********************************************************/ 
String mensajeEstadoEntrada(uint8_t id, uint8_t estado)
  {
  //validaciones previas
  if(id <0 || id>=MAX_ENTRADAS) return "ERROR";
  if(estado>2) return "ERROR";
       
  return entradas[id].mensajes[estado];
  }   

/********************************************************/
/*                                                      */
/*  Devuelve el numero de entradas configuradas         */
/*                                                      */
/********************************************************/ 
int entradasConfiguradas(void)
  {
  int resultado=0;
  
  for(int8_t i=0;i<MAX_ENTRADAS;i++)
    {
    if(entradas[i].configurada==CONFIGURADO) resultado++;
    }
  return resultado;
  }
/********************************************* Fin entradas *******************************************************************/
  
/****************************************** Funciones de estado ***************************************************************/
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
String generaJsonEstadoEntradas(boolean filtro)
  {
  String salida="";

  //const size_t bufferSize = JSON_ARRAY_SIZE(2) + JSON_OBJECT_SIZE(1) + 2*JSON_OBJECT_SIZE(3);
  const size_t bufferSize = JSON_ARRAY_SIZE(3) + JSON_OBJECT_SIZE(1) + 3*JSON_OBJECT_SIZE(10);
  DynamicJsonBuffer jsonBuffer(bufferSize);
  
  JsonObject& root = jsonBuffer.createObject();
  
  JsonArray& Entradas = root.createNestedArray("entradas");
  for(int8_t id=0;id<MAX_ENTRADAS;id++)
    {
    if(entradas[id].configurada==CONFIGURADO || !filtro) 
      { 
      JsonObject& Entradas_0 = Entradas.createNestedObject(); 
      Entradas_0["id"] = id;
      Entradas_0["nombre"] = nombreEntrada(id);
      Entradas_0["estado"] = estadoEntrada(id);
      Entradas_0["nombreEstado"] = nombreEstadoEntrada(id,estadoEntrada(id));

      if(!filtro) {
        Entradas_0["configurada"] = entradaConfigurada(id);
        Entradas_0["tipo"] = tipoEntrada(id);
        Entradas_0["pin"] = pinEntrada(id);
        Entradas_0["estadoActivo"] = estadoActivoEntrada(id);
        Entradas_0["estadoFisico"] = estadoFisicoEntrada(id);
        Entradas_0["mensajeEstado"] = mensajeEstadoEntrada(id,estadoEntrada(id));
        }
      }
    }

  root.printTo(salida);
  return salida;  
  }
String generaJsonEstadoEntradas(void){return generaJsonEstadoEntradas(true);}