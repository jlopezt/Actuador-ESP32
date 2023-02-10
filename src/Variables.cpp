/*****************************************************/
/*                                                   */
/*        Coleccion de Variables para actuador        */
/*                                                   */
/*****************************************************/

/***************************** Defines *****************************/
#define TOPIC_HUMBRAL_VARIABLES "humbrales"
/***************************** Defines *****************************/

/***************************** Includes *****************************/
#include <Variables.h>
#include <Sensores.h>
#include <Ficheros.h>
#include <ArduinoJson.h>
#include <MQTT.h>
/***************************** Includes *****************************/

/***************************** Variables globales *****************************/
Variables variables;
/***************************** Variables globales *****************************/

/***********************************Variables*********************************************/
/***************************** Constructor *****************************/
Variables::Variables(void){
    numeroVariables=0;
    pVariable=NULL;
    pCabeza=NULL;
}
/************************************** Fin constructor ****************************************/

/************************************** Funciones de configuracion ****************************************/
/*********************************************/
/* Inicializa los valores de los registros de*/
/* las entradas y recupera la configuracion  */
/*********************************************/
void Variables::inicializa(void)
  {
  Wire.begin();

  //recupero datos del fichero de configuracion
  if (!recuperaDatos(false)) 
    {
    Serial.printf("error al recuperar config de los variables.\nConfiguracion por defecto.\n");
    return;
    }
  }
    
/************************************************/
/* Recupera los datos de configuracion          */
/* del archivo de variables                     */
/************************************************/
boolean Variables::recuperaDatos(boolean debug)
  {
  String cad="";

  if (debug) Serial.println("Recupero configuracion de archivo...");
 
  if(!leeFichero(VARIABLES_CONFIG_FILE, cad))
    {
    //Algo salio mal, confgiguracion por defecto
    Serial.printf("No existe fichero de configuracion de Variables o esta corrupto\n");
    cad="{}";
    }
    
  return parseaConfiguracion(cad);    
  }  
    
/*********************************************/
/* Parsea el json leido del fichero de       */
/* configuracio de sensores                  */
/*********************************************/
boolean Variables::parseaConfiguracion(String contenido){  
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(contenido.c_str());
  //json.printTo(Serial);
  if (root.success()){
    Serial.println("parsed json");
//******************************Parte especifica del json a leer********************************

    if(root.containsKey("Variables")){
      JsonArray& VariablesJson = root["Variables"];
    
      numeroVariables=VariablesJson.size();      
    
      for(uint8_t i=0;i<numeroVariables;i++){//For para las variables
        JsonObject& VariablesJsonVariable = VariablesJson[i];

        Variable* p=(Variable*) new Variable(VariablesJsonVariable);

        if(p==NULL) {
          Serial.printf("Error al asignar memoria a la variable %s\n", VariablesJsonVariable.get<String>("nombre").c_str());
          return false;
        }

        if(pCabeza==NULL) pCabeza=p; //Si es el primero
        else pVariable->setSiguiente(p);

        pVariable=p;
        }
      }

      pVariable=pCabeza;

      if(0){
        for(uint8_t i=0;i<numeroVariables;i++){
          if(pVariable==NULL) Serial.printf("Variable %i-->PVariable es NULL!!!\n");
          else Serial.printf("Variable %i | nombre: %s | descripcion: %s | unidades: %s\n",i,pVariable->getNombre().c_str(),pVariable->getDescripcion().c_str(),pVariable->getUnidades().c_str());
          pVariable=pVariable->getSiguiente();
        }
      }
//************************************************************************************************
    return true;
   } 
  return false;
  }
/*********************************************** FIN Metodos de configuracion *****************************************************/

