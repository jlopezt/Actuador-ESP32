/*************************************************************************/
/*  Funciones de manejo de las ordenes que llegan por HTTP o por MQTT    */
/*  Inlcuye enrutador general y uno por tipo de orden                    */
/*
tipos de ordenes:
config-->se sacan de los ficheros en la plataforma
/nombre"
/servicios"
/configVariables
/configSensores
/configEntradas
/configSalidas
/configSecuenciador

Estado-->Se saca de los mensajes de estado MQTT y la base de datos
/estadoVariables
/estadoSensores
/estadoEntradas
/estadoSalidas
/estadoSecuenciador
/estadoMaquinaEstados

Accion-->recibidas por HTTP y MQTT
/activaSalida
/desactivaSalida
/fuerzaSalidaManual
/recuperaSalidaManual
/pulsoSalida
/activaSecuenciador
/desactivaSecuenciador

Utilidades-->
    Ficheros-->¿Hacen falta?
    /listaFicheros
    /creaFichero
    /borraFichero
    /leeFichero

    Particiones-->recibidas por HTTP y MQTT
    /particiones
    /setNextBoot

    NVS-->recibidas por HTTP y MQTT
    /resetWiFi
    /resetUser
    /resetID
    /resetTotal
    /inicializaNVS

    Restart-->recibidas por HTTP y MQTT
    /restart

    Info-->recibidas por HTTP y MQTT
    /info
*/
/*                                            */
/* El formato general del json que se recibe  */
/* en cada accion es este                     */
/*{
 	"tipo":"<tipo de la orden>",
	"subtipo":"<subtipo de orden>",
	"orden":"<tipo de accion>",
    "id": <id sobre el que actua si aplica>,
	"valor":"<valor a aplicar>
}*/
/*************************************************************************/

/***************************** Defines *****************************/
#define TIPO_ACCION                          "actuador"
#define TIPO_UTILIDAD                        "utilidad"
#define TIPO_CONFIGURACION                   "configura"

#define SUBTIPO_ACCION_SALIDAS               "salidas"
#define SUBTIPO_ACCION_SECUENCIADOR          "secuenciador"
#define SUBTIPO_UTILIDAD_PARTICIONES         "particiones" 
#define SUBTIPO_UTILIDAD_NVS                 "NVS"
#define SUBTIPO_UTILIDAD_RESTART             "restart"
#define SUBTIPO_UTILIDAD_INFO                "info"
#define SUBTIPO_CONFIGURACION_FICHERO        "fichero"

#define ORDEN_ACTUADOR_SALIDAS_ESTADO        "estado"
#define ORDEN_ACTUADOR_SALIDAS_MANUAL        "manual"
#define ORDEN_ACTUADOR_SECUENCIADOR_ESTADO   "estado"
#define ORDEN_UTILIDAD_PARTICIONES_ESTADO    "estado"
#define ORDEN_UTILIDAD_PARTICIONES_NEXTBOOT  "nextBoot"
#define ORDEN_UTILIDAD_NVS_INICIALIZANVS     "inicializaNVS"
#define ORDEN_UTILIDAD_NVS_VALORESDEFECTO    "valoresDefecto"
#define ORDEN_UTILIDAD_NVS_RESETTOTAL        "resetTotal"
#define ORDEN_UTILIDAD_NVS_RESETUSER         "resetUser"
#define ORDEN_UTILIDAD_NVS_RESETWIFI         "resetWiFi"
#define ORDEN_UTILIDAD_NVS_RESETID           "resetID"
#define ORDEN_CONFIGURACION_FICHERO_UPLOAD   "upload" //el dispositivo debe solicitar el fichero de configuracion indicado a la plataforma por HTTP
#define ORDEN_CONFIGURACION_FICHERO_DOWNLOAD "download" //el dispositivo debe enviar el fichero de configuracion indicado a la plataforma por HTTP

#define VALOR_ACCION_SALIDAS_ON              "on"
#define VALOR_ACCION_SALIDAS_OFF             "off"
#define VALOR_ACCION_SALIDAS_MANUAL_ON       "on"
#define VALOR_ACCION_SALIDAS_MANUAL_OFF      "off"
#define VALOR_ACCION_SALIDAS_PULSO           "pulso"
#define VALOR_ACCION_SECUENCIADOR_ON         "on"
#define VALOR_ACCION_SECUENCIADOR_OFF        "off"

