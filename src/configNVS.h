/*********************************************************************/
/*                                                                   */
/*Funciones de gestion de la actualizacion OTA                       */
/*                                                                   */
/*********************************************************************/

/***************************** Defines *****************************/
#ifndef _NVS_
#define _NVS_
/***************************** Defines *****************************/

/***************************** Includes *****************************/
#include <Arduino.h>
/***************************** Includes *****************************/

/***************************** Tipos *****************************/
typedef struct valoresNVS{
  uint64_t deviceID;
  String nombreServicio;
  String nombremDNS;
  String SSID;
  String pass;
  String usuario;
  String contrasena;  
} configNVS_t;
/***************************** Tipos *****************************/

/***************************** Prototipos *****************************/
void inicializaNVS(void);
boolean leeConfigNVS(configNVS_t *c);
int escribeConfigNVS(configNVS_t c);
int escribeConfigNVSDefecto(void);

String calculaContrasena(configNVS_t c);

void escribeNVS_WiFi(configNVS_t c);
void escribeNVS_ID(configNVS_t c);

void resetNVS_WiFi(void);
void resetNVS_user(void);
void resetNVS_ID(void);
void resetNVS_Total(void);
/***************************** Prototipos *****************************/

/***************************** Variables *****************************/
extern configNVS_t configNVS;
/***************************** Variables *****************************/

#endif