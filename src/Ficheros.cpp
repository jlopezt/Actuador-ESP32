/************************************************/
/*                                              */
/* Funciones para la gestion de ficheros en     */
/* memoria del modulo esp6288                   */
/*                                              */
/************************************************/

/***************************** Defines *****************************/
#ifndef FILE_APPEND
#define FILE_APPEND "a"
#endif
#ifndef FILE_WRITE
#define FILE_WRITE "w"
#endif
/***************************** Defines *****************************/

/***************************** Includes *****************************/
#include <Ficheros.h>
#include <FS.h>     //this needs to be first, or it all crashes and burns...
#include <SPIFFS.h> //para el ESP32
#include <SNTP.h>
/***************************** Includes *****************************/

/************************************************/
/* Inicializa el sistema de ficheros del modulo */
/************************************************/
boolean inicializaFicheros(int debug)
{
  //inicializo el sistema de ficheros
  if (!SPIFFS.begin(true)) 
    {
    Traza.mensaje("No se puede inicializar el sistema de ficheros\n");
    return (false);
    }
  return (true);
}

/************************************************/
/* Recupera los datos de                        */
/* de un archivo cualquiera                     */
/************************************************/
boolean leeFichero(String nombre, String &contenido)
  {
  boolean leido=false;
  
  Traza.mensaje("Inicio de lectura de fichero %s\n",nombre.c_str());

  if (SPIFFS.exists(nombre)) 
    {
    //file exists, reading and loading
    Traza.mensaje("Encontrado fichero de configuracion %s.\n",nombre.c_str());
    File configFile = SPIFFS.open(nombre, "r");
    if (configFile) 
      {
      Traza.mensaje("Abierto fichero de configuracion %s.\n",configFile.name());
      size_t size = configFile.size();

      // Allocate a buffer to store contents of the file.
      char *buff=NULL;
      buff=(char *)malloc(size+1);

      if(buff==NULL) 
        {
        Serial.printf("Fallo en allocar memoria\n");
        return false;
        }

      configFile.readBytes(buff, size);
      buff[size]=0;//pongo el fin de cadena
        
      contenido=String(buff);
      Traza.mensaje("Contenido del fichero: #%s#\n",contenido.c_str());
      free(buff);
      leido=true;
        
      configFile.close();//cierro el fichero
      Traza.mensaje("Cierro el fichero\n");
      }//la de abrir el fichero de configuracion del WiFi
      else Traza.mensaje("Fichero no se puede abrir\n");
    }//la de existe fichero
    else Traza.mensaje("Fichero no existe\n");
  
  return leido;
  }

/**********************************************************************/
/* Salva la cadena pasada al fichero especificado                     */
/* Si ya existe lo sobreescribe                                       */
/**********************************************************************/  
boolean salvaFichero(String nombreFichero, String nombreFicheroBak, String contenidoFichero)
  {
  boolean salvado=false;

  //file exists, reading and loading
  if(SPIFFS.exists(nombreFichero.c_str())) 
    {
    if(nombreFicheroBak!="")
      {
      Traza.mensaje("El fichero %s ya existe, se copiara con el nombre %s.\n",nombreFichero.c_str(),nombreFicheroBak.c_str());
        
      if(SPIFFS.exists(nombreFicheroBak.c_str())) SPIFFS.remove(nombreFicheroBak.c_str());  
      SPIFFS.rename(nombreFichero.c_str(),nombreFicheroBak.c_str());  
      }
    else Traza.mensaje("El fichero %s ya existe, sera sobreescrito.\n",nombreFichero.c_str());
    }

  Traza.mensaje("Nombre fichero: %s\nContenido fichero: %s\n",nombreFichero.c_str(),contenidoFichero.c_str());
   
  File newFile = SPIFFS.open(nombreFichero.c_str(), FILE_WRITE);//abro el fichero, si existe lo borra
  if (newFile) 
    {
    Traza.mensaje("Abierto fichero %s.\nGuardo contenido:\n#%s#\n",newFile.name(),contenidoFichero.c_str());
  
    newFile.print(contenidoFichero);
    newFile.close();//cierro el fichero
    Traza.mensaje("Cierro el fichero\n");
    salvado=true;
    }
  else Traza.mensaje("El fichero no se pudo abrir para escritura.\n");
      
  return salvado;
  }

