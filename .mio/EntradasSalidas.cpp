/*****************************************/
/*                                       */
/*  Control de entradas y salidas        */
/*                                       */
/*****************************************/

/***************************** Defines *****************************/
/***************************** Defines *****************************/

/***************************** Includes *****************************/
#include <EntradasSalidas.h>
#include <MQTT.h>
#include <Ficheros.h>
/***************************** Includes *****************************/

//definicion de los tipos de dataos para las entradas y salidas
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

//Salidas
typedef struct{
  int8_t configurado;       //0 si el rele no esta configurado, 1 si lo esta
  String nombre;            //nombre configurado para el rele
  int8_t estado;            //1 activo, 0 no activo (respecto a nivelActivo), modo pulso y modo maquina
  int8_t modo;              //0: manual, 1: secuanciador, 2: seguimiento
  int8_t modo_inicial;      //Modo incial, cuando se fuerza a manual este no cambia y cuando vuelve, se recupera de aqui  
  int8_t pin;               // Pin al que esta conectado el rele
  int8_t tipo;              //Tipo dgital normal o LED(PWM)
  int16_t valorPWM;         //Valor de la salida en PWM
  int8_t canal;             //Canal de PWM al que se va a a asociar el pin
  int16_t frecuencia;       //Frecuencia de trabajo del PWM para ese canal
  int8_t resolucion;        //Resolucion en bits del PWM  
  int16_t anchoPulso;       // Ancho en milisegundos del pulso para esa salida
  int8_t controlador;       //1 si esta asociado a un secuenciador que controla la salida, 0 si no esta asociado
  unsigned long finPulso;   //fin en millis del pulso para la activacion de ancho definido
  int8_t inicio;            // modo inicial del rele "on"-->1/"off"-->0
  String nombreEstados[2];  //Son salidas binarias, solo puede haber 2 mensajes. El 0 nombre del estado en valor 0 y el 1 nombre del estado en valor 1
  String mensajes[2];       //Son salidas binarias, solo puede haber 2 mensajes. El 0 cuando pasa a 0 y el 1 cuando pasa a 1
  }salida_t; 
salida_t salidas[MAX_SALIDAS];

