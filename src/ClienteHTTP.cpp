#include <Global.h>
#include <ClienteHTTP.h>
#include <HTTPClient.h>
#include <configNVS.h>
#include <Ficheros.h>

HTTPClient clienteHTTP;

int enviaDatosHTTP(String URL, String datos, uint8_t metodo);

void inicializaPlataforma(boolean debug){
    if(configNVS.usuario==String() ||configNVS.nombreServicio==String()){
        Serial.printf("No hay CID o SID para la asociación\n");
        return;
    }

    if(testHTTP()==200){
        int ret=asocia();
        Serial.printf("Resultado de la asociacion: %i\n",ret);
    }
}

int asocia(void){
    char DID[17]="";
    sprintf(DID,"%016llx",configNVS.deviceID);
    String URL=String(URL_PLATAFORMA) + "/asocia/" + configNVS.usuario + "/" + String(DID) + "?nombreServicio=" + configNVS.nombreServicio;
    
    return enviaDatosHTTP(URL,"",METODO_POST);
}

int testHTTP(){    
    String URL=String(URL_PLATAFORMA) + "/test";
    
    return enviaDatosHTTP(URL,"",METODO_POST);
}

int enviaDatosHTTP(String URL, String datos, uint8_t metodo){
    int ret=0;

    clienteHTTP.begin(URL);

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
    
    clienteHTTP.end();

    return ret;
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

    String URL=String(URL_PLATAFORMA) + "/configuracion/" + configNVS.usuario + "/" + configNVS.nombreServicio + "/" + file;
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
    String URL = String(URL_PLATAFORMA) + "/configuracion/" + configNVS.usuario + "/" + configNVS.nombreServicio + "/" + servicio;
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