/**********************************************************************/
/* Salva la cadena pasada al fichero especificado                     */
/* Si ya existe añade                                                 */
/**********************************************************************/  
boolean anadeFichero(String nombreFichero, String contenidoFichero,int debug)
  {
  boolean salvado=false;

  if(debug==1) Traza.mensaje("Nombre fichero: %s\nContenido fichero: %s\n",nombreFichero.c_str(),contenidoFichero.c_str());
    
  File newFile = SPIFFS.open(nombreFichero.c_str(), FILE_APPEND);//abro el fichero, si existe añade
  if (newFile) 
    {
    if(debug==1) Traza.mensaje("Abierto fichero %s.\nGuardo contenido:\n#%s#\n",newFile.name(),contenidoFichero.c_str());
  
    newFile.print(contenidoFichero);
    newFile.close();//cierro el fichero
    if(debug==1) Traza.mensaje("Cierro el fichero\n");
    salvado=true;
    }
  else Traza.mensaje("El fichero no se pudo abrir para escritura.\n");
      
  return salvado;
  }

/****************************************************/
/* Borra el fichero especificado                    */
/****************************************************/  
boolean borraFichero(String nombreFichero)
  {
  boolean borrado=false;

  //file exists, reading and loading
  if(!SPIFFS.exists(nombreFichero)) Traza.mensaje("El fichero %s no existe.\n", nombreFichero.c_str());
  else
    {
    if (SPIFFS.remove(nombreFichero)) 
      {
      borrado=true;
      Traza.mensaje("El fichero %s ha sido borrado.\n", nombreFichero.c_str());
      }
    else Traza.mensaje("No se pudo borrar el fichero %s .\n", nombreFichero.c_str());
    }  

  return borrado;
  }  

/************************************************/
/* Devuelve el nombre del direcotrio del        */
/* fichro que se pasa como parametro            */
/************************************************/
String directorioFichero(String nombreFichero)
  {
  if (!nombreFichero.startsWith("/")) nombreFichero="/" + nombreFichero;
  String cad=nombreFichero.substring(0,nombreFichero.lastIndexOf("/"));
  return(cad);
  }

/************************************************/
/* Devuelve si un nombre de fichero incluye     */
/* un directorio por que encuentre mas de una / */
/************************************************/
boolean esDirectorio(String nombre)
  {
  if(nombre.startsWith("/")) nombre=nombre.substring(1);//si empieza por / se lo quito

  if(nombre.indexOf("/")!=-1) return true;
  return false;
  }

