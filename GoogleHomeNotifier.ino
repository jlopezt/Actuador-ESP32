/*********************************************************/
/*                                                       */
/*                                                       */
/*   Gestion de notificaciones a traves de Google Home   */
/*                                                       */
/*                                                       */
/*********************************************************/
#define NO_CONECTAR "NO_CONECTAR"

#include <esp8266-google-home-notifier.h>
//#include "D:\arduino\desarrollos\libraries\esp8266-google-home-notifier\src\esp8266mDNS.h"

/*Variable locale sa este modulo*/
GoogleHomeNotifier ghn;
int activaGoogleHomeNotifier=0;//no activo por defecto
String nombreEquipo="";//nombreEquipo: nombre del altavoz google home
String idioma="";//idioma: idioma del texto a reproducir: es español;en ingles

/*********************************************inicio configuracion**************************************************************************/
/***************************************************/
/*                                                 */
/* Inicializa la comunicacion con los dispositivos */
/* google Home de la casa. Los descubre a traves   */
/* del nombre por mDNS                             */
/*                                                 */
/***************************************************/
void inicializaGHN(void) 
  {
  //recupero datos del fichero de configuracion
  if (!recuperaDatosGHN(false)) Serial.printf("error al recuperar config de GHN.\nConfiguracion por defecto.\n");
    
  if(activaGoogleHomeNotifier==0 || strcmp(nombreEquipo.c_str(),NO_CONECTAR)==0) //Si se configura copmo nombre de equipo el valor de NO_CONECTAR, no se intenta la conexion
    {
    Serial.println("Google Home desactivado por configuración.\n");
    return;
    }

  Serial.println("conectando a Google Home...");
  if (ghn.device((const char*)nombreEquipo.c_str(), (const char*)idioma.c_str()) != true) 
    {
    Serial.println(ghn.getLastError());
    return;
    }
    
  Serial.printf("Google Home(%s:%i) encontrado\n",ghn.getIPAddress().toString().c_str(),ghn.getPort());
  }

/***************************************************/
/*                                                 */
/* Recupera los datos del archivo de configuracion */
/*                                                 */
/***************************************************/
boolean recuperaDatosGHN(boolean debug)
  {
  String cad="";
  if (debug) Serial.println("Recupero configuracion de archivo...");

  nombreEquipo="";
  idioma="";
    
  if(!leeFicheroConfig(GHN_CONFIG_FILE, cad))
    {
    //Algo salio mal, confgiguracion por defecto
    Serial.printf("No existe fichero de configuracion GHN o esta corrupto\n");
    cad="{\"activaGoogleHomeNotifier\": 0,\"nombreEquipo\": \"-----\", \"idioma\": \"es\"}";
    Serial.printf("Generada configuracion por defecto: \n%s\n",cad.c_str());
    //if (salvaFichero(GHN_CONFIG_FILE, GHN_CONFIG_BAK_FILE, cad)) Serial.printf("Fichero de configuracion de GHN creado por defecto\n");
    }
  return parseaConfiguracionGHN(cad);    
  }  

/***********************************************************/
/*                                                         */
/* Parsea el json leido del fichero de configuracio de GHN */
/*                                                         */
/***********************************************************/
boolean parseaConfiguracionGHN(String contenido)
  {  
  DynamicJsonBuffer jsonBuffer;
  JsonObject& json = jsonBuffer.parseObject(contenido.c_str());
  //json.printTo(Serial);
  if (json.success()) 
    {
    Serial.println("parsed json");
//******************************Parte especifica del json a leer********************************
    nombreEquipo=json.get<String>("nombreEquipo");
    idioma=json.get<String>("idioma");
    activaGoogleHomeNotifier=json.get<unsigned int>("activaGoogleHomeNotifier");

    Serial.printf("Configuracion leida:\nactivaGoogleHomeNotifier: %i\nnombreEquipo: [%s]\nidioma: [%s]\n",activaGoogleHomeNotifier,nombreEquipo.c_str(),idioma.c_str());
//************************************************************************************************
    return true;
    }
  return false;
  }
/*********************************************fin configuracion*****************************************************************************/
/*********************************************Inicio funcionalidad**************************************************************************/
/***************************************************/
/*                                                 */
/* Envia la peticion a google para reproducir el   */
/* mensaje deseado en el google Home alque se      */
/* conecto                                         */
/*                                                 */
/***************************************************/
boolean enviaNotificacion(char *mensaje)
  {  
  if(activaGoogleHomeNotifier==0) return true; //Si se configura como nombre de equipo el valor de NO_CONECTAR, no se intenta la conexion
    
  if (ghn.notify(mensaje) != true) 
    {
    Serial.println(ghn.getLastError());
    return false;
    }
    
  Serial.println("Done.");
  return true;
  }
/*********************************************fin funcionalidad*****************************************************************************/