#define TOPIC_UTILIDAD_PARTICIONES_ESTADO    "particiones"
#define TOPIC_UTILIDAD_INFO                  "info"
/***************************** Defines *****************************/
/***************************** Includes *****************************/
#include <Global.h>
#include <Salidas.h>
#include <Secuenciador.h>

#include <configNVS.h>
#include <OTA.h>
#include <MQTT.h>
#include <ClienteHTTP.h>
/***************************** Includes *****************************/
/***************************** Prototipos *****************************/
boolean actuadorSalidas(String comando);
boolean actuadorSecuenciador(String comando);

boolean utilidadParticiones(String comando);
boolean utilidadRestart(String comando);
boolean utilidadNVS(String comando);
boolean utilidadInfo(String comando);

boolean configraFichero(String comando);
/***************************** Prototipos *****************************/
/***************************** Var globales *****************************/
/***************************** Var globales *****************************/

/************************************************/
/* Recibe un json con la accion y asigna la     */
/* funcion que lo debe gestionar                */
/************************************************/
boolean enrutador(String comando){
  DynamicJsonBuffer jsonBuffer;
  JsonObject& json = jsonBuffer.parseObject(comando.c_str());

  if (json.success()) 
    {
    Traza.mensaje("\ncomando: json parseado\n");
    String tipo="";
    String subtipo="";
//******************************Parte especifica del json a leer********************************
    if (json.containsKey("tipo"))  tipo = json.get<String>("tipo");
    if (json.containsKey("subtipo"))  subtipo = json.get<String>("subtipo");

    Traza.mensaje("comando leido:\ntipo: %s\nsubtipo: %s\n",tipo.c_str(),subtipo.c_str());
//************************************************************************************************

    if(tipo==TIPO_ACCION){
        if(subtipo==SUBTIPO_ACCION_SALIDAS) return actuadorSalidas(comando);
        else if (subtipo==SUBTIPO_ACCION_SECUENCIADOR) return actuadorSecuenciador(comando);
        else return false;
    }
    else if(tipo==TIPO_UTILIDAD){
        if(subtipo==SUBTIPO_UTILIDAD_PARTICIONES) return utilidadParticiones(comando);
        else if (subtipo==SUBTIPO_UTILIDAD_RESTART) return utilidadRestart(comando);
        else if(subtipo==SUBTIPO_UTILIDAD_NVS) return utilidadNVS(comando);
        else if (subtipo==SUBTIPO_UTILIDAD_INFO) return utilidadInfo(comando);        
        else return false;
    }
    else if(tipo==TIPO_CONFIGURACION){
        if(subtipo==SUBTIPO_CONFIGURACION_FICHERO)return configraFichero(comando);
        else return false;
    }
    else return false;
    }

  Serial.printf("Mal rollo, salgo con error\n");
  return false;
  }

/****************************** Configuracion ****************************
Atiende estas ordenes
{
 	"tipo":"configura",
	"subtipo":"fichero",
	"orden":"upload",
    "id": #,
	"valor":"<tipo de servicio>"
}

{
 	"tipo":"actuador",
	"subtipo":"secuenciador",
	"orden":"estado",
    "id": #,    
	"valor":"on"/"off"
}
*/
boolean configraFichero(String comando){
  DynamicJsonBuffer jsonBuffer;
  JsonObject& json = jsonBuffer.parseObject(comando.c_str());

  if (json.success()) 
    {
    Traza.mensaje("\ncomando: json parseado\n");

    String orden="";
    uint8_t id=0;
    String valor="";
//******************************Parte especifica del json a leer********************************
    if (json.containsKey("orden"))  orden = json.get<String>("orden");
    //if (json.containsKey("id"))  id = json.get<uint8_t>("id");
    if (json.containsKey("valor"))  valor = json.get<String>("valor");

    Traza.mensaje("configura fichero\ncomando leida:\norden: %s\nid: %02u\nvalor: %s\n",orden.c_str(),id,valor.c_str());
//************************************************************************************************
    if(orden==ORDEN_CONFIGURACION_FICHERO_DOWNLOAD) return leeFicheroConfig(valor);//valor es el sevicio a recuperar
    else if(orden==ORDEN_CONFIGURACION_FICHERO_UPLOAD) return enviaFicheroConfig("/" + valor + ".json"); //fichero a enviar
    else return false;
    }
return false;
}

