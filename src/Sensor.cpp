/*****************************************************/
/*                                                   */
/*        Coleccion de sensores para actuador        */
/*                                                   */
/*****************************************************/

/***************************** Defines *****************************/
#define MAX_INTENTOS_MEDIDA    5  //Numero de intentos maximos de leer del Dallas
#define HDC_DIRECCION_I2C    0x40 //Direccion I2C del HDC1080
#define BME280_DIRECCION_I2C 0x76 //Direccion I2C del BME280
#define BMP280_DIRECCION_I2C 0x76 //Direccion I2C del BMP280
#define SEALEVELPRESSURE_HPA 1024 //Presion a nivel del mar
#define BH1750_FONDO_ESCALA  3800 //Fondo de escala del sensor BH1750
#define SECO                  250 //Lectura del sensor en vacio
#define MOJADO                750 //Lectura del sensor en vaso de agua

//Valor de dato no leido
#define NO_LEIDO      -100.0

//Defines de pines de los captadores
#define ONE_WIRE_BUS              4//Pin donde esta el DS18B20
#define PIN_SENSOR_HUMEDAD_SUELO  8//Pin del sensor de humedad del suelo
#define LDR_PIN                  10

#define TOPIC_HUMBRAL_SENSORES "sensores"
/***************************** Defines *****************************/

/***************************** Includes *****************************/
#include <Sensores.h>
#include <Ficheros.h>
#include <MQTT.h>
/***************************** Includes *****************************/

/***************************** Variables globales *****************************/
// Declaracion de variables de los sensores
OneWire oneWire(ONE_WIRE_BUS);
/***************************** Variables globales *****************************/

/***********************************DS18B20*********************************************/
DallasTemperature* SensorDS18B20::ds18B20=(DallasTemperature *) new DallasTemperature(&oneWire);

SensorDS18B20::SensorDS18B20(){}

void SensorDS18B20::inicializa(String _nombre, uint8_t _tipo, String parametros){
  if(ds18B20!=NULL) SensorDS18B20::ds18B20->begin();

  for(uint8_t i=0;i<8;i++) direccion[i]=0;
  temperatura=NO_LEIDO;
  comparaHumbralTemperatura=0;
  humbralTemperatura=0;

  setNombre(_nombre);
  setTipo(_tipo);
  parseaConfiguracion(parametros);    
}

boolean SensorDS18B20::parseaConfiguracion(String contenido)  {  
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(contenido.c_str());
  //json.printTo(Serial);
  if (!root.success()){
      Serial.printf("error al parsear parametros de DS18B20\n");
      return false;
    }

  Serial.println("parsed json");
//******************************Parte especifica del json a leer********************************
  if(!root.containsKey("direccion")) return false; 
  String _direccion=root.get<String>("direccion");
  convierteDireccionOneWire(_direccion, direccion);

  if(!root.containsKey("compararTemperatura")) {Serial.printf("No se confiigura comparacion de temperatura\n");}
  else{
    if(!root.containsKey("humbralTemperatura")) {Serial.printf("ERROR: No hay humbral de temperatura\n");}
    else{
      comparaHumbralTemperatura=root.get<int>("compararTemperatura");
      humbralTemperatura=root.get<float>("humbralTemperatura");
    }
  }
   
  Serial.printf("Configuracion del DS18B20 leida:\nParamteros: %s\ndireccion (str): %s\n comparar T: %i | humbral T: %.2f\n",contenido.c_str(),_direccion.c_str(),comparaHumbralTemperatura,humbralTemperatura);
  Serial.printf("Direccion convertida: %02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X\n",direccion[0],direccion[1],direccion[2],direccion[3],direccion[4],direccion[5],direccion[6],direccion[7]);
//************************************************************************************************
  return true;
  }

void SensorDS18B20::lee(void){
  int8_t i=0;//hago como mucho MAX_INTENTOS_MEDIDA
  float tempC=0;
  
  if(ds18B20==NULL){
    Serial.printf("No se puede usar DS18B20\n");
    return;
    }

  do 
    {
    ds18B20->requestTemperatures(); 
    //tempC = ds18B20->getTempCByIndex(0);
    tempC = ds18B20->getTempC(direccion);
    if(tempC != 85.0 && tempC != (-127.0)) {
        temperatura=tempC;
        return;
        }
        
    delay(100);
    } while (i++<MAX_INTENTOS_MEDIDA);

    temperatura= NO_LEIDO;
    return;
  }

String SensorDS18B20::generaJsonEstado(JsonObject& root){
  root["nombre"] = getNombre();
  root["temperatura"] = temperatura;

  String salida="";
  root.printTo(salida);
  //Serial.printf("%s\n",salida.c_str());
  return salida;
}

String SensorDS18B20::generaJsonConfiguracion(JsonObject& root){
  root["nombre"]=getNombre();
  root["tipo"]="DS18B20";
  root["direccion"]=getDirecciontoString();  

  String salida="";
  root.printTo(salida);
  return salida;
}

String SensorDS18B20::getDirecciontoString(void){
  char temp[24]="";
  sprintf(temp,"%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X",direccion[0],direccion[1],direccion[2],direccion[3],direccion[4],direccion[5],direccion[6],direccion[7]);
  //Serial.printf("direccion: %s\n",temp);
  return String(temp);
}

