/*****************************************************/
/*                                                   */
/*        Coleccion de sensores para actuador        */
/*                                                   */
/*****************************************************/

/***************************** Defines *****************************/

/***************************** Defines *****************************/

/***************************** Includes *****************************/
#include <Sensores.h>
#include <Ficheros.h>
/***************************** Includes *****************************/

/***************************** Variables globales *****************************/
Sensores sensores;
/***************************** Variables globales *****************************/

/***************************** Constructor *****************************/
Sensores::Sensores(void){
    numeroSensores=0;
    pSensor=NULL;
    pCabeza=NULL;
}
/************************************** Fin constructor ****************************************/

/************************************** Funciones de configuracion ****************************************/
/*********************************************/
/* Inicializa los valores de los registros de*/
/* las entradas y recupera la configuracion  */
/*********************************************/
void Sensores::inicializa(void)
  {
  Wire.begin();

  //scannerI2C();
  //scannerOneWire();

  //recupero datos del fichero de configuracion
  if (!recuperaDatos(false)) 
    {
    Serial.printf("error al recuperar config de los sensores.\nConfiguracion por defecto.\n");
    return;
    }
  }
    
/************************************************/
/* Recupera los datos de configuracion          */
/* del archivo de sensores                      */
/************************************************/
boolean Sensores::recuperaDatos(boolean debug)
  {
  String cad="";

  if (debug) Serial.println("Recupero configuracion de archivo...");
 
  if(!leeFichero(SENSORES_CONFIG_FILE, cad))
    {
    //Algo salio mal, confgiguracion por defecto
    Serial.printf("No existe fichero de configuracion de Sensores o esta corrupto\n");
    cad="{}";
    }
    
  return parseaConfiguracion(cad);    
  }  

/*********************************************/
/* Parsea el json leido del fichero de       */
/* configuracio de sensores                  */
/*********************************************/
boolean Sensores::parseaConfiguracion(String contenido){  
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(contenido.c_str());
  //json.printTo(Serial);
  if (root.success()){
    Serial.println("parsed json");
//******************************Parte especifica del json a leer********************************
      
    String _nombre="";
    String _tipo="";
    String _parametros="";

    if(root.containsKey("Sensores")){
      JsonArray& SensoresJson = root["Sensores"];
    
      numeroSensores=SensoresJson.size();
      //sensores.sensor=new Sensor[numeroSensores];
    
      for(uint8_t i=0;i<numeroSensores;i++){
        JsonObject& SensoresJsonSensor = SensoresJson[i];
Serial.printf("********************************************************\n");
        _nombre=SensoresJsonSensor.get<String>("nombre");
        _tipo=SensoresJsonSensor.get<String>("tipo");
        if(SensoresJsonSensor.containsKey("parametros")) _parametros=SensoresJsonSensor.get<String>("parametros");
        else _parametros="";

        uint8_t miTipo=tipoSensor(_tipo);
        Sensor* p=NULL;
        switch (miTipo)
          {
          case TIPO_DS18B20:
            p=(SensorDS18B20*)  new SensorDS18B20();
            ((SensorDS18B20*)p)->inicializa(_nombre,tipoSensor(_tipo),_parametros);
            break;
          case TIPO_DHT22:
            p=(SensorDHT*)  new SensorDHT();
            ((SensorDHT*)p)->inicializa(_nombre,tipoSensor(_tipo),_parametros);
            break;
          case TIPO_HDC1080:
            p=(SensorHDC1080*)  new SensorHDC1080();
            ((SensorHDC1080*)p)->inicializa(_nombre,tipoSensor(_tipo),_parametros);
            break;
          case TIPO_BME280:
            p=(SensorBME280*)  new SensorBME280();
            ((SensorBME280*)p)->inicializa(_nombre,tipoSensor(_tipo),_parametros);
            break;
          case TIPO_BMP280:
            p=(SensorBMP280*)  new SensorBMP280();
            ((SensorBMP280*)p)->inicializa(_nombre,tipoSensor(_tipo),_parametros);
            break;
          case TIPO_GL5539:
            p=(SensorGL5539*)  new SensorGL5539();
            ((SensorGL5539*)p)->inicializa(_nombre,tipoSensor(_tipo),_parametros);
            break;
          case TIPO_BH1750:
            p=(SensorBH1750*)  new SensorBH1750();
            ((SensorBH1750*)p)->inicializa(_nombre,tipoSensor(_tipo),_parametros);
            break;
          case TIPO_SOILMOISTURECAPACITIVEV2:
            p=(SensorHumedadSuelo*)  new SensorHumedadSuelo();
            ((SensorHumedadSuelo*)p)->inicializa(_nombre,tipoSensor(_tipo),_parametros);
            break;
          default:
            Serial.printf("¿Pero que tipo es este? %s=%i\n",_tipo.c_str(),miTipo);
          }

        if(p==NULL) {
          Serial.printf("Error al asignar memoria al sensor %s\n", _nombre.c_str());
          return false;
        }

        if(pCabeza==NULL) pCabeza=p;
        else pSensor->setSiguiente(p);

        pSensor=p;

        Serial.printf("Configuracion leida:\nnombre sensor: %s\ntipo sensor : %s\nparametros: %s\n",_nombre.c_str(),_tipo.c_str(),_parametros.c_str());
        }
      }
//************************************************************************************************
    return true;
    }
  return false;
  }

