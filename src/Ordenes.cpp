/**********************************************************************************/
/* ordenes.ino                                                                    */
/*                                                                                */
/* Comandos para el  control a traves del puerto serie                            */
/*                                                                                */
/**********************************************************************************/

/***************************** Defines *****************************/
#define LONG_COMANDO   40
#define LONG_PARAMETRO 30
#define LONG_ORDEN     22 //Comando (espacio) Parametros (fin de cadena)
#define MAX_COMANDOS   40
/***************************** Defines *****************************/

/***************************** Includes *****************************/
#include <Global.h>
#include <Ordenes.h>
#include <Sensores.h>
#include <Entradas.h>
#include <Salidas.h>
#include <RedWifi.h>
#include <SNTP.h>
#include <Secuenciador.h>
#include <ComprobacionErrores.h>
#include <MaqEstados.h>
#include <GoogleHomeNotifier.h>
#include <MQTT.h>
#include <Ficheros.h>

#include <SPIFFS.h>
#include <Time.h>
/***************************** Includes *****************************/

typedef struct 
  {
  String comando;
  String descripcion;
  void (*p_func_comando) (int, char*, float)=NULL;
  }tipo_comando;
tipo_comando comandos[MAX_COMANDOS];

char ordenRecibida[LONG_ORDEN]="";
int lonOrden=0;
/*********************************************************************/
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
        if (debug) Traza.mensaje("Orden recibida: %s\n",ordenRecibida);
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
  float fParametro=0;
  int inicioParametro=0;

  if (debug) Traza.mensaje("Orden recibida: %s\n",ordenRecibida);
  
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
            break;
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

  if (debug) Traza.mensaje("comando: %s\niParametro: %i\nsParametro: %s\nfParametro: %f\n",comando.c_str(),iParametro,sParametro,fParametro);
    
/**************Nueva funcion ***************************/
  int8_t indice=0;
  for(indice=0;indice<MAX_COMANDOS;indice++)
    {
    if (debug) Traza.mensaje("Comando[%i]: {%s} - {%s}\n",indice,comando.c_str(),comandos[indice].comando.c_str());

    if (comandos[indice].comando==comando) 
      {
      //Ejecuta la funcion asociada
      comandos[indice].p_func_comando(iParametro, sParametro, fParametro);
      return(0);
      }    
    }

  //Si llega aqui es que no ha encontrado el comando
  Traza.mensaje("Comando no encontrado\n");
  return(-1);//Comando no encontrado  
/*******************************************************/
}

void limpiaOrden(void)
  {
  lonOrden=0;
  ordenRecibida[0]=0;
  }

/*********************************************************************/
/*  Funciones para los comandos                                      */
/*  void (*p_func_comando) (int, char*, float)                       */
/*********************************************************************/  
void func_comando_vacio(int iParametro, char* sParametro, float fParametro) //"vacio"
{}

void func_comando_help(int iParametro, char* sParametro, float fParametro) //"help"
  {
  Traza.mensaje("\n\nComandos:");  
  for(int8_t i=0;i<MAX_COMANDOS;i++) if (comandos[i].comando!=String("vacio")) Traza.mensaje("Comando %i: [%s]\n",i, comandos[i].comando.c_str());
  Traza.mensaje("\n------------------------------------------------------------------------------\n");
  }

void func_comando_IP(int iParametro, char* sParametro, float fParametro) //"IP"
  {
  boolean debug=false;
  Traza.mensaje("%s\n",getIP(debug).c_str());  
  }  

void func_comando_nivelActivo(int iParametro, char* sParametro, float fParametro) //"nivelActivo"
  {
  if(sParametro[0]!=0) 
    {
    nivelActivo=iParametro;

    String cad="";
    
    if(!leeFichero(GLOBAL_CONFIG_FILE, cad)) Traza.mensaje("No se pudo leer el fichero\n");
    cad=generaJsonConfiguracionNivelActivo(cad, nivelActivo);
    if(!salvaFichero(GLOBAL_CONFIG_FILE, GLOBAL_CONFIG_BAK_FILE, cad)) Traza.mensaje("No se pudo salvar el fichero\n");      
    }
  Traza.mensaje("\nNivel activo: %i\n",nivelActivo);  
  }  

