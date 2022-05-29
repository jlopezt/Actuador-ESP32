const FONDO     ="#DDDDDD";
const TEXTO     ="#000000";
const ACTIVO    ="#FFFF00";
const DESACTIVO ="#DDDDDD";

function inicializa() {
    var xhEntradas = new XMLHttpRequest();
    xhEntradas.onreadystatechange = function(){
        if (xhEntradas.readyState == 4){
            if(xhEntradas.status == 200) {
            console.log("JSON: " + xhEntradas.responseText)
            actualizaEntradas(xhEntradas.responseText);
            }
        }
    };
    xhEntradas.open("GET", "estadoEntradas", true);
    xhEntradas.send(null); 

    var xhSalidas = new XMLHttpRequest();
    xhSalidas.onreadystatechange = function(){
        if (xhSalidas.readyState == 4){
            if(xhSalidas.status == 200) {
            console.log("JSON: " + xhSalidas.responseText)
            actualizaSalidas(xhSalidas.responseText);
            }
        }
    };
    xhSalidas.open("GET", "estadoSalidas", true);
    xhSalidas.send(null); 

    var xhME = new XMLHttpRequest();
    xhME.onreadystatechange = function(){
        if (xhME.readyState == 4){
            if(xhME.status == 200) {
            console.log("JSON: " + xhME.responseText)
            actualizaMaquinaEstados(xhME.responseText);
            }
        }
    };
    xhME.open("GET", "estadoMaquinaEstados", true);
    xhME.send(null); 

    var xhSecuenciador = new XMLHttpRequest();
    xhSecuenciador.onreadystatechange = function(){
        if (xhSecuenciador.readyState == 4){
            if(xhSecuenciador.status == 200) {
            console.log("JSON: " + xhSecuenciador.responseText)
            actualizaSecuenciador(xhSecuenciador.responseText);
            }
        }
    };
    xhSecuenciador.open("GET", "estadoSecuenciador", true);
    xhSecuenciador.send(null); 

    connect();
}

function actualizaEntradas(datos) {
    var res = JSON.parse(datos);

    //entradas
    var entradas=res.entradas;
    console.log("numero entradas: " + entradas.length);

    entradas.forEach(function(entrada,indice,array) {
        var hilera = document.getElementById("entrada_"+indice);
        if(hilera==null){
            console.log("no existe la fila, la creo");
            var hilera = document.createElement("tr");
            hilera.setAttribute("id", "entrada_" + indice);
            hilera.setAttribute("class","modo2");
            document.getElementById("body_tabla_entradas").appendChild(hilera);

            var celda = document.createElement("td");
            celda.setAttribute("id","entrada_id_"+indice);
            celda.setAttribute("align","right");
            hilera.appendChild(celda);

            celda = document.createElement("td");
            celda.setAttribute("id","entrada_nombre_"+indice);
            celda.setAttribute("align","right");
            hilera.appendChild(celda);

            celda = document.createElement("td");
            celda.setAttribute("id","entrada_estado_"+indice);
            celda.setAttribute("align","center");
            hilera.appendChild(celda);
        }

        document.getElementById("entrada_id_" + indice).innerHTML=entrada.id;
        document.getElementById("entrada_nombre_" + indice).innerHTML=entrada.nombre;
        document.getElementById("entrada_estado_" + indice).innerHTML=(entrada.estado==0?"Off":"On");
    });

    //Si hay filas de mas, las borro
    var tabla = document.getElementById("tabla_entradas");
    if(tabla.rows.length-1>entradas.length){ //si hay mas filas que habitaciones, le resto la cabecera
        console.log("filas en tabla: " + (tabla.rows.length-1) + " | entradas: " + entradas.length);
        for(i=tabla.rows.length;i>entradas.length+1;i--){
            console.log("borro la fila " + (i));
            tabla.deleteRow(i-1);
        }
    }
}

