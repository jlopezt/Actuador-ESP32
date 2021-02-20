const MAX_INTERVALOS_EN_HORA=60;

function inicializa(){    
    var xhSecuenciador = new XMLHttpRequest();
    xhSecuenciador.onreadystatechange = function(){
        if (xhSecuenciador.readyState == 4){
            if(xhSecuenciador.status == 200) {
                console.log("JSON: " + xhSecuenciador.responseText)
                var res = JSON.parse(xhSecuenciador.responseText);

                var planes=res.Planes;
                console.log("numero planes: " + planes.length);
                document.getElementById("numeroPlanes").innerHTML=planes.length;
                if (planes.length<=0) return;

                //La guardo para todas las demas tablas
                var cabecera0 = document.getElementById("cabecera_0");
                
                //para cada plan, relleno su tabla
                planes.forEach(function(plan,nPlan,array) {
                    console.log("Plan: " + nPlan);

/*******************************HAY QUE CREAR LA TABLA SI NO EXSTE!!!****************************************/
                    var tabla=document.getElementById("tabla_plan_" + nPlan);
                    if(tabla==null){
                        var br=document.createElement("br");
                        document.getElementById("body").appendChild(br);

                        var tabla=document.createElement("table");
                        tabla.setAttribute("id","tabla_plan_" + nPlan);
                        tabla.setAttribute("width","90%");
                        tabla.setAttribute("border","0");
                        tabla.setAttribute("cellpadding","0");
                        tabla.setAttribute("cellspacing","0");
                        tabla.setAttribute("class","tabla");
                        document.getElementById("body").appendChild(tabla);

                        var caption=document.createElement("caption");
                        caption.setAttribute("id","caption_" + nPlan)
                        //caption.innerHTML="Plan " + plan.nombre + " salida: " + plan.salida;
                        tabla.appendChild(caption);

                        var tbody = document.createElement("tbody");
                        tbody.setAttribute("id","body_tabla_plan_" + nPlan);
                        tabla.appendChild(tbody);

                        var cabecera = cabecera0.cloneNode(true);
                        tbody.appendChild(cabecera);
                    }
/***********************************************************************/
                    var numeroIntervalos=MAX_INTERVALOS_EN_HORA;
                    if(plan.intervalos.length<numeroIntervalos) numeroIntervalos=plan.intervalos.length;
                    console.log("numero intervalos: " + numeroIntervalos);
/*******************************Creo las filas siempre****************************************/
                    //cambio la caption de la tabla
                    var _caption=document.getElementById("caption_" + nPlan);
                    _caption.innerHTML="Plan " + plan.nombre + " salida: " + plan.salida;                                

                    //Creo las filas que necesite, tantas como intervalos. La cero la tengo
                    for(fila=0;fila<numeroIntervalos;fila++){
                        var nuevaFila=document.createElement("tr");
                        nuevaFila.setAttribute("id", "intervalo_" + fila);
                        nuevaFila.setAttribute("class", "modo2");
                        document.getElementById("body_tabla_plan_" + nPlan).appendChild(nuevaFila);

                        var nuevaCelda=document.createElement("th");
                        nuevaCelda.innerHTML="min. " + fila;
                        nuevaFila.appendChild(nuevaCelda);
                        
                        for(_hora=0;_hora<24;_hora++){
                            nuevaCelda=document.createElement("td");
                            nuevaCelda.setAttribute("id", "plan_" + nPlan + "_hora_" + _hora + "_" + fila);
                            nuevaCelda.setAttribute("style","text-align:center");
                            nuevaFila.appendChild(nuevaCelda);
                        }
                    }
/***********************************************************************/

                    //Con la tabla creada, la lleno de izquierda a derecha
                    intervalos=plan.intervalos;
                    intervalos.forEach(function(intervalo,nIntervalo,array){
                        mascara=1;     
                        if(nIntervalo<numeroIntervalos){
                            for(_hora=0;_hora<24;_hora++){//hora_<hora>_<intervalo>
                                /*
                                console.log("hora: " + _hora + " minuto: " + nIntervalo);
                                console.log("valor: " + intervalo.valor + " mascara: " + mascara);
                                estado=(intervalo.valor &  mascara?1:0);
                                console.log("estado = " + estado);
                                */
                                document.getElementById("plan_" + nPlan + "_hora_" + _hora + "_" + nIntervalo).innerHTML=(intervalo.valor &  mascara?1:0);
                                mascara*=2;
                            }
                        }
                    });
                });
            }
        }
    };
    xhSecuenciador.open("GET", "configSecuenciador", true);
    xhSecuenciador.send(null);					
};