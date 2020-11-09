/*****************************************/
/*                                       */
/*  Secuenciador de fases de entradas    */
/*                                       */
/*****************************************/

/***************************** Defines *****************************/
#define HORAS_EN_DIA 24
/***************************** Defines *****************************/

/***************************** Includes *****************************/
#include <Global.h>
#include <Secuenciador.h>
#include <EntradasSalidas.h>
#include <SNTP.h>
#include <Ficheros.h>
/***************************** Includes *****************************/

//define un array de HORAS_EN_DIA enteros de 16 bits. Para los 12 primeros, es el valor on/off de los 5 minutos de esa hora
typedef struct{
  int8_t configurado;              //indica si el plan esta disponible y bien configurado o no
  int8_t rele;                     //salida a la que se asocia la secuencia
  int    horas[HORAS_EN_DIA];      //el valor es un campo de bit. los primeros 12 son los intervalos de 5 min de cada hora
  }plan; 
plan planes[MAX_PLANES];

boolean secuenciadorActivo=false; //plag para activar o desactivar el secuenciador

boolean recuperaDatosSecuenciador(boolean debug);
boolean parseaConfiguracionSecuenciador(String contenido);
/************************************** Funciones de configuracion ****************************************/
void inicializaSecuenciador()
  {
  //Valor por defecto de las variables
  secuenciadorActivo=false;
  
  for(int8_t i=0;i<MAX_PLANES;i++)
    {
    for(int8_t j=0;j<12;j++) 
      {
      planes[i].rele=NO_CONFIGURADO;
      planes[i].configurado=NO_CONFIGURADO;  
      planes[i].horas[j]=0;
      }
    } 
        
  //leo la configuracion del fichero
  if(!recuperaDatosSecuenciador(debugGlobal)) Traza.mensaje("Configuracion del secuenciador por defecto\n");
  else
    { 
    //compruebo si la salida asociada a cada plan esta configurada
    for(int8_t i=0;i<MAX_PLANES;i++)
      {
      if(planConfigurado(i)==CONFIGURADO)
        {  
        if (releConfigurado(planes[i].rele)==NO_CONFIGURADO || modoSalida(planes[i].rele)!=MODO_SECUENCIADOR)
          {
          Traza.mensaje("La salida asociada al plan %i no esta configurada\n", planes[i].rele);
          planes[i].configurado=NO_CONFIGURADO;
          }
        //Esta bien configurado  
        else asociarSecuenciador(planes[i].rele, i); //Asocio el rele al plan
        }
      else Traza.mensaje("Plan %i no configurado\n",i);        
      }
    }
  }

boolean recuperaDatosSecuenciador(boolean debug)
  {
  String cad="";

  if (debug) Traza.mensaje("Recupero configuracion de archivo...\n");
  
  if(!leeFichero(SECUENCIADOR_CONFIG_FILE, cad)) 
    {
    //Confgiguracion por defecto
    Traza.mensaje("No existe fichero de configuracion del secuenciador\n");
    //cad="{ \"estadoInicial\": 0, \"Planes\":[ {\"id_plan\": 1, \"salida\": 1, \"intervalos\": [{\"id\":  0, \"valor\": 0},{\"id\":  1, \"valor\": 1}, {\"id\":  2, \"valor\": 0}, {\"id\":  3, \"valor\": 1}, {\"id\":  4, \"valor\": 0}, {\"id\":  5, \"valor\": 1}, {\"id\":  6, \"valor\": 0}, {\"id\":  7, \"valor\": 1}, {\"id\":  8, \"valor\": 0}, {\"id\":  9, \"valor\": 1}, {\"id\": 10, \"valor\": 0}, {\"id\": 11, \"valor\": 1},{\"id\":  12, \"valor\": 0},{\"id\":  13, \"valor\": 1}, {\"id\":  14, \"valor\": 0}, {\"id\":  15, \"valor\": 1}, {\"id\":  16, \"valor\": 0}, {\"id\":  17, \"valor\": 1}, {\"id\":  18, \"valor\": 0}, {\"id\":  19, \"valor\": 1}, {\"id\":  20, \"valor\": 0}, {\"id\":  21, \"valor\": 1}, {\"id\": 22, \"valor\": 0}, {\"id\": 23, \"valor\": 1} ] } ] }";
    cad="{\"estadoInicial\": 0,\"Planes\":[]}";
    //if(salvaFichero(SECUENCIADOR_CONFIG_FILE, SECUENCIADOR_CONFIG_BAK_FILE, cad)) Traza.mensaje("Fichero de configuracion del secuenciador creado por defecto\n");
    }      
    
  return parseaConfiguracionSecuenciador(cad);
  }