/************************************************/
/* Recupera los ficheros almacenados en el      */
/* dispositivo. Devuelve una cadena separada    */
/* por SEPARADOR                                */
/************************************************/
String listadoFicheros2(String prefix)
  {   
  String salida="";

  if(!prefix.startsWith("/")) prefix="/" + prefix; Serial.printf("Prefix: %s\n",prefix.c_str());

  const size_t capacity = 2*JSON_ARRAY_SIZE(15) + JSON_OBJECT_SIZE(31);
  DynamicJsonBuffer jsonBuffer(capacity);

  JsonObject& json = jsonBuffer.createObject();
  json["padre"] = prefix;

  JsonArray& subdirectorios = json.createNestedArray("subdirectorios");
  JsonArray& ficheros = json.createNestedArray("ficheros");

  File root = SPIFFS.open(prefix);
  File file = root.openNextFile();

  while(file)
    {
    String fichero=String(file.name());Serial.printf("Nombre fichero: %s\n",fichero.c_str());
    //Si el nombre incluye el prefix, se lo quito
    uint8_t inicio=(fichero.indexOf(prefix)==-1?0:fichero.indexOf(prefix));
    fichero=fichero.substring(inicio+prefix.length()); Serial.printf("Nombre fichero: %s\n",fichero.c_str());

    if(esDirectorio(fichero)) 
      {Serial.printf("Es directorio: %s\n",fichero.c_str());
      //verifico que el directorio no este ya en la lista
      boolean existe=false;
      String subdir=fichero.substring(0,fichero.indexOf("/"));
      for(uint8_t i=0;i<subdirectorios.size();i++)
        {
        if(subdir==subdirectorios[i]) 
          {
          existe=true;
          break;
          }
        }
      if(!existe) subdirectorios.add(fichero.substring(0,fichero.indexOf("/")));
      }
    else 
      {Serial.printf("Es fichero: %s\n",fichero.c_str());
      JsonObject& fichero_nuevo = ficheros.createNestedObject();
      fichero_nuevo["nombre"] = fichero;
      fichero_nuevo["tamano"] = file.size();
      fichero_nuevo["fechaEdicion"] = horaYfecha(file.getLastWrite());
      }

    file = root.openNextFile();
    }   
  json.printTo(salida);
  return (salida);
  }  
/////////////////////////////////////////////////////
String listadoFicheros(String prefix)
  {   
  String salida="";

  if(!prefix.startsWith("/")) prefix="/" + prefix;

  const size_t capacity = 2*JSON_ARRAY_SIZE(15) + JSON_OBJECT_SIZE(31);
  DynamicJsonBuffer jsonBuffer(capacity);

  JsonObject& json = jsonBuffer.createObject();
  json["padre"] = prefix;

  JsonArray& subdirectorios = json.createNestedArray("subdirectorios");
  JsonArray& ficheros = json.createNestedArray("ficheros");

  File root = SPIFFS.open(prefix);
  
  if(!root){
      Serial.println("Error al abrir el sistema de ficheros\n");
      return "";
  }  
  
  if(!root.isDirectory()){
      Serial.println("El prefijo indicado no es un directorio valido\n");
      return "";
  }
    
  File file = root.openNextFile();

  while(file)
    {
    String fichero=String(file.name());Serial.printf("Nombre fichero: %s\n",fichero.c_str());Serial.printf("Path del fichero: %s\n",root.path());
    
    if(file.isDirectory()) 
      {
      //Serial.printf("Es directorio: %s\n",fichero.c_str());
      //verifico que el directorio no este ya en la lista
      boolean existe=false;
      String subdir=fichero.substring(0,fichero.indexOf("/"));//!!!!!!!!!!!
      //Serial.printf("Direcotrio encontrado: %s\n",fichero);
      for(uint8_t i=0;i<subdirectorios.size();i++)
        {
        if(subdir==subdirectorios[i]) 
          {
          existe=true;
          break;
          }
        }
      if(!existe) subdirectorios.add(fichero.substring(0,fichero.indexOf("/")));
      }
    else 
      {
      //Serial.printf("Es fichero: %s\n",fichero.c_str());
      JsonObject& fichero_nuevo = ficheros.createNestedObject();
      fichero_nuevo["nombre"] = fichero;
      fichero_nuevo["tamano"] = file.size();
      fichero_nuevo["fechaEdicion"] = horaYfecha(file.getLastWrite());
      }

    file = root.openNextFile();
    }   
  json.printTo(salida);
  return (salida);
  }  

/************************************************/
/* Devuelve si existe o no un fichero en el     */
/* dispositivo                                  */
/************************************************/
boolean existeFichero(String nombre)
  {  
  return (SPIFFS.exists(nombre));
  }


/************************************************/
/* Formatea el sistemas de ficheros del         */
/* dispositivo                                  */
/************************************************/
boolean formatearFS(void)
  {  
  return (SPIFFS.format());
  }
  