function actualizaSalidas(datos) {
    var res = JSON.parse(datos);

    //salidas
    var salidas=res.salidas;
    console.log("numero salidas: " + salidas.length);

    salidas.forEach(function(salida,indice,array) {
        var hilera = document.getElementById("salida_"+indice);
        if(hilera==null){
            console.log("no existe la fila, la creo");
            var hilera = document.createElement("tr");
            hilera.setAttribute("id", "salida_" + indice);
            hilera.setAttribute("class","modo2");
            document.getElementById("body_tabla_salidas").appendChild(hilera);

            var celda = document.createElement("td");
            celda.setAttribute("id","salida_id_"+indice);
            celda.setAttribute("align","right");
            hilera.appendChild(celda);

            celda = document.createElement("td");
            celda.setAttribute("id","salida_nombre_"+indice);
            celda.setAttribute("align","right");
            hilera.appendChild(celda);

            celda = document.createElement("td");
            celda.setAttribute("id","salida_estado_"+indice);
            celda.setAttribute("align","center");
            hilera.appendChild(celda);

            celda = document.createElement("td");
            celda.setAttribute("id","salida_accion_"+indice);
            celda.setAttribute("align","center");
            hilera.appendChild(celda);
        }

        document.getElementById("salida_id_" + indice).innerHTML=salida.id;
        document.getElementById("salida_nombre_" + indice).innerHTML=salida.nombre;
        
        //document.getElementById("salida_estado_" + indice).innerHTML=salida.nombreEstado;
        var estado=document.getElementById("salida_estado_" + indice);
        estado.innerHTML=salida.nombreEstado;

        var accion=document.getElementById("salida_accion_" + indice);
        if(salida.modo=="Manual"){
            if(salida.estado==0) {
                estado.style.backgroundColor=DESACTIVO;

                accion.innerHTML  = "<form action='activaSalida'><input  type='hidden' id='activa_" + indice + "' name='id' value='" + indice + "'><input STYLE='color: #000000; text-align: center; background-color: #FFFF00; width: 80px' type='submit' value='activar'></form>";
                accion.innerHTML += "<form action='pulsoSalida'><input  type='hidden' id='pulso_" + indice + "' name='id' value='" + indice + "'><input STYLE='color: #000000; text-align: center; background-color: #FFFF00; width: 80px' type='submit' value='pulso'></form>";
            }
            else {
                estado.style.backgroundColor=ACTIVO;
                
                //accion.innerHTML  = "<form action='desactivaSalida'><input  type='hidden' id='id' name='id' value='0'><input STYLE='color: #000000; text-align: center; background-color: #DDDDDD; width: 80px' type='submit' value='desactivar'></form>";
                accion.innerHTML  = "<form action='desactivaSalida'><input  type='hidden' id='desactiva_" + indice + "' name='id' value='" + indice + "'><input STYLE='color: #000000; text-align: center; background-color: #DDDDDD; width: 80px' type='submit' value='desactivar'></form>";
            }
        }
        else{
            accion.innerHTML="<p>Modo " + salida.modo + "</p>";
        }
    });

    //Si hay filas de mas, las borro
    var tabla = document.getElementById("tabla_salidas");
    if(tabla.rows.length-1>salidas.length){ //si hay mas filas que salidas, le resto la cabecera
        console.log("filas en tabla: " + (tabla.rows.length-1) + " | salidas: " + salidas.length);
        for(i=tabla.rows.length;i>salidas.length+1;i--){
            console.log("borro la fila " + (i));
            tabla.deleteRow(i-1);
        }
    }
}

function actualizaSecuenciador(datos) {
    var res = JSON.parse(datos);

    //Secuenciador
    var planes=res.planes;
    console.log("numero planes: " + planes.length);
    
    //Boton de activacion/desactivacion
    if(res.estado==0){
        document.getElementById("secuenciadorCaption").innerHTML  = "Secuenciadores <form action='activaSecuenciador'><input STYLE='color: #000000; text-align: center; background-color: #FFFF00; width: 80px' type='submit' value='activar'></form>";
    }
    else{
    document.getElementById("secuenciadorCaption").innerHTML  = "Secuenciadores <form action='desactivaSecuenciador'><input STYLE='color: #000000; text-align: center; background-color: #DDDDDD; width: 80px' type='submit' value='desactivar'></form>";
    }

    planes.forEach(function(plan,indice,array) {
        var hilera = document.getElementById("plan_"+indice);
        if(hilera==null){
            console.log("no existe la fila, la creo");
            var hilera = document.createElement("tr");
            hilera.setAttribute("id", "plan_" + indice);
            hilera.setAttribute("class","modo2");
            document.getElementById("body_tabla_secuenciador").appendChild(hilera);

            var celda = document.createElement("td");
            celda.setAttribute("id","plan_id_"+indice);
            celda.setAttribute("align","right");
            hilera.appendChild(celda);

            celda = document.createElement("td");
            celda.setAttribute("id","plan_nombre_"+indice);
            celda.setAttribute("align","right");
            hilera.appendChild(celda);

            celda = document.createElement("td");
            celda.setAttribute("id","plan_salida_"+indice);
            celda.setAttribute("align","center");
            hilera.appendChild(celda);

            celda = document.createElement("td");
            celda.setAttribute("id","plan_estado_"+indice);
            celda.setAttribute("align","center");
            hilera.appendChild(celda);
        }

        document.getElementById("plan_id_" + indice).innerHTML=plan.id;
        document.getElementById("plan_nombre_" + indice).innerHTML=plan.nombre;
        document.getElementById("plan_salida_" + indice).innerHTML=plan.salida;                
        document.getElementById("plan_estado_" + indice).innerHTML=plan.estado;
    });
}

function actualizaMaquinaEstados(datos) {
    var res = JSON.parse(datos);

    //salidas
    var estado=res.estado;
    console.log("estado: " + estado);
    document.getElementById("estadoME").innerHTML=estado;
}

