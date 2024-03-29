#define SCREEN_WIDTH  240
#define SCREEN_HEIGHT 240

#include <Global.h>

#ifdef TIENE_PANTALLA
#include <TFT_eSPI.h> // Master copy here: https://github.com/Bodmer/TFT_eSPI
#include <SPI.h>

#include "NotoSansBold15.h"

void inicializaPantalla(void);
void actualizaPantalla(void);

void setTiempoBloqueo(void);
unsigned long getTiempoBloqueo(void);

//extern TFT_eSPI tft;  //definido en Pantalla.cpp
extern TFT_eSprite face; //definido en Pantalla.cpp
#endif