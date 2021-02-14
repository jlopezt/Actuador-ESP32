//<script type="text/javascript" src="./paho-mqtt.js"></script>
//import './paho-mqtt-min.js';

//var client       = null;
var hostname       = "mqtt.lopeztola.com";
//var hostname       = "10.68.0.101";
var port           = 443;//9000;
var clientId       = "web_id_" + parseInt(Math.random() * 100000, 10);
var topic_ping     = "ping";
var topic_status   = "casaPre/PuertaGaraje/#";


function connect(){
    console.log("conectando:\nhostname: " + hostname + "\npuerto: " + port + "\nclientId: " + clientId + "\ntopics: " + topic_ping + " : " + topic_status);
    //client = new Paho.MQTT.Client(hostname, Number(port), clientId);
    client = new Paho.MQTT.Client(hostname, Number(port), clientId);
    if(client==null) console.log("error al conectar");

    client.onConnectionLost = onConnectionLost;
    client.onMessageArrived = onMessageArrived;

    var options = {
        useSSL: true,
        //useSSL: false,
        cleanSession: true,
        userName : "domoticae",//"jlopezt",
        password : "88716",
        onSuccess: onConnect,
        onFailure: onFail
    };
    client.connect(options);
}

function onConnect(context) {
    console.log("Conectado...");

    options = {qos:0}
    client.subscribe(topic_ping, options);
    client.subscribe(topic_status, options);

    console.log("Subscrito a los topics...");
}

function onFail(context) {
    console.log("Error al conectar!");
}

function onConnectionLost(responseObject) {
    if (responseObject.errorCode !== 0) console.log("Error: " + responseObject.errorCode + " | mensaje: " + responseObject.errorMessage + " | Connection Lost!\nPlease Refresh.");
    connect();
}

function onMessageArrived(message) {
    console.log("mensaje: " + message.payloadString + " para:" + message.destinationName);
    if(message.destinationName.substr(0,4)=="ping") { //si empieza por ping
        if(message.destinationName=="ping") {//y no es un ping/response
            respondePingMQTT();
            return;
        }
    }
    //document.getElementById("topic").value = message.destinationName;
    //document.getElementById("texto").value = message.payloadString;    
    actualizaDatos(message.destinationName,message.payloadString);
}

function respondePingMQTT(){
    var payload='{"myIP": "clienteWeb","ID_MQTT": ' + clientId + ', "IPBroker": ' + hostname + ', "IPPuertoBroker": ' + port + '}';
    console.log("payload: " + payload);
    message = new Paho.MQTT.Message(payload);
    message.destinationName = 'ping/respuesta';
    message.retained = true;
    client.send(message);            
}

function actualiza(id,valor){
    var a=document.getElementById(id);
    a.innerHTML=valor;        
}; 

function actualizaDatos(topic,datos) {
    switch (topic) {
        case "casaPre/PuertaGaraje/entradas":
            actualizaEntradas(datos);
            break;
        case "casaPre/PuertaGaraje/salidas":
            actualizaSalidas(datos);
            break;
        case "casaPre/PuertaGaraje/maquinaEstados":
            actualizaMaquinaEstados(datos);
            break;
        case "casaPre/PuertaGaraje/secuenciador":
            actualizaSecuenciador(datos);
            break;
    }
}
