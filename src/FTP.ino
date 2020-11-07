/**********************************************/
/*                                            */
/*  Servidor FTP para el intercambio de       */
/*  ficheros                                  */
/*                                            */
/*  Utiliza libreria externa                  */
/*                                            */
/**********************************************/

#include <FtpServer.h>

String usuarioFTP="";
String passwordFTP="";

FtpServer ftpSrv;
/************************************** Funciones de configuracion ****************************************/
/*********************************************/
/* Inicializa el servidor FTP, recupera los  */
/* valores del fichero deconfiguracion       */
/*********************************************/
boolean inicializaFTP(boolean debug)
  {
  //Configuracion por defecto
  usuarioFTP= "usuario";
  passwordFTP="password";

  //leo la configuracion del fichero
  if(!recuperaDatosFTP(debug)) Serial.println("Configuracion del servidor FTP por defecto");
  else
    {
    Serial.printf("Servidor FTP:\nusuario: %s\nPassword: %s\n",usuarioFTP.c_str(), passwordFTP.c_str());  
    ftpSrv. begin(usuarioFTP, passwordFTP);
    }
  return true;  
  }

boolean recuperaDatosFTP(boolean debug)
  {
  String cad="";

  if (debug) Serial.println("Recupero configuracion de archivo...");
  
  if(!leeFichero(FTP_CONFIG_FILE, cad)) 
    {
    //Confgiguracion por defecto
    Serial.printf("No existe fichero de configuracion del servidor FTP\n");    
    cad="{\"Usuario\": \"usuario\",\"Password\": \"password\"}";
    //Machaco el fichero de configuracion guardado con la configuracion por defecto
    //if(SistemaFicheros.salvaFichero(FTP_CONFIG_FILE, FTP_CONFIG_BAK_FILE, cad)) Serial.printf("Fichero de configuracion del servidor FTP creado por defecto\n");
    }      
    
  return parseaConfiguracionFTP(cad);
  }

/*********************************************/
/* Parsea el json leido del fichero de       */
/* configuracio de los reles                 */
/*********************************************/
boolean parseaConfiguracionFTP(String contenido)
  { 
  DynamicJsonBuffer jsonBuffer;
  JsonObject& json = jsonBuffer.parseObject(contenido.c_str());
  
  json.printTo(Serial);
  if (!json.success()) return false;
        
  Serial.println("parsed json");
//******************************Parte especifica del json a leer********************************
  //const char *user=(const char *)json["Usuario"];
  if (json.containsKey("Usuario")) usuarioFTP=String((const char *)json["Usuario"]);
  if (json.containsKey("Password")) passwordFTP=String((const char *)json["Password"]);

  Serial.printf("FTP:\nUsuario: %s\nPassword: %s\n",usuarioFTP.c_str(),passwordFTP.c_str()); 
//************************************************************************************************
  return true; 
  }
/**********************************************************Fin configuracion******************************************************************/  

void gestionaFTP(void) {ftpSrv.handleFTP();}
