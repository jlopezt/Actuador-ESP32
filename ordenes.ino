/*
 * ordenes.ino
 *
 * Comandos para la actualizacion de la hora del reloj
 *
 * Permite la puesta en hora del reloj a traves de comandos enviados por el puesto
 * serie desde el PC.
 *
 * Para actualizar la hora <comado> <valor>; Ejemplo: "hora 3;"
 * Se pueden anidar: "hora 2;minuto 33;"
 *
 */

#define LONG_COMANDO 40
#define LONG_PARAMETRO 30
#define LONG_ORDEN 22 //Comando (espacio) Parametros (fin de cadena)
#define MAX_COMANDOS   35

#include <Time.h>

char ordenRecibida[LONG_ORDEN]="";
int lonOrden=0;

typedef struct 
  {
  String comando;
  String descripcion;
  void (*p_func_comando) (int, char*, float)=NULL;
  }tipo_comando;
tipo_comando comandos[MAX_COMANDOS];

int HayOrdenes(int debug)
  {
  char inChar=0;
  
  while (Serial.available())
    {
    inChar=(char)Serial.read(); 
    switch (inChar)
      {
      case ';':
        //Recibido final de orden
        if (debug) Serial.printf("Orden recibida: %s\n",ordenRecibida);
        return(1);
        break;
      default:
        //Nuevo caracter recibido. Añado y sigo esperando
        ordenRecibida[lonOrden++]=inChar;
        ordenRecibida[lonOrden]=0;
        break;
      }
    }  
  return(0); //No ha llegado el final de orden
  }

int EjecutaOrdenes(int debug){
  String comando="";
  String parametros="";
  int iParametro=0;
  char sParametro[LONG_PARAMETRO]="";//LONG_PARAMETRO longitud maxmima del parametro
  float fParametro;
  int inicioParametro=0;

  if (debug) Serial.printf("Orden recibida: %s\n",ordenRecibida);
  
  for(int i=0;i<LONG_COMANDO;i++)
    {
    switch (ordenRecibida[i])
      {
      case ' ':
        //fin del comando, hay parametro
        inicioParametro=i+1;
        
        //Leo el parametro
        for (int j=0;j<LONG_ORDEN;j++)
          {  //Parsea la orden      
          if(ordenRecibida[j+inicioParametro]==0) 
            {
            strncpy(sParametro,ordenRecibida+inicioParametro,j+1);//copio el parametro como texto
            break;//j=LONG_ORDEN;//Cuando encuentro el final de la cadena
            }
          else iParametro=(iParametro*10)+(int)ordenRecibida[j+inicioParametro]-48; //hay que convertir de ASCII a decimal
          }
        fParametro=String(sParametro).toFloat();
        
        i=LONG_COMANDO;
        break;
      case 0:
        //fin de la orden. No hay parametro
        i=LONG_COMANDO;
        break;
      default:
        comando+=ordenRecibida[i];
        break;
      }
    }

  //Limpia las variables que3 recogen la orden
  lonOrden=0;
  ordenRecibida[0]=0;

  if (debug) Serial.printf("comando: %s\niParametro: %i\nsParametro: %s\nfParametro: %f\n",comando.c_str(),iParametro,sParametro,fParametro);
    
/**************Nueva funcion ***************************/
  int8_t indice=0;
  for(indice=0;indice<MAX_COMANDOS;indice++)
    {
    if (debug) Serial.printf("Comando[%i]: {%s} - {%s}\n",indice,comando.c_str(),comandos[indice].comando.c_str());

    if (comandos[indice].comando==comando) 
      {
      //Ejecuta la funcion asociada
      comandos[indice].p_func_comando(iParametro, sParametro, fParametro);
      return(0);
      }    
    }

  //Si llega aqui es que no ha encontrado el comando
  Serial.println("Comando no encontrado");
  return(-1);//Comando no encontrado  
/*******************************************************/
}

void limpiaOrden(void)
  {
  lonOrden=0;
  ordenRecibida[0]=0;
  }
  
