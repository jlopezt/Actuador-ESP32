/**********************************************/
/*                                            */
/*  Gestion del Websockets                    */
/*  Dimaniza la interfaz Web                  */
/*                                            */
/**********************************************/

#define PUERTO_WEBSOCKETS 81
#define NOT_CONNECTED     -1

//#include <WebSockets.h>
#include <WebSocketsServer.h>

//boolean enviarWSTXT(String mensaje);
typedef struct
  {
  int8_t id=NOT_CONNECTED;//No hay cliente conectado
  IPAddress IP={0,0,0,0};
  }cliente_t;
cliente_t cliente;

WebSocketsServer webSocket= WebSocketsServer(PUERTO_WEBSOCKETS);    // create a websocket server on port 81
/****************************************************************************************************************/
/**********************************************/
/*                                            */
/*   Inicializacion del servicio de Websocket */
/*   para ser llmadao desde setup             */
/*                                            */
/**********************************************/
void inicializaWebSockets()
  {
  //Inicializo la estructura de cliente
  cliente.id=NOT_CONNECTED;
  cliente.IP={0,0,0,0};

  //*******Configuracion del WS server***********
  if (debugGlobal) Serial.printf("Iniciamos el servidor de websockets\n");
  webSocket.begin();
  if (debugGlobal) Serial.printf("Asignado gestor de mensajes\n");
  webSocket.onEvent(webSocketEvent);
  if (debugGlobal) Serial.printf("Finalizado\n");  
  }

/**********************************************/
/*                                            */
/*   Llamada desde el loop para identificar   */
/*   los eventos WebSockets                   */
/*                                            */
/**********************************************/
void atiendeWebSocket(int debug)
  {
  webSocket.loop();
  } 

/**********************************************/
/*                                            */
/*   Encapsula el envio de mensajes a traves  */
/*   de websockets                            */
/*                                            */
/**********************************************/
boolean enviarWSTXT(String mensaje)
  {
  boolean salida=false;  

  if(cliente.id!=NOT_CONNECTED) salida=webSocket.sendTXT(cliente.id, (const uint8_t *)mensaje.c_str());

  if (debugGlobal)
    {
    if (salida ) Serial.println(mensaje);
    else Serial.printf("Error en el envio de [%s] | id de cliente: %i | IP de cliente: %s\n",mensaje.c_str(),cliente.id,cliente.IP.toString().c_str());
    }

  return salida; 
  }

/**********************************************/
/*                                            */
/*   Gestiona los eventos que llegan a traves */
/*   del interfaz websockets                  */
/*                                            */
/**********************************************/  
void webSocketEvent(uint8_t clienteId, WStype_t type, uint8_t * payload, size_t length) 
  {
  switch(type) 
    {
    case WStype_DISCONNECTED:
        Serial.printf("[%u] Desconectado\n", clienteId);
        cliente.id=NOT_CONNECTED;//Se ha Desconectado
        cliente.IP={0,0,0,0};
        break;
    case WStype_CONNECTED:
        {
        IPAddress ip = webSocket.remoteIP(clienteId);
        Serial.printf("[%u] Connectado desde IP %d.%d.%d.%d url: %s\n", clienteId, ip[0], ip[1], ip[2], ip[3], payload);
        //Nuevo cliente conectado
        cliente.id=clienteId;
        cliente.IP=ip;

        //Lo paso al estado inicial
        }
        break;
    case WStype_TEXT:
        {
        String datos="";
        for(uint8_t i=0;i<length;i++) datos+=(char)payload[i];

        Serial.printf("[%u] recibido el texto: %s | datos: %s\n", clienteId, payload,datos.c_str());
        gestionaMensajes(clienteId,datos);
        }
        break;
    case WStype_BIN:
        Serial.printf("[%u] recibidos datos binarios length: %u\n", clienteId, length);
        hexdump(payload, length);

        // send message to client
        // webSocket.sendBIN(num, payload, length);
        break;
    case WStype_ERROR:      
    case WStype_FRAGMENT_TEXT_START:
    case WStype_FRAGMENT_BIN_START:
    case WStype_FRAGMENT:
    case WStype_FRAGMENT_FIN:
    case WStype_PING:
    case WStype_PONG:
      break;
    }
  }

