/************************************************/
/*                                              */
/* Funciones para la gestion de ficheros en     */
/* memoria del modulo esp6288                   */
/*                                              */
/************************************************/

/************************************************/
/* Inicializa el sistema de ficheros del modulo */
/************************************************/
boolean inicializaFicheros(int debug)
{
  //inicializo el sistema de ficheros
  if (!SPIFFS.begin(true)) 
    {
    Serial.println("No se puede inicializar el sistema de ficheros");
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
  
  Serial.println("Inicio de lectura de fichero " + nombre);

  if (SPIFFS.exists(nombre)) 
    {
    //file exists, reading and loading
    Serial.printf("Encontrado fichero de configuracion %s.\n",nombre.c_str());
    File configFile = SPIFFS.open(nombre, "r");
    if (configFile) 
      {
      Serial.printf("Abierto fichero de configuracion %s.\n",configFile.name());
      size_t size = configFile.size();

      // Allocate a buffer to store contents of the file.
      char *buff=NULL;
      buff=(char *)malloc(size+1);

      configFile.readBytes(buff, size);
      buff[size]=0;//pongo el fin de cadena
        
      contenido=String(buff);
      Serial.printf("Contenido del fichero: #%s#\n",contenido.c_str());
      free(buff);
      leido=true;
        
      configFile.close();//cierro el fichero
      Serial.println("Cierro el fichero");
      }//la de abrir el fichero de configuracion del WiFi
      else Serial.println("Fichero no se puede abrir");
    }//la de existe fichero
    else Serial.println("Fichero no existe");
  
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
      Serial.printf("El fichero %s ya existe, se copiara con el nombre %s.\n",nombreFichero.c_str(),nombreFicheroBak.c_str());
        
      if(SPIFFS.exists(nombreFicheroBak.c_str())) SPIFFS.remove(nombreFicheroBak.c_str());  
      SPIFFS.rename(nombreFichero.c_str(),nombreFicheroBak.c_str());  
      }
    else Serial.printf("El fichero %s ya existe, sera sobreescrito.\n",nombreFichero.c_str());
    }

  Serial.print("Nombre fichero: ");
  Serial.println(nombreFichero.c_str());
  Serial.print("Contenido fichero: ");
  Serial.println(contenidoFichero.c_str());
   
  File newFile = SPIFFS.open(nombreFichero.c_str(), FILE_WRITE);//abro el fichero, si existe lo borra
  if (newFile) 
    {
    Serial.printf("Abierto fichero %s.\nGuardo contenido:\n#%s#\n",newFile.name(),contenidoFichero.c_str());
  
    newFile.print(contenidoFichero);
    newFile.close();//cierro el fichero
    Serial.println("Cierro el fichero");
    salvado=true;
    }
  else Serial.println("El fichero no se pudo abrir para escritura.\n");
      
  return salvado;
  }


/****************************************************/
/* Borra el fichero especificado                    */
/****************************************************/  
boolean borraFichero(String nombreFichero)
  {
  boolean borrado=false;

  //file exists, reading and loading
  if(!SPIFFS.exists(nombreFichero)) Serial.println("El fichero " + nombreFichero + " no existe.");
  else
    {
    if (SPIFFS.remove(nombreFichero)) 
      {
      borrado=true;
      Serial.println("El fichero " + nombreFichero + " ha sido borrado.");
      }
    else Serial.println("No se pudo borrar el fichero " + nombreFichero + ".");
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
    Serial.print("FILE: ");
    Serial.println(file.name());

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
  
