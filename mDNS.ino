/*
#include <ESPmDNS.h>

boolean inicializamDNS(const char *nombre)
  {  
  String mDNSnombre;
  if(nombre==NULL) mDNSnombre="actuador";
  else mDNSnombre=String(nombre);
  
  if (MDNS.begin(mDNSnombre.c_str()))
    {
    Traza.mensaje("mDNS iniciado. Nombre del dispositivo: %s\n",nombre);
    MDNS.addService("_http", "_tcp", 80);

  	return true;    
    }
  else Traza.mensaje("Error al iniciar mDNS\n");

	return false;    
  }
*/
