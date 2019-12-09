/*****************************************/
/*                                       */
/*  Control de entradas y salidas        */
/*                                       */
/*****************************************/

//Definicion de pines
#define MAX_PINES        7 //numero de pines disponibles para entradas y salidas
#define MAX_ENTRADAS     4 //numero maximo de reles soportado
#define MAX_RELES        MAX_PINES-MAX_ENTRADAS //numero maximo de salidas

#define ANCHO_PULSO 1*1000 //Ancho del pulso en milisegundos

#ifndef NO_CONFIGURADO 
#define NO_CONFIGURADO -1
#endif

#ifndef CONFIGURADO 
#define CONFIGURADO     1
#endif

#define TOPIC_MENSAJES "mensajes"

//definicion de los tipos de dataos para las entradas y salidas
//Salidas
typedef struct{
  int8_t configurado;     //0 si el rele no esta configurado, 1 si lo esta
  String nombre;          //nombre configurado para el rele
  int8_t estado;          //1 activo, 0 no activo (respecto a nivelActivo)
  int8_t pin;             // Pin al que esta conectado el rele
  int8_t secuenciador;    //1 si esta asociado a un secuenciador que controla la salida, 0 si no esta asociado
  unsigned long finPulso; //fin en millis del pulso para la activacion de ancho definido
  int8_t inicio;          // modo inicial del rele "on"-->1/"off"-->0
  }rele_t; 
rele_t reles[MAX_RELES];

//Entradas
typedef struct{
  int8_t configurada;
  String nombre;
  int8_t estado;
  String tipo;        //Puede ser INPUT, INPUT_PULLUP, No valido!!-->INPUT_PULLDOWN
  int8_t pin;
  String nombreEstados[2];  //Son entradas binarias, solo puede haber 2 mensajes. El 0 nombre del estado en valor 0 y el 1 nombre del estado en valor 1
  String mensajes[2]; //Son entradas binarias, solo puede haber 2 mensajes. El 0 cuando pasa a 0 y el 1 cuando pasa a 1
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
    entradas[i].configurada=NO_CONFIGURADO ;//la inicializo a no configurada
    entradas[i].nombre="No configurado";
    entradas[i].estado=0;    
    entradas[i].tipo="INPUT";
    entradas[i].pin=-1;
    entradas[i].nombreEstados[0]="0";
    entradas[i].nombreEstados[1]="1";
    entradas[i].mensajes[0]="";
    entradas[i].mensajes[1]="";
    }

  //leo la configuracion del fichero
  if(!recuperaDatosEntradas(debugGlobal)) Serial.println("Configuracion de los reles por defecto");
  else
    { 
    //Entradas
    for(int8_t i=0;i<MAX_ENTRADAS;i++)
      {
      if(entradas[i].configurada==CONFIGURADO)
        {  
        Serial.printf("Nombre entrada[%i]=%s | pin entrada: %i | tipo: %s\n",i,entradas[i].nombre.c_str(),entradas[i].pin,entradas[i].tipo.c_str());
        if(entradas[i].tipo=="INPUT_PULLUP") pinMode(entradas[i].pin, INPUT_PULLUP);
        else if(entradas[i].tipo=="INPUT_PULLDOWN") pinMode(entradas[i].pin, INPUT_PULLDOWN);
        else pinMode(entradas[i].pin, INPUT); //PULLUP
        }
      }
    }  
  }

