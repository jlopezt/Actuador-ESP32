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
  uint8_t nEntradas=entradasConfiguradas() ;
  
  cad += "configuradas " + String(nEntradas) + " entradas\n";
  for(uint8_t entrada=0;entrada<nEntradas;entrada++)
    {
    if(entradas[entrada].getConfigurada())
      {
      if(entradas[entrada].getNombre()=="") cad += "Entrada " + String(entrada) + " | Nombre no configurado\n";  
      if(entradas[entrada].getEstadoActivo()!=ESTADO_DESACTIVO && entradas[entrada].getEstadoFisico()!=ESTADO_ACTIVO) cad += "Entrada " + String(entrada) + " | estadoActivo valor no valido\n";
      if(entradas[entrada].getPin()<0 || entradas[entrada].getPin()>MAX_PINES_ESP32) cad += "Entrada " + String(entrada) + " | pin no valido\n";  
      if(entradas[entrada].getTipo()!="INPUT" && entradas[entrada].getTipo()!="INPUT_PULLUP") cad += "Entrada " + String(entrada) + " | tipo no valido\n";        
      }
    }

  //SALIDAS
  cad += "Salidas\n";
  uint8_t nSalidas=salidasConfiguradas() ;
  
  cad += "configuradas " + String(nSalidas) + " salidas\n";
  for(int8_t salida=0;salida<nSalidas;salida++)
    {
    if(salidas[salida].getConfigurada())
      {
      if(salidas[salida].getNombre()=="") cad += "Salida " + String(salida) + " | Nombre no configurado\n";
      if(salidas[salida].getModo()!=MODO_MANUAL && salidas[salida].getModo()!=MODO_SECUENCIADOR && salidas[salida].getModo()!=MODO_SEGUIMIENTO && salidas[salida].getModo()!=MODO_MAQUINA ) cad += "Salida " + String(salida) + " | Modo valor no valido\n";      
      if(salidas[salida].getPin()<0 || salidas[salida].getPin()>MAX_PINES_ESP32) cad += "Salida " + String(salida) + " | pin no valido\n";  
      if(salidas[salida].getAnchoPulso()<0) cad += "Salida " + String(salida) + " | ancho de pulso no valido\n";  
      if(salidas[salida].getControlador()>0 && salidas[salida].getModo()!=MODO_SECUENCIADOR && salidas[salida].getModo()!=MODO_SEGUIMIENTO) cad += "Salida " + String(salida) + " | controlador configurado en un modo incorrecto\n";  
      if(!entradas[salidas[salida].getControlador()].getConfigurada() && salidas[salida].getModo()==MODO_SEGUIMIENTO) cad += "Salida " + String(salida) + " | no hay un controlador configurado en un modo que lo requiere\n";  
      if(!planConfigurado(salidas[salida].getControlador()) && salidas[salida].getModo()==MODO_SECUENCIADOR) cad += "Salida " + String(salida) + " | no hay un controlador configurado en un modo que lo requiere\n";  
      if(salidas[salida].getModo()==MODO_SEGUIMIENTO && entradas[salidas[salida].getControlador()].getConfigurada()) cad += "Salida " + String(salida) + " | configurada en modo seguimiento pero el controlador configurado no es una entrada valida\n";  
      if(salidas[salida].getModo()==MODO_SECUENCIADOR && planConfigurado(salidas[salida].getControlador())!=CONFIGURADO) cad += "Salida " + String(salida) + " | configurada en modo secuenciador pero el controlador configurado no es un plan valido\n";  
      if(salidas[salida].getEstadoInicial()!=ESTADO_DESACTIVO && salidas[salida].getEstadoInicial()!=ESTADO_ACTIVO) cad += "Salida " + String(salida) + " | inicio no valido\n";
      }
    }

  return cad;
  }

String compruebaSecuenciador(void)
  {
  String cad = "";  
  uint8_t nPlanes=getNumPlanes();

  cad += "Secuenciador\n";
  cad += "configurados " + String(nPlanes) + " planes de secuenciador\n";
  for(uint8_t plan=0;plan<nPlanes;plan++)
    {
    if(planConfigurado(plan))
      {
      if(salidas[getSalidaPlan(plan)].getModo()!=MODO_SECUENCIADOR) cad += "Secuenciador " + String(plan) + " | la salida asociada al secuenciador no tiene el modo correcto\n";  
      }
    }
  return cad;
  }

