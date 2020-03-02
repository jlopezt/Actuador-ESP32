/***********************************************************
 *    Modulo de gestion de SNTP
 * 
 *    https://github.com/espressif/arduino-esp32/issues/680
 ***********************************************************/
#define MES_CAMBIO_HORARIO_UP   3 //marzo
#define MES_CAMBIO_HORARIO_DOWN 10 //octubre
#define DOMINGO 7

#include <TimeLib.h>  // download from: http://www.arduino.cc/playground/Code/Time
#include <time.h>

String Semana[7]={"Lunes","Martes","Miercoles","Jueves","Viernes","Sabado","Domingo"};

const char* NTP_SERVER = "pool.ntp.org";//"ntp.mydomain.com";
//const char* TZ_INFO    = "EST5EDT4,M3.2.0/02:00:00,M11.1.0/02:00:00";
const char* TZ_INFO    = "CET-1CEST-2,M3.5.0/02:00:00,M10.5.0/02:00:00";
/*****************************************************
The format of TZ values recognized by tzset() is as follows:
stdoffset[dst[offset][,rule]]copy to clipboard

std and dst
Indicate no fewer than three, but not more than TZNAME_MAX, bytes that are the designation for the standard (std) and daylight savings (dst) time zones. If more than TZNAME_MAX 
bytes are specified for std or dst, tzset() truncates to TZNAME_MAX bytes. Only std is required; if dst is missing, daylight savings time does not apply in this locale. Uppercase 
and lowercase letters are explicitly allowed. Any character except a leading colon (:) or digits, the comma (,), the minus (-), the plus (+), and the NULL character are permitted 
to appear in these fields. The meaning of these letters and characters is unspecified.

offset
Indicates the value that must be added to the local time to arrive at Coordinated Universal Time (UTC). offset has the form: hh[:mm[:ss]] The minutes (mm) and seconds (ss) are 
optional. The hour (hh) is required and may be a single digit. offset following std is required. If no offset follows dst, daylight savings time is assumed to be 1 hour ahead of 
standard time. One or more digits may be used; the value is always interpreted as a decimal number. The hour must be between 0 and 24; minutes and seconds, if present, between 0 
and 59. The difference between standard time offset and daylight savings time offset must be greater than or equal to 0, but the difference may not be greater than 24 hours. Use 
of values outside of these ranges causes tzset() to use the LC_TOD category rather than the TZ environment variable for time conversion information. An offset preceded by a minus 
(-) indicates a time zone east of the Prime Meridian. A plus (+) preceding offset is optional and indicates the time zone west of the Prime Meridian.

rule
Indicates when to change to and back from daylight savings time. The rule has the form: date[/time],date[/time]
The first date describes when the change from standard to daylight savings time occurs and the second date describes when the change back happens. Each time field describes when, 
in current local time, the change to the other time is made.

The format of date must be one of the following:
Jn: The Julian day n (1≤n≤365). Leap days are not counted. That is, in all years—including leap years—February 28 is day 59 and March 1 is day 60. It is impossible to explicitly 
refer to the occasional February 29.
n: The zero-based Julian day (0≤n≤365). Leap days are counted, and it is possible to refer to February 29.
Mm.n.d
The dth day (0≤d≤6) of week n of month m of the year (1≤n≤5, and 1≤m≤12, where week 5 means “ the last d day in month m,” which may occur in either the fourth or the fifth 
week). Week 1 is the first week in which the dth day occurs. Day zero is Sunday.
The time has the same format as offset except that no leading sign, minus (-) or plus (+), is allowed. The default, if time is not given, is 02:00:00.

If dst is specified and rule is not specified by TZ or in LC_TOD category, the default for the daylight savings time start date is M4.1.0 and for the daylight savings time end 
date is M10.5.0.
******************************************************/


struct tm timeinfo;
/***********************
Member  Type  Meaning Range
tm_sec  int seconds after the minute  0-61*
tm_min  int minutes after the hour  0-59
tm_hour int hours since midnight  0-23
tm_mday int day of the month  1-31
tm_mon  int months since January  0-11
tm_year int years since 1900  
tm_wday int days since Sunday 0-6
tm_yday int days since January 1  0-365
tm_isdst int Daylight Saving Time flag 
***********************/

/****************************************************/
/*                                                  */
/*    Inicializa el modulo de SNTP                  */ 
/*                                                  */
/****************************************************/
void inicializaReloj(void)
  {
  configTzTime(TZ_INFO, NTP_SERVER);
  
  if (getLocalTime(&timeinfo, 10000))  // wait up to 10sec to sync
    {
    Serial.println(&timeinfo, "Time set: %B %d %Y %H:%M:%S (%A)");
    } 
  else 
    {
    Serial.println("No se pudo inicializar el reloj por SNTP");
    }    
  }
  