/*********************************************/
/* Inicializa los valores de los registros de*/
/* las salidas y recupera la configuracion   */
/*********************************************/
void inicializaSalidas(void)
  {  
  //Salidas
  for(int8_t i=0;i<MAX_RELES;i++)
    {
    //inicializo la parte logica
    reles[i].configurado=NO_CONFIGURADO;
    reles[i].nombre="No configurado";
    reles[i].estado=0;
    reles[i].pin=-1;
    reles[i].secuenciador=NO_CONFIGURADO;
    reles[i].finPulso=0;
    reles[i].inicio=0;
    }
         
  //leo la configuracion del fichero
  if(!recuperaDatosSalidas(debugGlobal)) Serial.println("Configuracion de los reles por defecto");
  else
    {  
    //Salidas
    for(int8_t i=0;i<MAX_RELES;i++)
      {    
      if(reles[i].configurado==CONFIGURADO) 
        {   
        pinMode(reles[i].pin, OUTPUT); //es salida

        //parte logica
        reles[i].estado=reles[i].inicio;  
        //parte fisica
        if(reles[i].inicio==1) digitalWrite(reles[i].pin, nivelActivo);  //lo inicializo a apagado
        else digitalWrite(reles[i].pin, !nivelActivo);  //lo inicializo a apagado 
        
        Serial.printf("Nombre rele[%i]=%s | pin rele: %i | inicio: %i\n",i,reles[i].nombre.c_str(),reles[i].pin,reles[i].inicio);
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

  if (debug) Serial.println("Recupero configuracion de archivo...");

  if(!leeFicheroConfig(ENTRADAS_CONFIG_FILE, cad)) 
    {
    //Confgiguracion por defecto
    Serial.printf("No existe fichero de configuracion de Entradas\n");    
    cad="{\"Entradas\": []}";
    //salvo la config por defecto
    //if(salvaFicheroConfig(ENTRADAS_CONFIG_FILE, ENTRADAS_CONFIG_BAK_FILE, cad)) Serial.printf("Fichero de configuracion de Entradas creado por defecto\n");
    }      
  return parseaConfiguracionEntradas(cad);
  }

/*********************************************/
/* Lee el fichero de configuracion de las    */
/* salidas o genera conf por defecto         */
/*********************************************/
boolean recuperaDatosSalidas(int debug)
  {
  String cad="";

  if (debug) Serial.println("Recupero configuracion de archivo...");
  
  if(!leeFicheroConfig(SALIDAS_CONFIG_FILE, cad)) 
    {
    //Confgiguracion por defecto
    Serial.printf("No existe fichero de configuracion de Salidas\n");    
    cad="{\"Salidas\": []}";
    //salvo la config por defecto
    //if(salvaFicheroConfig(SALIDAS_CONFIG_FILE, SALIDAS_CONFIG_BAK_FILE, cad)) Serial.printf("Fichero de configuracion de Salidas creado por defecto\n");
    }      
    
  return parseaConfiguracionSalidas(cad);
  }

/*********************************************/
/* Parsea el json leido del fichero de       */
/* configuracio de las entradas              */
/*********************************************/
boolean parseaConfiguracionEntradas(String contenido)
  {
  DynamicJsonBuffer jsonBuffer;
  JsonObject& json = jsonBuffer.parseObject(contenido.c_str());

  json.printTo(Serial);
  if (!json.success()) return false;

  Serial.println("\nparsed json");
//******************************Parte especifica del json a leer********************************
  JsonArray& Entradas = json["Entradas"];

  int8_t max;
  max=(Entradas.size()<MAX_ENTRADAS?Entradas.size():MAX_ENTRADAS);
  for(int8_t i=0;i<max;i++)
    { 
    entradas[i].configurada=CONFIGURADO; //Cambio el valor para configurarla  
    entradas[i].nombre=String((const char *)Entradas[i]["nombre"]);
    entradas[i].tipo=String((const char *)Entradas[i]["tipo"]);
    entradas[i].pin=atoi(Entradas[i]["GPIO"]);
    JsonObject& entrada = json["Entradas"][i];
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

  Serial.printf("Entradas:\n"); 
  for(int8_t i=0;i<MAX_ENTRADAS;i++) 
    {
    Serial.printf("%02i: %s| pin: %i| configurado= %i| tipo=%s\n",i,entradas[i].nombre.c_str(),entradas[i].pin,entradas[i].configurada,entradas[i].tipo.c_str());
    Serial.printf("Mensajes:\n");
    for(int8_t m=0;m<2;m++) 
      {
      Serial.printf("Mensaje[%02i]: %s\n",m,entradas[i].mensajes[m].c_str());     
      }    
    }
//************************************************************************************************
  return true; 
  }

/*********************************************/
/* Parsea el json leido del fichero de       */
/* configuracio de las salidas               */
/*********************************************/
boolean parseaConfiguracionSalidas(String contenido)
  { 
  DynamicJsonBuffer jsonBuffer;
  JsonObject& json = jsonBuffer.parseObject(contenido.c_str());
  
  json.printTo(Serial);
  if (!json.success()) return false;
        
  Serial.println("\nparsed json");
//******************************Parte especifica del json a leer********************************
  JsonArray& Salidas = json["Salidas"];

  int8_t max;
  max=(Salidas.size()<MAX_RELES?Salidas.size():MAX_RELES);
  for(int8_t i=0;i<max;i++)
    { 
    reles[i].configurado=CONFIGURADO;//lo marco como configurado
    reles[i].nombre=String((const char *)Salidas[i]["nombre"]);//Pongo el nombre que indoca el fichero
    reles[i].pin=atoi(Salidas[i]["GPIO"]);
    //Si de inicio debe estar activado o desactivado
    if(String((const char *)Salidas[i]["inicio"])=="on") reles[i].inicio=1;
    else reles[i].inicio=0;   
    }
    
  Serial.printf("Salidas:\n"); 
  for(int8_t i=0;i<MAX_RELES;i++) Serial.printf("%01i: %s| pin: %i| configurado= %i\n",i,reles[i].nombre.c_str(),reles[i].pin,reles[i].configurado); 
//************************************************************************************************
  return true; 
  }
/**********************************************************Fin configuracion******************************************************************/  

/**********************************************************SALIDAS******************************************************************/    
/*************************************************/
/*Logica de los reles:                           */
/*Si esta activo para ese intervalo de tiempo(1) */
/*Si esta por debajo de la tMin cierro rele      */
/*si no abro rele                                */
/*************************************************/
void actualizaSalidas(bool debug)
  {
  for(int8_t id=0;id<MAX_RELES;id++)
    {
    if (reles[id].configurado==CONFIGURADO && reles[id].estado==2) //esta configurado y pulsando
      {
      if(reles[id].finPulso>ANCHO_PULSO)//el contador de millis no desborda durante el pulso
        {
        if(millis()>=reles[id].finPulso) //El pulso ya ha acabado
          {
          conmutaRele(id,!nivelActivo,debugGlobal);  
          Serial.printf("Fin del pulso. millis()= %i\n",millis());
          }//del if del fin de pulso
        }//del if de desboda
      else //El contador de millis desbordar durante el pulso
        {
        if(UINT64_MAX-ANCHO_PULSO>millis())//Ya ha desbordado
          {
          if(millis()>=reles[id].finPulso) 
            {
            conmutaRele(id,!nivelActivo,debugGlobal);
            Serial.printf("Fin del pulso. millis()= %i\n",millis());
            }//del if del fin de pulso
          }//del if ha desbordado ya
        }//else del if de no desborda
      }//del if configurado
    }//del for    
  }//de la funcion

/*************************************************/
/*                                               */
/*  Devuelve el estado  del rele indicado en id  */
/*  puede ser 0 apagado, 1 encendido, 2 pulsando */
/*                                               */
/*************************************************/
int8_t estadoRele(int8_t id)
  {
  if(id <0 || id>=MAX_RELES) return NO_CONFIGURADO; //Rele fuera de rango
  if(reles[id].configurado!=CONFIGURADO) return -1; //No configurado
  
  return reles[id].estado;
 }

/********************************************************/
/*                                                      */
/*  Devuelve el nombre del rele con el id especificado  */
/*                                                      */
/********************************************************/
String nombreRele(int8_t id)
  { 
  if(id <0 || id>=MAX_RELES) return "ERROR"; //Rele fuera de rango    
  return reles[id].nombre;
  } 

/*************************************************/
/*conmuta el rele indicado en id                 */
/*devuelve 1 si ok, -1 si ko                     */
/*************************************************/
int8_t conmutaRele(int8_t id, boolean estado_final, int debug)
  {
  //validaciones previas
  if(id <0 || id>=MAX_RELES) return NO_CONFIGURADO; //Rele fuera de rango
  if(reles[id].configurado==NO_CONFIGURADO) return -1; //El rele no esta configurado
  
  //parte logica
  if(estado_final==nivelActivo) reles[id].estado=1;
  else reles[id].estado=0;
  
  //parte fisica
  digitalWrite(reles[id].pin, estado_final); //controlo el rele
  
  if(debug)
    {
    Serial.printf("id: %i; GPIO: %i; estado: ",(int)id,(int)reles[id].pin);
    Serial.println(digitalRead(reles[id].pin));
    }
    
  return 1;
  }

/****************************************************/
/*   Genera un pulso en rele indicado en id         */
/*   devuelve 1 si ok, -1 si ko                     */
/****************************************************/
int8_t pulsoRele(int8_t id)
  {
  //validaciones previas
  if(id <0 || id>=MAX_RELES) return NO_CONFIGURADO;
      
  //Pongo el rele en nivel Activo  
  if(!conmutaRele(id, nivelActivo, debugGlobal)) return 0; //Si no puede retorna -1

  //cargo el campo con el valor definido para el ancho del pulso
  reles[id].estado=2;//estado EN_PULSO
  reles[id].finPulso=millis()+ANCHO_PULSO; 

  Serial.printf("Incio de pulso %i| fin calculado %i\n",millis(),reles[id].finPulso);
  
  return 1;  
  }

/********************************************************/
/*                                                      */
/*     Recubre las dos funciones anteriores para        */
/*     actuar sobre un rele                             */
/*                                                      */
/********************************************************/ 
int8_t actuaRele(int8_t id, int8_t estado)
  {
  switch(estado)
    {
    case 0:
      return conmutaRele(id, !nivelActivo, debugGlobal);
      break;
    case 1:
      return conmutaRele(id, nivelActivo, debugGlobal);
      break;
    case 2:
      return pulsoRele(id);
      break;      
    default://no deberia pasar nunca!!
      return -1;
    }
  }

/********************************************************/
/*                                                      */
/*     Devuelve si el reles esta configurados           */
/*                                                      */
/********************************************************/ 
int releConfigurado(uint8_t id)
  {
  if(id <0 || id>=MAX_RELES) return NO_CONFIGURADO;
    
  return reles[id].configurado;
  } 
  
/********************************************************/
/*                                                      */
/*     Devuelve el numero de reles configurados         */
/*                                                      */
/********************************************************/ 
int relesConfigurados(void)
  {
  int resultado=0;
  
  for(int8_t i=0;i<MAX_RELES;i++)
    {
    if(reles[i].configurado==CONFIGURADO) resultado++;
    }
  return resultado;
  } 

/********************************************************/
/*                                                      */
/*     Asocia la salida a un plan de secuenciador       */
/*                                                      */
/********************************************************/ 
void asociarSecuenciador(int8_t id, int8_t plan)
  {
  //validaciones previas
  if(id >=0 && id<MAX_RELES) reles[id].secuenciador=plan; 
  }  

/********************************************************/
/*                                                      */
/*     Devuelve si la salida esta asociada              */
/*     a un plan de secuenciador                        */
/*                                                      */
/********************************************************/ 
int8_t asociadaASecuenciador(int8_t id)
  {
  //validaciones previas
  if(id <0 || id>=MAX_RELES) return NO_CONFIGURADO;
      
  return reles[id].secuenciador;  
  }   
/********************************************************** Fin salidas ******************************************************************/  
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
      int8_t valor_inicial=  entradas[i].estado;
      entradas[i].estado=digitalRead(entradas[i].pin); //si la entrada esta configurada
      
      if(valor_inicial!=entradas[i].estado) enviaMensajeEntrada(i,entradas[i].estado);
      }
    }   
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
  Serial.printf("Envia mensaje para la entrada con id %i y por cambiar a estado %i. Mensaje: %s\n\n",id_entrada,estado,entradas[id_entrada].mensajes[estado].c_str());

  mensaje="{\"origen\": \"" + entradas[id_entrada].nombre + "\",\"mensaje\":\"" + entradas[id_entrada].mensajes[estado] + "\"}";
  
  enviarMQTT(TOPIC_MENSAJES, mensaje);
  }

/*************************************************/
/*                                               */
/*Devuelve el estado 0|1 del rele indicado en id */
/*                                               */
/*************************************************/
int8_t estadoEntrada(int8_t id)
  {
  if(id <0 || id>=MAX_ENTRADAS) return NO_CONFIGURADO; //Rele fuera de rango
  
  if(entradas[id].configurada==CONFIGURADO) return (entradas[id].estado); //si la entrada esta configurada
  else return NO_CONFIGURADO;
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
String generaJsonEstadoSalidas(void)
  {
  String salida="";

  const size_t bufferSize = JSON_ARRAY_SIZE(2) + JSON_OBJECT_SIZE(1) + 2*JSON_OBJECT_SIZE(3);
  DynamicJsonBuffer jsonBuffer(bufferSize);
  
  JsonObject& root = jsonBuffer.createObject();
  
  JsonArray& Salidas = root.createNestedArray("Salidas");
  for(int8_t id=0;id<MAX_RELES;id++)
    {
    if(reles[id].configurado==CONFIGURADO)
      {
      JsonObject& Salidas_0 = Salidas.createNestedObject();
      Salidas_0["id"] = id;
      Salidas_0["nombre"] = reles[id].nombre;
      Salidas_0["valor"] = reles[id].estado;    
      }
    }
    
  root.printTo(salida);
  return salida;  
  }  

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
String generaJsonEstadoEntradas(void)
  {
  String salida="";

  const size_t bufferSize = JSON_ARRAY_SIZE(2) + JSON_OBJECT_SIZE(1) + 2*JSON_OBJECT_SIZE(3);
  DynamicJsonBuffer jsonBuffer(bufferSize);
  
  JsonObject& root = jsonBuffer.createObject();
  
  JsonArray& Entradas = root.createNestedArray("Entradas");
  for(int8_t id=0;id<MAX_ENTRADAS;id++)
    {
    if(entradas[id].configurada==CONFIGURADO)
      {
      JsonObject& Entradas_0 = Entradas.createNestedObject();
      Entradas_0["id"] = id;
      Entradas_0["nombre"] = entradas[id].nombre;
      Entradas_0["valor"] = entradas[id].estado;
      }
    }

  root.printTo(salida);
  return salida;  
  }

/********************************************************/
/*                                                      */
/*   Devuelve el estado de los reles en formato json    */
/*   devuelve un json con el formato:                   */
/* {
  "Entradas": [ 
    {"id":  "0", "nombre": "P. abierta", "valor": "1" },
    {"id":  "1", "nombre": "P. cerrada", "valor": "0" }
  ],
  "Salidas": [  
    {"id":  "0", "nombre": "P. abierta", "valor": "1" },
    {"id":  "1", "nombre": "P. cerrada", "valor": "0" }
  ]
}
                                                        */
/********************************************************/   
String generaJsonEstado(void)
  {
  String salida="";

  const size_t bufferSize = 2*JSON_ARRAY_SIZE(2) + JSON_OBJECT_SIZE(2) + 4*JSON_OBJECT_SIZE(3);
  DynamicJsonBuffer jsonBuffer(bufferSize);
  
  JsonObject& root = jsonBuffer.createObject();
  
  JsonArray& Entradas = root.createNestedArray("Entradas");
  for(int8_t id=0;id<MAX_ENTRADAS;id++)
    {
    if(entradas[id].configurada==CONFIGURADO)
      {
      JsonObject& Entradas_0 = Entradas.createNestedObject();
      Entradas_0["id"] = id;
      Entradas_0["nombre"] = entradas[id].nombre;
      Entradas_0["valor"] = entradas[id].estado;
      }
    }

  JsonArray& Salidas = root.createNestedArray("Salidas");
  for(int8_t id=0;id<MAX_RELES;id++)
    {
    if(reles[id].configurado==CONFIGURADO)
      {
      JsonObject& Salidas_0 = Salidas.createNestedObject();
      Salidas_0["id"] = id;
      Salidas_0["nombre"] = reles[id].nombre;
      Salidas_0["valor"] = reles[id].estado;    
      }
    }

  root.printTo(salida);  
  return salida;  
  }   
