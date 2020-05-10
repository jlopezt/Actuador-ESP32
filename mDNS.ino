/*
#ifdef ESP32
  #include <ESPmDNS.h>
#else
  #include <ESP8266mDNS.h>;
#endif

boolean inicializamDNS(const char *nombre)
  {  
  String mDNSnombre;
  if(nombre==NULL) mDNSnombre="actuador";
  else mDNSnombre=String(nombre);
  
  if (MDNS.begin(mDNSnombre.c_str()))
    {
    Serial.printf("mDNS iniciado. Nombre del dispositivo: %s\n",nombre);
    MDNS.addService("_http", "_tcp", 80);

  	return true;    
    }
  else Serial.printf("Error al iniciar mDNS\n");

	return false;    
  }
*/