void Variables::lee(boolean debug){
  pVariable=pCabeza;
  Sensor *pSensor=NULL;

  while(pVariable!=NULL){
    Sensor* q;
    pSensor=pVariable->getSensor();
    
    if(pSensor==NULL) {
      Serial.printf("El sensor asociado a la variable %s no existe\nNo se pude leer el valor de la variable\n",pVariable->getNombre().c_str());
      break;
    }

    if(debug){
      Serial.printf("Comienzo la lectura de la variable %s\n",pVariable->getNombre().c_str());
      Serial.printf("Tipo de var: %i | sensor: %s | valor: %.2f\n",pVariable->getTipo(),pSensor->getNombre().c_str(),pSensor->getValor(pVariable->getTipo()));
    }

    pVariable->setValor(pSensor->getValor(pVariable->getTipo()));
    /*
    switch (pSensor->getTipo()){
      case TIPO_DS18B20:
        q= (SensorDS18B20*)pSensor;
        ((SensorDS18B20*)q)->lee();        
        pVariable->setValor(q->getValor(pVariable->getTipo()));
        break;
      case TIPO_DHT22:
        q= (SensorDHT*)pSensor;
        ((SensorDHT*)q)->lee();
        pVariable->setValor(q->getValor(pVariable->getTipo()));
        break;
      case TIPO_HDC1080:
        q= (SensorHDC1080*)pSensor;
        ((SensorHDC1080*)q)->lee();
        pVariable->setValor(q->getValor(pVariable->getTipo()));
        break;
      case TIPO_BME280:
        q= (SensorBME280*)pSensor;
        ((SensorBME280*)q)->lee();
        pVariable->setValor(q->getValor(pVariable->getTipo()));
        break;
      case TIPO_BMP280:
        q= (SensorBMP280*)pSensor;
        ((SensorBMP280*)q)->lee();
        pVariable->setValor(q->getValor(pVariable->getTipo()));
        break;
      case TIPO_GL5539:
        q= (SensorGL5539*)pSensor;
        ((SensorGL5539*)q)->lee();
        pVariable->setValor(q->getValor(pVariable->getTipo()));
        break;
      case TIPO_BH1750:
        q= (SensorBH1750*)pSensor;
        ((SensorBH1750*)q)->lee();
        pVariable->setValor(q->getValor(pVariable->getTipo()));
        break;
      case TIPO_SOILMOISTURECAPACITIVEV2:
        q= (SensorHumedadSuelo*)pSensor;
        ((SensorHumedadSuelo*)q)->lee();
        pVariable->setValor(q->getValor(pVariable->getTipo()));
        break;
    }
    */
    if(debug) Serial.printf("(%i) Variable: %s | valor: %.2f\n",millis(),pVariable->getNombre().c_str(),pVariable->getValor());

    pVariable->comparaHumbrales();

    pVariable=pVariable->getSiguiente();    
  }
}

void Variables::comparaHumbrales(void){
  pVariable=pCabeza;

  while(pVariable!=NULL){
    Serial.printf("Comparando humbrales de la variable %s\n", pVariable->getNombre());
    pVariable->comparaHumbrales();
    pVariable=pVariable->getSiguiente();
  }
};

String Variables::generaJsonEstado(boolean debug){
  Variable* p=pCabeza;
  String salida="";

  const size_t capacity = JSON_ARRAY_SIZE(numeroVariables) + JSON_OBJECT_SIZE(1) + JSON_OBJECT_SIZE(2) + 3*JSON_OBJECT_SIZE(3);
  DynamicJsonBuffer jsonBuffer(capacity);

  JsonObject& root = jsonBuffer.createObject();

  JsonArray& variablesJson = root.createNestedArray("datos");////Variables

  while(p!=NULL){
    //Serial.printf("recuperando valores de  %s\n",p->getNombre().c_str());
    JsonObject& variableJson = variablesJson.createNestedObject();
    p->generaJsonEstado(variableJson);

    p=p->getSiguiente();
  }

  root.printTo(salida);
  //Serial.printf("%s\n",salida.c_str());
  return salida;   
}
/*********************************Fin Variables*******************************************/


/***********************************Variable**********************************************/
Variable::Variable(void){};
Variable::Variable(JsonObject& _config){
  String _nombre="";
  String _descripcion="";
  String _unidades="";
  String _sensor="";
  String _tipo=""; 

  Serial.printf("********************************************************\n");
  _nombre=_config.get<String>("nombre");
  _unidades=_config.get<String>("unidades");
  _descripcion=_config.get<String>("descripcion");
  _sensor=_config.get<String>("sensor");
  _tipo=_config.get<String>("tipo");
  Serial.printf("Valores parseados del JSON: nombre: %s | nombre: %s | descripcion: %s | sensor: %s | tipo: [%s]\n",_nombre.c_str(),_unidades.c_str(),_descripcion.c_str(),_sensor.c_str(),_tipo.c_str());
Serial.println("Tipo: [" + _tipo + "]");

  nombre=_nombre;
  unidades=_unidades;
  descripcion=_descripcion;
  
  if(sensores.getSensor(_sensor)==NULL) {sensor=NULL; Serial.printf("El sensor asociado a %s no existe\n",nombre.c_str());}
  else sensor=sensores.getSensor(_sensor);

  tipo=TIPO_NULO;
  if(_tipo.equals("TEMPERATURA")) tipo=TIPO_TEMPERATURA;
  if(_tipo.equals("HUMEDAD")) tipo=TIPO_HUMEDAD;
  if(_tipo.equals("PRESION")) tipo=TIPO_PRESION;
  if(_tipo.equals("LUZ")) tipo=TIPO_LUZ;

  pHumbral=NULL;
  numeroHumbrales=0;
  siguiente=NULL;

  //Traza resumen de la variable, antes de leer los humbrales
  Serial.printf("Configuracion leida:\nnombre variable: %s\ndescripcion: %s\nunidades: %s\nsensor : %s\ntipo: %i\n",nombre.c_str(),descripcion.c_str(),unidades.c_str(),sensor->getNombre().c_str(),tipo);

  //Cargo los humbrales
  if(_config.containsKey("Humbrales")){
    JsonArray& HumbralesJson = _config["Humbrales"];

    numeroHumbrales=HumbralesJson.size();

    for(uint8_t i=0;i<numeroHumbrales;i++){//For para los humbrales de una variable
      JsonObject& humbralJson = HumbralesJson[i];

      Humbral* q=(Humbral*) new Humbral(humbralJson);
      if(q==NULL) {
        Serial.printf("Error al asignar memoria a la variable %s\n", humbralJson.get<String>("nombre").c_str());
        return;
        }   

      if(pHumbral==NULL) pHumbral=q;    
      else {
        Humbral *busca=pHumbral;
        while(busca->getSiguiente()!=NULL){
          busca=busca->getSiguiente();
        }
        busca->setSiguiente(q);
      }    
    }
  }
};
/*********************************************** FIN Metodos de configuracion *****************************************************/