void inicializaOrden(void)
  { 
  int i =0;  

  limpiaOrden();
  
  comandos[i].comando="help";
  comandos[i].descripcion="Listado de comandos";
  comandos[i++].p_func_comando=func_comando_help;
  
  comandos[i].comando="IP";
  comandos[i].descripcion="Direccion IP";
  comandos[i++].p_func_comando=func_comando_IP;

  comandos[i].comando="nivelActivo";
  comandos[i].descripcion="Configura el nivel activo de los reles";
  comandos[i++].p_func_comando=func_comando_nivelActivo;
  
  comandos[i].comando="restart";
  comandos[i].descripcion="Reinicia el modulo";
  comandos[i++].p_func_comando=func_comando_restart;
  
  comandos[i].comando="activa";
  comandos[i].descripcion="Activa el rele indicado";
  comandos[i++].p_func_comando=func_comando_activa;
    
  comandos[i].comando="desactiva";
  comandos[i].descripcion="Desactiva el rele indicado";
  comandos[i++].p_func_comando=func_comando_desactiva;

  comandos[i].comando="estadoRele";
  comandos[i].descripcion="Devuelve el estado del rele indicado";
  comandos[i++].p_func_comando=func_comando_estadoRele;

  comandos[i].comando="info";
  comandos[i].descripcion="Devuelve informacion del hardware";
  comandos[i++].p_func_comando=func_comando_info;
  
  comandos[i].comando="flist";
  comandos[i].descripcion="Lista los ficheros en el sistema de ficheros";
  comandos[i++].p_func_comando=func_comando_flist;

  comandos[i].comando="fexist";
  comandos[i].descripcion="Indica si existe un fichero en el sistema de ficheros";
  comandos[i++].p_func_comando=func_comando_fexist;
  
  comandos[i].comando="fopen";
  comandos[i].descripcion="Devuelve el contenido del fichero especificado";
  comandos[i++].p_func_comando=func_comando_fopen;
  
  comandos[i].comando="fremove";
  comandos[i].descripcion="Borra el fichero especificado";
  comandos[i++].p_func_comando=func_comando_fremove;
  
  comandos[i].comando="format";
  comandos[i].descripcion="Formatea el sistema de ficheros";
  comandos[i++].p_func_comando=func_comando_format;
  
  comandos[i].comando="hora";
  comandos[i].descripcion="Consulta la hora del sistema";
  comandos[i++].p_func_comando=func_comando_hora;
      
  comandos[i].comando="minuto";
  comandos[i].descripcion="Consulta los minutos del sistema";
  comandos[i++].p_func_comando=func_comando_minuto;
       
  comandos[i].comando="segundo";
  comandos[i].descripcion="Consulta los segundos del sistema";
  comandos[i++].p_func_comando=func_comando_segundo;
       
  comandos[i].comando="reloj";
  comandos[i].descripcion="Consulta el reloj del sistema";
  comandos[i++].p_func_comando=func_comando_reloj;

  comandos[i].comando="echo";
  comandos[i].descripcion="Devuelve el eco del sistema";
  comandos[i++].p_func_comando=func_comando_echo;
   
  comandos[i].comando="debug";
  comandos[i].descripcion="Activa/desactiva el modo debug";
  comandos[i++].p_func_comando=func_comando_debug;

  comandos[i].comando="enviarKeepAlive";
  comandos[i].descripcion="Activa/desactiva el envio de Kepp Alive MQTT";
  comandos[i++].p_func_comando=func_comando_enviarKeepAlive;
  
  comandos[i].comando="ES";
  comandos[i].descripcion="Entradas y Salidas";
  comandos[i++].p_func_comando=func_comando_ES;
  
  comandos[i].comando="actSec";
  comandos[i].descripcion="Activa secuenciador";
  comandos[i++].p_func_comando=func_comando_actSec;
  
  comandos[i].comando="desSec";
  comandos[i].descripcion="Desactiva secuenciador";
  comandos[i++].p_func_comando=func_comando_desSec;
  
  comandos[i].comando="estSec";
  comandos[i].descripcion="Estado del secuenciador";
  comandos[i++].p_func_comando=func_comando_estSec;    

  comandos[i].comando="MQTTConfig";
  comandos[i].descripcion="Configuración de MQTT";
  comandos[i++].p_func_comando=func_comando_MQTTConfig;

  comandos[i].comando="entradas";
  comandos[i].descripcion="JSON entradas";
  comandos[i++].p_func_comando=func_comando_Entradas;

  comandos[i].comando="salidas";
  comandos[i].descripcion="JSON salidas";
  comandos[i++].p_func_comando=func_comando_Salidas;

  //resto
  for(;i<MAX_COMANDOS;)
    {
    comandos[i].comando="vacio";
    comandos[i].descripcion="Comando vacio";
    comandos[i++].p_func_comando=func_comando_vacio;  
    }
    
  func_comando_help(0,"",0.0);
  }

