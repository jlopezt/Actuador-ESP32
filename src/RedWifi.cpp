/**********************************************/
/*                                            */
/*  Gestion del WiFi                          */
/*  Control de la conexion, SSID, clave       */
/*  Control de la IP, DefaulGw, DNS...        */
/*                                            */
/**********************************************/

/***************************** Defines *****************************/
#define NOMBRE_AP          "AP_actuador"
#define NOMBRE_mDNS_CONFIG "configurame"

#define MAX_LONG_NOMBRE_DISPOSITIVO 32
#define WIFI_PORTAL_TIMEOUT 5*60 //5 minutos en segundos
#define TIME_OUT 10000
#define DELAY_MULTIBASE 1000
/***************************** Defines *****************************/

/***************************** Includes *****************************/
#include <Global.h>
#include <RedWifi.h>
#include <Ficheros.h>

//needed for library
#include <WiFi.h> 
#include <WiFiMulti.h>
#include <ESPmDNS.h>
#include <DNSServer.h>
#include <ESPAsyncWiFiManager.h>          //https://github.com/alanswx/ESPAsyncWiFiManager
/***************************** Includes *****************************/

IPAddress wifiIP(0, 0, 0, 0);//0.0.0.0 significa que no hay IP fija
IPAddress wifiNet(0, 0, 0, 0);
IPAddress wifiGW(0, 0, 0, 0);
IPAddress wifiDNS1(0, 0, 0, 0);
IPAddress wifiDNS2(0, 0, 0, 0);
String mDNS="";

const char* ssid;
const char* password;

WiFiMulti MiWiFiMulti;

boolean conectado=false; //Si el portal de configuracion devolvio OK a la conexion

String generaJsonConfiguracionWifi(String configActual, String ssid, String password);
boolean conectaMultibase(boolean debug);
boolean conectaAutodetect(boolean debug);
boolean parseaConfiguracionWifi(String contenido);
boolean recuperaDatosWiFi(boolean debug);

/************************************************************************************/
boolean inicializamDNS(const char *nombre)
  {  
  String mDNSnombre;
  
  if(nombre==NULL) mDNSnombre=mDNS;
  else mDNSnombre=nombre;
  
  if (MDNS.begin(mDNSnombre.c_str()))
    {
    Traza.mensaje("mDNS iniciado. Nombre del dispositivo: %s\n",mDNSnombre.c_str());
    MDNS.addService("_http", "_tcp", 80);

    return true;    
    }
  else Traza.mensaje("Error al iniciar mDNS\n");

  return false;    
  }  

void salvaConfiguracion(void)
  {
  String cad="";
  
  Traza.mensaje("---------------------Salvando configuracion---------------\n");
  Traza.mensaje("Valores que voy a salvar:\nSSID : %s | Password : $s\n",WiFi.SSID().c_str(),WiFi.psk().c_str());

  if(!leeFichero(WIFI_CONFIG_FILE, cad)) Traza.mensaje("No se pudo leer el fichero\n");
  cad=generaJsonConfiguracionWifi(cad, WiFi.SSID(),WiFi.psk());
  if(!salvaFichero(WIFI_CONFIG_FILE, WIFI_CONFIG_BAK_FILE, cad)) Traza.mensaje("No se pudo salvar el fichero\n");  
  Traza.mensaje("---------------------Fin salvando configuracion---------------\n");
  
  conectado=true;
  }

/************************************************/
/* Recupera los datos de configuracion          */
/* del archivo de Wifi                          */
/************************************************/
boolean recuperaDatosWiFi(boolean debug)
  {
  String cad="";
  if (debug) Traza.mensaje("Recupero configuracion de archivo...\n");

  //cargo el valores por defecto
  wifiIP=IPAddress(0,0,0,0);
  wifiGW=IPAddress(0,0,0,0);
  wifiNet=IPAddress(0,0,0,0);
  wifiDNS1=IPAddress(0,0,0,0);
  wifiDNS2=IPAddress(0,0,0,0);
  mDNS=NOMBRE_mDNS_CONFIG;
  
  if(!leeFichero(WIFI_CONFIG_FILE, cad)) 
    {
    //Confgiguracion por defecto
    Traza.mensaje("No existe fichero de configuracion WiFi o no es valido\n");
    //cad="{\"wifi\": [ {\"ssid\": \"BASE0\" ,\"password\": \"11223344556677889900abcdef\"}, {\"ssid\": \"BASE1\" ,\"password\": \"11223344556677889900abcdef\"}, {\"ssid\": \"BASE2\" ,\"password\": \"11223344556677889900abcdef\"}, {\"ssid\": \"BASE-1\",\"password\": \"11223344556677889900abcdef\"}]}";
    //cad="{\"mDNS\":\"actuador\",\"wifiIP\":\"0.0.0.0\",   \"wifiGW\":\"0.0.0.0\",  \"wifiNet\": \"0.0.0.0\",   \"wifiDNS1\":\"0.0.0.0\",\"wifiDNS2\":\"0.0.0.0\",\"wifi\": []}";
    //if(salvaFichero(WIFI_CONFIG_FILE, WIFI_CONFIG_BAK_FILE, cad)) Traza.mensaje("Fichero de configuracion WiFi creado por defecto\n");
    }

  return(parseaConfiguracionWifi(cad));
  }

