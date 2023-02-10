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

Entradas entradas;

/************************************** Constructor ****************************************/
Entradas::Entradas(void){numeroEntradas=0;}
/************************************** Fin constructor ****************************************/

/************************************** Funciones de configuracion ****************************************/
/*********************************************/
/* Inicializa los valores de los registros de*/
/* las entradas y recupera la configuracion  */
/*********************************************/
void Entradas::inicializa(void){  
  //leo la configuracion del fichero
  if(!recuperaDatos(debugGlobal)) Traza.mensaje("Configuracion de los reles por defecto\n");
  else{ 
    //parte fisica
    for(int8_t i=0;i<entradas.getNumEntradas();i++){
      if(entrada[i].getTipo()=="INPUT_PULLUP") pinMode(entrada[i].getPin(), INPUT_PULLUP);
      else if(entrada[i].getTipo()=="INPUT_PULLDOWN") pinMode(entrada[i].getPin(), INPUT_PULLDOWN);
      else pinMode(entrada[i].getPin(), INPUT); //PULLUP

      Traza.mensaje("Nombre entrada[%i]=%s | pin entrada: %i | tipo: %s | estado activo %i\n",i,entrada[i].getNombre().c_str(),entrada[i].getPin(),entrada[i].getTipo().c_str(),entrada[i].getEstadoActivo());
      Traza.mensaje("\tEstados y mensajes:\n");
      for(uint8_t j=0;j<2;j++) Traza.mensaje("\t\tEstado %i: %s | mensaje: %s\n",j,entrada[i].getNombreEstado(j).c_str(),entrada[i].getMensajeEstado(j).c_str());        
    }
  }  
}

/*********************************************/
/* Lee el fichero de configuracion de las    */
/* entradas o genera conf por defecto        */
/*********************************************/
boolean Entradas::recuperaDatos(int debug){
  String cad="";

  if (debug) Traza.mensaje("Recupero configuracion de archivo...\n");

  if(!leeFichero(ENTRADAS_CONFIG_FILE, cad)){
    //Confgiguracion por defecto
    Traza.mensaje("No existe fichero de configuracion de Entradas\n");    
    cad="{\"Entradas\": []}";
    //salvo la config por defecto
    //if(salvaFichero(ENTRADAS_CONFIG_FILE, ENTRADAS_CONFIG_BAK_FILE, cad)) Traza.mensaje("Fichero de configuracion de Entradas creado por defecto\n");
  }
  return parseaConfiguracion(cad);
}