/*********************************************************************/
/*  Funciones para los comandos                                      */
/*  void (*p_func_comando) (int, char*, float)                       */
/*********************************************************************/  
void func_comando_vacio(int iParametro, char* sParametro, float fParametro) //"vacio"
{}

void func_comando_help(int iParametro, char* sParametro, float fParametro) //"help"
  {
  Serial.printf("\n\nComandos:");  
  for(int8_t i=0;i<MAX_COMANDOS;i++) if (comandos[i].comando!=String("vacio")) Serial.printf("Comando %i: [%s]\n",i, comandos[i].comando.c_str());
  Serial.printf("\n------------------------------------------------------------------------------\n");
  }

void func_comando_IP(int iParametro, char* sParametro, float fParametro) //"IP"
  {
  boolean debug=false;
  Serial.println(getIP(debug));  
  }  

void func_comando_nivelActivo(int iParametro, char* sParametro, float fParametro) //"nivelActivo"
  {
  if(sParametro[0]!=0) 
    {
    nivelActivo=iParametro;

    String cad="";
    
    if(!leeFicheroConfig(GLOBAL_CONFIG_FILE, cad)) Serial.println("No se pudo leer el fichero");
    cad=generaJsonConfiguracionNivelActivo(cad, nivelActivo);
    if(!salvaFicheroConfig(GLOBAL_CONFIG_FILE, GLOBAL_CONFIG_BAK_FILE, cad)) Serial.println("No se pudo salvar el fichero");      
    }
  Serial.printf("\nNivel activo: %i\n",nivelActivo);  
  }  

void func_comando_activa(int iParametro, char* sParametro, float fParametro)//"activa")
  {
  conmutaRele(iParametro, nivelActivo, debugGlobal);  
  Serial.printf("\nRele %i activado\n",iParametro);
  }  

void func_comando_desactiva(int iParametro, char* sParametro, float fParametro)//"desactiva")
  {
  conmutaRele(iParametro, !nivelActivo, debugGlobal);
  Serial.printf("\nRele %i desactivado\n",iParametro);  
  }  

void func_comando_estadoRele(int iParametro, char* sParametro, float fParametro)//"estadoRele")
  { 
  Serial.printf("\nEl estado logico del rele %i es ",iParametro);  
  if (estadoRele(iParametro))Serial.printf("activado");
  else Serial.printf("desactivado");

  Serial.printf("\nEl estado fisico del rele %i es %i\nPines:\npin rele: %i\n",iParametro, digitalRead(reles[iParametro].pin),reles[iParametro].pin);
  }  
    
void func_comando_restart(int iParametro, char* sParametro, float fParametro)//"restart")
  {
  ESP.restart();
  }
  