/*********************************************/
/* Parsea el json leido del fichero de       */
/* configuracio Wifi                         */
/*********************************************/
boolean parseaConfiguracionWifi(String contenido)
  {  
  DynamicJsonBuffer jsonBuffer;
  JsonObject& json = jsonBuffer.parseObject(contenido.c_str());
  if (json.success()) 
    {
    Traza.mensaje("\nparsed json\n");
//******************************Parte especifica del json a leer********************************
    if (json.containsKey("wifiIP")) wifiIP.fromString((const char *)json["wifiIP"]); 
    if (json.containsKey("wifiGW")) wifiGW.fromString((const char *)json["wifiGW"]);
    if (json.containsKey("wifiNet")) wifiNet.fromString((const char *)json["wifiNet"]); 
    if (json.containsKey("wifiDNS1")) wifiDNS1.fromString((const char *)json["wifiDNS1"]);
    if (json.containsKey("wifiDNS2")) wifiDNS2.fromString((const char *)json["wifiDNS2"]);
    if (json.containsKey("mDNS")) mDNS=String((const char *)json["mDNS"]);
    
    Traza.mensaje("Configuracion leida:\nmDNS: %s\nIP actuador: %s\nIP Gateway: %s\nIPSubred: %s\nIP DNS1: %s\nIP DNS2: %s\n",mDNS.c_str(),wifiIP.toString().c_str(),wifiGW.toString().c_str(),wifiNet.toString().c_str(),wifiDNS1.toString().c_str(),wifiDNS2.toString().c_str());    

    if (!json.containsKey("wifi")) return false;
    
    JsonArray& wifi = json["wifi"];
    if(wifi.size()==0) return false;
        
    for(uint8_t i=0;i<wifi.size();i++)
      {
      const char* wifi_ssid = wifi[i]["ssid"];
      const char* wifi_password = wifi[i]["password"];
      MiWiFiMulti.addAP(wifi_ssid , wifi_password);
      Traza.mensaje("Red *%s* añadida.\n",wifi_ssid);
      }//del for
//************************************************************************************************
    return true;
    }
  return false;
  }

boolean inicializaWifi(boolean debug)
  {
  //Desconecto si esta conectado
  WiFi.disconnect(true);//(false);   
  //No reconecta a la ultima WiFi que se conecto
  WiFi.persistent(false);  
  //Activo el modo de autoreconexion nuevo en version 1.5 (con el cambio a esp8266 2.4.2)
  WiFi.setAutoReconnect(true);   
  //Activo el modo solo estacion, no access point
  WiFi.mode(WIFI_OFF);
  WiFi.mode(WIFI_STA);

  if(recuperaDatosWiFi(debug))
    {
    Traza.mensaje("Conectando multibase\n");
    if (conectaMultibase(debug)) 
      {
      //Configuro la IP fija
      if (wifiIP!=IPAddress(0,0,0,0) && wifiGW!=IPAddress(0,0,0,0))
        {
        Traza.mensaje("Datos WiFi: IP fija-> %s, GW-> %s, subnet-> %s, DNS1-> %s, DNS2-> %s\n",wifiIP.toString().c_str(), wifiGW.toString().c_str(), wifiNet.toString().c_str(), wifiDNS1.toString().c_str(), wifiDNS2.toString().c_str());
        WiFi.config(wifiIP, wifiGW, wifiNet, wifiDNS1, wifiDNS2);
        }
      else Traza.mensaje("No hay IP fija\n");

      //Inicializo mDNS para localizar el dispositivo
      inicializamDNS(nombre_dispositivo.c_str());
  
      Traza.mensaje("------------------------WiFi conectada (configuracion almacenada)--------------------------------------\n");
      Traza.mensaje("WiFi conectada. IP: %s\n",WiFi.localIP().toString().c_str());
      WiFi.printDiag(Serial);
      Traza.mensaje("-------------------------------------------------------------------------------------------------------\n");

      return true;
      }
    }

  Traza.mensaje("Conectando autodetect\n");
  if (conectaAutodetect(debug))
    {
    Traza.mensaje("------------------------WiFi conectada(autodetect)--------------------------------------\n");
    Traza.mensaje("WiFi conectada. IP: %s\n",WiFi.localIP().toString().c_str());
    WiFi.printDiag(Serial);
    Traza.mensaje("----------------------------------------------------------------------------------------\n");

    return true;
    }
  return false;
  }