uint8_t SensorDS18B20::compararHumbral(void){
  if(comparaHumbralTemperatura==0) return 0;

  uint8_t retorno=0;

  //Serial.printf("comparaHumbralTemperatura: %i | Valor: %.2f | humbra: %.2f\n",comparaHumbralTemperatura,temperatura,humbralTemperatura);
  if(comparaHumbralTemperatura>0){ 
    //Serial.printf("Entro en comparar\n");    
    if(temperatura>humbralTemperatura){
      //Serial.printf("Entro en si es mayo el valor que el humbral\n");
      retorno++;
      String mensaje="{\"sensor\":\"" + getNombre() + "\", \"variable\": \"temperatura\", \"humbral\":" + String(getHumbralTemperatura()) + ", \"nivel\":" + String(comparaHumbralTemperatura) + "}";
      enviarMQTT(TOPIC_HUMBRAL_SENSORES, mensaje);
    }
  }
  else{
    if(temperatura<humbralTemperatura){
      //Serial.printf("Entro en si es mayor el humbral que el valor\n");
      retorno++;
      String mensaje="{\"sensor\":\"" + getNombre() + "\", \"variable\": \"temperatura\", \"humbral\":" + String(getHumbralTemperatura()) + ", \"nivel\":" + String(comparaHumbralTemperatura) + "}";
      enviarMQTT(TOPIC_HUMBRAL_SENSORES, mensaje);
    }
  }

  return retorno;
}

uint8_t HextoInt(uint8_t c){
    if(c>=97 && c<=122) return c-97+10;//minusculas
    if(c>=65 && c<90) return c-65+10;//mayusculas
    if(c>=48 && c<57) return c-48;//numeros
    return 255;
    }
/***********************************FIN DS18B20*********************************************/

/***********************************DHT*********************************************/
SensorDHT::SensorDHT(void){}

void SensorDHT::inicializa(String _nombre, uint8_t _tipo, String parametros){
  pin=0;
  dht=NULL;
  temperatura=NO_LEIDO;
  humedad=NO_LEIDO;
  comparaHumbralTemperatura=0;
  humbralTemperatura=0;
  comparaHumbralHumedad=0;
  humbralHumedad=0;

  setNombre(_nombre);
  setTipo(_tipo);
  if (parseaConfiguracion(parametros)){
    dht=(DHT *) new DHT(pin,DHT22);
    if(dht!=NULL) dht->begin();
    }
  }

boolean SensorDHT::parseaConfiguracion(String contenido)  {  
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(contenido.c_str());
  //json.printTo(Serial);
  if (!root.success()){
    Serial.printf("error al parsear parametros de DHT\n");
    return false;
    }

  Serial.println("parsed json");
//******************************Parte especifica del json a leer********************************   
  if(!root.containsKey("pin")) {Serial.printf("No hay pin en <%s>\n",contenido.c_str());return false;}
  uint8_t _pin=root.get<int>("pin");
  setPin(_pin);

  if(!root.containsKey("compararTemperatura")) {Serial.printf("No se confiigura comparacion de temperatura\n");}
  else{
    if(!root.containsKey("humbralTemperatura")) {Serial.printf("ERROR: No hay humbral de temperatura\n");}
    else{
      comparaHumbralTemperatura=root.get<int>("compararTemperatura");
      humbralTemperatura=root.get<float>("humbralTemperatura");
    }
  }

  if(!root.containsKey("compararHumedad")) {Serial.printf("No se confiigura comparacion de humedad\n");}
  else{
    if(!root.containsKey("humbralHumedad")) {Serial.printf("ERROR: No hay humbral de humedad\n");}
    else{
      comparaHumbralHumedad=root.get<int>("compararHumedad");
      humbralHumedad=root.get<float>("humbralHumedad");
    }
  }
  
  Serial.printf("Configuracion del DHT leida:\nParamteros: %s\npin (int): %i\n comparar T: %i | humbral T: %.2f\n comparar H: %i | humbral H: %.2f\n",contenido.c_str(),_pin,comparaHumbralTemperatura,humbralTemperatura,comparaHumbralHumedad,humbralHumedad);
//************************************************************************************************
  return true;
  }

void SensorDHT::lee(void){
  float tempC=0;
  float h;

  if(dht==NULL) {
    Serial.printf("No se puede usar DHT\n");
    return;
    }

  // Lee el valor desde el sensor
  tempC = dht->readTemperature();
  if(!isnan(tempC)) temperatura=tempC;
  else temperatura=NO_LEIDO;

  // Lee el valor desde el sensor 
  h = dht->readHumidity();  //leo el sensor
  if(!isnan(h)) humedad=h;  //si no es nan lo guardo
  else humedad=NO_LEIDO;
  }

String SensorDHT::generaJsonEstado(JsonObject& root){
  root["nombre"] = getNombre();
  root["temperatura"] = temperatura;
  root["humedad"] = humedad;

  String salida="";
  root.printTo(salida);
  //Serial.printf("%s\n",salida.c_str());
  return salida;
}

String SensorDHT::generaJsonConfiguracion(JsonObject& root){
  root["nombre"]=getNombre();
  root["tipo"]="DHT22";
  root["pin"]=pin;  

  String salida="";
  root.printTo(salida);
  return salida;
}