int8_t salidaActiva=-1;
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
/* Inicializa los valores de los registros de*/
/* las salidas y recupera la configuracion   */
/*********************************************/
void inicializaSalidas(void)
  {  
  //Salidas
  for(int8_t i=0;i<MAX_SALIDAS;i++)
    {
    //inicializo la parte logica
    salidas[i].configurado=NO_CONFIGURADO;
    salidas[i].nombre="No configurado";
    salidas[i].estado=NO_CONFIGURADO;
    salidas[i].modo=NO_CONFIGURADO;
    salidas[i].modo_inicial=NO_CONFIGURADO;
    salidas[i].pin=-1;
    salidas[i].tipo=TIPO_DIGITAL;
    salidas[i].valorPWM=0;
    salidas[i].canal=-1;
    salidas[i].frecuencia=FRECUENCIA_PWM;
    salidas[i].resolucion=RESOLUCION_PWM;
    salidas[i].anchoPulso=0;
    salidas[i].controlador=NO_CONFIGURADO;
    salidas[i].finPulso=0;
    salidas[i].inicio=0;
    salidas[i].nombreEstados[0]="0";
    salidas[i].nombreEstados[1]="1";    
    salidas[i].mensajes[0]="";
    salidas[i].mensajes[1]="";
    }
         
  //leo la configuracion del fichero
  if(!recuperaDatosSalidas(debugGlobal)) Traza.mensaje("Configuracion de los reles por defecto\n");
  else
    {  
    //Salidas
    for(int8_t i=0;i<MAX_SALIDAS;i++)
      {    
      if(salidas[i].configurado==CONFIGURADO) 
        {   
        //Guardo el modo para recuperarlo si se pasa a manual
        salidas[i].modo_inicial=salidas[i].modo;
        
        //parte logica
        salidas[i].estado=salidas[i].inicio;  
        
        //parte fisica
        switch (salidas[i].tipo)
          {
          case TIPO_DIGITAL:
            pinMode(salidas[i].pin, OUTPUT); //es salida
            break;  
          case TIPO_LED:
            ledcSetup(salidas[i].canal,salidas[i].frecuencia,salidas[i].resolucion);
            ledcAttachPin(salidas[i].pin,salidas[i].canal);
            break;
          }

        //Lo inicializo segun lo configurado               
        if(salidas[i].inicio==1) 
          {
          switch (salidas[i].tipo)
            {
            case TIPO_DIGITAL:
              digitalWrite(salidas[i].pin, nivelActivo);  
              break;
            case TIPO_LED:
              if(nivelActivo==1) ledcWrite(salidas[i].canal,salidas[i].valorPWM);
              else ledcWrite(salidas[i].canal,(1<<RESOLUCION_PWM)-salidas[i].valorPWM);            
              break;
            }
          }
        else 
          {
          switch (salidas[i].tipo)
            {
            case TIPO_DIGITAL:
              digitalWrite(salidas[i].pin, !nivelActivo); 
              break;
            case TIPO_LED:
              if(nivelActivo==1) ledcWrite(salidas[i].canal,0);
              else ledcWrite(salidas[i].canal,(1<<RESOLUCION_PWM));            
              break;
            }          
          }
        
        Traza.mensaje("Nombre salida[%i]=%s | pin salida: %i | estado= %i | inicio: %i | modo: %i\n",i,salidas[i].nombre.c_str(),salidas[i].pin,salidas[i].estado,salidas[i].inicio, salidas[i].modo);
        Traza.mensaje("\tEstados y  mensajes:\n");
        for(uint8_t j=0;j<2;j++) Traza.mensaje("\t\tEstado %i: %s | mensaje: %s\n",j,salidas[i].nombreEstados[j].c_str(),salidas[i].mensajes[j].c_str());
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
/* Lee el fichero de configuracion de las    */
/* salidas o genera conf por defecto         */
/*********************************************/
boolean recuperaDatosSalidas(int debug)
  {
  String cad="";

  if (debug) Traza.mensaje("Recupero configuracion de archivo...\n");
  
  if(!leeFichero(SALIDAS_CONFIG_FILE, cad)) 
    {
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

/*********************************************/
/* Parsea el json leido del fichero de       */
/* configuracio de las salidas               */
/*********************************************/
boolean parseaConfiguracionSalidas(String contenido)
  { 
  DynamicJsonBuffer jsonBuffer;
  JsonObject& json = jsonBuffer.parseObject(contenido.c_str());
  
  String salida;
  json.printTo(salida);//pinto el json que he creado
  Traza.mensaje("json creado:\n#%s#\n",salida.c_str());
  
  if (!json.success()) return false;
        
  Traza.mensaje("\nparsed json\n");
//******************************Parte especifica del json a leer********************************
  JsonArray& Salidas = json["Salidas"];

  int8_t max;
  max=(Salidas.size()<MAX_SALIDAS?Salidas.size():MAX_SALIDAS);
  for(int8_t i=0;i<max;i++)
    { 
    JsonObject& salida = json["Salidas"][i];

    salidas[i].configurado=CONFIGURADO;//lo marco como configurado  
    if(salida.containsKey("GPIO")) salidas[i].pin=salida.get<int>("GPIO"); else return false;
    if(salida.containsKey("nombre")) salidas[i].nombre=salida.get<String>("nombre");
    if(salida.containsKey("inicio")) 
      {
      if(salida.get<String>("inicio")=="on") salidas[i].inicio=1; //Si de inicio debe estar activado o desactivado
      else salidas[i].inicio=0;
      }      
    if(salida.containsKey("tipo")) salidas[i].tipo=salida.get<int>("tipo");
    if(salida.containsKey("valorPWM")) salidas[i].valorPWM=salida.get<int>("valorPWM");
    if(salida.containsKey("anchoPulso")) salidas[i].anchoPulso=salida.get<int>("anchoPulso");
    if(salida.containsKey("modo")) salidas[i].modo=salida.get<int>("modo");
    if(salida.containsKey("tipo")) salidas[i].tipo=salida.get<int>("tipo");
    if(salida.containsKey("valorPWM")) salidas[i].valorPWM=salida.get<int>("valorPWM");
    if(salida.containsKey("canal")) salidas[i].canal=salida.get<int>("canal");
    if(salida.containsKey("frecuencia")) salidas[i].frecuencia=salida.get<int>("frecuencia");
    if(salida.containsKey("resolucion")) salidas[i].resolucion=salida.get<int>("resolucion");   
    if(salida.containsKey("controlador")) salidas[i].controlador=salida.get<int>("controlador");
   
    if(salida.containsKey("Estados"))
      {
      int8_t est_max=salida["Estados"].size();//maximo de mensajes en el JSON
      if (est_max>2) est_max=2;               //Si hay mas de 2, solo leo 2
      for(int8_t e=0;e<est_max;e++)  
        {
        if (salida["Estados"][e]["valor"]==e) salidas[i].nombreEstados[e]=String((const char *)salida["Estados"][e]["texto"]);
        }
      }
    if(salida.containsKey("Mensajes"))
      {
      int8_t men_max=salida["Mensajes"].size();//maximo de mensajes en el JSON
      if (men_max>2) men_max=2;                //Si hay mas de 2, solo leo 2
      for(int8_t m=0;m<men_max;m++)  
        {
        if (salida["Mensajes"][m]["valor"]==m) salidas[i].mensajes[m]=String((const char *)salida["Mensajes"][m]["texto"]);
        }
      }
    }

  Traza.mensaje("*************************\nSalidas:\n"); 
  for(int8_t i=0;i<MAX_SALIDAS;i++) 
    {
    //Traza.mensaje("%01i: %s| pin: %i| ancho del pulso: %i| configurado= %i| entrada asociada= %i\n",i,salidas[i].nombre.c_str(),salidas[i].pin,salidas[i].anchoPulso,salidas[i].configurado,salidas[i].entradaAsociada); 
    Traza.mensaje("%01i: %s | configurado= %i | pin: %i | modo: %i | controlador: %i | ancho del pulso: %i\n",i,salidas[i].nombre.c_str(),salidas[i].configurado,salidas[i].pin, salidas[i].modo,salidas[i].controlador, salidas[i].anchoPulso); 
    Traza.mensaje("Estados:\n");
    for(int8_t e=0;e<2;e++) 
      {
      Traza.mensaje("Estado[%02i]: %s\n",e,salidas[i].nombreEstados[e].c_str());     
      }   
    Traza.mensaje("Mensajes:\n");
    for(int8_t m=0;m<2;m++) 
      {
      Traza.mensaje("Mensaje[%02i]: %s\n",m,salidas[i].mensajes[m].c_str());     
      }    
    }    
  Traza.mensaje("*************************\n");  
//************************************************************************************************
  return true; 
  }
/**********************************************************Fin configuracion******************************************************************/  

/**********************************************************SALIDAS******************************************************************/    
int8_t actualizaPulso(int8_t salida)
  {
  if(salida <0 || salida>=MAX_SALIDAS) return NO_CONFIGURADO; //Rele fuera de rango
  if(salidas[salida].configurado!=CONFIGURADO) return NO_CONFIGURADO; //No configurado
  if(salidas[salida].estado!=ESTADO_PULSO) return NO_CONFIGURADO; //No configurado
    
  if(salidas[salida].finPulso>salidas[salida].anchoPulso)//el contador de millis no desborda durante el pulso
    {
    if(millis()>=salidas[salida].finPulso) //El pulso ya ha acabado
      {
      conmutaSalida(salida,ESTADO_DESACTIVO,debugGlobal);  
      Traza.mensaje("Fin del pulso. millis()= %i\n",millis());
      }//del if del fin de pulso
    }//del if de desboda
  else //El contador de millis desbordar durante el pulso
    {
    if(UINT64_MAX-salidas[salida].anchoPulso>millis())//Ya ha desbordado
      {
      if(millis()>=salidas[salida].finPulso) //El pulso ha acabado
        {
        conmutaSalida(salida,ESTADO_DESACTIVO,debugGlobal);
        Traza.mensaje("Fin del pulso. millis()= %i\n",millis());
        }//del if del fin de pulso
      }//del if ha desbordado ya
    }//else del if de no desborda  

  return CONFIGURADO;  
  }
  
/******************************************************/
/* Evalua el estado de cada salida y la actualiza     */
/* segun el modo de operacion                         */
/* estado=0 o 1 encendido o apagado segun nivelActivo */
/* estado=2 modo secuenciador                         */
/* estado=3 modo seguimiento de entrada (anchoPulso)  */
/******************************************************/
void actualizaSalida(int8_t salida)
  {
  switch (salidas[salida].modo)
    {
    case MODO_MANUAL://manual
      if(salidas[salida].estado==ESTADO_PULSO) actualizaPulso(salida);//si esta en modo pulso
      break;
    case MODO_SECUENCIADOR://Controlada por un secuenciador
      break;
    case MODO_SEGUIMIENTO://Sigue a una entradaseguimiento      
      //Si es un seguidor de pulso
      if(salidas[salida].anchoPulso>0)
        {
        if(estadoEntrada(salidas[salida].controlador)==ESTADO_ACTIVO) pulsoSalida(salida); //Si la entrada esta activa, actualizo
        else if(salidas[salida].estado==ESTADO_PULSO) actualizaPulso(salida);//si no esta activa, reviso el pulso
        }
      
      //Si es un seguidor tal cual
      else if(actuaSalida(salida, estadoEntrada(salidas[salida].controlador))==-1) Traza.mensaje("Error al actualizar la salida seguidor %i\n\n",salida);

      break;
    case MODO_MAQUINA://Controlada por una maquina de estados  
      break;
    default:  
      break;    
    }
  }

/*************************************************/
/*Logica de los reles:                           */
/*Si esta activo para ese intervalo de tiempo(1) */
/*Si esta por debajo de la tMin cierro rele      */
/*si no abro rele                                */
/*************************************************/
void actualizaSalidas(bool debug)
  {
  for(int8_t id=0;id<MAX_SALIDAS;id++)
    {
    if (salidas[id].configurado==CONFIGURADO) actualizaSalida(id);
    }//del for    
  }//de la funcion

/*************************************************/
/*                                               */
/*  Devuelve el estado  del rele indicado en id  */
/*  puede ser 0 apagado, 1 encendido, 2 pulsando */
/*                                               */
/*************************************************/
int8_t estadoSalida(int8_t id)
  {
  if(id <0 || id>=MAX_SALIDAS) return NO_CONFIGURADO; //Rele fuera de rango
  if(salidas[id].configurado!=CONFIGURADO) return -1; //No configurado
  
  return salidas[id].estado;
 }

/********************************************************/
/*                                                      */
/*  Devuelve el tipo de salida digital/PWM              */
/*                                                      */
/********************************************************/
int8_t getTipo(int8_t id)
  {
  if(id <0 || id>=MAX_SALIDAS) return NO_CONFIGURADO; //Rele fuera de rango
  if(salidas[id].configurado!=CONFIGURADO) return -1; //No configurado
  
  return salidas[id].tipo;
  }

/********************************************************/
/*                                                      */
/*  Devuelve el nombre del tipo de salida digital/PWM   */
/*                                                      */
/********************************************************/
String getTipoNombre(int8_t id)
  {
  if(id <0 || id>=MAX_SALIDAS) return "ERROR"; //Rele fuera de rango
  if(salidas[id].configurado!=CONFIGURADO) return "ERROR"; //No configurado

  String cad="";
  switch(salidas[id].tipo)
    {
    case TIPO_DIGITAL:
      cad="Digital";
      break;
    case TIPO_LED:  
      cad="LED";
      break;
    default:
      cad="Error";
      break;      
    }
    
  return cad;
  }

/********************************************************/
/*                                                      */
/*  Devuelve el valor de PWM,                           */
/*  si la salida es de ese tipo                         */
/*                                                      */
/********************************************************/
int16_t getValorPWM(int8_t id)
  {
  if(id <0 || id>=MAX_SALIDAS) return NO_CONFIGURADO; //Rele fuera de rango
  if(salidas[id].configurado!=CONFIGURADO) return -1; //No configurado
  
  if(salidas[id].tipo==TIPO_PWM) return salidas[id].valorPWM;
  else return NO_CONFIGURADO;
  }

/********************************************************/
/*                                                      */
/*  establece el valor de la salida PWM                 */
/*                                                      */
/********************************************************/
int8_t setValorPWM(int8_t id, int16_t valor) 
  {
  if(id <0 || id>=MAX_SALIDAS) return NO_CONFIGURADO; //Rele fuera de rango
  if(salidas[id].configurado!=CONFIGURADO) return -1; //No configurado

  if(salidas[id].tipo==TIPO_PWM) 
    {
    uint16_t PWMMax=1;//b0000000000000001
    PWMMax<<= RESOLUCION_PWM;
    if(valor>=PWMMax) valor=PWMMax-1;
    salidas[id].valorPWM=valor;
    return 0;
    }
    
  return NO_CONFIGURADO;
  }

/********************************************************/
/*                                                      */
/*  Devuelve el nombre del rele con el id especificado  */
/*                                                      */
/********************************************************/
String nombreSalida(int8_t id)
  { 
  if(id <0 || id>=MAX_SALIDAS) return "ERROR"; //Rele fuera de rango    
  return salidas[id].nombre;
  } 

/*************************************************/
/*conmuta el rele indicado en id                 */
/*devuelve 1 si ok, -1 si ko                     */
/* LA ENTRADA ES EL ESTADO LOGICO. ADAPTO EL     */
/*  ESTADO FISICO SEGUN nivelActivo              */
/*************************************************/
int8_t conmutaSalida(int8_t id, int8_t estado_final, int debug)
  {
  //validaciones previas
  if(id <0 || id>=MAX_SALIDAS) return NO_CONFIGURADO; //Rele fuera de rango
  if(salidas[id].configurado==NO_CONFIGURADO) return NO_CONFIGURADO; //El rele no esta configurado
  
  //parte logica
  salidas[id].estado=estado_final;//Lo que llega es el estado logico. No hace falta mapeo
  
  //parte fisica. Me he hecho un mapa de karnaugh y sale asi
  if(estado_final==nivelActivo) 
    {
    switch (salidas[id].tipo)
      {
      case TIPO_DIGITAL:
        digitalWrite(salidas[id].pin, HIGH); //controlo el rele
        break;
      case TIPO_LED:
        if(nivelActivo==1) ledcWrite(salidas[id].canal,salidas[id].valorPWM);
        else ledcWrite(salidas[id].canal,(1<<RESOLUCION_PWM)-salidas[id].valorPWM);
        break;
      }    
    }
  else 
    {
    switch (salidas[id].tipo)
      {
      case TIPO_DIGITAL:
        digitalWrite(salidas[id].pin, LOW); //controlo el rele
        break;
      case TIPO_LED:
        ledcWrite(salidas[id].canal,0);
        if(nivelActivo==1) ledcWrite(salidas[id].canal,0);
        else ledcWrite(salidas[id].canal,(1<<RESOLUCION_PWM));
        break;
      }    
    }
  
  if(debug)
    {
    Traza.mensaje("id: %i; GPIO: %i; estado: ",(int)id,(int)salidas[id].pin);
    Traza.mensaje("%i\n",digitalRead(salidas[id].pin));
    }
    
  return 1;
  }

/****************************************************/
/*   Genera un pulso en rele indicado en id         */
/*   devuelve 1 si ok, -1 si ko                     */
/****************************************************/
int8_t pulsoSalida(int8_t id)
  {
  //validaciones previas
  if(id <0 || id>=MAX_SALIDAS) return NO_CONFIGURADO;
  if(salidas[id].configurado==NO_CONFIGURADO) return -1; //El rele no esta configurado
  if(salidas[id].modo!=MODO_MANUAL && salidas[id].modo!=MODO_SEGUIMIENTO) return NO_CONFIGURADO;
      
  //Pongo el rele en nivel Activo  
  if(!conmutaSalida(id, ESTADO_ACTIVO, debugGlobal)) return 0; //Si no puede retorna -1

  //cargo el campo con el valor definido para el ancho del pulso
  salidas[id].estado=ESTADO_PULSO;//estado EN_PULSO
  salidas[id].finPulso=millis()+salidas[id].anchoPulso; 

  Traza.mensaje("Incio de pulso %i| fin calculado %i\n",millis(),salidas[id].finPulso);
  
  return 1;  
  }

/********************************************************/
/*                                                      */
/*     Recubre las dos funciones anteriores para        */
/*     actuar sobre un rele                             */
/*                                                      */
/********************************************************/ 
int8_t actuaSalida(int8_t id, int8_t estado)
  {
  //Si esta en modo secuenciador o modo maquina no deberia actuar, solo si esta en modo manual o seguimiento
  if(salidas[id].modo!=MODO_MANUAL && salidas[id].modo!=MODO_SEGUIMIENTO) return -1;
   
  switch(estado)
    {
    case ESTADO_DESACTIVO:
      return conmutaSalida(id, ESTADO_DESACTIVO, debugGlobal);
      break;
    case ESTADO_ACTIVO:
      return conmutaSalida(id, ESTADO_ACTIVO, debugGlobal);
      break;
    case ESTADO_PULSO:
      return pulsoSalida(id);
      break;      
    default://no deberia pasar nunca!!
      return -1;
    }
  }

/****************************************************/
/*   Genera un pulso en rele indicado en id         */
/*   devuelve 1 si ok, -1 si ko                     */
/****************************************************/
int8_t salidaMaquinaEstados(int8_t id, int8_t estado)
  {
  //validaciones previas
  if(id <0 || id>=MAX_SALIDAS) return NO_CONFIGURADO;
  if(salidas[id].modo!=MODO_MAQUINA) return NO_CONFIGURADO;

  return conmutaSalida(id, estado, false);
  }

/********************************************************/
/*                                                      */
/*     Devuelve si el reles esta configurados           */
/*                                                      */
/********************************************************/ 
int releConfigurado(uint8_t id)
  {
  if(id <0 || id>=MAX_SALIDAS) return NO_CONFIGURADO;
    
  return salidas[id].configurado;
  } 
  
/********************************************************/
/*                                                      */
/*     Devuelve el numero de reles configurados         */
/*                                                      */
/********************************************************/ 
int relesConfigurados(void)
  {
  int resultado=0;
  
  for(int8_t i=0;i<MAX_SALIDAS;i++)
    {
    if(salidas[i].configurado==CONFIGURADO) resultado++;
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
  if(id >=0 && id<MAX_SALIDAS) 
    {
    salidas[id].modo=MODO_SECUENCIADOR;
    salidas[id].controlador=plan; 
    }
  }  

/********************************************************/
/*                                                      */
/*     Fuerza el modo manual en una salida que esta en  */
/*     en otro modo                                     */
/*                                                      */
/********************************************************/ 
int8_t forzarModoManualSalida(int8_t id)
  {
  //validaciones previas
  if(id <0 || id>=MAX_SALIDAS) return NO_CONFIGURADO;

  salidas[id].modo=MODO_MANUAL;

  return CONFIGURADO;
  }

/********************************************************/
/*                                                      */
/*     Fuerza el modo manual en una salida que esta en  */
/*     en otro modo                                     */
/*                                                      */
/********************************************************/ 
int8_t recuperarModoSalida(int8_t id)
  {
  //validaciones previas
  if(id <0 || id>=MAX_SALIDAS) return NO_CONFIGURADO;

  salidas[id].modo=salidas[id].modo_inicial;
  conmutaSalida(id, ESTADO_DESACTIVO, debugGlobal);
  return CONFIGURADO;
  }  

/********************************************************/
/*                                                      */
/*     Devuelve el estado incial de la salida           */
/*                                                      */
/********************************************************/ 
int8_t inicioSalida(int8_t id)
  {
  //validaciones previas
  if(id <0 || id>=MAX_SALIDAS) return NO_CONFIGURADO;
       
  return salidas[id].inicio;  
  }  

/********************************************************/
/*                                                      */
/*     Devuelve el nombre de la salida                  */
/*                                                      */
/********************************************************/ 
String nombreSalida(uint8_t id)
  {
  //validaciones previas
  if(id <0 || id>=MAX_SALIDAS) return "ERROR";
       
  return salidas[id].nombre;  
  }   

/********************************************************/
/*                                                      */
/*     Devuelve el modo de la salida                    */
/*                                                      */
/********************************************************/ 
uint8_t pinSalida(uint8_t id)
  {
  //validaciones previas
  if(id <0 || id>=MAX_SALIDAS) return NO_CONFIGURADO;
       
  return salidas[id].pin;  
  }   

/********************************************************/
/*                                                      */
/*     Devuelve el ancho del pulso de la salida         */
/*                                                      */
/********************************************************/ 
uint16_t anchoPulsoSalida(uint8_t id)
  {
  //validaciones previas
  if(id <0 || id>=MAX_SALIDAS) return NO_CONFIGURADO;
       
  return salidas[id].anchoPulso;  
  }   

/********************************************************/
/*                                                      */
/*     Devuelve el fin del pulso de la salida           */
/*                                                      */
/********************************************************/ 
unsigned long finPulsoSalida(uint8_t id)
  {
  //validaciones previas
  if(id <0 || id>=MAX_SALIDAS) return NO_CONFIGURADO;
       
  return salidas[id].finPulso;  
  }   

/********************************************************/
/*                                                      */
/*     Devuelve el valor de inicio de la salida         */
/*                                                      */
/********************************************************/ 
int8_t inicioSalida(uint8_t id)
  {
  //validaciones previas
  if(id <0 || id>=MAX_SALIDAS) return NO_CONFIGURADO;
       
  return salidas[id].inicio;
  }   

/********************************************************/
/*                                                      */
/*  Devuelve el nombre del estado de una salida         */
/*                                                      */
/********************************************************/ 
String nombreEstadoSalida(uint8_t id, uint8_t estado)
  {
  //validaciones previas
  if(id <0 || id>=MAX_SALIDAS) return "ERROR";
  if(estado>2) return "ERROR";
       
  if (estado!=ESTADO_DESACTIVO) return salidas[id].nombreEstados[ESTADO_ACTIVO];
  
  return salidas[id].nombreEstados[ESTADO_DESACTIVO];
  }   

/********************************************************/
/*                                                      */
/*  Devuelve el nombre del estado actual de una salida  */
/*                                                      */
/********************************************************/ 
String nombreEstadoSalidaActual(uint8_t id)
  {
  //validaciones previas
  if(id <0 || id>=MAX_SALIDAS) return "ERROR";
       
  if (salidas[id].estado!=ESTADO_DESACTIVO) return salidas[id].nombreEstados[ESTADO_ACTIVO];
  
  return salidas[id].nombreEstados[ESTADO_DESACTIVO];
  }   

/********************************************************/
/*                                                      */
/*  Devuelve el mensaje de una salida en un estado      */
/*                                                      */
/********************************************************/ 
String mensajeEstadoSalida(uint8_t id, uint8_t estado)
  {
  //validaciones previas
  if(id <0 || id>=MAX_SALIDAS) return "ERROR";
  if(estado>2) return "ERROR";
       
  return salidas[id].mensajes[estado];
  }   

/********************************************************/
/*                                                      */
/*  Devuelve el mensaje del estado actual una salida    */
/*                                                      */
/********************************************************/ 
String mensajeEstadoSalidaActual(uint8_t id)
  {
  //validaciones previas
  if(id <0 || id>=MAX_SALIDAS) return "ERROR";
       
  return salidas[id].mensajes[salidas[id].estado];
  }   

/********************************************************/
/*                                                      */
/*     Devuelve el controlador de la salida si esta     */
/*     asociada a un plan de secuenciador               */
/*                                                      */
/********************************************************/ 
int8_t controladorSalida(int8_t id)
  {
  //validaciones previas
  if(id <0 || id>=MAX_SALIDAS) return NO_CONFIGURADO;
  if(salidas[id].modo!=MODO_SECUENCIADOR && salidas[id].modo!=MODO_SEGUIMIENTO) return NO_CONFIGURADO;
       
  return salidas[id].controlador;  
  }   

/********************************************************/
/*                                                      */
/*     Devuelve el modo de la salida                    */
/*                                                      */
/********************************************************/ 
uint8_t modoSalida(uint8_t id)
  {
  //validaciones previas
  if(id <0 || id>=MAX_SALIDAS) return NO_CONFIGURADO;
       
  return salidas[id].modo;  
  }   

/********************************************************/
/*                                                      */
/*     Devuelve el modo de la salida                    */
/*                                                      */
/********************************************************/ 
String modoSalidaNombre(uint8_t id)
  {
  //validaciones previas
  if(id <0 || id>=MAX_SALIDAS) return "ERROR";

  String cad="";
  switch(salidas[id].modo)
    {
    case MODO_MANUAL:
      cad="Manual";
      break;
    case MODO_SECUENCIADOR:
      cad="Secuenciador";
      break;
    case MODO_SEGUIMIENTO:
      cad="Seguimiento";
      break;
    case MODO_MAQUINA:
      cad="Maq. Estados";
      break;    
    default:
      cad="Error";
      break;
    }
  return cad;
  }   

/********************************************************/
/*                                                      */
/*     Devuelve el modo inicial de la salida            */
/*                                                      */
/********************************************************/ 
uint8_t modoInicalSalida(uint8_t id)
  {
  //validaciones previas
  if(id <0 || id>=MAX_SALIDAS) return NO_CONFIGURADO;
       
  return salidas[id].modo_inicial;  
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
String generaJsonEstadoSalidas(boolean filtro=true)//void)
  {
  String salida="";

  //const size_t bufferSize = JSON_ARRAY_SIZE(2) + JSON_OBJECT_SIZE(1) + 2*JSON_OBJECT_SIZE(8);
  const size_t bufferSize = JSON_ARRAY_SIZE(1) + JSON_OBJECT_SIZE(1) + JSON_OBJECT_SIZE(15);

  DynamicJsonBuffer jsonBuffer(bufferSize);
  
  JsonObject& root = jsonBuffer.createObject();
  
  JsonArray& Salidas = root.createNestedArray("salidas");
  for(int8_t id=0;id<MAX_SALIDAS;id++)
    {
    JsonObject& Salidas_0 = Salidas.createNestedObject();
    Salidas_0["id"] = id;
    Salidas_0["nombre"] = nombreSalida(id);
    Salidas_0["pin"] = pinSalida(id);
    Salidas_0["modo"] = modoSalidaNombre(id);
    Salidas_0["controlador"] = controladorSalida(id);
    Salidas_0["estado"] = estadoSalida(id);
    Salidas_0["nombreEstado"] = nombreEstadoSalida(id,estadoSalida(id));
    Salidas_0["anchoPulso"] = anchoPulsoSalida(id);
    Salidas_0["finPulso"] = finPulsoSalida(id);

    if(!filtro) {
      Salidas_0["tipo"] = getTipoNombre(id);
      Salidas_0["valorPWM"] = getValorPWM(id);
      Salidas_0["inicioPulso"] = inicioSalida(id);
      Salidas_0["mensajeEstado"] = mensajeEstadoSalida(id,estadoSalida(id));
      Salidas_0["configurada"] = releConfigurado(id);
      Salidas_0["inicio"] = inicioSalida(id);
      }    
    }
    
  root.printTo(salida);
  return salida;  
  }  
String generaJsonEstadoSalidas(void) {return generaJsonEstadoSalidas(true);}

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
String generaJsonEstadoEntradas(boolean filtro)//void)
  {
  String salida="";

  //const size_t bufferSize = JSON_ARRAY_SIZE(2) + JSON_OBJECT_SIZE(1) + 2*JSON_OBJECT_SIZE(3);
  const size_t bufferSize = JSON_ARRAY_SIZE(3) + JSON_OBJECT_SIZE(1) + 3*JSON_OBJECT_SIZE(10);
  DynamicJsonBuffer jsonBuffer(bufferSize);
  
  JsonObject& root = jsonBuffer.createObject();
  
  JsonArray& Entradas = root.createNestedArray("entradas");
  for(int8_t id=0;id<MAX_ENTRADAS;id++)
    {
    JsonObject& Entradas_0 = Entradas.createNestedObject(); 
    Entradas_0["id"] = id;
    Entradas_0["nombre"] = nombreEntrada(id);
    Entradas_0["estado"] = estadoEntrada(id);
    Entradas_0["nombreEstado"] = nombreEstadoEntrada(id,estadoEntrada(id));

    if(!filtro){
      Entradas_0["configurada"] = entradaConfigurada(id);
      Entradas_0["tipo"] = tipoEntrada(id);
      Entradas_0["pin"] = pinEntrada(id);
      Entradas_0["estadoActivo"] = estadoActivoEntrada(id);
      Entradas_0["estadoFisico"] = estadoFisicoEntrada(id);
      Entradas_0["mensajeEstado"] = mensajeEstadoEntrada(id,estadoEntrada(id));
      }
    }

  root.printTo(salida);
  return salida;  
  }
String generaJsonEstadoEntradas(void){return generaJsonEstadoEntradas(true);}

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
      Entradas_0["nombreEstado"] = nombreEstadoEntrada(id,entradas[id].estado);
      }
    }

  JsonArray& Salidas = root.createNestedArray("Salidas");
  for(int8_t id=0;id<MAX_SALIDAS;id++)
    {
    if(salidas[id].configurado==CONFIGURADO)
      {
      JsonObject& Salidas_0 = Salidas.createNestedObject();
      Salidas_0["id"] = id;
      Salidas_0["nombre"] = salidas[id].nombre;
      Salidas_0["pin"] = salidas[id].pin;
      Salidas_0["modo"] = salidas[id].modo;
      Salidas_0["controlador"] = salidas[id].controlador;
      Salidas_0["valor"] = salidas[id].estado;    
      Salidas_0["nombreEstado"] = nombreEstadoSalida(id,salidas[id].estado);
      Salidas_0["anchoPulso"] = salidas[id].anchoPulso;
      Salidas_0["finPulso"] = salidas[id].finPulso;
      }
    }

  root.printTo(salida);  
  return salida;  
  }   

int8_t setSalidaActiva(int8_t id)
  {
  if(id <0 || id>=MAX_SALIDAS) return NO_CONFIGURADO;
  if(salidas[id].configurado==NO_CONFIGURADO) return -1; //El rele no esta configurado
  
  salidaActiva=id;

  return CONFIGURADO;
  }  
  
int8_t getSalidaActiva(void){return salidaActiva;}