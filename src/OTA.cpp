/*********************************************************************
Funciones d egestion de la actualizacion OTA

Manual: http://esp8266.github.io/Arduino/versions/2.0.0/doc/ota_updates/ota_updates.html
Libreria: https://github.com/esp8266/Arduino/tree/master/libraries/ArduinoOTA

Funcines que provee al libreria:  
  void setPort(uint16_t port);//Sets the service port. Default 8266

  void setHostname(const char *hostname);  //Sets the device hostname. Default esp8266-xxxxxx
  String getHostname();
  
  void setPassword(const char *password);  //Sets the password that will be required for OTA. Default NULL
  void setPasswordHash(const char *password);//Sets the password as above but in the form MD5(password). Default NULL
  void setRebootOnSuccess(bool reboot);  //Sets if the device should be rebooted after successful update. Default true
  void onStart(THandlerFunction fn);  //This callback will be called when OTA connection has begun
  void onEnd(THandlerFunction fn);  //This callback will be called when OTA has finished
  void onError(THandlerFunction_Error fn);  //This callback will be called when OTA encountered Error
  void onProgress(THandlerFunction_Progress fn);  //This callback will be called when OTA is receiving data
  void begin();  //Starts the ArduinoOTA service
  void handle();  //Call this in loop() to run the service
  int getCommand();  //Gets update command type after OTA has started. Either U_FLASH or U_SPIFFS
**********************************************************************/

/***************************** Defines *****************************/
/***************************** Defines *****************************/

/***************************** Includes *****************************/
#include <Global.h>
#include <OTA.h>
#include <configNVS.h>
#include <ArduinoOTA.h>
#include <esp_ota_ops.h>
#include <esp_partition.h>
#include <Traza.h>
/***************************** Includes *****************************/

void gestionaOTA(void) {ArduinoOTA.handle();}

void inicioOTA(void) {Traza.mensaje("Actualizacion OTA\n");}
  
void finOTA(void) {Traza.mensaje("Fin actualizacion\n");}
/*
void progresoOTA(unsigned int progress, unsigned int total)
  {
  String cad="";
  float avance=100*(float)progress/total;

  Traza.mensaje("progreso: %5.1f %%\r",avance);
  }
*/

void progresoOTA(unsigned int progress, unsigned int total)
  {
  String cad="";
  float avance=100*(float)progress/total;

  Serial.printf("progreso: % 5.1f %%\r",avance);
  }

void erroresOTA(ota_error_t error)
  {
  Traza.mensaje("Error en actualizacion OTA ");    Traza.mensaje("Error[%u]: ", error);
  
  if (error == OTA_AUTH_ERROR) Traza.mensaje("Auth Failed\n");
  else if (error == OTA_BEGIN_ERROR) Traza.mensaje("Begin Failed\n");
  else if (error == OTA_CONNECT_ERROR) Traza.mensaje("Connect Failed\n");
  else if (error == OTA_RECEIVE_ERROR) Traza.mensaje("Receive Failed\n");
  else if (error == OTA_END_ERROR) Traza.mensaje("End Failed\n");
  }

void inicializaOTA(boolean debug)
  {    
  //OTA
  //Sets the device hostname. Default esp32-xxxxxx
  //ArduinoOTAClass&/void setHostname(const char *hostname);
  //Sets if the device should be rebooted after successful update. Default true
  //ArduinoOTAClass&/void setRebootOnSuccess(bool reboot);

  ArduinoOTA.setHostname(configNVS.nombreServicio.c_str());
  ArduinoOTA.setRebootOnSuccess(true);
  ArduinoOTA.setPassword((const char *)"88716");// No authentication by default

  //Configuramos las funciones CallBack
  ArduinoOTA.onStart(inicioOTA);
  ArduinoOTA.onEnd(finOTA);
  ArduinoOTA.onProgress(progresoOTA);
  ArduinoOTA.onError(erroresOTA);
  
  //iniciamos la gestion OTA
  ArduinoOTA.begin();
  } 