uint8_t SensorDHT::compararHumbral(void){
  uint8_t retorno=0;

  //Humbral temperatura
  if(comparaHumbralTemperatura>0){ 
    if(temperatura>humbralTemperatura){
      retorno++;
      String mensaje="{\"sensor\":\"" + getNombre() + "\", \"variable\": \"temperatura\", \"humbral\":" + String(getHumbralTemperatura()) + ", \"nivel\":" + String(comparaHumbralTemperatura) + "}";
      enviarMQTT(TOPIC_HUMBRAL_SENSORES, mensaje);      
    }
  }
  else{
    if(temperatura<humbralTemperatura){
      retorno++;
      String mensaje="{\"sensor\":\"" + getNombre() + "\", \"variable\": \"temperatura\", \"humbral\":" + String(getHumbralTemperatura()) + ", \"nivel\":" + String(comparaHumbralTemperatura) + "}";
      enviarMQTT(TOPIC_HUMBRAL_SENSORES, mensaje);      
    }
  }

  //Humbral humedad
  if(comparaHumbralHumedad>0){ 
    if(humedad>humbralHumedad){
      retorno++;
      String mensaje="{\"sensor\":\"" + getNombre() + "\", \"variable\": \"humedad\", \"humbral\":" + String(getHumbralHumedad()) + ", \"nivel\":" + String(comparaHumbralHumedad) + "}";
      enviarMQTT(TOPIC_HUMBRAL_SENSORES, mensaje);      
    }
  }
  else{
    if(humedad<humbralHumedad){
      retorno++;
      String mensaje="{\"sensor\":\"" + getNombre() + "\", \"variable\": \"humedad\", \"humbral\":" + String(getHumbralHumedad()) + ", \"nivel\":" + String(comparaHumbralHumedad) + "}";
      enviarMQTT(TOPIC_HUMBRAL_SENSORES, mensaje);      
    }
  }

  return retorno;
}
/***********************************FIN DHT*********************************************/

/***********************************HDC1080*********************************************/
//ClosedCube_HDC1080* SensorHDC1080::hdc1080=(ClosedCube_HDC1080*) new ClosedCube_HDC1080();

SensorHDC1080::SensorHDC1080(void){}

void SensorHDC1080::inicializa(String _nombre, uint8_t _tipo, String parametros){
  temperatura=NO_LEIDO;
  humedad=NO_LEIDO;
  comparaHumbralTemperatura=0;
  humbralTemperatura=0;
  comparaHumbralHumedad=0;
  humbralHumedad=0;

  setNombre(_nombre);
  setTipo(_tipo);
  if(parseaConfiguracion(parametros)){
    hdc1080=(ClosedCube_HDC1080*) new ClosedCube_HDC1080();
    if(hdc1080!=NULL) hdc1080->begin(direccionI2C);
    } 
  }

boolean SensorHDC1080::parseaConfiguracion(String contenido)  {
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(contenido.c_str());
  //json.printTo(Serial);
  if (!root.success()){
      Serial.printf("error al parsear parametros de HDC1080\n");
      return false;
    }

  Serial.println("parsed json");
//******************************Parte especifica del json a leer********************************
  if(!root.containsKey("direccionI2C")) {Serial.printf("No hay direccionI2C en HDC1080\n");return false;}
  String _direccionI2CString=root.get<String>("direccionI2C");
  uint8_t _direccionI2C=convierteDireccionI2C(_direccionI2CString);
  setDireccionI2C(_direccionI2C);

  if(!root.containsKey("compararTemperatura")) {Serial.printf("No se confiigura comparacion de temperatura\n");}
  else{
    if(!root.containsKey("humbralTemperatura")) {Serial.printf("ERROR: No hay humbral de temperatura\n");}
    else{
      comparaHumbralTemperatura=root.get<int>("compararTemperatura");
      humbralTemperatura=root.get<float>("humbralTemperatura");
    }
  }

  if(!root.containsKey("compararHumedad")) {Serial.printf("No se confiigura comparacion de humedad\n");}
  else{
    if(!root.containsKey("humbralHumedad")) {Serial.printf("ERROR: No hay humbral de humedad\n");}
    else{
      comparaHumbralHumedad=root.get<int>("compararHumedad");
      humbralHumedad=root.get<float>("humbralHumedad");
    }
  }

  Serial.printf("Configuracion del HDC1080 leida:\nParamteros: %s\ndireccion (int): %i\n comparar T: %i | humbral T: %.2f\n comparar H: %i | humbral H: %.2f\n",contenido.c_str(),_direccionI2C,comparaHumbralTemperatura,humbralTemperatura,comparaHumbralHumedad,humbralHumedad);
//************************************************************************************************
  return true;
  }

void SensorHDC1080::lee(void){
  float tempC=0;
  float h;

  if(hdc1080==NULL){
    Serial.printf("No se puede usar HDC1080\n");
    return;
    }

  // Lee el valor desde el sensor
  delay(20);
  tempC = hdc1080->readTemperature();      
  if(!isnan(tempC) && tempC!=125) temperatura=tempC;
  else temperatura=NO_LEIDO;

  // Lee el valor desde el sensor
  h = hdc1080->readHumidity();
  if(!isnan(h)) humedad=h;
  else humedad= NO_LEIDO;
  }

String SensorHDC1080::generaJsonEstado(JsonObject& root){
  root["nombre"] = getNombre();
  root["temperatura"] = temperatura;
  root["humedad"] = humedad;

  String salida="";
  root.printTo(salida);
  //Serial.printf("%s\n",salida.c_str());
  return salida;
}

