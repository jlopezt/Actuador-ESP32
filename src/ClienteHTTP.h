#include <Global.h>
#define METODO_POST   1
#define METODO_GET    2
#define METODO_PUT    3
#define METODO_PATCH  4
#define METODO_DELETE 5

#define NO_ASOCIADO                     0
#define ASOCIADO_PENDIENTE_VALIDACION   1
#define ASOCIADO_VALIDADO               2

void inicializaPlataforma(boolean debug);
//int enviaDatosHTTP(String URL, String datos, uint8_t metodo);
int asocia(void);
int testHTTP();
boolean enviaFicheroConfig(String fichero);
boolean leeFicheroConfig(String servicio);
uint8_t getEstadoAsociacion(void);
boolean setEstadoAsociacion(uint8_t estado);