void func_comando_info(int iParametro, char* sParametro, float fParametro)//"info")
  {
  Serial.printf("\n-----------------info logica-----------------\n");
  Serial.printf("Uptime: %lu segundos\n", (esp_timer_get_time()/(unsigned long)1000000)); //la funcion esp_timer_get_time() devuelve el contador de microsegundos desde el arranque. rota cada 292.000 años
  Serial.printf("-----------------------------------------------\n");  

  Serial.printf("\n-----------------info logica-----------------\n");
  Serial.printf("IP: %s\n", String(getIP(debugGlobal)).c_str());
  Serial.printf("nivelActivo: %s\n", String(nivelActivo).c_str());  
  for(int8_t i=0;i<MAX_RELES;i++) Serial.printf("Rele %i | nombre: %s | estado: %i\n", i,nombreRele(i).c_str(), estadoRele(i));
  Serial.printf("-----------------------------------------------\n");  
  
  Serial.printf("-------------------WiFi info-------------------\n");
  Serial.printf("SSID: %s\n",nombreSSID().c_str());
  Serial.printf("IP: %s\n", WiFi.localIP().toString().c_str());
  Serial.printf("Potencia: %s\n",String(WiFi.RSSI()).c_str());
  Serial.printf("-----------------------------------------------\n");   
      
  Serial.printf("-----------------Hardware info-----------------\n");
  //Serial.printf("Vcc: %i\n",ESP.getVcc());
  Serial.printf("FreeHeap: %i\n",ESP.getFreeHeap());
  Serial.printf("ChipId: %i\n",ESP.getChipRevision());
  Serial.printf("SdkVersion: %s\n",ESP.getSdkVersion());
  //Serial.printf("CoreVersion: %s\n",ESP.getCoreVersion().c_str());
  //Serial.printf("FullVersion: %s\n",ESP.getFullVersion().c_str());
  //Serial.printf("BootVersion: %i\n",ESP.getBootVersion());
  //Serial.printf("BootMode: %i\n",ESP.getBootMode());
  Serial.printf("CpuFreqMHz: %i\n",ESP.getCpuFreqMHz());

  //Serial.printf("FlashChipId: %i\n",ESP.getFlashChipId());
      //gets the actual chip size based on the flash id
  //Serial.printf("FlashChipRealSize: %i\n",ESP.getFlashChipRealSize());

      //gets the size of the flash as set by the compiler
  Serial.printf("FlashChipSize: %i\n",ESP.getFlashChipSize());
  Serial.printf("FlashChipSpeed: %i\n",ESP.getFlashChipSpeed());
      //FlashMode_t ESP.getFlashChipMode());
      
  //Serial.printf("FlashChipSizeByChipId: %i\n",ESP.getFlashChipSizeByChipId()); 
  Serial.printf("-----------------------------------------------\n");
  }  

void func_comando_flist(int iParametro, char* sParametro, float fParametro)//"fexist")
  {
  String contenido="";  
  if(listaFicheros(contenido)) 
    {
    contenido.replace("|","\n");
    Serial.printf("Contendio del sistema de ficheros:\n %s\n",contenido.c_str());
    }
  else Serial.printf("Ha habido un problema.....\n");
  }

void func_comando_fexist(int iParametro, char* sParametro, float fParametro)//"fexist")
  {
  if (sParametro=="") Serial.println("Es necesario indicar un nombre de fichero");
  else
    {
    if(SPIFFS.exists(sParametro)) Serial.printf("El fichero %s existe.\n",sParametro);
    else Serial.printf("NO existe el fichero %s.\n",sParametro);
    }
  }

void func_comando_fopen(int iParametro, char* sParametro, float fParametro)//"fopen")
  {
  if (sParametro=="") Serial.println("Es necesario indicar un nombre de fichero");
  else
    {
    File f = SPIFFS.open(sParametro, "r");
    if (f)
      { 
      Serial.println("Fichero abierto");
      size_t tamano_fichero=f.size();
      Serial.printf("El fichero tiene un tamaño de %i bytes.\n",tamano_fichero);
      char buff[tamano_fichero+1];
      f.readBytes(buff,tamano_fichero);
      buff[tamano_fichero+1]=0;
      Serial.printf("El contenido del fichero es:\n******************************************\n%s\n******************************************\n",buff);
      f.close();
      }
    else Serial.printf("Error al abrir el fichero %s.\n", sParametro);
    } 
  } 

void func_comando_fremove(int iParametro, char* sParametro, float fParametro)//"fremove")
  {
  if (sParametro=="") Serial.println("Es necesario indicar un nombre de fichero");
  else
    { 
    if (SPIFFS.remove(sParametro)) Serial.printf("Fichero %s borrado\n",sParametro);
    else Serial.printf("Error al borrar el fichero%s\n",sParametro);
    } 
 }

void func_comando_format(int iParametro, char* sParametro, float fParametro)//"format")
  {     
  if (formatearFS()) Serial.println("Sistema de ficheros formateado");
  else Serial.println("Error al formatear el sistema de ficheros");
  } 