String SensorHDC1080::generaJsonConfiguracion(JsonObject& root){
  root["nombre"]=getNombre();
  root["tipo"]="HDC1080";
  //root["direccion"]=direccionI2C;  
  root["direccion"]="0x" + String(direccionI2C,HEX);

  String salida="";
  root.printTo(salida);
  return salida;
}

uint8_t SensorHDC1080::compararHumbral(void){
  uint8_t retorno=0;

  //Humbral temperatura
  if(comparaHumbralTemperatura>0){ 
    if(temperatura>humbralTemperatura){
      retorno++;
      String mensaje="{\"sensor\":\"" + getNombre() + "\", \"variable\": \"temperatura\", \"humbral\":" + String(getHumbralTemperatura()) + ", \"nivel\":" + String(comparaHumbralTemperatura) + "}";
      enviarMQTT(TOPIC_HUMBRAL_SENSORES, mensaje);      
    }
  }
  else{
    if(temperatura<humbralTemperatura){
      retorno++;
      String mensaje="{\"sensor\":\"" + getNombre() + "\", \"variable\": \"temperatura\", \"humbral\":" + String(getHumbralTemperatura()) + ", \"nivel\":" + String(comparaHumbralTemperatura) + "}";
      enviarMQTT(TOPIC_HUMBRAL_SENSORES, mensaje);      
    }
  }

  //Humbral humedad
  if(comparaHumbralHumedad>0){ 
    if(humedad>humbralHumedad){
      retorno++;
      String mensaje="{\"sensor\":\"" + getNombre() + "\", \"variable\": \"humedad\", \"humbral\":" + String(getHumbralHumedad()) + ", \"nivel\":" + String(comparaHumbralHumedad) + "}";
      enviarMQTT(TOPIC_HUMBRAL_SENSORES, mensaje);      
    }
  }
  else{
    if(humedad<humbralHumedad){
      retorno++;
      String mensaje="{\"sensor\":\"" + getNombre() + "\", \"variable\": \"humedad\", \"humbral\":" + String(getHumbralHumedad()) + ", \"nivel\":" + String(comparaHumbralHumedad) + "}";
      enviarMQTT(TOPIC_HUMBRAL_SENSORES, mensaje);      
    }
  }

  return retorno;
}
/***********************************FIN HDC1080*********************************************/

/***********************************BMP280*********************************************/
SensorBMP280::SensorBMP280(void){}

void SensorBMP280::inicializa(String _nombre, uint8_t _tipo, String parametros){
  temperatura=NO_LEIDO;
  presion=NO_LEIDO;
  comparaHumbralTemperatura=0;
  humbralTemperatura=0;
  comparaHumbralPresion=0;
  humbralPresion=0;

  setNombre(_nombre);
  setTipo(_tipo);

  if (parseaConfiguracion(parametros)) {
    bmp280=(Adafruit_BMP280*) new Adafruit_BMP280();
    if(bmp280!=NULL) bmp280->begin(direccionI2C);    
    }
  }

boolean SensorBMP280::parseaConfiguracion(String contenido)  {
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(contenido.c_str());
  //json.printTo(Serial);
  if (!root.success()){
      Serial.printf("error al parsear parametros de BMP280\n");
      return false;
    }

  Serial.println("parsed json");
//******************************Parte especifica del json a leer********************************
  if(!root.containsKey("direccionI2C")) {Serial.printf("No hay direccionI2C en BMP280\n");return false;}
  Serial.printf("Hay direccionI2C en BMP280\n");
  
  String _direccionI2CString=root.get<String>("direccionI2C");
  uint8_t _direccionI2C=convierteDireccionI2C(_direccionI2CString);
  setDireccionI2C(_direccionI2C);

  if(!root.containsKey("compararTemperatura")) {Serial.printf("No se confiigura comparacion de temperatura\n");}
  else{
    if(!root.containsKey("humbralTemperatura")) {Serial.printf("ERROR: No hay humbral de temperatura\n");}
    else{
      comparaHumbralTemperatura=root.get<int>("compararTemperatura");
      humbralTemperatura=root.get<float>("humbralTemperatura");
    }
  }

  if(!root.containsKey("compararPresion")) {Serial.printf("No se confiigura comparacion de presion\n");}
  else{
    if(!root.containsKey("humbralPresion")) {Serial.printf("ERROR: No hay humbral de presion\n");}
    else{
      comparaHumbralPresion=root.get<int>("compararPresion");
      humbralPresion=root.get<float>("humbralPresion");
    }
  }

  Serial.printf("Configuracion del BMp280 leida:\nParamteros: %s\ndireccion (int): %i\n comparar T: %i | humbral T: %.2f\n comparar P: %i | humbral P: %.2f\n",contenido.c_str(),_direccionI2C,comparaHumbralTemperatura,humbralTemperatura,comparaHumbralPresion,humbralPresion);  
//************************************************************************************************
  return true;
  }

void SensorBMP280::lee(void){
    float tempC=0;
    float p=0;

  if(bmp280==NULL){
    Serial.printf("No se puede usar BMP280\n");
    return;
    }

    // Lee el valor desde el sensor
    tempC = bmp280->readTemperature();      
    if(!isnan(tempC) && tempC!=125) temperatura=tempC;
    else temperatura=NO_LEIDO;

    // Lee el valor desde el sensor
    p = bmp280->readPressure();
    if(!isnan(p)) presion=p;
    else presion= NO_LEIDO;
  }