String compruebaMaquinaEstados(void)
  {
  String cad = "Maquina de estados\n";
  
  //Entradas
  cad += "Entradas\n";
  uint8_t nEntradas=getNumEntradasME();
  
  cad += "configuradas " + String(nEntradas) + " entradas\n";
  for(uint8_t entrada=0;entrada<nEntradas;entrada++)
    {      
    if(getNumEntradaME(entrada)>MAX_ENTRADAS) cad += "Entrada " + String(entrada) + " | el numero de entrada es mayor a MAX_ENTRADAS\n";  
    if(entradas[getNumEntradaME(entrada)].getConfigurada()) cad += "Entrada " + String(entrada) + " | la entrada asociada no esta configurada\n";  
    }

  //Salidas
  cad += "Salidas\n";
  uint8_t nSalidas=getNumSalidasME() ;
  
  cad += "configuradas " + String(nSalidas) + " salidas\n";
  for(uint8_t salida=0;salida<nSalidas;salida++)
    {
    if(getNumSalidaME(salida)>MAX_SALIDAS) cad += "Salida " + String(salida) + " | el numero de salida es mayor a MAX_SALIDAS\n";  
    if(!salidas[salida].getConfigurada()) cad += "Salida " + String(salida) + " | salida no configurada\n";  
    else
      {
      if(salidas[getNumSalidaME(salida)].getModo()!=MODO_MAQUINA) cad += "Salida " + String(salida) + " | la salida asociada no tiene el modo correcto\n";  
      }
    }

  //Estados
  cad += "Estados\n";
  uint8_t nEstados=getNumEstados() ;
  
  cad += "configurados " + String(nEstados) + " estados\n";
  for(uint8_t estado=0;estado<nEstados;estado++)
    {
    if(getNombreEstado(estado)=="") cad += "Estado " + String(estado) + " | el nombre no esta configurado\n";  

    for(int8_t i=0;i<nSalidas;i++)
      {
      for(int8_t j=0;j<nSalidas;j++) 
        {
        if(getValorSalidaEstados(estado,i)!=ESTADO_DESACTIVO && getValorSalidaEstados(estado,i)!=ESTADO_ACTIVO) cad += "Estado " + String(estado) + " Salida " + String(j) + " | el valor configurado no es valido\n";  
        }
      }
    }

  //Trasnsiciones
  cad += "Trasnsiciones\n";
  uint8_t nTransiciones=getNumTransiciones() ;
  
  cad += "configuradas " + String(nTransiciones) + " trasnsiciones\n";
  for(uint8_t transi=0;transi<nTransiciones;transi++)
    {
    boolean correctoInicial=false;
    boolean correctoFinal=false;
    for(int8_t estado=0;estado<nEstados;estado++) 
      {
      if(getEstadoInicialTransicion(transi)==getIdEstados(estado)) correctoInicial=true;
      if(getEstadoFinalTransicion(transi)==getIdEstados(estado)) correctoFinal=true;
      }
    if(!correctoInicial) cad += "Transicion " + String(transi) + " | el estado inicial no es valido\n";  
    if(!correctoFinal) cad += "Transicion " + String(transi) + " | el estado final no es valido\n";  

    for(int8_t entrada=0;entrada<nEntradas;entrada++)
      {
      if(getValorEntradaTransicion(transi, entrada)!=ESTADO_DESACTIVO && getValorEntradaTransicion(transi, entrada)!=ESTADO_ACTIVO) cad += "Transicion " + String(transi) + " Entrada " + String(entrada) + " | el valor configurado no es valido\n";
      }
    }
  return cad;
  }
  
uint8_t compruebaConfiguracion(uint8_t nivelTraza)
  {
  boolean salvar=false;
  String cad="";

  Traza.mensaje("Init Verificar errores configuracion---------------------------------------------------\n");
  Traza.mensaje("     ficheros--------------------------------------------------------------------------\n");
  salvar=compruebaFicheros();
  Traza.mensaje("     GHN-------------------------------------------------------------------------------\n");
  cad=compruebaGHN();
  if(salvar) anadeFichero(FICHERO_ERRORES, cad);
  Traza.mensaje("     Wifi------------------------------------------------------------------------------\n");
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