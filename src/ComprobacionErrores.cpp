/*****************************************/
/*                                       */
/*  Control de errores en configuración  */
/*  realiza un chequeo por configuracion */
/*  buscando incongruencias              */
/*                                       */
/*****************************************/
/*
From https://github.com/espressif/esp-idf/blob/357a277603/components/driver/include/driver/gpio.h
/// Check whether it is a valid GPIO number
#define GPIO_IS_VALID_GPIO(gpio_num)        (((1ULL << (gpio_num)) & SOC_GPIO_VALID_GPIO_MASK) != 0)
/// Check whether it can be a valid GPIO number of output mode
#define GPIO_IS_VALID_OUTPUT_GPIO(gpio_num) (((1ULL << (gpio_num)) & SOC_GPIO_VALID_OUTPUT_GPIO_MASK) != 0)
*/

/***************************** Defines *****************************/
#define MAX_PINES_ESP32  39
/***************************** Defines *****************************/

/***************************** Includes *****************************/
#include <Global.h>
#include <ComprobacionErrores.h>
#include <Entradas.h>
#include <Salidas.h>
#include <Secuenciador.h>
#include <MaqEstados.h>
#include <SNTP.h>
#include <Ficheros.h>
/***************************** Includes *****************************/

boolean compruebaFicheros(void) 
  {
  String cad = "Log verificacion configuracion - " + getFecha() + " - " + getHora() + "\n";

  return salvaFichero(FICHERO_ERRORES, "", cad);
  }

String compruebaGHN(void)
  {
  String cad = "";  
  return cad;
  }

String compruebaWifi(void)
  {
  String cad = "";  
  return cad;
  }

String compruebaWebserver(void)
  {
  String cad = "";  
  return cad;
  }

String comrpuebaMQTT(void)
  {
  String cad = "";  
  return cad;
  }

String compruebaES(void)
  {
  String cad = "";  

  //ENTRADAS
  cad += "Entradas\n";
  //uint8_t nEntradas=entradasConfiguradas() ;
  uint8_t nEntradas=entradas.getNumEntradas();
  cad += "configuradas " + String(nEntradas) + " entradas\n";
  for(uint8_t _entrada=0;_entrada<nEntradas;_entrada++)
    {
    if(entradas.getEntrada(_entrada).getNombre()=="") cad += "Entrada " + String(_entrada) + " | Nombre no configurado\n";  
    if(entradas.getEntrada(_entrada).getEstadoActivo()!=ESTADO_DESACTIVO && entradas.getEntrada(_entrada).getEstadoActivo()!=ESTADO_ACTIVO) cad += "Entrada " + String(_entrada) + " | estadoActivo valor no valido\n";
    if(entradas.getEntrada(_entrada).getPin()<0 || entradas.getEntrada(_entrada).getPin()>MAX_PINES_ESP32) cad += "Entrada " + String(_entrada) + " | pin no valido\n";  
    if(entradas.getEntrada(_entrada).getTipo()!="INPUT" && entradas.getEntrada(_entrada).getTipo()!="INPUT_PULLUP") cad += "Entrada " + String(_entrada) + " | tipo no valido\n";        
    }

  //SALIDAS
  cad += "Salidas\n";
  uint8_t nSalidas=salidas.salidasConfiguradas() ;
  
  cad += "configuradas " + String(nSalidas) + " salidas\n";
  for(int8_t salida=0;salida<nSalidas;salida++)
    {
    if(salidas.getSalida(salida).getNombre()=="") cad += "Salida " + String(salida) + " | Nombre no configurado\n";
    if(salidas.getSalida(salida).getModo()!=MODO_MANUAL && salidas.getSalida(salida).getModo()!=MODO_SECUENCIADOR && salidas.getSalida(salida).getModo()!=MODO_SEGUIMIENTO && salidas.getSalida(salida).getModo()!=MODO_MAQUINA ) cad += "Salida " + String(salida) + " | Modo valor no valido\n";      
    if(salidas.getSalida(salida).getPin()<0 || salidas.getSalida(salida).getPin()>MAX_PINES_ESP32) cad += "Salida " + String(salida) + " | pin no valido\n";  
    if(salidas.getSalida(salida).getAnchoPulso()<0) cad += "Salida " + String(salida) + " | ancho de pulso no valido\n";  
    if(salidas.getSalida(salida).getControlador()>0 && salidas.getSalida(salida).getModo()!=MODO_SECUENCIADOR && salidas.getSalida(salida).getModo()!=MODO_SEGUIMIENTO) cad += "Salida " + String(salida) + " | controlador configurado en un modo incorrecto\n";  
    if(salidas.getSalida(salida).getModo()==MODO_SEGUIMIENTO) cad += "Salida " + String(salida) + " | no hay un controlador configurado en un modo que lo requiere\n";  
    if(salidas.getSalida(salida).getModo()==MODO_SECUENCIADOR) cad += "Salida " + String(salida) + " | no hay un controlador configurado en un modo que lo requiere\n";  
    if(salidas.getSalida(salida).getModo()==MODO_SEGUIMIENTO) cad += "Salida " + String(salida) + " | configurada en modo seguimiento pero el controlador configurado no es una entrada valida\n";  
    if(salidas.getSalida(salida).getModo()==MODO_SECUENCIADOR) cad += "Salida " + String(salida) + " | configurada en modo secuenciador pero el controlador configurado no es un plan valido\n";  
    if(salidas.getSalida(salida).getEstadoInicial()!=ESTADO_DESACTIVO && salidas.getSalida(salida).getEstadoInicial()!=ESTADO_ACTIVO) cad += "Salida " + String(salida) + " | inicio no valido\n";
    }

  return cad;
  }