String SensorBMP280::generaJsonEstado(JsonObject& root){
  root["nombre"] = getNombre();
  root["temperatura"] = temperatura;
  root["presion"] = presion;

  String salida="";
  root.printTo(salida);
  //Serial.printf("%s\n",salida.c_str());
  return salida;
}

String SensorBMP280::generaJsonConfiguracion(JsonObject& root){
  root["nombre"]=getNombre();
  root["tipo"]="BMP280";
  //root["direccion"]=direccionI2C;  
  root["direccion"]="0x" + String(direccionI2C,HEX);

  String salida="";
  root.printTo(salida);
  return salida;
}

uint8_t SensorBMP280::compararHumbral(void){
  uint8_t retorno=0;

  //Humbral temperatura
  if(comparaHumbralTemperatura>0){ 
    if(temperatura>humbralTemperatura){
      retorno++;
      String mensaje="{\"sensor\":\"" + getNombre() + "\", \"variable\": \"temperatura\", \"humbral\":" + String(getHumbralTemperatura()) + ", \"nivel\":" + String(comparaHumbralTemperatura) + "}";
      enviarMQTT(TOPIC_HUMBRAL_SENSORES, mensaje);      
    }
  }
  else{
    if(temperatura<humbralTemperatura){
      retorno++;
      String mensaje="{\"sensor\":\"" + getNombre() + "\", \"variable\": \"temperatura\", \"humbral\":" + String(getHumbralTemperatura()) + ", \"nivel\":" + String(comparaHumbralTemperatura) + "}";
      enviarMQTT(TOPIC_HUMBRAL_SENSORES, mensaje);      
    }
  }

  //Humbral humedad
  if(comparaHumbralPresion>0){ 
    if(presion>humbralPresion){
      retorno++;
      String mensaje="{\"sensor\":\"\"" + getNombre() + "\"\", \"variable\": \"presion\", \"humbral\":" + String(getHumbralPresion()) + ", \"nivel\":" + String(comparaHumbralPresion) + "}";
      enviarMQTT(TOPIC_HUMBRAL_SENSORES, mensaje);      
    }
  }
  else{
    if(presion<humbralPresion){
      retorno++;
      String mensaje="{\"sensor\":\"" + getNombre() + "\", \"variable\": \"presion\", \"humbral\":" + String(getHumbralPresion()) + ", \"nivel\":" + String(comparaHumbralPresion) + "}";
      enviarMQTT(TOPIC_HUMBRAL_SENSORES, mensaje);      
    }
  }

  return retorno;
}
/***********************************FIN BMP280*********************************************/

/***********************************BME280*********************************************/
SensorBME280::SensorBME280(void){}

void SensorBME280::inicializa(String _nombre, uint8_t _tipo, String parametros){  
  temperatura=NO_LEIDO;
  humedad=NO_LEIDO;
  presion=NO_LEIDO;
  comparaHumbralTemperatura=0;
  humbralTemperatura=0;
  comparaHumbralPresion=0;
  humbralPresion=0;
  comparaHumbralHumedad=0;
  humbralHumedad=0;

  setNombre(_nombre);
  setTipo(_tipo);

  if (parseaConfiguracion(parametros)){
    bme280=(Adafruit_BME280*) new Adafruit_BME280();
    bme280->begin(direccionI2C);    
    }
  }

boolean SensorBME280::parseaConfiguracion(String contenido)  {
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(contenido.c_str());
  //json.printTo(Serial);
  if (!root.success()){
      Serial.printf("error al parsear parametros de BME280\n");
      return false;
    }

  Serial.println("parsed json");
//******************************Parte especifica del json a leer********************************
  if(!root.containsKey("direccionI2C")) {Serial.printf("No hay direccionI2C en BME280\n");return false;}
  Serial.printf("Hay direccionI2C en BME280\n");
  
  String _direccionI2CString=root.get<String>("direccionI2C");
  uint8_t _direccionI2C=convierteDireccionI2C(_direccionI2CString);
  setDireccionI2C(_direccionI2C);

  if(!root.containsKey("compararTemperatura")) {Serial.printf("No se confiigura comparacion de temperatura\n");}
  else{
    if(!root.containsKey("humbralTemperatura")) {Serial.printf("ERROR: No hay humbral de temperatura\n");}
    else{
      comparaHumbralTemperatura=root.get<int>("compararTemperatura");
      humbralTemperatura=root.get<float>("humbralTemperatura");
    }
  }

  if(!root.containsKey("compararHumedad")) {Serial.printf("No se confiigura comparacion de humedad\n");}
  else{
    if(!root.containsKey("humbralHumedad")) {Serial.printf("ERROR: No hay humbral de humedad\n");}
    else{
      comparaHumbralHumedad=root.get<int>("compararHumedad");
      humbralHumedad=root.get<float>("humbralHumedad");
    }
  }

  if(!root.containsKey("compararPresion")) {Serial.printf("No se confiigura comparacion de presion\n");}
  else{
    if(!root.containsKey("humbralPresion")) {Serial.printf("ERROR: No hay humbral de presion\n");}
    else{
      comparaHumbralPresion=root.get<int>("compararPresion");
      humbralPresion=root.get<float>("humbralPresion");
    }
  }

  Serial.printf("Configuracion del BME280 leida:\nParamteros: %s\ndireccion (int): %i\n comparar T: %i | humbral T: %.2f\n comparar H: %i | humbral H: %.2f\n comparar P: %i | humbral P: %.2f\n",contenido.c_str(),_direccionI2C,comparaHumbralTemperatura,humbralTemperatura,comparaHumbralHumedad,humbralHumedad,comparaHumbralPresion,humbralPresion);
//************************************************************************************************
  return true;
  }