/*********************************************/
/* Parsea el json leido del fichero de       */
/* configuracio de las entradas              */
/*********************************************/
boolean Entradas::parseaConfiguracion(String contenido){
  DynamicJsonBuffer jsonBuffer;
  JsonObject& json = jsonBuffer.parseObject(contenido.c_str());

  String cad;
  json.printTo(cad);//pinto el json que he creado
  Traza.mensaje("json leido:\n#%s#\n",cad.c_str());
  
  if (!json.success()) return false;

  Traza.mensaje("\nparsed json\n");
  //******************************Parte especifica del json a leer********************************
  JsonArray& Entradas = json["Entradas"];

  numeroEntradas=(Entradas.size()<MAX_ENTRADAS?Entradas.size():MAX_ENTRADAS);
  Traza.mensaje("Se configurarán %i entradas\n",numeroEntradas);

  entradas.entrada=new Entrada[numeroEntradas];

  for(int8_t i=0;i<numeroEntradas;i++){ 
    String nombre="";
    String tipo="";
    int8_t pin=-1;
    int8_t estadoActivo=-1;
    String nombres[2];
    String mensajes[2];

    JsonObject& _entrada = json["Entradas"][i];
          
    if(_entrada.containsKey("nombre")) nombre=_entrada.get<String>("nombre"); 
    if(_entrada.containsKey("tipo")) tipo=_entrada.get<String>("tipo"); 
    if(_entrada.containsKey("GPIO")) pin=_entrada.get<int>("GPIO"); else return false;
    if(_entrada.containsKey("estadoActivo")) estadoActivo=_entrada.get<int>("estadoActivo");
   
    if(_entrada.containsKey("Estados")){
      int8_t est_max=_entrada["Estados"].size();//maximo de mensajes en el JSON
      if (est_max>2) est_max=2;                     //Si hay mas de 2, solo leo 2
      for(int8_t e=0;e<est_max;e++){
        if (_entrada["Estados"][e]["valor"]==e) nombres[e]=String((const char *)_entrada["Estados"][e]["texto"]);
        else nombres[e]="";
      }
    }

    if(_entrada.containsKey("Mensajes")){
      int8_t men_max=_entrada["Mensajes"].size();//maximo de mensajes en el JSON
      if (men_max>2) men_max=2;                     //Si hay mas de 2, solo leo 2
      for(int8_t m=0;m<men_max;m++){
        if (_entrada["Mensajes"][m]["valor"]==m) mensajes[m]=String((const char *)_entrada["Mensajes"][m]["texto"]);
        else mensajes[m]="";
      }
    }
    //Traza.mensaje("Entrada %i: nombre: %s, tipo: %s, pin: %i, estadoActivo: %i, nombre[0]: %s, nombre[1]: %s, mensaje[0]: %s, nombre[1]: %s\n",i,nombre.c_str(),tipo.c_str(),pin,estadoActivo,nombres[0].c_str(),nombres[1].c_str(),mensajes[0].c_str(),mensajes[1].c_str());
    configura(i,nombre,tipo,pin,estadoActivo,nombres,mensajes);
  }

  if(debugGlobal || true) {
    Traza.mensaje("*************************\nEntradas:"); 
    for(uint8_t i=0;i<entradas.getNumEntradas();i++) {
      Traza.mensaje("\n%01i: %s | pin: %i | tipo=%s | estado activo: %i\n",i,entrada[i].getNombre().c_str(),entrada[i].getPin(),entrada[i].getTipo().c_str(),entrada[i].getEstadoActivo());
      Traza.mensaje("Estados:\n");
      for(uint8_t m=0;m<2;m++){
        Traza.mensaje("Estado[%02i]: %s\n",m,entrada[i].getNombreEstado(m).c_str());     
      }      
      Traza.mensaje("Mensajes:\n");
      for(uint8_t m=0;m<2;m++) {
        Traza.mensaje("Mensaje[%02i]: %s\n",m,entrada[i].getMensajeEstado(m).c_str());     
      }        
    }
    Traza.mensaje("*************************\n");  
  }
//************************************************************************************************
  return true; 
}

/**********************************************************ENTRADAS******************************************************************/  
void Entradas::configura(uint8_t id, String _nombre, String _tipo, int8_t _pin, int8_t _estadoActivo, String _nombres[2], String _mensajes[2]){    
  entrada[id].configura(_nombre,  _tipo,  _pin,  _estadoActivo,  _nombres,  _mensajes);
}

/*************************************************/
/*                                               */
/*       Lee el estado de las entradas           */
/*                                               */
/*************************************************/
void Entradas::actualiza(bool debug)
  {
  //Actualizo las entradas  
  for(uint8_t i=0;i<entradas.getNumEntradas();i++) entrada[i].setEstado();
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
String Entradas::generaJsonEstado(boolean filtro)
  {
  String cad="";

  //const size_t bufferSize = JSON_ARRAY_SIZE(2) + JSON_OBJECT_SIZE(1) + 2*JSON_OBJECT_SIZE(3);
  const size_t bufferSize = JSON_ARRAY_SIZE(3) + JSON_OBJECT_SIZE(1) + 3*JSON_OBJECT_SIZE(10);
  DynamicJsonBuffer jsonBuffer(bufferSize);
  
  JsonObject& root = jsonBuffer.createObject();
  
  JsonArray& Entradas = root.createNestedArray("datos");////entradas
  for(int8_t id=0;id<entradas.getNumEntradas();id++)
    {
    JsonObject& Entradas_0 = Entradas.createNestedObject(); 
    Entradas_0["id"] = id;
    Entradas_0["nombre"] = entrada[id].getNombre();
    Entradas_0["estado"] = entrada[id].getEstado();
    Entradas_0["nombreEstado"] = entrada[id].getNombreEstado(entrada[id].getEstado());

    if(!filtro) {
      Entradas_0["tipo"] = entrada[id].getTipo();
      Entradas_0["pin"] = entrada[id].getPin();
      Entradas_0["estadoActivo"] = entrada[id].getEstadoActivo();
      Entradas_0["estadoFisico"] = entrada[id].getEstadoFisico();
      Entradas_0["mensajeEstado"] = entrada[id].getMensajeEstado(entrada[id].getEstado());
      }
    }

  root.printTo(cad);
  return cad;  
  }
String Entradas::generaJsonEstado(void){return generaJsonEstado(true);}