boolean conectaAutodetect(boolean debug)
  {
  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  AsyncWebServer server(80);
  DNSServer dns;
  AsyncWiFiManager wifiManager(&server,&dns);

  Traza.mensaje("\n Entrando...\n");
  
  //IP
  AsyncWiFiManagerParameter IPParametro("IP","IP",wifiIP.toString().c_str(),15+1);//,"Nombre del dispositivo");
  wifiManager.addParameter(&IPParametro);
  //Gateway
  AsyncWiFiManagerParameter GatewayParametro("Gateway","Gateway",wifiGW.toString().c_str(),15+1);//,"Nombre del dispositivo");
  wifiManager.addParameter(&GatewayParametro);
  //Subnet
  AsyncWiFiManagerParameter SubnetParametro("Subnet","Subnet",wifiNet.toString().c_str(),15+1);//,"Nombre del dispositivo");
  wifiManager.addParameter(&SubnetParametro);
  //DNS1
  AsyncWiFiManagerParameter DNS1Parametro("DNS1","DNS1",wifiDNS1.toString().c_str(),15+1);//,"Nombre del dispositivo");
  wifiManager.addParameter(&DNS1Parametro);
  //DNS2
  AsyncWiFiManagerParameter DNS2Parametro("DNS2","DNS2",wifiDNS2.toString().c_str(),15+1);//,"Nombre del dispositivo");
  wifiManager.addParameter(&DNS2Parametro);
  //mDNS name
  AsyncWiFiManagerParameter mDNSParametro("mDNS","mDNS",mDNS.c_str(),15+1);//,"Nombre del dispositivo");  
  wifiManager.addParameter(&mDNSParametro);
  
  //preparo la llamada a la funcion para salvar configuracion, 
  //wifiManager.setSaveConfigCallback(miSaveConfigCallback);//Preparo la funcion para salvar la configuracion
  //wifiManager.setAPCallback(miAPCallback);//llamada cuando se actie el portal de configuracion
  wifiManager.setConfigPortalTimeout(WIFI_PORTAL_TIMEOUT);
  
  //Si se ha configurado IP fija
  //if (wifiIP!=IPAddress(0,0,0,0)) wifiManager.setSTAStaticIPConfig(wifiIP,wifiGW,wifiNet);//Preparo la IP fija (IPAddress ip, IPAddress gw, IPAddress sn) 

  //Inicializo mDNS para localizar la base
  inicializamDNS(NOMBRE_mDNS_CONFIG);
  
  if (!wifiManager.startConfigPortal(NOMBRE_AP)) Traza.mensaje("Error al conectar. Salida por time-out\n");      
  else 
    {
    wifiIP.fromString(String(IPParametro.getValue()));
    wifiGW.fromString(String(GatewayParametro.getValue()));
    wifiNet.fromString(String(SubnetParametro.getValue()));
    wifiDNS1.fromString(String(DNS1Parametro.getValue()));
    wifiDNS2.fromString(String(DNS2Parametro.getValue()));
    mDNS=String(mDNSParametro.getValue()); //Serial.printf("longitud mDNS: %i\n",mDNSParametro.getValueLength());
    Traza.mensaje("Datos leidos del portal: \n IP fija-> %s\n GW-> %s\n subnet-> %s\n DNS1-> %s\n DNS2-> %s\n",wifiIP.toString().c_str(), wifiGW.toString().c_str(), wifiNet.toString().c_str(), wifiDNS1.toString().c_str(), wifiDNS2.toString().c_str());

    salvaConfiguracion();
    Traza.mensaje("Configuracion finalizada correctamente.\n Reinciando...\n");
    }
  
  ESP.restart();
  return true;
  }

boolean conectaMultibase(boolean debug)
  {
  // wait for WiFi connection
  int time_out=0;
  while(MiWiFiMulti.run()!=WL_CONNECTED)
    {
    Traza.mensaje("(Multi) Conectando Wifi...\n");
    //incluido en la funcion parpadeo como delay(DELAY/2);  
    parpadeaLed(1,DELAY_MULTIBASE);
    time_out += DELAY_MULTIBASE;
    if (time_out>TIME_OUT) 
      {
      if (debug) Traza.mensaje("No se pudo conectar al Wifi...\n");
      return FALSE; //No se ha conectado y sale con KO
      }
    }
    
  return TRUE; //se ha conectado y sale con OK
  }