void SensorBME280::lee(void){
    float tempC=0;
    float h=0;
    float p=0;

  if(bme280==NULL){
    Serial.printf("No se puede usar BME280\n");
    return;
    }

  // Lee el valor desde el sensor
  tempC = bme280->readTemperature();      
  if(!isnan(tempC) && tempC!=125) temperatura=tempC;
  else temperatura=NO_LEIDO;

  // Lee el valor desde el sensor
  h = bme280->readHumidity();
  if(!isnan(h)) humedad=h;
  else humedad= NO_LEIDO;

  // Lee el valor desde el sensor
  h = bme280->readPressure();
  if(!isnan(h)) presion=p;
  else presion= NO_LEIDO;
  }

String SensorBME280::generaJsonEstado(JsonObject& root){
  root["nombre"] = getNombre();
  root["temperatura"] = temperatura;
  root["humedad"] = humedad;
  root["presion"] = presion;

  String salida="";
  root.printTo(salida);
  //Serial.printf("%s\n",salida.c_str());
  return salida;
}

String SensorBME280::generaJsonConfiguracion(JsonObject& root){
  root["nombre"]=getNombre();
  root["tipo"]="BME280";
  //root["direccion"]=direccionI2C;  
  root["direccion"]="0x" + String(direccionI2C,HEX);

  String salida="";
  root.printTo(salida);
  return salida;
}

uint8_t SensorBME280::compararHumbral(void){
  uint8_t retorno=0;

  //Humbral temperatura
  if(comparaHumbralTemperatura>0){ 
    if(temperatura>humbralTemperatura){
      retorno++;
      String mensaje="{\"sensor\":\"" + getNombre() + "\", \"variable\": \"temperatura\", \"humbral\":" + String(getHumbralTemperatura()) + ", \"nivel\":" + String(comparaHumbralTemperatura) + "}";
      enviarMQTT(TOPIC_HUMBRAL_SENSORES, mensaje);      
    }
  }
  else{
    if(temperatura<humbralTemperatura){
      retorno++;
      String mensaje="{\"sensor\":\"" + getNombre() + "\", \"variable\": \"temperatura\", \"humbral\":" + String(getHumbralTemperatura()) + ", \"nivel\":" + String(comparaHumbralTemperatura) + "}";
      enviarMQTT(TOPIC_HUMBRAL_SENSORES, mensaje);      
    }
  }

  //Humbral humedad
  if(comparaHumbralHumedad>0){ 
    if(humedad>humbralHumedad){
      retorno++;
      String mensaje="{\"sensor\":\"" + getNombre() + "\", \"variable\": \"humedad\", \"humbral\":" + String(getHumbralHumedad()) + ", \"nivel\":" + String(comparaHumbralHumedad) + "}";
      enviarMQTT(TOPIC_HUMBRAL_SENSORES, mensaje);      
    }
  }
  else{
    if(humedad<humbralHumedad){
      retorno++;
      String mensaje="{\"sensor\":\"" + getNombre() + "\", \"variable\": \"humedad\", \"humbral\":" + String(getHumbralHumedad()) + ", \"nivel\":" + String(comparaHumbralHumedad) + "}";
      enviarMQTT(TOPIC_HUMBRAL_SENSORES, mensaje);      
    }
  }

  //Humbral presion
  if(comparaHumbralPresion>0){ 
    if(presion>humbralPresion){
      retorno++;
      String mensaje="{\"sensor\":\"" + getNombre() + "\", \"variable\": \"presion\", \"humbral\":" + String(getHumbralPresion()) + ", \"nivel\":" + String(comparaHumbralPresion) + "}";
      enviarMQTT(TOPIC_HUMBRAL_SENSORES, mensaje);      
    }
  }
  else{
    if(presion<humbralPresion){
      retorno++;
      String mensaje="{\"sensor\":\"" + getNombre() + "\", \"variable\": \"presion\", \"humbral\":" + String(getHumbralPresion()) + ", \"nivel\":" + String(comparaHumbralPresion) + "}";
      enviarMQTT(TOPIC_HUMBRAL_SENSORES, mensaje);      
    }
  }

  return retorno;
}
/***********************************FIN HDC1080*********************************************/

/***********************************BH1750*********************************************/
SensorBH1750::SensorBH1750(){}

void SensorBH1750::inicializa(String _nombre, uint8_t _tipo, String parametros){
  luz=NO_LEIDO;
  comparaHumbralLuz=0;
  humbralLuz=0;  

  setNombre(_nombre);
  setTipo(_tipo);

  if (parseaConfiguracion(parametros)) {
    bh1750=(BH1750*) new BH1750(direccionI2C);
    if(bh1750!=NULL) bh1750->begin(BH1750::CONTINUOUS_LOW_RES_MODE);    
    }
  }