/*
   https://github.com/espressif/arduino-esp32/blob/master/tools/sdk/include/spi_flash/esp_partition.h

typedef struct {
    esp_partition_type_t type;          //!< partition type (app/data) 
    esp_partition_subtype_t subtype;    //!< partition subtype 
    uint32_t address;                   //!< starting address of the partition in flash 
    uint32_t size;                      //!< size of the partition, in bytes 
    char label[17];                     //!< partition label, zero-terminated ASCII string 
    bool encrypted;                     //!< flag is set to true if partition is encrypted 
} esp_partition_t;

typedef enum {
    ESP_PARTITION_TYPE_APP = 0x00,       //!< Application partition type
    ESP_PARTITION_TYPE_DATA = 0x01,      //!< Data partition type
} esp_partition_type_t;

typedef enum {
    ESP_PARTITION_SUBTYPE_APP_FACTORY = 0x00,                                 //!< Factory application partition
    ESP_PARTITION_SUBTYPE_APP_OTA_MIN = 0x10,                                 //!< Base for OTA partition subtypes
    ESP_PARTITION_SUBTYPE_APP_OTA_0 = ESP_PARTITION_SUBTYPE_APP_OTA_MIN + 0,  //!< OTA partition 0
    ESP_PARTITION_SUBTYPE_APP_OTA_1 = ESP_PARTITION_SUBTYPE_APP_OTA_MIN + 1,  //!< OTA partition 1
    ESP_PARTITION_SUBTYPE_APP_OTA_2 = ESP_PARTITION_SUBTYPE_APP_OTA_MIN + 2,  //!< OTA partition 2
    ESP_PARTITION_SUBTYPE_APP_OTA_3 = ESP_PARTITION_SUBTYPE_APP_OTA_MIN + 3,  //!< OTA partition 3
    ESP_PARTITION_SUBTYPE_APP_OTA_4 = ESP_PARTITION_SUBTYPE_APP_OTA_MIN + 4,  //!< OTA partition 4
    ESP_PARTITION_SUBTYPE_APP_OTA_5 = ESP_PARTITION_SUBTYPE_APP_OTA_MIN + 5,  //!< OTA partition 5
    ESP_PARTITION_SUBTYPE_APP_OTA_6 = ESP_PARTITION_SUBTYPE_APP_OTA_MIN + 6,  //!< OTA partition 6
    ESP_PARTITION_SUBTYPE_APP_OTA_7 = ESP_PARTITION_SUBTYPE_APP_OTA_MIN + 7,  //!< OTA partition 7
    ESP_PARTITION_SUBTYPE_APP_OTA_8 = ESP_PARTITION_SUBTYPE_APP_OTA_MIN + 8,  //!< OTA partition 8
    ESP_PARTITION_SUBTYPE_APP_OTA_9 = ESP_PARTITION_SUBTYPE_APP_OTA_MIN + 9,  //!< OTA partition 9
    ESP_PARTITION_SUBTYPE_APP_OTA_10 = ESP_PARTITION_SUBTYPE_APP_OTA_MIN + 10,//!< OTA partition 10
    ESP_PARTITION_SUBTYPE_APP_OTA_11 = ESP_PARTITION_SUBTYPE_APP_OTA_MIN + 11,//!< OTA partition 11
    ESP_PARTITION_SUBTYPE_APP_OTA_12 = ESP_PARTITION_SUBTYPE_APP_OTA_MIN + 12,//!< OTA partition 12
    ESP_PARTITION_SUBTYPE_APP_OTA_13 = ESP_PARTITION_SUBTYPE_APP_OTA_MIN + 13,//!< OTA partition 13
    ESP_PARTITION_SUBTYPE_APP_OTA_14 = ESP_PARTITION_SUBTYPE_APP_OTA_MIN + 14,//!< OTA partition 14
    ESP_PARTITION_SUBTYPE_APP_OTA_15 = ESP_PARTITION_SUBTYPE_APP_OTA_MIN + 15,//!< OTA partition 15
    ESP_PARTITION_SUBTYPE_APP_OTA_MAX = ESP_PARTITION_SUBTYPE_APP_OTA_MIN + 16,//!< Max subtype of OTA partition
    ESP_PARTITION_SUBTYPE_APP_TEST = 0x20,                                    //!< Test application partition

    ESP_PARTITION_SUBTYPE_DATA_OTA = 0x00,                                    //!< OTA selection partition
    ESP_PARTITION_SUBTYPE_DATA_PHY = 0x01,                                    //!< PHY init data partition
    ESP_PARTITION_SUBTYPE_DATA_NVS = 0x02,                                    //!< NVS partition
    ESP_PARTITION_SUBTYPE_DATA_COREDUMP = 0x03,                               //!< COREDUMP partition
    ESP_PARTITION_SUBTYPE_DATA_NVS_KEYS = 0x04,                               //!< Partition for NVS keys
    ESP_PARTITION_SUBTYPE_DATA_EFUSE_EM = 0x05,                               //!< Partition for emulate eFuse bits

    ESP_PARTITION_SUBTYPE_DATA_ESPHTTPD = 0x80,                               //!< ESPHTTPD partition
    ESP_PARTITION_SUBTYPE_DATA_FAT = 0x81,                                    //!< FAT partition
    ESP_PARTITION_SUBTYPE_DATA_SPIFFS = 0x82,                                 //!< SPIFFS partition

    ESP_PARTITION_SUBTYPE_ANY = 0xff,                                         //!< Used to search for partitions with any subtype
} esp_partition_subtype_t;

const esp_partition_t *esp_ota_get_running_partition(void); //La quew se esta ejecutando
const esp_partition_t *esp_ota_get_boot_partition(void); //La proxima en arrancar
const esp_partition_t *esp_ota_get_next_update_partition(const esp_partition_t *start_from);//Donde va a ir el proximo update
esp_err_t esp_ota_set_boot_partition(const esp_partition_t *partition);//Cambia la del proximo arranque

esp_partition_iterator_t esp_partition_find(esp_partition_type_t type, esp_partition_subtype_t subtype, const char* label);
const esp_partition_t* esp_partition_find_first(esp_partition_type_t type, esp_partition_subtype_t subtype, const char* label);
const esp_partition_t* esp_partition_get(esp_partition_iterator_t iterator);
esp_partition_iterator_t esp_partition_next(esp_partition_iterator_t iterator);
void esp_partition_iterator_release(esp_partition_iterator_t iterator);
*/

