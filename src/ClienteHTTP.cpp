/***************************** Includes *****************************/
#include <Global.h>
#include <ClienteHTTP.h>
#include <HTTPClient.h>
#include <configNVS.h>
#include <Ficheros.h>
/***************************** Fin includes *****************************/

/***************************** Defines *****************************/
/***************************** FIN defines *****************************/

/***************************** variables *****************************/
const char* test_root_ca= \
  "-----BEGIN CERTIFICATE-----\n" \
  "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\n" \
  "-----END CERTIFICATE-----\n";

uint8_t estadoAsociacion=NO_ASOCIADO;

HTTPClient clienteHTTP;
/***************************** Fin variables *****************************/

/***************************** Funciones *****************************/
int enviaDatosHTTP(String URL, String datos, uint8_t metodo);

void inicializaPlataforma(boolean debug){
    estadoAsociacion=NO_ASOCIADO;

    if(configNVS.usuario==String() ||configNVS.nombreServicio==String()){
        Serial.printf("No hay CID o SID para la asociación\n");
        return;
    }

    int test_result=testHTTP();    
    if (test_result == HTTP_CODE_OK || test_result == HTTP_CODE_MOVED_PERMANENTLY){
        int ret=asocia();
        Serial.printf("Resultado de la asociacion: %i | %s\n",ret, clienteHTTP.getString().c_str());

        switch(ret){
            case HTTP_CODE_OK:
                estadoAsociacion=ASOCIADO_VALIDADO;                
                break;
            case HTTP_CODE_CREATED:
            case HTTP_CODE_NON_AUTHORITATIVE_INFORMATION:
                estadoAsociacion=ASOCIADO_PENDIENTE_VALIDACION;
                break;
            case HTTP_CODE_CONFLICT:
            case HTTP_CODE_NOT_ACCEPTABLE:                
            case HTTP_CODE_UNAUTHORIZED:
            default:
                estadoAsociacion=NO_ASOCIADO;              
        }
    }
    else Serial.printf("Test plataforma KO: %i | %s\n",test_result,clienteHTTP.errorToString(test_result).c_str());
}

int asocia(void){
    char DID[17]="";
    sprintf(DID,"%016llx",configNVS.deviceID);
    String URL=URLPlataforma + "/asocia/" + configNVS.usuario + "/" + String(DID) + "?nombreServicio=" + configNVS.nombreServicio + "&tipoDispositivo=Actuador";
    
    return enviaDatosHTTP(URL,"",METODO_POST);
}

int testHTTP(){    
    String URL=URLPlataforma + "/test";
    Serial.printf("URL de la plataforma: %s\n",URL.c_str());
    return enviaDatosHTTP(URL,"",METODO_POST);
}

int enviaDatosHTTP(String URL, String datos, uint8_t metodo){
    int ret=0;

    clienteHTTP.begin(URL);//,test_root_ca);

    switch (metodo){
        case METODO_POST:
            ret = clienteHTTP.POST(datos);
            break;
        case METODO_GET:
            Serial.printf("Alla voy... con URL: %s\n",URL.c_str());
            ret = clienteHTTP.GET();
            break;
        case METODO_PUT:
            ret = clienteHTTP.PUT(datos);
            break;
        case METODO_PATCH:
            ret = clienteHTTP.PATCH(datos);
            break;
        /*    
        case METODO_DELETE:
            ret = clienteHTTP.DELETE();
            break;
        */
    }    

    //clienteHTTP.end();

    return ret;
}

uint8_t getEstadoAsociacion(void){
    return estadoAsociacion;
}

boolean setEstadoAsociacion(uint8_t estado){
    switch (estado){
        case NO_ASOCIADO:
        case ASOCIADO_PENDIENTE_VALIDACION:
        case ASOCIADO_VALIDADO:
            estadoAsociacion=estado;
            return true;
            break;
    }
    return false;
}

/***************************************************/
/*                                                 */
/* Envia ficheros de configuracion a la plataforma */
/* fichero es el nombre del fichero                */
/* file vale para generar la ruta, sin / ni .json  */
/*                                                 */
/***************************************************/
boolean enviaFicheroConfig(String fichero){
    String contenido="";
    String file=fichero;

    if(file.startsWith("/")) file.remove(0,1);//Si tiene barra al principio la quito
    file=file.substring(0,file.indexOf('.'));

    String URL=URLPlataforma + "/configuracion/" + configNVS.usuario + "/" + configNVS.nombreServicio + "/" + file;
    URL += "?address=" + configNVS.contrasena;
    
    if(leeFichero(fichero, contenido)) 
        if (enviaDatosHTTP(URL, contenido,METODO_POST)) return true;

    return false;
}

/***************************************************/
/*                                                 */
/* Solicita el envio de ficheros de configuracion  */
/* a la plataforma                                 */
/* servicio es el nombre de un servicio            */
/* para salvarlo como fichero hay que añadirle     */
/* / por delante y .json por detras                */
/*                                                 */
/***************************************************/
boolean leeFicheroConfig(String servicio){
    String URL = URLPlataforma + "/configuracion/" + configNVS.usuario + "/" + configNVS.nombreServicio + "/" + servicio;
    URL = URL + "?address=" + configNVS.contrasena;
    Serial.printf("Invoco con \nURL: %s\nservicio: %s\n",URL.c_str(),servicio.c_str());

    if(!clienteHTTP.begin(URL)){
        Serial.printf("No se puede conectar a %s\n",URL.c_str());
        clienteHTTP.end();
        return false;
    }

    int ret = clienteHTTP.sendRequest("GET",(uint8_t *)NULL,0);    //HTTPC_ERROR_SEND_HEADER_FAILED  -2
    //int ret = enviaDatosHTTP(URL, "", METODO_GET);
    if(ret < 200 || ret >= 300){
        Serial.printf("Salgo con codigo %i\n",ret);
        clienteHTTP.end();
        return false;
    }

    String respuesta = clienteHTTP.getString();
    Serial.printf("Respuesta recibida:\n[%s]\n",respuesta.c_str());
    
    clienteHTTP.end();

    return salvaFichero("/" + servicio + ".json", "/" + servicio + ".json.bak", respuesta);
}
/***************************** Fin funciones *****************************/