void func_comando_activa(int iParametro, char* sParametro, float fParametro)//"activa")
  {
  salidas.conmuta(iParametro,ESTADO_ACTIVO);  
  Traza.mensaje("\nRele %i activado\n",iParametro);
  }  

void func_comando_desactiva(int iParametro, char* sParametro, float fParametro)//"desactiva")
  {
  salidas.conmuta(iParametro,ESTADO_DESACTIVO);  
  Traza.mensaje("\nRele %i desactivado\n",iParametro);  
  }  

void func_comando_estadoSalida(int iParametro, char* sParametro, float fParametro)
  { 
  Traza.mensaje("\nEl estado logico de la salida %i es ",iParametro);  
  if (salidas.getSalida(iParametro).getEstado()==ESTADO_ACTIVO)Traza.mensaje("activada");
  else if (salidas.getSalida(iParametro).getEstado()==ESTADO_PULSO)Traza.mensaje("pulso");
  else Traza.mensaje("desactivada");

  Traza.mensaje("\nEl estado fisico de la salida %i es %i\nPines:\npin rele: %i\n",iParametro, digitalRead(salidas.getSalida(iParametro).getPin()),salidas.getSalida(iParametro).getPin());
  }  
    
void func_comando_restart(int iParametro, char* sParametro, float fParametro)//"restart")
  {
  ESP.restart();
  }
  
void func_comando_info(int iParametro, char* sParametro, float fParametro)//"info")
  {
  Traza.mensaje("\n-----------------info uptime-----------------\n");
  Traza.mensaje("Uptime: %l segundos\n", (esp_timer_get_time()/(unsigned long)1000000)); //la funcion esp_timer_get_time() devuelve el contador de microsegundos desde el arranque. rota cada 292.000 años
  Traza.mensaje("-----------------------------------------------\n");  

  Traza.mensaje("\n-----------------info logica-----------------\n");
  Traza.mensaje("IP: %s\n", String(getIP(debugGlobal)).c_str());
  Traza.mensaje("nivelActivo: %s\n", String(nivelActivo).c_str());  
  for(int8_t i=0;i<salidas.getNumSalidas();i++) Traza.mensaje("Rele %i | nombre: %s | estado: %i\n", i,salidas.getSalida(i).getNombre().c_str(), salidas.getSalida(i).getEstado());
  Traza.mensaje("-----------------------------------------------\n");  
  
  Traza.mensaje("-------------------WiFi info-------------------\n");
  Traza.mensaje("SSID: %s\n",nombreSSID().c_str());
  Traza.mensaje("IP: %s\n", WiFi.localIP().toString().c_str());
  Traza.mensaje("Potencia: %s\n",String(WiFi.RSSI()).c_str());
  Traza.mensaje("-----------------------------------------------\n");   
      
  Traza.mensaje("-----------------Hardware info-----------------\n");
  Traza.mensaje("FreeHeap: %i\n",ESP.getFreeHeap());
  Traza.mensaje("ChipId: %i\n",ESP.getChipRevision());
  Traza.mensaje("SdkVersion: %s\n",ESP.getSdkVersion());
  Traza.mensaje("CpuFreqMHz: %i\n",ESP.getCpuFreqMHz());
      //gets the size of the flash as set by the compiler
  Traza.mensaje("FlashChipSize: %i\n",ESP.getFlashChipSize());
  Traza.mensaje("FlashChipSpeed: %i\n",ESP.getFlashChipSpeed());

  Traza.mensaje("-----------------------------------------------\n");
  }  

void func_comando_flist(int iParametro, char* sParametro, float fParametro)//"fexist")
  {
  Serial.printf("listado de ficheros:\n%s",listadoFicheros(String(sParametro)).c_str()); 
  }

void func_comando_fexist(int iParametro, char* sParametro, float fParametro)//"fexist")
  {
  if (strlen(sParametro)==0) Traza.mensaje("Es necesario indicar un nombre de fichero\n");
  else
    {
    if(SPIFFS.exists(sParametro)) Traza.mensaje("El fichero %s existe.\n",sParametro);
    else Traza.mensaje("NO existe el fichero %s.\n",sParametro);
    }
  }

