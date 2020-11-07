#include <Traza.h>

const char* ssid     = "BASE1";
const char* password = "11223344556677889900abcdef";

uint8_t vuelta=0;

void setup(void)
  {
  Serial.begin(115200);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) 
    {
    delay(500);
    Serial.print(".");
    }

  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());  

  Traza.begin(12,serie);
  Serial.printf("Traza iniciada con nivel %i y salida %i\n",Traza.getNivelDebug(),Traza.getMedio());
  }

void loop(void)
  {
  char nombre[]="Carmela";
  
  //Serial.printf("Hola %s, llevamos ---->%0.2i milisegundos\n", nombre, millis());
  //Traza.mensaje("vuelta %i | hola, escribiendo en IP %s, puerto %i\n", vuelta++, WiFi.localIP().toString().c_str(), PUERTO_DEBUG);
  Serial.printf("vuelta %i | hola, escribiendo en IP %s, puerto %i\n", vuelta++, WiFi.localIP().toString().c_str(), PUERTO_DEBUG);
  delay(100);
  }