boolean SensorBH1750::parseaConfiguracion(String contenido){
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(contenido.c_str());
  //json.printTo(Serial);
  if (!root.success()){
      Serial.printf("error al parsear parametros de BH1750\n");
      return false;
    }

  Serial.println("parsed json");
//******************************Parte especifica del json a leer********************************
  if(!root.containsKey("direccionI2C")) {Serial.printf("No hay direccionI2C en BH1750\n");return false;}
  
  String _direccionI2CString=root.get<String>("direccionI2C");
  uint8_t _direccionI2C=convierteDireccionI2C(_direccionI2CString);
  setDireccionI2C(_direccionI2C);

  if(!root.containsKey("compararLuz")) {Serial.printf("No se confiigura comparacion de luz\n");}
  else{
    if(!root.containsKey("humbralLuz")) {Serial.printf("ERROR: No hay humbral de luz\n");}
    else{
      comparaHumbralLuz=root.get<int>("compararLuz");
      humbralLuz=root.get<float>("humbralLuz");
    }
  }

  Serial.printf("Configuracion del BH1750 leida:\nParamteros: %s\ndireccion (int): %i\n comparar L: %i | humbral L: %.2f\n",contenido.c_str(),_direccionI2C,comparaHumbralLuz,humbralLuz);
//************************************************************************************************
  return true;  
}

void SensorBH1750::lee(void){
    // Lee el valor desde el sensor
    float l;
    
  if(bh1750==NULL){
    Serial.printf("No se puede usar BH1750\n");
    return;
    }

  l = bh1750->readLightLevel()*100.0/BH1750_FONDO_ESCALA;//fondo de escala tomo BH1750_FONDO_ESCALA, es aun mayor pero eso es mucha luz. responde entre 0 y 100
  if(isnan(l)){
    luz=NO_LEIDO;  
    return;
    }
  else{
    if(l>100) l=100; //valor entre 0 y 100. 100 luz intensa 0 oscuridad, si es mayor topo 100
    luz=l;
    }
  }

String SensorBH1750::generaJsonEstado(JsonObject& root){
  root["nombre"] = getNombre();
  root["luz"] = luz;

  String salida="";
  root.printTo(salida);
  //Serial.printf("%s\n",salida.c_str());
  return salida;
}

String SensorBH1750::generaJsonConfiguracion(JsonObject& root){
  root["nombre"]=getNombre();
  root["tipo"]="BH1750";
  root["direccion"]="0x" + String(direccionI2C,HEX);

  String salida="";
  root.printTo(salida);
  return salida;
}

uint8_t SensorBH1750::compararHumbral(void){
  uint8_t retorno=0;

  //Humbral temperatura
  if(comparaHumbralLuz>0){ 
    if(luz>humbralLuz){
      retorno++;
      String mensaje="{\"sensor\":\"" + getNombre() + "\", \"variable\": \"luz\", \"humbral\":" + String(getHumbralLuz()) + ", \"nivel\":" + String(comparaHumbralLuz) + "}";
      enviarMQTT(TOPIC_HUMBRAL_SENSORES, mensaje);      
    }
  }
  else{
    if(luz<humbralLuz){
      retorno++;
      String mensaje="{\"sensor\":\"" + getNombre() + "\", \"variable\": \"luz\", \"humbral\":" + String(getHumbralLuz()) + ", \"nivel\":" + String(comparaHumbralLuz) + "}";
      enviarMQTT(TOPIC_HUMBRAL_SENSORES, mensaje);      
    }
  }

  return retorno;
}
/***********************************FIN BH1750*********************************************/

/***********************************GL5539*********************************************/
SensorGL5539::SensorGL5539(void){}

void SensorGL5539::inicializa(String _nombre, uint8_t _tipo, String parametros){
  luz=NO_LEIDO;
  comparaHumbralLuz=0;
  humbralLuz=0;  

  setNombre(_nombre);
  setTipo(_tipo);
  if (parseaConfiguracion(parametros));
}

boolean SensorGL5539::parseaConfiguracion(String contenido) {
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(contenido.c_str());
  //json.printTo(Serial);
  if (!root.success()){
      Serial.printf("error al parsear parametros de GL5539\n");
      return false;
    }

  Serial.println("parsed json");
//******************************Parte especifica del json a leer********************************
  if(!root.containsKey("compararLuz")) {Serial.printf("No se confiigura comparacion de luz\n");}
  else{
    if(!root.containsKey("humbralLuz")) {Serial.printf("ERROR: No hay humbral de luz\n");}
    else{
      comparaHumbralLuz=root.get<int>("compararLuz");
      humbralLuz=root.get<float>("humbralLuz");
    }
  }

  Serial.printf("Configuracion del GL5539 leida:\nParamteros: %s\n comparar L: %i | humbral L: %.2f\n",contenido.c_str(),comparaHumbralLuz,humbralLuz);
//**********************************************************************************************  
  return true;  
}

void SensorGL5539::lee(void){
  // Lee el valor desde el sensor
  float l;
  
  l = (analogRead(LDR_PIN)*100/1024);//valor entre 0 y 100. 100 luz intensa 0 oscuridad
  if(!isnan(l)) luz=l;
  else luz= NO_LEIDO;
  }

String SensorGL5539::generaJsonEstado(JsonObject& root){
  root["nombre"] = getNombre();
  root["luz"] = luz;

  String salida="";
  root.printTo(salida);
  //Serial.printf("%s\n",salida.c_str());
  return salida;
}

String SensorGL5539::generaJsonConfiguracion(JsonObject& root){
  root["nombre"]=getNombre();
  root["tipo"]="GL5539";

  String salida="";
  root.printTo(salida);
  return salida;
}