void func_comando_fopen(int iParametro, char* sParametro, float fParametro)//"fopen")
  {
  if (strlen(sParametro)==0) Traza.mensaje("Es necesario indicar un nombre de fichero\n");
  else
    {
    File f = SPIFFS.open(sParametro, "r");
    if (f)
      { 
      Traza.mensaje("Fichero abierto\n");
      size_t tamano_fichero=f.size();
      Traza.mensaje("El fichero tiene un tamaño de %i bytes.\n",tamano_fichero);
      char buff[tamano_fichero+1];
      f.readBytes(buff,tamano_fichero);
      buff[tamano_fichero+1]=0;
      Traza.mensaje("El contenido del fichero es:\n******************************************\n%s\n******************************************\n",buff);
      f.close();
      }
    else Traza.mensaje("Error al abrir el fichero %s.\n", sParametro);
    } 
  } 

void func_comando_fremove(int iParametro, char* sParametro, float fParametro)//"fremove")
  {
  if (strlen(sParametro)==0) Traza.mensaje("Es necesario indicar un nombre de fichero\n");
  else
    { 
    if (SPIFFS.remove(sParametro)) Traza.mensaje("Fichero %s borrado\n",sParametro);
    else Traza.mensaje("Error al borrar el fichero%s\n",sParametro);
    } 
 }

void func_comando_format(int iParametro, char* sParametro, float fParametro)//"format")
  {     
  if (formatearFS()) Traza.mensaje("Sistema de ficheros formateado\n");
  else Traza.mensaje("Error al formatear el sistema de ficheros\n");
  } 

void func_comando_hora(int iParametro, char* sParametro, float fParametro)//"hora"    
  {
  Traza.mensaje("La hora es %i\n",hora());
  }
  
void func_comando_minuto(int iParametro, char* sParametro, float fParametro)//"minuto"    
  {
  Traza.mensaje("Los minutos son %i\n",minuto());
  }
  
void func_comando_segundo(int iParametro, char* sParametro, float fParametro)//"segundo"
  {
  Traza.mensaje("Los segundos son %i\n",segundo());
  }

void func_comando_reloj(int iParametro, char* sParametro, float fParametro)//"reloj") 
  {
  imprimeDatosReloj();  
  if(cambioHorario()==1) Traza.mensaje("Horario de verano\n");
  else Traza.mensaje("Horario de invierno\n");
  } 
  
void func_comando_echo(int iParametro, char* sParametro, float fParametro)//"echo") 
  {
  Traza.mensaje("echo; %s\n",sParametro);
  }

void func_comando_debugGlobal(int iParametro, char* sParametro, float fParametro)//"debug")
  {
  ++debugGlobal=debugGlobal % 2;
  if (debugGlobal) Traza.mensaje("debugGlobal esta on\n");
  else Traza.mensaje("debugGlobal esta off\n");
  }

void func_comando_debugMain(int iParametro, char* sParametro, float fParametro)//"debug")
  {
  ++debugMain=debugMain % 2;
  if (debugMain) Traza.mensaje("debugMain esta on\n");
  else Traza.mensaje("debugMain esta off\n");
  }

void func_comando_ES(int iParametro, char* sParametro, float fParametro)//"debug")
  {
  Traza.mensaje("Entradas\n");
  for(int8_t i=0;i<entradas.getNumEntradas();i++) Traza.mensaje("%i: nombre: %s | estado: %i | tipo: %s | pin: %i\n",i,entradas.getEntrada(i).getNombre().c_str(),entradas.getEntrada(i).getEstado(),entradas.getEntrada(i).getTipo().c_str(),entradas.getEntrada(i).getPin());
  Traza.mensaje("Salidas\n");
  for(int8_t i=0;i<salidas.getNumSalidas();i++) Traza.mensaje("%i: nombre: %s | estado: %i | inicio: %i | pin: %i | modo: %i | controlador: %i | ancho pulso: %i | fin pulso: %i\n",i,salidas.getSalida(i).getNombre().c_str(),salidas.getSalida(i).getEstado(),salidas.getSalida(i).getEstadoInicial(),salidas.getSalida(i).getPin(),salidas.getSalida(i).getModo(),salidas.getSalida(i).getControlador(),salidas.getSalida(i).getAnchoPulso(),salidas.getSalida(i).getFinPulso());  
  } 

