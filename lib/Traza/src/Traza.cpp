#include "Traza.h"

TrazaClass Traza;

void TrazaClass::mensaje(const char *str, ...)
{
  String cad="";
  
  va_list argv;
  va_start(argv, str);
  
  for(int i=0; str[i]!='\0';i++)
    {
    if(str[i]=='%')
      {    
      boolean flag=true;  
      while(flag)
        {
        flag=false;
        switch(str[++i])
          {
          case '%':
            cad += "%";
            break;
          case 'i':
          case 'd': 
            cad += String(va_arg(argv, int));
            break;
          case 'l': 
            cad += String(va_arg(argv, long));
            break;
          case 'f': 
            cad += String(va_arg(argv, double));
            break;
          case 'c': 
            cad += String((char)va_arg(argv, int));
            break;
          case 's':
            cad += String(va_arg(argv, char *));
            break;
          default: 
            flag=true;//para saltar los argumentos entre el % y la letra 
          }
        }
      }
    else cad += String(str[i]);
  }

  switch (_medio)
    {
    case serie:
      Serial.print(cad); 
      break;
    case ethernet:
      escribeTCP(cad);
      break;
    case fichero:
      break;
    default:
      break;
    }
}

void TrazaClass::escribeTCP(String mensaje)
  {
  if(!_cliente.connected()) _cliente = _servidor.available(); 
  
  if (_cliente) _cliente.print(mensaje.c_str());
  }

void TrazaClass::begin(uint8_t nivel=MAX_NIVELDEBUG, medios_t medio=serie)
  {
  _nivelDebug=nivel;
  _medio=medio;

  _servidor.begin();
  }

