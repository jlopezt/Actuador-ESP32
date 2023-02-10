#include <configNVS.h>
#include <Preferences.h>
#include <MD5.h>

Preferences prefs;
configNVS_t configNVS;

void inicializaNVS(void){
  if(!prefs.begin("configNVS",false)) Serial.printf("No se pudo iniciar NVS"); // use "config" namespace

  if(!prefs.isKey("DeviceID")) Serial.printf("No existe DeviceID\n");
  if(!prefs.isKey("nombreServicio")) Serial.printf("No existe nombreServicio\n");
  if(!prefs.isKey("nombremDNS")) Serial.printf("No existe nombremDNS\n");
  if(!prefs.isKey("SSID")) Serial.printf("No existe SSID\n");
  if(!prefs.isKey("PASS")) Serial.printf("No existe PASS\n");
  if(!prefs.isKey("usuario")) Serial.printf("No existe usuario\n");
  if(!prefs.isKey("contrasena")) Serial.printf("No existe contrasena\n");

  if (!leeConfigNVS(&configNVS)) {
    Serial.printf("error al leer NVS.\nBorrando e inicializando a valores por defecto\n");
    resetNVS_Total();

    escribeConfigNVSDefecto();
  }
  else{
    Serial.printf("Config NVS leida\nDevice ID: %016llx\nnombre Servicio: %s\nnombre mDNS: %s\nSSID: %s\nPass: (***...%s)\nusuario: %s\nuserPass: (***...%s)\n",configNVS.deviceID,configNVS.nombreServicio,configNVS.nombremDNS.c_str(),configNVS.SSID.c_str(),configNVS.pass.substring(configNVS.pass.length()-4).c_str(),configNVS.usuario.c_str(),configNVS.contrasena.substring(configNVS.contrasena.length()-4).c_str());
  }
}

boolean leeConfigNVS(configNVS_t *c){ 
  boolean ret=true;

  //deviceID
  if(!prefs.isKey("DeviceID")) Serial.printf("No existe DeviceID\n");
  else{
    c->deviceID=prefs.getULong64("DeviceID",0);
    ret=ret && true;
  }
  //Nombre de servicio
  if(!prefs.isKey("nombreServicio")) Serial.printf("No existe nombreServicio\n");
  else{
    c->nombreServicio=prefs.getString("nombreServicio",String());
    ret=ret && true;
  }  
  //nombre mDNS
  if(!prefs.isKey("nombremDNS")) Serial.printf("No existe nombremDNS\n");
  else{
    c->nombremDNS=prefs.getString("nombremDNS",String());
    ret=ret && true;
  }
  //SSID
  if(!prefs.isKey("SSID")) Serial.printf("No existe SSID\n");
  else{
    c->SSID=prefs.getString("SSID",String());
    ret=ret && true;
  }
  //pass
  if(!prefs.isKey("PASS")) Serial.printf("No existe PASS\n");
  else{
    c->pass=prefs.getString("PASS",String());
    ret=ret && true;
  }
  //usuario
  if(!prefs.isKey("usuario")) Serial.printf("No existe usuario\n");
  else{
    c->usuario=prefs.getString("usuario",String());
    ret=ret && true;
  }
  //pass
  if(!prefs.isKey("contrasena")) Serial.printf("No existe contrasena\n");
  else{
    c->contrasena=prefs.getString("contrasena",String());
    ret=ret && true;
  }

  return ret;
}

void escribeConfigNVS(configNVS_t c){
  size_t ret=0;

  ret=prefs.putULong64("DeviceID",c.deviceID);
  //Serial.printf("Salida de la escritura de deviceID (%016llx): %u\n",c.deviceID, ret);
  ret=prefs.putString("nombreServicio",c.nombreServicio);
  //Serial.printf("Salida de la escritura de nombre Servicio (%s): %u\n",c.nombreServicio.c_str(),ret);  
  ret=prefs.putString("nombremDNS",c.nombremDNS);
  //Serial.printf("Salida de la escritura de nombre mDNS (%s): %u\n",c.nombremDNS.c_str(),ret);
  ret=prefs.putString("SSID",c.SSID);
  //Serial.printf("Salida de la escritura de SSID (%s): %u\n",c.SSID.c_str(), ret);
  ret=prefs.putString("PASS",c.pass);
  //Serial.printf("Salida de la escritura de password (***...%s): %u\n",c.pass.substring(c.pass.length()-3).c_str(), ret);
  ret=prefs.putString("usuario",c.usuario);
  //Serial.printf("Salida de la escritura de usuario (%s): %u\n",c.usuario.c_str(), ret);
  
  c.contrasena=calculaContrasena(c);
  ret=prefs.putString("contrasena",c.contrasena);
  //Serial.printf("Salida de la escritura de contrasena (***...%s): %u\n",c.contrasena.substring(c.contrasena.length()-3).c_str(), ret);
}

void escribeConfigNVSDefecto(void){
  configNVS_t c;

    /*Inicio de valores por defecto*/
    c.deviceID=ESP.getEfuseMac();//DEVICE_ID;
    c.nombreServicio="";
    c.nombremDNS="";
    c.SSID="";
    c.pass="";
    c.usuario="";
    c.contrasena="";    
    /*Fin de valores por defecto*/

    escribeConfigNVS(c);
}

void resetNVS_WiFi(void){
  Serial.printf("Borrando datos de WiFi en NVS...\n");
  configNVS.nombremDNS="";
  configNVS.SSID="";
  configNVS.pass="";  
  
  escribeConfigNVS(configNVS); 
}

void resetNVS_user(void){
  Serial.printf("Borrando datos de usuario en NVS...\n");
  configNVS.usuario="";
  configNVS.contrasena="";
  configNVS.nombreServicio="";
  
  escribeConfigNVS(configNVS); 
}

void resetNVS_ID(void){
  configNVS.deviceID=ESP.getEfuseMac();//0;
  escribeConfigNVS(configNVS);   
}

void resetNVS_Total(void){
  prefs.clear();
}

String calculaContrasena(configNVS_t c){
  //Genero una contrasena estable, depende del usaurio y del chip ID
  char hexID[17];
  sprintf(hexID,"%016llx",c.deviceID);
  String temp=c.usuario+String(hexID);
  char c_temp[temp.length()];
  strcpy(c_temp,temp.c_str());
  //Serial.printf("@@@@@@@@@@@@@@@@@@@@@@@@ ID en hexa: %s\n",hexID);
  //Serial.printf("@@@@@@@@@@@@@@@@@@@@@@@@ Clave txt generada: %s\n",c_temp);
  unsigned char* hash=MD5::make_hash(c_temp);//generate the MD5 hash for our string
  char *md5str = MD5::make_digest(hash, 16);//generate the digest (hex encoding) of our hash
  free(hash);
  return String(md5str);  
}