void Variable::comparaHumbrales(void){
  Humbral* pMovil=pHumbral;

  while(pMovil!=NULL){
    //Serial.printf("variable: %s | nombre humbral: %s |tipo de humbral: %i valor de la variable: %.2f |valor del humbral %.2f\n",nombre.c_str(),pMovil->getNombre().c_str(),pMovil->getTipo(),valor,pMovil->getValor());    
    if(((pMovil->getTipo()==TIPO_VALOR_ES_SUPERIOR) && (valor>pMovil->getValor())) || 
      ((pMovil->getTipo()==TIPO_VALOR_ES_INFERIOR) && (valor<pMovil->getValor()))){ 
      String mensaje="{\"variable\": \"" + nombre + "\", \"humbral\":\"" + pMovil->getNombre() + "\", \"tipo\":" + pMovil->getTipo() + ", \"mensajeHumbral\":\"" + pMovil->getMensaje() + "\", \"valor\":" + valor + ", \"valorHumbral\":" + pMovil->getValor() + "}";
      //Serial.printf("Mensaje de humbral: %s\n",mensaje.c_str());
      enviarMQTT(TOPIC_HUMBRAL_VARIABLES, mensaje);      
    }
    pMovil=pMovil->getSiguiente();//siguiente humbral en la variable
  }
};

String Variable::generaJsonEstado(JsonObject& root){
  root["nombre"] = nombre;
  root["descripcion"]=descripcion;
  root["tipo"]=tipo;
  root["valor"] = valor;
  root["unidades"]=unidades;

  JsonArray& humbralesJson = root.createNestedArray("Humbrales");

  Humbral* pMovil=pHumbral;
  while(pMovil!=NULL){
    JsonObject& humbralJson = humbralesJson.createNestedObject();

    pMovil->generaJsonEstado(humbralJson);
    pMovil=pMovil->getSiguiente();
  }

  String salida="";
  root.printTo(salida);
  //Serial.printf("%s\n",salida.c_str());
  return salida;
}
/*********************************Fin Variable********************************************/

/************************************Humbral**********************************************/
Humbral::Humbral(JsonObject& _config){
  String _nombre=_config.get<String>("nombre");
  String _tipo=_config.get<String>("tipo");
  float _valor=_config.get<float>("valor");
  String _mensaje=_config.get<String>("mensaje");

  nombre=_nombre;  
  if(_tipo.equals("superior")) tipo=TIPO_VALOR_ES_SUPERIOR;
  else if(_tipo.equals("inferior")) tipo=TIPO_VALOR_ES_INFERIOR;
  else tipo=0;
  valor=_valor;
  mensaje=_mensaje;
  siguiente=NULL;

  //Traza para el humbral
  Serial.printf("\tHumbral nombre: %s | tipo: %i | valor: %.2f | mensaje: %s\n",nombre.c_str(),tipo,valor,mensaje.c_str());
};

String Humbral::generaJsonEstado(JsonObject& root){
  root["nombre"] = nombre;
  root["tipo"]=tipo;
  root["valor"] = valor;
  root["mensaje"]=mensaje;

  String salida="";
  root.printTo(salida);
  //Serial.printf("%s\n",salida.c_str());
  return salida;  
};
/*********************************Fin Variable********************************************/