void Sensores::scannerI2C(void)
{
  byte error, address;
  int nDevices=0;
 
  Serial.printf("\n--------------------------------Scanner I2C--------------------------------\n");
 
  for(address = 1; address < 127; address++ ) 
  {
 
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
 
    if (error == 0)
    {
      Serial.printf("Dispositivo I2C encontrando en la direccion 0x%02X\n",address);
      nDevices++;
    }
    else if (error==4) Serial.printf("Error desconocido en la direccion 0x%02X\n",address);
  }

  Serial.printf("\nSe encontraron %i dispositivos I2C\n",nDevices);
  Serial.printf("------------------------------Fin scanner I2C------------------------------\n\n");
}

void Sensores::scannerOneWire(void){
  uint8_t newAddr[8];
  int nDevices=0;
  oneWire.reset_search();

  Serial.printf("\n--------------------------------Scanner OneWire--------------------------------\n");
  while(oneWire.search(newAddr)){
    nDevices++;

    if (OneWire::crc8(newAddr, 7) != newAddr[7]) {
        Serial.println("CRC no es valido\n");
    }    
    else{
      Serial.printf("Dispositivo OneWire encontrado en la direccion %02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X",newAddr[0],newAddr[1],newAddr[2],newAddr[3],newAddr[4],newAddr[5],newAddr[6],newAddr[7]);
      switch(newAddr[0]){
        case 0x10:
          Serial.printf(" --> DS18B20/DS18S20\n");
          break;
        case 0x28:
          Serial.printf(" --> DS18B20\n");
          break;
        case 0x22:
          Serial.println(" --> DS1822\n");
          break;
        case 0x26:
          Serial.println(" --> DS2438\n");
          break;
        default:
          Serial.println("Tipo desconocido\n");          
      }
    }
  }

  Serial.printf("\nSe encontraron %i dispositivos OneWire\n",nDevices);
  Serial.printf("------------------------------Fin scanner OneWire------------------------------\n\n");
}

uint8_t Sensores::tipoSensor(String tipo){
  if(tipo=="DS18B20") return TIPO_DS18B20;
  else if(tipo=="HDC1080") return TIPO_HDC1080;
  else if(tipo=="DHT22") return TIPO_DHT22;
  else if(tipo=="BME280") return TIPO_BME280;
  else if(tipo=="BMP280") return TIPO_BMP280;
  else if(tipo=="GL5539") return TIPO_GL5539;
  else if(tipo=="BH1750") return TIPO_BH1750;
  else if(tipo=="SOILMOISTUREV2") return TIPO_SOILMOISTURECAPACITIVEV2;  
  //else if(tipo="NULO") return TIPO_NULO;
  return TIPO_NULO;
}
/*********************************************** FIN Metodos de configuracion *****************************************************/

void Sensores::lee(boolean debug){
  Sensor* p=pCabeza;
  
  while(p!=NULL){
    Sensor* q;
    switch (p->getTipo()){
      case TIPO_DS18B20:
        q= (SensorDS18B20*)p;
        ((SensorDS18B20*)q)->lee();
        break;
      case TIPO_DHT22:
        q= (SensorDHT*)p;
        ((SensorDHT*)q)->lee();
        break;
      case TIPO_HDC1080:
        q= (SensorHDC1080*)p;
        ((SensorHDC1080*)q)->lee();
        break;
      case TIPO_BME280:
        q= (SensorBME280*)p;
        ((SensorBME280*)q)->lee();
        break;
      case TIPO_BMP280:
        q= (SensorBMP280*)p;
        ((SensorBMP280*)q)->lee();
        break;
      case TIPO_GL5539:
        q= (SensorGL5539*)p;
        ((SensorGL5539*)q)->lee();
        break;
      case TIPO_BH1750:
        q= (SensorBH1750*)p;
        ((SensorBH1750*)q)->lee();
        break;
      case TIPO_SOILMOISTURECAPACITIVEV2:
        q= (SensorHumedadSuelo*)p;
        ((SensorHumedadSuelo*)q)->lee();
        break;
    }

      p=p->getSiguiente();
  }  
}

String Sensores::generaJsonEstado(boolean debug){
  Sensor* p=pCabeza;
  String salida="";

  const size_t capacity = JSON_ARRAY_SIZE(numeroSensores) + JSON_OBJECT_SIZE(1) + JSON_OBJECT_SIZE(2) + 3*JSON_OBJECT_SIZE(3);
  DynamicJsonBuffer jsonBuffer(capacity);

  JsonObject& root = jsonBuffer.createObject();

  JsonArray& medidas = root.createNestedArray("Medidas");

  while(p!=NULL){
    //Serial.printf("recuperando valores de  %s\n",p->getNombre().c_str());
    JsonObject& medida = medidas.createNestedObject();
    switch (p->getTipo()){
      case TIPO_DS18B20:
        ((SensorDS18B20*)p)->generaJsonEstado(medida);
        break;
      case TIPO_DHT22:
        ((SensorDHT*)p)->generaJsonEstado(medida);
        break;
      case TIPO_HDC1080:
        ((SensorHDC1080*)p)->generaJsonEstado(medida);
        break;
      case TIPO_BME280:
        ((SensorBME280*)p)->generaJsonEstado(medida);
        break;
      case TIPO_BMP280:
        ((SensorBMP280*)p)->generaJsonEstado(medida);
        break;
      case TIPO_GL5539:
        ((SensorGL5539*)p)->generaJsonEstado(medida);
        break;
      case TIPO_BH1750:
        ((SensorBH1750*)p)->generaJsonEstado(medida);
        break;
      case TIPO_SOILMOISTURECAPACITIVEV2:
        ((SensorHumedadSuelo*)p)->generaJsonEstado(medida);
        break;
      }
      p=p->getSiguiente();
  }

  root.printTo(salida);
  //Serial.printf("%s\n",salida.c_str());
  return salida;   
}