String compruebaSecuenciador(void){
  String cad = "";  
  int8_t nPlanes=secuenciador.getNumPlanes();

  cad += "Secuenciador\n";
  cad += "configurados " + String(nPlanes) + " planes de secuenciador\n";
  Serial.printf("%s",cad.c_str());
  for(uint8_t plan=0;plan<nPlanes;plan++){
    if(secuenciador.getSalida(plan)>=salidas.getNumSalidas()) cad +=  "Plan " + secuenciador.getNombrePlan(plan) + " | la salida asociada al plan no esta configurada";  
    else{
      if(salidas.getSalida(secuenciador.getSalida(plan)).getModo()!=MODO_SECUENCIADOR) cad += "Plan " + secuenciador.getNombrePlan(plan) + " | la salida asociada al plan no tiene el modo correcto\n";  
    }
  }
  return cad;
}

String compruebaMaquinaEstados(void)
  {
  String cad = "Maquina de estados\n";
  
  //Entradas
  cad += "Entradas\n";
  uint8_t nEntradas=maquinaEstados.getNumEntradas();
  
  cad += "configuradas " + String(nEntradas) + " entradas\n";
  for(uint8_t entrada=0;entrada<nEntradas;entrada++)
    {      
    if(maquinaEstados.getMapeoEntrada(entrada)>entradas.getNumEntradas()) cad += "Entrada " + String(entrada) + " | el numero de entrada es mayor al numero de entradas configurado\n";  
    }

  //Salidas
  cad += "Salidas\n";
  uint8_t nSalidas=maquinaEstados.getNumSalidas();
  
  cad += "configuradas " + String(nSalidas) + " salidas\n";
  for(uint8_t salida=0;salida<nSalidas;salida++)
    {
    if(maquinaEstados.getMapeoSalida(salida)>salidas.getNumSalidas()) cad += "Salida " + String(salida) + " | el numero de salida es mayor que el numero de salidas configuradas\n";  
    else
      {
      if(salidas.getSalida(maquinaEstados.getMapeoSalida(salida)).getModo()!=MODO_MAQUINA) cad += "Salida " + String(salida) + " | la salida asociada no tiene el modo correcto\n";  
      }
    }

  //Estados
  cad += "Estados\n";
  uint8_t nEstados=maquinaEstados.getNumEstados() ;
  
  cad += "configurados " + String(nEstados) + " estados\n";
  for(uint8_t estado=0;estado<nEstados;estado++)
    {
    if(maquinaEstados.estados[estado].getNombre()=="") cad += "Estado " + String(estado) + " | el nombre no esta configurado\n";  

    for(int8_t i=0;i<nSalidas;i++)
      {
      for(int8_t j=0;j<nSalidas;j++) 
        {
        if(maquinaEstados.estados[estado].getValorSalida(i)!=ESTADO_DESACTIVO && maquinaEstados.estados[estado].getValorSalida(i)!=ESTADO_ACTIVO) cad += "Estado " + String(estado) + " Salida " + String(j) + " | el valor configurado no es valido\n";  
        }
      }
    }

  //Trasnsiciones
  cad += "Trasnsiciones\n";
  uint8_t nTransiciones=maquinaEstados.getNumTransiciones() ;
  
  cad += "configuradas " + String(nTransiciones) + " trasnsiciones\n";
  for(uint8_t transi=0;transi<nTransiciones;transi++)
    {
    boolean correctoInicial=false;
    boolean correctoFinal=false;
    for(int8_t estado=0;estado<nEstados;estado++) 
      {
      if(maquinaEstados.transiciones[transi].getEstadoInicial()==maquinaEstados.estados[estado].getId()) correctoInicial=true;
      if(maquinaEstados.transiciones[transi].getEstadoFinal()==maquinaEstados.estados[estado].getId()) correctoFinal=true;
      }
    if(!correctoInicial) cad += "Transicion " + String(transi) + " | el estado inicial no es valido\n";  
    if(!correctoFinal) cad += "Transicion " + String(transi) + " | el estado final no es valido\n";  

    for(int8_t entrada=0;entrada<nEntradas;entrada++)
      {
      if(maquinaEstados.transiciones[transi].getValorEntrada(entrada)!=ESTADO_DESACTIVO && maquinaEstados.transiciones[transi].getValorEntrada(entrada)!=ESTADO_ACTIVO && maquinaEstados.transiciones[transi].getValorEntrada(entrada)!=ESTADO_NEUTRO_ME) cad += "Transicion " + String(transi) + " Entrada " + String(entrada) + " | el valor configurado no es valido\n";
      }
    }
  return cad;
  }
  