uint8_t SensorGL5539::compararHumbral(void){
  uint8_t retorno=0;

  //Humbral temperatura
  if(comparaHumbralLuz>0){ 
    if(luz>humbralLuz){
      retorno++;
      String mensaje="{\"sensor\":\"" + getNombre() + "\", \"variable\": \"luz\", \"humbral\":" + String(getHumbralLuz()) + ", \"nivel\":" + String(comparaHumbralLuz) + "}";
      enviarMQTT(TOPIC_HUMBRAL_SENSORES, mensaje);      
    }
  }
  else{
    if(luz<humbralLuz){
      retorno++;
      String mensaje="{\"sensor\":\"" + getNombre() + "\", \"variable\": \"luz\", \"humbral\":" + String(getHumbralLuz()) + ", \"nivel\":" + String(comparaHumbralLuz) + "}";
      enviarMQTT(TOPIC_HUMBRAL_SENSORES, mensaje);      
    }
  }

  return retorno;
}
/***********************************FIN GL5539*********************************************/

/***********************************HumedadSuelo*********************************************/
SensorHumedadSuelo::SensorHumedadSuelo(void){}

void SensorHumedadSuelo::inicializa(String _nombre, uint8_t _tipo, String parametros){
  humedad=NO_LEIDO;
  comparaHumbralHumedad=0;
  humbralHumedad=0;  

  setNombre(_nombre);
  setTipo(_tipo);
  if (parseaConfiguracion(parametros));    
}

boolean SensorHumedadSuelo::parseaConfiguracion(String contenido) {  
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(contenido.c_str());
  //json.printTo(Serial);
  if (!root.success()){
      Serial.printf("error al parsear parametros de HumedadSuelo\n");
      return false;
    }

  Serial.println("parsed json");
//******************************Parte especifica del json a leer********************************
  if(!root.containsKey("compararHumedad")) {Serial.printf("No se confiigura comparacion de humedad\n");}
  else{
    if(!root.containsKey("humbralHumedad")) {Serial.printf("ERROR: No hay humbral de humedad\n");}
    else{
      comparaHumbralHumedad=root.get<int>("compararHumedad");
      humbralHumedad=root.get<float>("humbralHumedad");
    }
  }

  Serial.printf("Configuracion de HumedadSuelo leida:\nParamteros: %s\n comparar H: %i | humbral H: %.2f\n",contenido.c_str(),comparaHumbralHumedad,humbralHumedad);
//**********************************************************************************************  
  return true;    
}

void SensorHumedadSuelo::lee(void){
  int sensorVal = analogRead(PIN_SENSOR_HUMEDAD_SUELO);
  humedad = map(sensorVal, MOJADO, SECO, 100, 0); 
  }

String SensorHumedadSuelo::generaJsonEstado(JsonObject& root){
  root["nombre"] = getNombre();
  root["humedad"] = humedad;

  String salida="";
  root.printTo(salida);
  //Serial.printf("%s\n",salida.c_str());
  return salida;
}

String SensorHumedadSuelo::generaJsonConfiguracion(JsonObject& root){
  root["nombre"]=getNombre();
  root["tipo"]="SOILMOISTURECAPACITIVEV2";

  String salida="";
  root.printTo(salida);
  return salida;
}

uint8_t SensorHumedadSuelo::compararHumbral(void){
  uint8_t retorno=0;

  //Humbral humedad
  if(comparaHumbralHumedad>0){ 
    if(humedad>humbralHumedad){
      retorno++;
      String mensaje="{\"sensor\":\"" + getNombre() + "\", \"variable\": \"humedad\", \"humbral\":" + String(getHumbralHumedad()) + ", \"nivel\":" + String(comparaHumbralHumedad) + "}";
      enviarMQTT(TOPIC_HUMBRAL_SENSORES, mensaje);      
    }
  }
  else{
    if(humedad<humbralHumedad){
      retorno++;
      String mensaje="{\"sensor\":\"" + getNombre() + "\", \"variable\": \"humedad\", \"humbral\":" + String(getHumbralHumedad()) + ", \"nivel\":" + String(comparaHumbralHumedad) + "}";
      enviarMQTT(TOPIC_HUMBRAL_SENSORES, mensaje);      
    }
  }

  return retorno;
}
/***********************************FIN HumedadSuelo*********************************************/

/***********************************Funciones comunes*********************************************/
uint8_t convierteDireccionI2C(String entrada){
  uint8_t salida=0;
  
  if(entrada.charAt(0)=='0' && entrada.charAt(1)=='x'){    
    for(uint8_t i=2;i<entrada.length();i++){
      salida *= 16;
      salida += HextoInt(entrada.charAt(i));
    }
  }
  else salida=(uint8_t)entrada.toInt();
  
  return salida;
}

void convierteDireccionOneWire(String s,uint8_t* direccion){
    if(s.length()==(2*8+7)){
        uint8_t h,l;
        for(uint8_t i=0;i<8;i++) {
            h=s.charAt(3*i);
            l=s.charAt(3*i+1);
            direccion[i]=HextoInt(h)*16+HextoInt(l);
            //Serial.printf("h: %c | l: %c | direccion[%i]: %02X\n",h,l,i,direccion[i]);
        }
    }
}
/*********************************Fin funciones comunes*******************************************/