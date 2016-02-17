// InternetDeLasCosas.cl
//
// Sketch Arduino que obtiene la temperatura y humedad desde un sensor DHTxx
// y la envia en forma inalambrica usando un modulo NRF2401L a otro Arduino
//
// Escrito por @joniuz (Jose Zorrilla) basado en el trabajo de @cuonic (Liam Jack)

// Libreria para controlar el modulo nRF24L01
#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"

// Declaracion de variables globales
// Configuracion de modulo de radio nRF24
// Pines CE y CSN
#define CE_PIN   9
#define CSN_PIN 10
// Canal de transmision o 'tuberia' Nota: Las "LL" al final de la constante indican que el tipo es "LongLong"
const uint64_t pipe = 0xE8E8F0F0E1LL; 
int sensordata[2]; 

// Inicializa modulo de radio nRF24L01
RF24 radio(CE_PIN,CSN_PIN);

// Inicializa el contador de paquetes
int packetCounter = 0;

// Funciones
// setupRadio: Configura el dispositivo de radio  
void setupRadio()
{
 // Inicializa la radio
 radio.begin();
 
 // Define el numero de reintentos
 radio.setRetries(15,15);
 
 // Define el canal de radio para broadcast (0 - 127)
 // Debe ser el mismo en el emisor y receptor
 radio.setChannel(30);
 
 // Define el bitrate (usar para tarjetas de bajo bitrate)
 radio.setDataRate(RF24_250KBPS);
 
 // Define el nivel de apmplificador del modulo de radio (RF24_PA_MIN para pruebas, RF24_PA_HIGH para distancias largas)
 radio.setPALevel(RF24_PA_MIN);
 
 // Habilita payloads dinamicos para los paquetes
 radio.enableDynamicPayloads();
 
 // Abre el canal para escucha
 radio.openReadingPipe(1, pipe);
 
 // Comienza escuchando paquetes
 radio.startListening();
}

void setup()
{
  // Inicializa la conexion serial
  Serial.begin(57600);
  
  // Configura el modulo de radio
  setupRadio();
}

void loop()
{ 
  // Chequea si la radio ha recibido datos
  if(radio.available()) {
    // Inicializa los datos en cero
    sensordata[0] = 0;
    sensordata[1] = 0;
    
    // Lee el paquete de datos
    if(!radio.read(sensordata, sizeof(sensordata))) {
      // El emisor no recibio el paquete ACK
      Serial.println("ACK no recibido por el emisor.");
    }
    
    // Imprime los datos recibidos
    Serial.print("Temperatura = ");
    Serial.print(sensordata[0]);
    Serial.print(" Humedad = ");      
    Serial.println(sensordata[1]);
    
    // Espera antes de seguir recibiendo paquetes
    delay(100);
  }
}