/**********************************************/
/*                                            */
/*   Gestiona los mensajes recibidos por WS   */
/*                                            */
/**********************************************/
void gestionaMensajes(uint8_t cliente, String mensaje)
  {
  Serial.printf("Procesando mensaje %s, cliente: %i\n",mensaje.c_str(),cliente);
  if(mensaje== "stream") 
    {
    webSocket.sendTXT(cliente, "STREAMING");
    Serial.println("Enviado STREAMING");
    }

  if (mensaje == "foto")  
    {
    Serial.println("Enviando foto al cliente...");
    webSocket.sendTXT(cliente, "foto");
    //Envio binario. Ej:
    //webSocket.sendBIN(cliente,fb->buf, fb->len);//uint8_t num, const uint8_t * payload, size_t length);
    }

  if (mensaje == "no_reconoce")  
    {
    Serial.println("Activa reconocimiento");
    //No hace falta enviar mensaje ej: 
    //activaRecon(false);
    }
  }

/**********************************************/
/*                                            */
/*   Trasnforma un mensaje hexadecimal en     */
/*   texto imprimible en pantalla             */
/*                                            */
/**********************************************/
void hexdump(const void *mem, uint32_t len) 
  {
  const uint8_t cols = 16;  
  const uint8_t* src = (const uint8_t*) mem;
  Serial.printf("\n[HEXDUMP] Address: 0x%08X len: 0x%X (%d)", (ptrdiff_t)src, len, len);
  for(uint32_t i = 0; i < len; i++) 
    {
    if(i % cols == 0) 
      {
      Serial.printf("\n[0x%08X] 0x%08X: ", (ptrdiff_t)src, i);
      }
    Serial.printf("%02X ", *src);
    src++;
    }
  Serial.printf("\n");
  }
/*****************Codigo JavaScript para enviar y procesar WS*****************************
<script>
document.addEventListener("DOMContentLoaded", function(event) {
  var baseHost = document.location.origin;
  var streamUrl = baseHost + ":81";
  const WS_URL = "ws://" + window.location.host + ":82";
  const ws = new WebSocket(WS_URL);

  const view = document.getElementById("stream");
  const personFormField = document.getElementById("person");
  const streamButton = document.getElementById("button-stream");
  const detectButton = document.getElementById("button-detect");
  const captureButton = document.getElementById("button-capture");
  const recogniseButton = document.getElementById("button-recognise");
  const deleteAllButton = document.getElementById("delete_all");

  ws.onopen = () => {
    console.log(`Connected to ${WS_URL}`);
  };
  ws.onmessage = message => {
    if (typeof message.data === "string") {
      if (message.data.substr(0, 8) == "listface") {
        addFaceToScreen(message.data.substr(9));
      } else if (message.data == "delete_faces") {
        deleteAllFacesFromScreen();
      } else {
        document.getElementById("current-status").innerHTML = message.data;
      }
    }
    if (message.data instanceof Blob) {
      var urlObject = URL.createObjectURL(message.data);
      view.src = urlObject;
    }
  };

  streamButton.onclick = () => {
    ws.send("stream");
  };
  detectButton.onclick = () => {
    ws.send("detect");
  };
  captureButton.onclick = () => {
    person_name = document.getElementById("person").value;
    ws.send("capture:" + person_name);
  };
  recogniseButton.onclick = () => {
    ws.send("recognise");
  };
  deleteAllButton.onclick = () => {
    ws.send("delete_all");
  };
  personFormField.onkeyup = () => {
    captureButton.disabled = false;
  };

  function deleteAllFacesFromScreen() {
    // deletes face list in browser only
    const faceList = document.querySelector("ul");
    while (faceList.firstChild) {
      faceList.firstChild.remove();
    }
    personFormField.value = "";
    captureButton.disabled = true;
  }

  function addFaceToScreen(person_name) {
    const faceList = document.querySelector("ul");
    let listItem = document.createElement("li");
    let closeItem = document.createElement("span");
    closeItem.classList.add("delete");
    closeItem.id = person_name;
    closeItem.addEventListener("click", function() {
      ws.send("remove:" + person_name);
    });
    listItem.appendChild(
      document.createElement("strong")
    ).textContent = person_name;
    listItem.appendChild(closeItem).textContent = "X";
    faceList.appendChild(listItem);
  }

  captureButton.disabled = true;
});
</script>
*******************************************************************/
