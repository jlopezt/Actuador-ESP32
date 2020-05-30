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
#include <ArduinoOTA.h>

void gestionaOTA(void)
  {
   ArduinoOTA.handle();
   }

void inicioOTA(void)
  {
  Traza.mensaje("Actualizacion OTA\n");
  }
  
void finOTA(void)
  {
  Traza.mensaje("Fin actualizacion\n");
  }

void progresoOTA(unsigned int progress, unsigned int total)
  {
  String cad="";
  float avance=100*(float)progress/total;

  Traza.mensaje("actualizacion OTA en progreso: %5.1f %%\r",avance);
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

 boolean inicializaOTA(boolean debug)
  {    
  //OTA
  //Sets the device hostname. Default esp32-xxxxxx
  //ArduinoOTAClass&/void setHostname(const char *hostname);
  //Sets if the device should be rebooted after successful update. Default true
  //ArduinoOTAClass&/void setRebootOnSuccess(bool reboot);

  ArduinoOTA.setHostname(nombre_dispositivo.c_str());
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