String getParticionProximoUpdate(void)
  {
  const esp_partition_t *particion=esp_ota_get_next_update_partition(NULL);

  String cad="";//"Particion proximo update:\n";
  cad +=particion->label;
  
  return cad;
  }

String getParticionEjecucion(void)
  {
  const esp_partition_t *particion=esp_ota_get_running_partition();

  String cad="";//"Particion en ejecucion:\n";
  cad +=particion->label;
  
  return cad;
  }

String getParticionProximoArranque(void)
  {
  const esp_partition_t *particion=esp_ota_get_boot_partition();
  String cad="";//"Particion del proximo arranque:\n";
  cad +=particion->label;
  
  return cad;
  }

boolean setParticionProximoArranque(String nombre)
  {
  esp_partition_iterator_t iterador=esp_partition_find(ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_ANY,nombre.c_str()); 
  const esp_partition_t *particion=esp_partition_get(iterador);
  esp_partition_iterator_release(iterador);  

  if(esp_ota_set_boot_partition(particion)==ESP_OK) return true;
  return false;
  }

String listaParticiones(void)
  {
  String salida="";
  const esp_partition_t *particion;  
  
  esp_partition_iterator_t iterador=esp_partition_find(ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_ANY, NULL); //omito el nombre para sacar todas

  DynamicJsonDocument doc(512);

  //JsonObject& json = jsonBuffer.createObject();
  
  doc["particionEjecucion"] = getParticionEjecucion();
  doc["particionProximoArranque"] = getParticionProximoArranque();
  doc["particionProximoUpdate"] = getParticionProximoUpdate();

  JsonArray particiones = doc.createNestedArray("particiones");

  while(iterador!=NULL)
    {
    particion=esp_partition_get(iterador);

    JsonObject particionNueva = particiones.createNestedObject();
    particionNueva["nombre"] = String(particion->label);      
    //tipo
    if(particion->type==0) particionNueva["tipo"] = "app"; 
    else if(particion->type==0) particionNueva["tipo"] = "data"; 
    else particionNueva["tipo"] = "otros";   
    //subtipo
    if (particion->subtype==0) particionNueva["subtipo"] = "factory";
    else if(particion->subtype>=10 && particion->subtype<=26) particionNueva["subtipo"] =  String("ota_") + String(particion->subtype);
    else if(particion->subtype==ESP_PARTITION_SUBTYPE_DATA_NVS_KEYS) particionNueva["subtipo"] = "nvs";   
    else if (particion->subtype==ESP_PARTITION_SUBTYPE_DATA_SPIFFS) particionNueva["subtipo"] = "SPIFFS";
    else particionNueva["subtipo"] = "otros";
    particionNueva["direccion"] = String(particion->address);    
    particionNueva["tamano"] =String(particion->size);    
    particionNueva["encriptado"] = (particion->encrypted?String("Si"):String("No"));
  
    iterador=esp_partition_next(iterador);
    }

  serializeJsonPretty(doc,salida);
  return (salida);
  }