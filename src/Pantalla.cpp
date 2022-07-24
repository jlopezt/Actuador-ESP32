#include <Global.h>
#include <Pantalla.h>
#include <reloj.h>
#include <entrada.h>
#include <Secuenciador.h>
#include <SNTP.h>

TFT_eSPI tft = TFT_eSPI();  // Invoke library, pins defined in User_Setup.h
TFT_eSprite face = TFT_eSprite(&tft);

unsigned long tiempoBloqueo=0;

void pantallaSecuenciador(void);

void inicializaPantalla(void){
  // Initialise the screen
  tft.init();

  // Ideally set orientation for good viewing angle range because
  // the anti-aliasing effectiveness varies with screen viewing angle
  // Usually this is when screen ribbon connector is at the bottom
  tft.setRotation(0);
  tft.fillScreen(TFT_BLACK);

  initReloj();
}

void actualizaPantalla(void){
    //if(millis()>tiempoBloqueo) updateReloj();

    pantallaSecuenciador();
}

/************************************** Pantalla Secuenciador **************************************/
#define SECUENCIADOR_W 200
#define SECUENCIADOR_H 200

#define ZONAS 4

#define SECUENCIADOR_X_POS (SCREEN_WIDTH-SECUENCIADOR_W)/2 //10
#define SECUENCIADOR_Y_POS (SCREEN_HEIGHT-SECUENCIADOR_H)/2 //10

#define LADO_IZQ_RECUADROS 100
#define TOP_ARRIBA_RECUADROS 26
#define ANCHO_LINEA_RECTANGULO 4
#define ANCHO_RECUADROS (SECUENCIADOR_W-LADO_IZQ_RECUADROS)
#define ARRIBA_RECUADROS 4 //separacion entre los recuadros
#define ALTO_RECUADROS (((SECUENCIADOR_H-TOP_ARRIBA_RECUADROS)/ZONAS)-ARRIBA_RECUADROS) //Ya esta descontado la separacion entre los recuadros

#define SECUENCIADOR_FG TFT_SKYBLUE
void pantallaSecuenciador(void){
  uint8_t zonas=secuenciador.getNumPlanes();

  // Create the clock face sprite
  //face.setColorDepth(8); // 8 bit will work, but reduces effectiveness of anti-aliasing
  face.createSprite(SECUENCIADOR_W, SECUENCIADOR_H);

  // Only 1 font used in the sprite, so can remain loaded
  face.loadFont(NotoSansBold15);

  // The face is completely redrawn - this can be done quickly
  face.fillSprite(TFT_BLACK);

  /*//recuadro exterior
  face.drawRect(0,0,SECUENCIADOR_W,SECUENCIADOR_H,TFT_WHITE);*/

  // The background colour will be read during the character rendering
  face.setTextColor(SECUENCIADOR_FG);    

  // Encabezado de la pantalla
  face.setTextDatum(ML_DATUM);
  face.drawString(nombre_dispositivo, 0, TOP_ARRIBA_RECUADROS/2);
  
  face.setTextDatum(MR_DATUM);
  face.drawString(String(hora()) + ":" + String(minuto()), SECUENCIADOR_W, TOP_ARRIBA_RECUADROS/2);

  //DATUM para los textos de las zonas
  face.setTextDatum(ML_DATUM);
  //Pinto el nombre de la salida y un cuadrado verde o rojo o un slide activo/desactivo
  for(uint8_t zona=0;zona<zonas;zona++){
    face.drawString(secuenciador.getNombrePlan(zona), 0, TOP_ARRIBA_RECUADROS+(ALTO_RECUADROS+ARRIBA_RECUADROS)*zona+(ALTO_RECUADROS/2));

    /*face.drawRect(LADO_IZQ_RECUADROS,TOP_ARRIBA_RECUADROS+(ALTO_RECUADROS+ARRIBA_RECUADROS)*zona,
                ANCHO_RECUADROS,ALTO_RECUADROS-ARRIBA_RECUADROS,TFT_WHITE);*/
    face.drawRect(LADO_IZQ_RECUADROS,TOP_ARRIBA_RECUADROS+(ALTO_RECUADROS+ARRIBA_RECUADROS)*zona,
                ANCHO_RECUADROS,ALTO_RECUADROS,TFT_WHITE);

    if(secuenciador.getEstadoPlan(zona)==ESTADO_ACTIVO) face.fillRect(LADO_IZQ_RECUADROS+ANCHO_LINEA_RECTANGULO+(ANCHO_RECUADROS-2*ANCHO_LINEA_RECTANGULO)/2,
                                TOP_ARRIBA_RECUADROS+(ALTO_RECUADROS+ARRIBA_RECUADROS)*zona+ANCHO_LINEA_RECTANGULO,
                                (ANCHO_RECUADROS-2*ANCHO_LINEA_RECTANGULO)/2,
                                (ALTO_RECUADROS-2*ANCHO_LINEA_RECTANGULO),
                                TFT_GREEN);

    else          face.fillRect(LADO_IZQ_RECUADROS+ANCHO_LINEA_RECTANGULO,
                                TOP_ARRIBA_RECUADROS+(ALTO_RECUADROS+ARRIBA_RECUADROS)*zona+ANCHO_LINEA_RECTANGULO,
                                (ANCHO_RECUADROS-2*ANCHO_LINEA_RECTANGULO)/2,
                                (ALTO_RECUADROS-2*ANCHO_LINEA_RECTANGULO),
                                TFT_LIGHTGREY);
    
  }

  //Pinto el sprite
  face.pushSprite(SECUENCIADOR_X_POS, SECUENCIADOR_Y_POS, TFT_TRANSPARENT);  
}