void func_comando_hora(int iParametro, char* sParametro, float fParametro)//"hora"    
  {
  Serial.printf("La hora es %i\n",hora());
  }
  
void func_comando_minuto(int iParametro, char* sParametro, float fParametro)//"minuto"    
  {
  Serial.printf("Los minutos son %i\n",minuto());
  }
  
void func_comando_segundo(int iParametro, char* sParametro, float fParametro)//"segundo"
  {
  Serial.printf("Los segundos son %i\n",segundo());
  }

void func_comando_reloj(int iParametro, char* sParametro, float fParametro)//"reloj") 
  {
  imprimeDatosReloj();  
  if(cambioHorario()==1) Serial.println("Horario de verano");
  else Serial.println("Horario de invierno");
  } 
  
void func_comando_echo(int iParametro, char* sParametro, float fParametro)//"echo") 
  {
  Serial.printf("echo; %s\n",sParametro);
  }

void func_comando_debug(int iParametro, char* sParametro, float fParametro)//"debug")
  {
  ++debugGlobal=debugGlobal % 2;
  if (debugGlobal) Serial.println("debugGlobal esta on");
  else Serial.println("debugGlobal esta off");
  }

void func_comando_enviarKeepAlive(int iParametro, char* sParametro, float fParametro)//"debug")
  {
  if(iParametro!=0) enviarKeepAlive=1;
  else  enviarKeepAlive=0;

  Serial.printf("enviarKeepAlive=%i\n",enviarKeepAlive);
  }

void func_comando_ES(int iParametro, char* sParametro, float fParametro)//"debug")
  {
  Serial.println("Entradas");  
  for(int8_t i=0;i<MAX_ENTRADAS;i++) Serial.printf("%i: nombre: %s | configurada: %i | estado: %i | tipo: %s | pin: %i\n",i,entradas[i].nombre.c_str(),entradas[i].configurada,entradas[i].estado,entradas[i].tipo.c_str(),entradas[i].pin);
  Serial.println("Salidas");  
  for(int8_t i=0;i<MAX_RELES;i++) Serial.printf("%i: nombre: %s | configurado: %i | estado: %i | inicio: %i | pin: %i\n",i,reles[i].nombre.c_str(),reles[i].configurado,reles[i].estado,reles[i].inicio,reles[i].pin);  
  } 

void func_comando_actSec(int iParametro, char* sParametro, float fParametro)//"debug")
  {
  activarSecuenciador();
  } 

void func_comando_desSec(int iParametro, char* sParametro, float fParametro)//"debug")
  {
  desactivarSecuenciador();
  } 
  
void func_comando_estSec(int iParametro, char* sParametro, float fParametro)//"debug")
  {
  if(estadoSecuenciador()) Serial.println("Secuenciador activado");
  else Serial.println("Secuenciador desactivado");

  Serial.printf("Hay %i planes definidos\n",getNumPlanes());
  }   

void func_comando_MQTTConfig(int iParametro, char* sParametro, float fParametro)//"debug")
  {
  Serial.printf("Configuracion leida:\nID MQTT: %s\nIP broker: %s\nIP Puerto del broker: %i\nUsuario: %s\nPassword: %s\nTopic root: %s\nEnviar KeepAlive: %i\nPublicar entradas: %i\nPublicar salidas: %i\nWill topic: %s\nWill msg: %s\nCelan session: %i\n",ID_MQTT.c_str(),IPBroker.toString().c_str(),puertoBroker,usuarioMQTT.c_str(),passwordMQTT.c_str(),topicRoot.c_str(),enviarKeepAlive,publicarEntradas,publicarSalidas,(topicRoot+"/"+String(WILL_TOPIC)).c_str(),String(WILL_MSG).c_str(), CLEAN_SESSION);
  }  

void func_comando_Salidas(int iParametro, char* sParametro, float fParametro)//"debug")
  {
  Serial.printf("%s\n",generaJsonEstadoSalidas().c_str());
  }  

void func_comando_Entradas(int iParametro, char* sParametro, float fParametro)//"debug")
  {
  Serial.printf("%s\n",generaJsonEstadoEntradas().c_str());
  }    
/***************************** FIN funciones para comandos ******************************************/ 