/****************************** Acciones ****************************
Atiende estas ordenes
{
 	"tipo":"actuador",
	"subtipo":"salidas",
	"orden":"estado"/"Manual",
    "id": #,
	"valor":"on"/"off"/"pulso"
}

{
 	"tipo":"actuador",
	"subtipo":"secuenciador",
	"orden":"estado",
    "id": #,    
	"valor":"on"/"off"
}
**********************************************************/
boolean actuadorSalidas(String comando){
  DynamicJsonBuffer jsonBuffer;
  JsonObject& json = jsonBuffer.parseObject(comando.c_str());

  if (json.success()) 
    {
    Traza.mensaje("\ncomando: json parseado\n");

    String orden="";
    uint8_t id=0;
    String valor="";
//******************************Parte especifica del json a leer********************************
    if (json.containsKey("orden"))  orden = json.get<String>("orden");
    if (json.containsKey("id"))  id = json.get<uint8_t>("id");
    if (json.containsKey("valor"))  valor = json.get<String>("valor");

    Traza.mensaje("Salidas\ncomando leida:\norden: %s\nid: %02u\nvalor: %s\n",orden.c_str(),id,valor.c_str());
//************************************************************************************************
    if(orden==ORDEN_ACTUADOR_SALIDAS_ESTADO){
        if(valor==VALOR_ACCION_SALIDAS_ON) salidas.conmuta(id,ESTADO_ACTIVO);
        else if (valor==VALOR_ACCION_SALIDAS_OFF) salidas.conmuta(id,ESTADO_DESACTIVO);
        else if(valor==VALOR_ACCION_SALIDAS_PULSO) salidas.setPulso(id);
        else return false;
    }
    else if(orden==ORDEN_ACTUADOR_SALIDAS_MANUAL){
        if(valor==VALOR_ACCION_SALIDAS_MANUAL_ON) salidas.setModoManual(id);
        else if(valor==VALOR_ACCION_SALIDAS_MANUAL_OFF) salidas.setModoInicial(id);
        else return false;
    }
    else return false;

    return true;
    }
return false;
}

boolean actuadorSecuenciador(String comando){
  DynamicJsonBuffer jsonBuffer;
  JsonObject& json = jsonBuffer.parseObject(comando.c_str());

  if (json.success()) 
    {
    Traza.mensaje("\ncomando: json parseado\n");

    String orden="";
    uint8_t id=0;
    String valor="";
//******************************Parte especifica del json a leer********************************
    if (json.containsKey("orden"))  orden = json.get<String>("orden");
    //if (json.containsKey("id"))  id = json.get<uint8_t>("id");
    //if (json.containsKey("valor"))  valor = json.get<String>("valor");

    Traza.mensaje("Secuenciador\ncomando leida:\norden: %s\nid: %02u\nvalor: %s\n",orden.c_str(),id,valor.c_str());
//************************************************************************************************
    if(orden==ORDEN_ACTUADOR_SECUENCIADOR_ESTADO){
        if(valor==VALOR_ACCION_SECUENCIADOR_ON) secuenciador.activar();
        else if (valor==VALOR_ACCION_SECUENCIADOR_OFF) secuenciador.desactivar();
        else return false;
    }

    return true;
    }
return false;
}
/****************************** Acciones ****************************/
  