/*******************************************************************/
/* Cambio horario:                                                 */
/* En España, esta medida se lleva adoptando desde 1974, aunque    */
/* la última regulación a la que nos hemos adaptado ha llegado     */
/* de la mano de la directiva Europea 2000/84, que entre otras     */
/* cosas unifica los días en los que se producen los cambios de    */
/* hora en todos los países de la Unión Europea, siendo estos      */
/* el último Domingo de Marzo y Octubre, respectivamente           */
/*                                                                 */
/* La funcion devuelve las horas que hay que sumar al reloj oficial*/
/*******************************************************************/
int8_t cambioHorario(void) 
  {
  getLocalTime(&timeinfo);
  return timeinfo.tm_isdst;
  }

int8_t diaSemana(void) 
  {
  getLocalTime(&timeinfo);
  if (timeinfo.tm_wday==0) return 7;
  return timeinfo.tm_wday;  
  }

/***********************************/
/*        Solo para ordenes        */
/***********************************/
/*String getHora(void)
  {
  char cad[6]="00:00";
  getLocalTime(&timeinfo);  
  sprintf(cad,"%2.0i:%2.0i",timeinfo.tm_hour,timeinfo.tm_min);    
  return String(cad);  
  }
*/
  
/***********************************/
/*        Solo para ordenes        */
/***********************************/
void imprimeDatosReloj(void)
  {
  getLocalTime(&timeinfo);
  Serial.printf("Fecha: %02i-%02i-%02i Hora: %02i:%02i:%02i\n",timeinfo.tm_mday,timeinfo.tm_mon+1,timeinfo.tm_year+1900,timeinfo.tm_hour,timeinfo.tm_min,timeinfo.tm_sec);    
  Serial.printf("Día de la semana (Hoy): %s\n", Semana[timeinfo.tm_wday].c_str());
  }

/***************************************************************/
/*                                                             */
/*  Retorna si es por la tarde en formato booleano             */
/*                                                             */
/***************************************************************/
bool relojPM() 
  {
  getLocalTime(&timeinfo);
  if(timeinfo.tm_hour>11) return true;
  return false;
  }  

/***************************************************************/
/*                                                             */
/*  Devuleve la hora del sistema                               */
/*                                                             */
/***************************************************************/
int hora() 
  {
  getLocalTime(&timeinfo); 
  return (timeinfo.tm_hour);
  } 

/***************************************************************/
/*                                                             */
/*  Devuleve el minuto del sistema                             */
/*                                                             */
/***************************************************************/
int minuto() 
  {
  getLocalTime(&timeinfo); 
  return (timeinfo.tm_min);
  } 

/***************************************************************/
/*                                                             */
/*  Devuleve el segundo del sistema                            */
/*                                                             */
/***************************************************************/
int segundo() 
  {
  getLocalTime(&timeinfo); 
  return (timeinfo.tm_sec);
  } 
  
/***************************************************************/
/*                                                             */
/*  Devuleve el dia del sistema                                */
/*                                                             */
/***************************************************************/
int dia() 
  {
  getLocalTime(&timeinfo); 
  return (timeinfo.tm_mday);
  } 

/***************************************************************/
/*                                                             */
/*  Devuleve el mes del sistema                                */
/*                                                             */
/***************************************************************/
int mes() 
  {
  getLocalTime(&timeinfo); 
  return (timeinfo.tm_mon+1);
  } 

/***************************************************************/
/*                                                             */
/*  Devuleve el año del sistema                                */
/*                                                             */
/***************************************************************/
int anno() 
  {
  getLocalTime(&timeinfo); 
  return (timeinfo.tm_year+1900);
  } 

  
/***************************************************************/
/*                                                             */
/*  Genera una cadena con la hora en formato HH:MM             */
/*                                                             */
/***************************************************************/
String getHora(void)
  {
  String cad="";  
  char ss[3];

  getLocalTime(&timeinfo);
  
  dtostrf(timeinfo.tm_hour,2,0,ss);ss[2]=0;
  if(timeinfo.tm_hour<10) ss[0]='0';
  cad=String(ss);

  dtostrf(timeinfo.tm_min,2,0,ss);ss[2]=0;
  if(timeinfo.tm_min<10)ss[0]='0';
  cad += ":" + String(ss);

  return cad;    
  }

/***************************************************************/
/*                                                             */
/*  Genera un acadena con la fecha en formato DD/MM/AAAA       */
/*                                                             */
/***************************************************************/
String getFecha(void)
  {
  String cad=""; 
  char ss[5];

  getLocalTime(&timeinfo);
  
  dtostrf(timeinfo.tm_mday,2,0,ss);ss[2]=0;
  if(timeinfo.tm_mday<10) ss[0]='0';
  cad=String(ss);

  dtostrf(timeinfo.tm_mon+1,2,0,ss);ss[2]=0;
  if(timeinfo.tm_mon+1<10)ss[0]='0';
  cad += "/" + String(ss);

  dtostrf(timeinfo.tm_year+1900,4,0,ss);ss[4]=0;
  cad += "/" + String(ss); 
  
  return cad;   
  }  
