/*****************************************/
/*                                       */
/*  Control de entradas                  */
/*                                       */
/*****************************************/

/***************************** Defines *****************************/
/***************************** Defines *****************************/

/***************************** Includes *****************************/
#include <Entradas.h>

#include <MQTT.h>
#include <Ficheros.h>
/***************************** Includes *****************************/

//definicion de los tipos de datos para las entradas
entrada* entradas=new entrada[MAX_ENTRADAS];

/************************************** Funciones de configuracion ****************************************/
/*********************************************/
/* Inicializa los valores de los registros de*/
/* las entradas y recupera la configuracion  */
/*********************************************/
void inicializaEntradas(void){  
  //inicializo la parte logica
  for(int8_t i=0;i<MAX_ENTRADAS;i++) entradas[i]=entrada();//Inicializo todas a valores por defecto, solo las configuradas vienen en el fichero

  //leo la configuracion del fichero
  if(!recuperaDatosEntradas(debugGlobal)) Traza.mensaje("Configuracion de los reles por defecto\n");
  else{ 
    //parte fisica
    for(int8_t i=0;i<MAX_ENTRADAS;i++){
      if(entradas[i].getConfigurada()){  
        if(entradas[i].getTipo()=="INPUT_PULLUP") pinMode(entradas[i].getPin(), INPUT_PULLUP);
        else if(entradas[i].getTipo()=="INPUT_PULLDOWN") pinMode(entradas[i].getPin(), INPUT_PULLDOWN);
        else pinMode(entradas[i].getPin(), INPUT); //PULLUP

        Traza.mensaje("Nombre entrada[%i]=%s | pin entrada: %i | tipo: %s | estado activo %i\n",i,entradas[i].getNombre().c_str(),entradas[i].getPin(),entradas[i].getTipo().c_str(),entradas[i].getEstadoActivo());
        Traza.mensaje("\tEstados y mensajes:\n");
        for(uint8_t j=0;j<2;j++) Traza.mensaje("\t\tEstado %i: %s | mensaje: %s\n",j,entradas[i].getNombreEstado(j).c_str(),entradas[i].getMensajeEstado(j).c_str());        
      }
    }
  }  
}

/*********************************************/
/* Lee el fichero de configuracion de las    */
/* entradas o genera conf por defecto        */
/*********************************************/
boolean recuperaDatosEntradas(int debug){
  String cad="";

  if (debug) Traza.mensaje("Recupero configuracion de archivo...\n");

  if(!leeFichero(ENTRADAS_CONFIG_FILE, cad)){
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
boolean parseaConfiguracionEntradas(String contenido){
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
  Traza.mensaje("Se configurarán %i entradas\n",max);
  for(int8_t i=0;i<max;i++){ 
    String nombre="";
    String tipo="";
    int8_t pin=-1;
    int8_t estadoActivo=-1;
    String nombres[2];
    String mensajes[2];

    JsonObject& entrada = json["Entradas"][i];
          
    if(entrada.containsKey("nombre")) nombre=entrada.get<String>("nombre"); 
    if(entrada.containsKey("tipo")) tipo=entrada.get<String>("tipo"); 
    if(entrada.containsKey("GPIO")) pin=entrada.get<int>("GPIO"); else return false;
    if(entrada.containsKey("estadoActivo")) estadoActivo=entrada.get<int>("estadoActivo");
   
    if(entrada.containsKey("Estados")){
      int8_t est_max=entrada["Estados"].size();//maximo de mensajes en el JSON
      if (est_max>2) est_max=2;                     //Si hay mas de 2, solo leo 2
      for(int8_t e=0;e<est_max;e++){
        if (entrada["Estados"][e]["valor"]==e) nombres[e]=String((const char *)entrada["Estados"][e]["texto"]);
        else nombres[e]="";
      }
    }

    if(entrada.containsKey("Mensajes")){
      int8_t men_max=entrada["Mensajes"].size();//maximo de mensajes en el JSON
      if (men_max>2) men_max=2;                     //Si hay mas de 2, solo leo 2
      for(int8_t m=0;m<men_max;m++){
        if (entrada["Mensajes"][m]["valor"]==m) mensajes[m]=String((const char *)entrada["Mensajes"][m]["texto"]);
        else mensajes[m]="";
      }
    }

    entradas[i].configuraEntrada(nombre,tipo,pin,estadoActivo,nombres,mensajes);
  }

  if(debugGlobal) {
    Traza.mensaje("*************************\nEntradas:"); 
    for(int8_t i=0;i<MAX_ENTRADAS;i++) {
      Traza.mensaje("\n%01i: %s | pin: %i | configurado= %i | tipo=%s | estado activo: %i\n",i,entradas[i].getNombre().c_str(),entradas[i].getPin(),entradas[i].getConfigurada(),entradas[i].getTipo().c_str(),entradas[i].getEstadoActivo());
      Traza.mensaje("Estados:\n");
      for(int8_t m=0;m<2;m++){
        Traza.mensaje("Estado[%02i]: %s\n",m,entradas[i].getNombreEstado(m).c_str());     
      }      
      Traza.mensaje("Mensajes:\n");
      for(int8_t m=0;m<2;m++) {
        Traza.mensaje("Mensaje[%02i]: %s\n",m,entradas[i].getMensajeEstado(m).c_str());     
      }        
    }
    Traza.mensaje("*************************\n");  
  }
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
    if(entradas[i].getConfigurada()) entradas[i].setEstadoFisico();
    }
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
    if(entradas[i].getConfigurada()) resultado++;
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
    if(entradas[id].getConfigurada() || !filtro) 
      { 
      JsonObject& Entradas_0 = Entradas.createNestedObject(); 
      Entradas_0["id"] = id;
      Entradas_0["nombre"] = entradas[id].getNombre();
      Entradas_0["estado"] = entradas[id].getEstado();
      Entradas_0["nombreEstado"] = entradas[id].getNombreEstado(entradas[id].getEstado());

      if(!filtro) {
        Entradas_0["configurada"] = entradas[id].getConfigurada();
        Entradas_0["tipo"] = entradas[id].getTipo();
        Entradas_0["pin"] = entradas[id].getPin();
        Entradas_0["estadoActivo"] = entradas[id].getEstadoActivo();
        Entradas_0["estadoFisico"] = entradas[id].getEstadoFisico();
        Entradas_0["mensajeEstado"] = entradas[id].getMensajeEstado(entradas[id].getEstado());
        }
      }
    }

  root.printTo(salida);
  return salida;  
  }
String generaJsonEstadoEntradas(void){return generaJsonEstadoEntradas(true);}