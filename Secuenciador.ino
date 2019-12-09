/*****************************************/
/*                                       */
/*  Secuenciador de fases de entradas    */
/*                                       */
/*****************************************/

#define MAX_PLANES 2
#define HORAS_EN_DIA 24

#ifndef NO_CONFIGURADO 
#define NO_CONFIGURADO -1
#endif

#ifndef CONFIGURADO 
#define CONFIGURADO     1
#endif

//define un array de HORAS_EN_DIA enteros de 16 bits. Para los 12 primeros, es el valor on/off de los 5 minutos de esa hora
typedef struct{
  int8_t configurado;              //indica si el plan esta disponible y bien configurado o no
  int8_t rele;                     //salida a la que se asocia la secuencia
  int    horas[HORAS_EN_DIA];      //el valor es un campo de bit. los primeros 12 son los intervalos de 5 min de cada hora
  }plan; 
plan planes[MAX_PLANES];

boolean secuenciadorActivo=false; //plag para activar o desactivar el secuenciador

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
  if(!recuperaDatosSecuenciador(debugGlobal)) Serial.println("Configuracion del secuenciador por defecto");
  else
    { 
    //compruebo si la salida asociada a cada plan esta configurada
    for(int8_t i=0;i<MAX_PLANES;i++)
      {
      if(planConfigurado(i)==CONFIGURADO)
        {  
        if (releConfigurado(planes[i].rele)==NO_CONFIGURADO)
          {
          Serial.printf("La salida asociada al plan %i no esta configurada\n", planes[i].rele);
          planes[i].configurado=NO_CONFIGURADO;
          }
        //Esta bien configurado  
        else asociarSecuenciador(planes[i].rele, i); //Asocio el rele al plan
        }
      else Serial.printf("Plan %i no configurado\n",i);        
      }
    }
  }

boolean recuperaDatosSecuenciador(boolean debug)
  {
  String cad="";

  if (debug) Serial.println("Recupero configuracion de archivo...");
  
  if(!leeFicheroConfig(SECUENCIADOR_CONFIG_FILE, cad)) 
    {
    //Confgiguracion por defecto
    Serial.printf("No existe fichero de configuracion del secuenciador\n");
    //cad="{ \"estadoInicial\": 0, \"Planes\":[ {\"id_plan\": 1, \"salida\": 1, \"intervalos\": [{\"id\":  0, \"valor\": 0},{\"id\":  1, \"valor\": 1}, {\"id\":  2, \"valor\": 0}, {\"id\":  3, \"valor\": 1}, {\"id\":  4, \"valor\": 0}, {\"id\":  5, \"valor\": 1}, {\"id\":  6, \"valor\": 0}, {\"id\":  7, \"valor\": 1}, {\"id\":  8, \"valor\": 0}, {\"id\":  9, \"valor\": 1}, {\"id\": 10, \"valor\": 0}, {\"id\": 11, \"valor\": 1},{\"id\":  12, \"valor\": 0},{\"id\":  13, \"valor\": 1}, {\"id\":  14, \"valor\": 0}, {\"id\":  15, \"valor\": 1}, {\"id\":  16, \"valor\": 0}, {\"id\":  17, \"valor\": 1}, {\"id\":  18, \"valor\": 0}, {\"id\":  19, \"valor\": 1}, {\"id\":  20, \"valor\": 0}, {\"id\":  21, \"valor\": 1}, {\"id\": 22, \"valor\": 0}, {\"id\": 23, \"valor\": 1} ] } ] }";
    cad="{\"estadoInicial\": 0,\"Planes\":[]}";
    //if(salvaFicheroConfig(SECUENCIADOR_CONFIG_FILE, SECUENCIADOR_CONFIG_BAK_FILE, cad)) Serial.printf("Fichero de configuracion del secuenciador creado por defecto\n");
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
  
  json.printTo(Serial);
  if (!json.success()) return false;
        
  Serial.println("parsed json");
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
  
    Serial.printf("Plan %i:\nSalida: %i\n", i, planes[i].rele); 
    for(int8_t j=0;j<HORAS_EN_DIA;j++) Serial.printf("hora %02i: valor: %01i\n",j,planes[i].horas[j]);    
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

      if(debug) Serial.printf("Hora: %02i:%02i\nMascara: %i | intervalo: %i\n",hora(),minuto(),mascara,planes[i].horas[hora()]);
      
      if(planes[i].horas[hora()] & mascara) conmutaRele(planes[i].rele, nivelActivo, debugGlobal);
      else conmutaRele(planes[i].rele, !nivelActivo, debugGlobal);
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

  cad += "<TABLE style=\"border: 2px solid black\">\n";
  cad += "<CAPTION>Plan " + String(plan) + "</CAPTION>\n";  

  //Cabecera
  cad += "<tr>";
  cad += "<th>Hora</th>";
  for(int8_t i=0;i<HORAS_EN_DIA;i++) cad += "<th style=\"width:40px\">" + String(i) + "</th>";
  cad += "</tr>";

  //Cada fila es un intervalo, cada columna un hora
  int mascara=1;  
  
  for(int8_t intervalo=0;intervalo<12;intervalo++)
    {
    Serial.printf("intervalo: %i | cad: %i\n",intervalo,cad.length());      
    cad += "<tr>";
    cad += "<td>" + String(intervalo) + ": (" + String(intervalo*5) + "-" + String(intervalo*5+4) + ")</td>";    
    for(int8_t i=0;i<HORAS_EN_DIA;i++) cad += "<td style=\"text-align:center;\">" + (planes[plan].horas[i] & mascara?String(1):String(0)) + "</td>";
    cad += "</tr>";
    
    mascara<<=1;
    }  
    
  return cad;  
  }
