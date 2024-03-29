/*********************************************************/
/*                                                       */
/*                                                       */
/*   Gestion de notificaciones a traves de Google Home   */
/*                                                       */
/*                                                       */
/*********************************************************/

/***************************** Defines *****************************/
#define NO_CONECTAR "NO_CONECTAR"
/***************************** Defines *****************************/

/***************************** Includes *****************************/
#include <GoogleHomeNotifier.h>
#include <Ficheros.h>

#include <esp8266-google-home-notifier.h>
/***************************** Includes *****************************/

/*Variable locale sa este modulo*/
GoogleHomeNotifier ghn;
uint8_t activaGoogleHomeNotifier=0;//no activo por defecto
String nombreEquipo="";//nombreEquipo: nombre del altavoz google home
String idioma="";//idioma: idioma del texto a reproducir: es español;en ingles

boolean recuperaDatosGHN(boolean debug);
boolean parseaConfiguracionGHN(String contenido);

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
  if (!recuperaDatosGHN(false)) Traza.mensaje("error al recuperar config de GHN.\nConfiguracion por defecto.\n");
    
  if(activaGoogleHomeNotifier==0 || strcmp(nombreEquipo.c_str(),NO_CONECTAR)==0) //Si se configura copmo nombre de equipo el valor de NO_CONECTAR, no se intenta la conexion
    {
    Traza.mensaje("Google Home desactivado por configuración.\n");
    return;
    }

  Traza.mensaje("conectando a Google Home...\n");
  if (ghn.device((const char*)nombreEquipo.c_str(), (const char*)idioma.c_str()) != true) 
    {
    Traza.mensaje("%s\n",ghn.getLastError());
    return;
    }
    
  Traza.mensaje("Google Home(%s:%i) encontrado\n",ghn.getIPAddress().toString().c_str(),ghn.getPort());
  }

/***************************************************/
/*                                                 */
/* Recupera los datos del archivo de configuracion */
/*                                                 */
/***************************************************/
boolean recuperaDatosGHN(boolean debug)
  {
  String cad="";
  if (debug) Traza.mensaje("Recupero configuracion de archivo...\n");

  nombreEquipo="";
  idioma="";
    
  if(!leeFichero(GHN_CONFIG_FILE, cad))
    {
    //Algo salio mal, confgiguracion por defecto
    Traza.mensaje("No existe fichero de configuracion GHN o esta corrupto\n");
    cad="{\"activaGoogleHomeNotifier\": 0,\"nombreEquipo\": \"-----\", \"idioma\": \"es\"}";
    Traza.mensaje("Generada configuracion por defecto: \n%s\n",cad.c_str());
    //if (salvaFichero(GHN_CONFIG_FILE, GHN_CONFIG_BAK_FILE, cad)) Traza.mensaje("Fichero de configuracion de GHN creado por defecto\n");
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
  DynamicJsonDocument doc(512);
  DeserializationError err = deserializeJson(doc,contenido);

  if (err) {
    Serial.printf("Error deserializando el json %s\n",err.c_str());
    return false;
  }
  else 
    {
    Traza.mensaje("parsed json\n");
//******************************Parte especifica del json a leer********************************
    nombreEquipo=doc["nombreEquipo"].as<String>();
    idioma=doc["idioma"].as<String>();
    activaGoogleHomeNotifier=doc["activaGoogleHomeNotifier"].as<unsigned int>();

    Traza.mensaje("Configuracion leida:\nactivaGoogleHomeNotifier: %i\nnombreEquipo: [%s]\nidioma: [%s]\n",activaGoogleHomeNotifier,nombreEquipo.c_str(),idioma.c_str());
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
    Traza.mensaje("%s\n",ghn.getLastError());
    return false;
    }
    
  Traza.mensaje("Done.\n");
  return true;
  }
/*********************************************fin funcionalidad*****************************************************************************/

/*********************************************acecso a datos*****************************************************************************/
uint8_t getActivaGoogleHomeNotifier(void){return activaGoogleHomeNotifier;}
String getNombreEquipo(void) {return nombreEquipo;}
String getIdioma(void) {return idioma;}
/*********************************************fin acceso a datos*****************************************************************************/
