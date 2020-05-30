/************************************************/
/*                                              */
/* Funciones para la gestion de ficheros en     */
/* memoria del modulo esp6288                   */
/*                                              */
/************************************************/
#include <FS.h>     //this needs to be first, or it all crashes and burns...
#include <SPIFFS.h> //para el ESP32

#ifndef FILE_APPEND
#define FILE_APPEND "a"
#endif
#ifndef FILE_WRITE
#define FILE_WRITE "w"
#endif

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
/* Recupera los datos de configuracion          */
/* de un archivo, si hay bloqyueo de            */
/* configuracion, devuelve KO                   */
/************************************************/
boolean leeFicheroConfig(String nombre, String &contenido)
  {
  if(candado) return false; //la utima configuracion fue mal, los ficheros no valen. Tomamos configuracion pro defecto en todos los modulos
  return (leeFichero(nombre, contenido));  
  }

/************************************************/
/* Salva los datos de configuracion             */
/* de un archivo, si hay bloqyueo de            */
/* configuracion, devuelve KO                   */
/************************************************/
boolean salvaFicheroConfig(String nombreFichero, String nombreFicheroBak, String contenidoFichero)
  {
  if(candado) return false; //la utima configuracion fue mal, los ficheros no valen. Tomamos configuracion pro defecto en todos los modulos
  return (salvaFichero(nombreFichero, nombreFicheroBak, contenidoFichero));  
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
boolean anadeFichero(String nombreFichero, String contenidoFichero,int debug=0)
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
/* Recupera los ficheros almacenados en el      */
/* dispositivo. Devuelve una cadena separada    */
/* por SEPARADOR                                */
/************************************************/
boolean listaFicheros(String &contenido)
  {   
  contenido="";

  File root = SPIFFS.open("/");
  File file = root.openNextFile();
 
  while(file)
    {
    contenido += String(file.name());
    contenido += SEPARADOR;
      
    file = root.openNextFile();
    }   
  return (true);
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
  