/****************************** Utilidades ****************************
Utilidades:
{
 	"tipo":"utilidad",
	"subtipo":"particiones",
	"orden":"estado",
	"valor":""
}

{
 	"tipo":"utilidad",
	"subtipo":"particiones",
	"orden":"nextBoot",
	"valor":"<particion>"
}

{
 	"tipo":"utilidad",
	"subtipo":"restart"
}

{
 	"tipo":"utilidad",
	"subtipo":"info"
}

{
 	"tipo":"utilidad",
	"subtipo":"NVS",
	"orden":"inicializaNVS",
    "id":"nombreServicio"/"mDNS"/"SSID"/"pass"/"usuario",
    "valor":"*"
}

{
 	"tipo":"utilidad",
	"subtipo":"NVS",
	"orden":"valoresDefecto"
}

{
 	"tipo":"utilidad",
	"subtipo":"NVS",
	"orden":"resetTotal"
}

{
 	"tipo":"utilidad",
	"subtipo":"NVS",
	"orden":"resetUser"
}

{
 	"tipo":"utilidad",
	"subtipo":"NVS",
	"orden":"resetWiFi"
}

{
 	"tipo":"utilidad",
	"subtipo":"NVS",
	"orden":"resetID"
}

**************************************************************************/
boolean utilidadParticiones(String comando){
  DynamicJsonBuffer jsonBuffer;
  JsonObject& json = jsonBuffer.parseObject(comando.c_str());

  if (json.success()) 
    {
    Traza.mensaje("\ncomando: json parseado\n");

    String orden="";
    String id="";
    String valor="";
//******************************Parte especifica del json a leer********************************
    if (json.containsKey("orden"))  orden = json.get<String>("orden");
    //if (json.containsKey("id"))  id = json.get<String>("id");
    if (json.containsKey("valor"))  valor = json.get<String>("valor");

    Traza.mensaje("Utilidad particiones\ncomando leida:\norden: %s\nid: %s\nvalor: %s\n",orden.c_str(),id.c_str(),valor.c_str());
//************************************************************************************************
    if (orden==ORDEN_UTILIDAD_PARTICIONES_ESTADO) enviarMQTT(TOPIC_UTILIDAD_PARTICIONES_ESTADO, listaParticiones());
    else if(orden==ORDEN_UTILIDAD_PARTICIONES_NEXTBOOT) setParticionProximoArranque(valor);//si existen esos argumentos
    else return false;

    return true;
  }
  return false;
}

boolean utilidadNVS(String comando){
  DynamicJsonBuffer jsonBuffer;
  JsonObject& json = jsonBuffer.parseObject(comando.c_str());

  if (json.success()) 
    {
    Traza.mensaje("\ncomando: json parseado\n");

    String orden="";
    String id="";
    String valor="";
//******************************Parte especifica del json a leer********************************
    if (json.containsKey("orden"))  orden = json.get<String>("orden");
    if (json.containsKey("id"))  id = json.get<String>("id");
    if (json.containsKey("valor"))  valor = json.get<String>("valor");

    Traza.mensaje("Utilidad NVS\ncomando leida:\norden: %s\nid: %s\nvalor: %s\n",orden.c_str(),id.c_str(),valor.c_str());
//************************************************************************************************

    if(orden==ORDEN_UTILIDAD_NVS_INICIALIZANVS){
        if(id=="DeviceID") configNVS.deviceID=valor.toInt();
        else if(id=="nombreServicio") configNVS.nombreServicio=valor;
        else if(id=="mDNS") configNVS.nombremDNS=valor;
        else if(id=="SSID") configNVS.SSID=valor;
        else if(id=="pass") configNVS.pass=valor;
        else if(id=="usuario") configNVS.usuario=valor;
        else return false;

        escribeConfigNVS(configNVS);
        return true;        
    }
    else if(orden==ORDEN_UTILIDAD_NVS_VALORESDEFECTO){
        escribeConfigNVSDefecto();
        return true;
        }
    else if(orden==ORDEN_UTILIDAD_NVS_RESETTOTAL){
        resetNVS_Total();
        return true;
        }
    else if(orden==ORDEN_UTILIDAD_NVS_RESETUSER){
        resetNVS_user();
        return true;
        }
    else if(orden==ORDEN_UTILIDAD_NVS_RESETWIFI){
        resetNVS_WiFi();
        return true;
        }
    else if(orden==ORDEN_UTILIDAD_NVS_RESETID){
        resetNVS_ID();
        return true;
        }
    else return false;
    }
  return false;
}  

boolean utilidadRestart(String comando){
  delay(1000);
  ESP.restart();
  return true;//por evitar el warning
}

boolean utilidadInfo(String comando){return enviarMQTT(TOPIC_UTILIDAD_INFO, generaJsonInfo());}
/****************************** Utilidades ****************************/