void func_comando_actSec(int iParametro, char* sParametro, float fParametro)//"debug")
  {
  secuenciador.activar();
  } 

void func_comando_desSec(int iParametro, char* sParametro, float fParametro)//"debug")
  {
  secuenciador.desactivar();
  } 
  
void func_comando_estSec(int iParametro, char* sParametro, float fParametro)//"debug")
  {
  if(secuenciador.getEstado()) Traza.mensaje("Secuenciador activado\n");
  else Traza.mensaje("Secuenciador desactivado\n");

  Traza.mensaje("Hay %i planes definidos\n",secuenciador.getNumPlanes());
  }   

void func_comando_estPlan(int iParametro, char* sParametro, float fParametro){
  if(sParametro==NULL) return;

  int _hora=0;
  int _minuto=0;
  char* p=sParametro;
  char delimitador[] = ":";

  Traza.mensaje("Entrada: %s, ",sParametro);

  char *token = strtok(p, delimitador);
  if(token != NULL) _hora=atoi(token);
  token = strtok(NULL, delimitador);
  if(token != NULL) _minuto=atoi(token);

  Traza.mensaje("hora %i, minuto %i\n", _hora, _minuto);
  for(uint8_t _plan=0;_plan<secuenciador.getNumPlanes();_plan++){
    int _respuesta=secuenciador.getEstadoPlan(_plan,_hora,_minuto);  
    Traza.mensaje("---------->plan %i: estado: %i\n", _plan, _respuesta);
  }
}   

void func_comando_MQTTConfig(int iParametro, char* sParametro, float fParametro)//"debug")
  {
  Traza.mensaje("Configuracion leida:\nID MQTT: %s\nIP broker: %s\nBroker Dir: %s\nIP Puerto del broker: %i\nUsuario: %s\nPassword: %s\nTopic root: %s\nPublicar entradas: %i\nPublicar salidas: %i\nWill topic: %s\nWill msg: %s\nCelan session: %i\n",getIDMQTT().c_str(),getIPBroker().toString().c_str(),getBrokerDir().c_str(),getPuertoBroker(),getUsuarioMQTT().c_str(),getPasswordMQTT().c_str(),getTopicRoot().c_str(),getPublicarEntradas(),getPublicarSalidas(),getTopicRoot()+"/"+getWillTopic().c_str(),getWillMsg().c_str(), getCleanSession());
  }  

void func_comando_Salidas(int iParametro, char* sParametro, float fParametro)//"debug")
  {
  Traza.mensaje("%s\n",salidas.generaJsonEstado().c_str());
  }  

void func_comando_Entradas(int iParametro, char* sParametro, float fParametro)//"debug")
  {
  Traza.mensaje("%s\n",entradas.generaJsonEstado().c_str());
  }    

void func_comando_GHN(int iParametro, char* sParametro, float fParametro)//"debug")
  {
  Traza.mensaje("Google Home Notifier:\n Nombre del equipo: %s | Idioma: %s | Activo: %i\n",getNombreEquipo().c_str(),getIdioma().c_str(), getActivaGoogleHomeNotifier());
  }    

void func_comando_debugMaquinaEstados(int iParametro, char* sParametro, float fParametro)//"debug")
  {
  //debugMaquinaEstados=!debugMaquinaEstados;
  maquinaEstados.setDebugMaquinaEstados(!maquinaEstados.getDebugMAquinaEstados());
  if (maquinaEstados.getDebugMAquinaEstados()) Traza.mensaje("El debug de la maquina de estados esta on\n");
  else Traza.mensaje("El debug de la maquina de estados esta off\n");
  }  

void func_comando_compruebaConfiguracion(int iParametro, char* sParametro, float fParametro)//"debug")
  {
  compruebaConfiguracion(0);
  }  

void func_comando_setPWM(int iParametro, char* sParametro, float fParametro)//"debug")
  {
  int8_t id=salidas.getSalidaActiva();
  if(id!=-1)  
    {
    salidas.setValorPWM(id,iParametro);
    Serial.printf("valor: %i\n",salidas.getSalida(id).getValorPWM());
    }
  else Serial.printf("valor de salida no valido (%i)\n",id);  
  }  

void func_comando_getPWM(int iParametro, char* sParametro, float fParametro)//"debug")
  {
  Serial.printf("valor: %i\n",salidas.getSalida(salidas.getSalidaActiva()).getValorPWM());
  }  