/**********************************************************************/
/*            Devuelve la IP configurada en el dispositivo            */
/**********************************************************************/ 
String getIP(int debug) { return WiFi.localIP().toString();}

/*********************************************************************/
/*       Devuelve el nombre de la red a la que se ha conectado       */
/*********************************************************************/ 
String nombreSSID(void) {return WiFi.SSID();}  

/*********************************************************************/
/*             Watchdog de control para la conexion WiFi             */
/*********************************************************************/ 
void WifiWD(void) {if(WiFi.status() != WL_CONNECTED) ESP.restart();}

/**********************************************************************/
/* Salva la configuracion de las bases wifi conectada en formato json */
/**********************************************************************/  
String generaJsonConfiguracionWifi(String configActual, String ssid, String password)
  {
  boolean nuevo=true;
  String salida="";

  //Genero el nuevo JSON
  //7 elementos, 2 arrays de 2 parametros cada uno
  const size_t capacity = JSON_ARRAY_SIZE(2) + 2*JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(7) + 230;
  DynamicJsonBuffer jsonBufferNuevo(capacity);
  JsonObject& nuevoJson = jsonBufferNuevo.createObject();      

  //añado al nuevo los campos de IPs
  char cadIp[16]="";
  char cadGw[16]="";
  char cadNet[16]="";
  char cadDNS1[16]="";
  char cadDNS2[16]="";

  strcpy(cadIp,wifiIP.toString().c_str());
  strcpy(cadGw,wifiGW.toString().c_str());
  strcpy(cadNet,wifiNet.toString().c_str());
  strcpy(cadDNS1,wifiDNS1.toString().c_str());
  strcpy(cadDNS2,wifiDNS2.toString().c_str());    

  nuevoJson["mDNS"] = mDNS.c_str(); 
  nuevoJson["wifiIP"] = cadIp; 
  nuevoJson["wifiGW"] = cadGw; 
  nuevoJson["wifiNet"] = cadNet; 
  nuevoJson["wifiDNS1"] = cadDNS1; 
  nuevoJson["wifiDNS2"] = cadDNS2; 

  JsonArray& nuevoWifi=nuevoJson.createNestedArray("wifi");
  
  //Abro el actual
  DynamicJsonBuffer jsonBuffer;  
  JsonObject& json = jsonBuffer.parseObject(configActual.c_str());

  String temp;
  json.printTo(temp);//pinto el json que he leido
  Traza.mensaje("json creado:\n#%s#\n",temp.c_str());

  if (json.success()) 
    {
    Traza.mensaje("\nparsed json\n");   
/************************/
    JsonArray& wifi=(json.containsKey("wifi")?json["wifi"]:json.createNestedArray("wifi"));//parseo del fichero que he leido

    for(uint8_t i=0;i<wifi.size();i++)
      {
      if(strcmp(wifi[i]["ssid"],"NONE")) //Si la base no es NONE que es la que pongo cuando no hay fichero
        {
        //Comparo el que he leido del json con el que acabao de usar para conectar
        if (!strcmp(wifi[i]["ssid"],ssid.c_str())) //si ya existe actualizo la password
          {
          wifi[i]["password"] = password; //si son iguales guardo la password nueva
          Traza.mensaje("Se ha modificado la pass de %s\n",(const char *)wifi[i]["ssid"]);
          nuevo=false;//no es nuevo, ya he guardado el que acabo de usar
          break;
          }        
        }
      }//del for

    //si es nuevo, lo añado al nuevo
    if(nuevo==true)
      {
      JsonObject& elemento = nuevoWifi.createNestedObject();
      elemento["ssid"] = ssid;
      elemento["password"] = password;
      Traza.mensaje("Red %s añadida\n",ssid.c_str());
      }

    //Copio del viejo al nuevo
    for(uint8_t i=0;i<wifi.size();i++)
      {
      JsonObject& elemento = nuevoWifi.createNestedObject();
      elemento["ssid"] = wifi[i]["ssid"];
      elemento["password"] = wifi[i]["password"];
      Traza.mensaje("Red %s copiada al nuevo fichero\n",elemento.get<const char*>("ssid"));    
      }      
    }//la de parsear el json
  else//si no pude parsear el original, añado al nuevo la red configurada
    {
    JsonObject& elemento = nuevoWifi.createNestedObject();
    elemento["ssid"] = ssid;
    elemento["password"] = password;
    Traza.mensaje("Red %s creada en el nuevo fichero\n",ssid);     
    }

/************************/
  nuevoJson.printTo(salida);//pinto el json que he creado
  Traza.mensaje("json creado:\n#%s#\n",salida.c_str());

  return salida;  
  }