/*********************************************/
/* Parsea el json leido del fichero de       */
/* configuracio de los reles                 */
/*********************************************/
boolean parseaConfiguracionSecuenciador(String contenido)
  {  
  DynamicJsonBuffer jsonBuffer;
  JsonObject& json = jsonBuffer.parseObject(contenido.c_str());
  
  String salida;
  json.printTo(salida);//pinto el json que he leido
  Traza.mensaje("json creado:\n#%s#\n",salida.c_str());
  
  if (!json.success()) return false;
        
  Traza.mensaje("\nparsed json\n");
//******************************Parte especifica del json a leer********************************  
  secuenciadorActivo=json["estadoInicial"];
  
  JsonArray& Planes = json["Planes"];  

  int8_t max;
  max=(Planes.size()<MAX_PLANES?Planes.size():MAX_PLANES);
  for(int8_t i=0;i<max;i++)
    { 
    //Plan configurado
    planes[i].configurado=CONFIGURADO;
      
    //Salida asociada
    planes[i].rele=Planes[i]["salida"];

    //Intevalos
    JsonArray& Intervalos = json["Planes"][i]["intervalos"];  
    for(int8_t j=0;j<HORAS_EN_DIA;j++) planes[i].horas[j]=Intervalos[j]["valor"];//el valor es un campo de bit. los primeros 12 son los intervalos de 5 min de cada hora
  
    Traza.mensaje("Plan %i:\nSalida: %i\n", i, planes[i].rele); 
    for(int8_t j=0;j<HORAS_EN_DIA;j++) Traza.mensaje("hora %02i: valor: %01i\n",j,planes[i].horas[j]);    
    }
//************************************************************************************************
  return true; 
  }
/**********************************************************Fin configuracion******************************************************************/  

/**********************************************************SALIDAS******************************************************************/    
/*************************************************/
/*Logica del secuenciador                        */
/*Comporueba como debe estar en ese peridodo de  */
/*cinco minutos parea esa hora y actualiza el    */
/*rele correspondiente                           */
/*************************************************/
void actualizaSecuenciador(bool debug)
  {
  if(!estadoSecuenciador()) return;
    
  for(int8_t i=0;i<getNumPlanes();i++)
    {
    if(planConfigurado(i))
      {
      int mascara=1;
      int8_t limite=minuto()/(int)5;
      mascara<<=limite;//calculo la mascara para leer el bit correspondiente al minuto adecuado

      if(debug) Traza.mensaje("Hora: %02i:%02i\nMascara: %i | intervalo: %i\n",hora(),minuto(),mascara,planes[i].horas[hora()]);

      if(planes[i].horas[hora()] & mascara) conmutaSalida(planes[i].rele, ESTADO_ACTIVO, debugGlobal);
      else conmutaSalida(planes[i].rele, ESTADO_DESACTIVO, debugGlobal);
      }  
    }
  }

/**************************************************/
/*                                                */
/* Devuelve el nuemro de planes definido          */
/*                                                */
/**************************************************/
int8_t getNumPlanes()
  {
  int resultado=0;
  
  for(int8_t i=0;i<MAX_PLANES;i++)
    {
    if(planes[i].configurado==CONFIGURADO) resultado++;
    }
  return resultado; 
  }  

/**************************************************/
/*                                                */
/* Devuelve el nuemro de salida asociada a un plan*/
/*                                                */
/**************************************************/
int8_t getSalidaPlan(uint8_t plan)
  {
  if(plan>MAX_PLANES) return NO_CONFIGURADO;

  return planes[plan].rele;  
  }  

/********************************************************/
/*                                                      */
/*     Devuelve si el plan esta configurados            */
/*                                                      */
/********************************************************/ 
int planConfigurado(uint8_t id)
  {
  if(id<0 || id>MAX_PLANES) return NO_CONFIGURADO;
    
  return planes[id].configurado;
  }

/********************************************************/
/*                                                      */
/*             Activa el secuenciador                   */
/*                                                      */
/********************************************************/ 
void activarSecuenciador(void)
  {
  secuenciadorActivo=true;
  }

/********************************************************/
/*                                                      */
/*             Desactiva el secuenciador                */
/*                                                      */
/********************************************************/ 
void desactivarSecuenciador(void)
  {
  secuenciadorActivo=false;
  }
  
/********************************************************/
/*                                                      */
/*     Devuelve el estado del secuenciador              */
/*                                                      */
/********************************************************/ 
boolean estadoSecuenciador(void)
  {
  return secuenciadorActivo;
  }  

/********************************************************/
/*                                                      */
/*     Genera codigo HTML para representar el plan      */
/*                                                      */
/********************************************************/ 
String pintaPlanHTML(int8_t plan)
  {
  String cad="";

  //validaciones previas
  if(plan<0 || plan>MAX_PLANES) return cad;

  cad += "<TABLE width=\"80%\" border=\"0\" cellpadding=\"0\" cellspacing=\"0\" class=\"tabla\">\n";
  cad += "<CAPTION>Plan " + String(plan) + "</CAPTION>\n";  

  //Cabecera
  cad += "<tr>";
  cad += "<th width=\"10%\">Hora</th>";
  for(int8_t i=0;i<HORAS_EN_DIA;i++) cad += "<th style=\"width:40px\">" + String(i) + "</th>";
  cad += "</tr>";

  //Cada fila es un intervalo, cada columna un hora
  int mascara=1;  
  
  for(int8_t intervalo=0;intervalo<12;intervalo++)
    {
    Traza.mensaje("intervalo: %i | cad: %i\n",intervalo,cad.length());      
    cad += "<TR class=\"modo2\">";
    cad += "<th>" + String(intervalo) + ": (min " + String(intervalo*5) + " a " + String(intervalo*5+4) + ")</th>";    
    for(int8_t i=0;i<HORAS_EN_DIA;i++) cad += "<td style=\"text-align:center;\">" + (planes[plan].horas[i] & mascara?String(1):String(0)) + "</td>";
    cad += "</tr>";
    
    mascara<<=1;
    }  
    
  cad+="</table>";
    
  return cad;  
  }