void func_comando_setSalida(int iParametro, char* sParametro, float fParametro)//"debug")
  {
  salidas.setSalidaActiva(iParametro);  
  }

void func_comando_sensores(int iParametro, char* sParametro, float fParametro)//"debug")
  {
  Serial.printf("Valores de los sensores:\n%s\n",sensores.generaJsonEstado().c_str());  
  }

void func_comando_scannerOW(int iParametro, char* sParametro, float fParametro)//"debug")
  {
  sensores.scannerOneWire();
  }
  
void func_comando_scannerI2C(int iParametro, char* sParametro, float fParametro)//"debug")
  {
  sensores.scannerI2C();
  }  

void func_comando_uptime(int iParametro, char* sParametro, float fParametro)//"debug")
  {
  Serial.printf("Hora del ultimo arranque: %s\n",horaYfechaArranque().c_str());
  }  
/***************************** FIN funciones para comandos ******************************************/ 

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

  comandos[i].comando="estadoSalida";
  comandos[i].descripcion="Devuelve el estado de la salida indicada";
  comandos[i++].p_func_comando=func_comando_estadoSalida;

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
  comandos[i].descripcion="Activa/desactiva el modo debug Global";
  comandos[i++].p_func_comando=func_comando_debugGlobal;

  comandos[i].comando="debugM";
  comandos[i].descripcion="Activa/desactiva el modo debug Main";
  comandos[i++].p_func_comando=func_comando_debugMain;

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

  comandos[i].comando="estPlan";
  comandos[i].descripcion="Estado del plan en una hora y minuto";
  comandos[i++].p_func_comando=func_comando_estPlan;    

  comandos[i].comando="MQTTConfig";
  comandos[i].descripcion="Configuración de MQTT";
  comandos[i++].p_func_comando=func_comando_MQTTConfig;

  comandos[i].comando="entradas";
  comandos[i].descripcion="JSON entradas";
  comandos[i++].p_func_comando=func_comando_Entradas;

  comandos[i].comando="salidas";
  comandos[i].descripcion="JSON salidas";
  comandos[i++].p_func_comando=func_comando_Salidas;

  comandos[i].comando="GHN";
  comandos[i].descripcion="Datos de GHN";
  comandos[i++].p_func_comando=func_comando_GHN;

  comandos[i].comando="debugME";
  comandos[i].descripcion="Debug de la maquina de estados";
  comandos[i++].p_func_comando=func_comando_debugMaquinaEstados;

  comandos[i].comando="CheckConfig";
  comandos[i].descripcion="Comprueba la configuracion del sistema";
  comandos[i++].p_func_comando=func_comando_compruebaConfiguracion;

  comandos[i].comando="setPWM";
  comandos[i].descripcion="Actualiza el valor de activo para la salida PWM";
  comandos[i++].p_func_comando=func_comando_setPWM;

  comandos[i].comando="getPWM";
  comandos[i].descripcion="Devuelve el valor de activo para la salida PWM";
  comandos[i++].p_func_comando=func_comando_getPWM;

  comandos[i].comando="setSalida";
  comandos[i].descripcion="Selecciona la salida a configurar PWM";
  comandos[i++].p_func_comando=func_comando_setSalida;

  comandos[i].comando="sensores";
  comandos[i].descripcion="Recipera los valores medidos por los sensores";
  comandos[i++].p_func_comando=func_comando_sensores;

  comandos[i].comando="ScannerOW";
  comandos[i].descripcion="Scanner del bus OneWire";
  comandos[i++].p_func_comando=func_comando_scannerOW;

  comandos[i].comando="ScannerI2C";
  comandos[i].descripcion="Scanner del bus I2C";
  comandos[i++].p_func_comando=func_comando_scannerI2C;

  comandos[i].comando="uptime";
  comandos[i].descripcion="Hora del ultimo arranque";
  comandos[i++].p_func_comando=func_comando_uptime;

  //resto
  for(;i<MAX_COMANDOS;)
    {
    comandos[i].comando="vacio";
    comandos[i].descripcion="Comando vacio";
    comandos[i++].p_func_comando=func_comando_vacio;  
    }
    
  func_comando_help(0,NULL,0.0);
  }