uint8_t compruebaConfiguracion(uint8_t nivelTraza)
  {
  boolean salvar=false;
  String cad="";

  Serial.printf("Init Verificar errores configuracion---------------------------------------------------\n");
  Serial.printf("     ficheros--------------------------------------------------------------------------\n");
  salvar=compruebaFicheros();
  Serial.printf("     GHN-------------------------------------------------------------------------------\n");
  cad=compruebaGHN();
  if(salvar) anadeFichero(FICHERO_ERRORES, cad);
  Serial.printf("     Wifi------------------------------------------------------------------------------\n");
  cad=compruebaWifi(); 
  if(salvar) anadeFichero(FICHERO_ERRORES, cad);
  Traza.mensaje("     WebServer-------------------------------------------------------------------------\n");
  cad=compruebaWebserver();
  if(salvar) anadeFichero(FICHERO_ERRORES, cad);
  Traza.mensaje("     MQTT------------------------------------------------------------------------------\n");
  cad=comrpuebaMQTT();
  if(salvar) anadeFichero(FICHERO_ERRORES, cad);
  Traza.mensaje("     E/S-------------------------------------------------------------------------------\n");
  cad=compruebaES();
  if(salvar) anadeFichero(FICHERO_ERRORES, cad);
  Traza.mensaje("     Secuenciador----------------------------------------------------------------------\n");
  cad=compruebaSecuenciador();
  if(salvar) anadeFichero(FICHERO_ERRORES, cad);
  Traza.mensaje("     MaquinaEstados--------------------------------------------------------------------\n");
  cad=compruebaMaquinaEstados();  
  if(salvar) anadeFichero(FICHERO_ERRORES, cad);
  Traza.mensaje("Fin Verificar errores configuracion----------------------------------------------------\n");

  return 1;
  }