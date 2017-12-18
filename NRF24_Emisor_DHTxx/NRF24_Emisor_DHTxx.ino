// InternetDeLasCosas.cl
//
// Sketch Arduino que obtiene la temperatura y humedad desde un sensor DHTxx
// y la envia en forma inalambrica usando un modulo NRF2401L a otro Arduino
//
// Escrito por @joniuz (Jose Zorrilla) basado en el trabajo de @cuonic (Liam Jack)

// Librerias para controlar el tiempo de inactividad y ahorro de energia
#include <avr/sleep.h>
#include <avr/power.h>

// Libreria del timer Watchdog
#include <avr/wdt.h>

// Libreria para controlar el modulo nRF24L01
#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"


// Libreria para controlar Sensores DHTxx
#include "DHT.h"

// Declaracion de variables globales
volatile int f_wdt = 1;
int counter = 0;
int messageCounter = 0;
float sensordata[3]; 

// Configuracion de Sensor DHTxx
// Pin del Arduino al cual esta conectado el pin 2 del sensor
#define DHTPIN 2     

// Descomentar segun el tipo de sensor DHTxx usado
#define DHTTYPE DHT11     // DHT11 
//#define DHTTYPE DHT22   // DHT22  (AM2302)
//#define DHTTYPE DHT21   // DHT21 (AM2301)

// Configuracion de modulo de radio nRF24
// Pines CE y CSN
#define CE_PIN   9
#define CSN_PIN 10
// Canal de transmision o 'tuberia' Nota: Las "LL" al final de la constante indican que el tipo es "LongLong"
const uint64_t pipe = 0xE8E8F0F0E1LL; 

// Inicializa modulo de radio nRF24L01
RF24 radio(CE_PIN,CSN_PIN);

// Inicializa el sensor DHTxx
DHT dht(DHTPIN, DHTTYPE);

// Declaracion de Funciones

// counterHandler: Controla el tiempo que debe permanecer dormido el controlador
void counterHandler()
{
  // Incrementa el contador 
  counter++;

  // If que controla el tiempo que debe permanecer dormido el controlador
  // 1  : Para pruebas
  // 75 : 10 minutos (75 * 8 = 600 segundos = 10 minutos)
  if(counter == 1) {    
    // Reinicia el contador cuando se cumple el tiempo
    counter = 0;
    
    // Enciende el dispositivo
    power_all_enable();
    
    // Enciende la radio
    radio.powerUp();
    
    // Espera que la radio se inicie
    delay(2);
    
  } else {
    // Si aun no se cumple el tiempo sigue durmiendo
    enterSleep();
  } 
}

// enterSleep: Apaga radio y mantiene al controlador en ahorro de energia
void enterSleep()
{
  // Inicia el timer watchdog 
  f_wdt = 0;
  
  // Apaga la radio
  radio.powerDown();
  
  // Pone en ahorro de energia al controlador
  sleep_enable();
  sleep_mode();
  
  // Despierta al controlador
  sleep_disable();
  
  // Incrementa el contador de interrupcion
  counterHandler();
}

ISR(WDT_vect)
{
  // Detiene el timer watchdog
  f_wdt = 1;
}

// setupWDT: Configura el timer watchdog
void setupWDT()
{
  // Configura el timer Watchdog para una interrupcion cada 8 segundos
  
  MCUSR &= ~(1<<WDRF);
  WDTCSR |= (1<<WDCE) | (1<<WDE);
  WDTCSR = 1<<WDP0 | 1<<WDP3;
  WDTCSR |= _BV(WDIE);
}

// setupRadio: Configura el dispositivo de radio 
void setupRadio()
{
 // Inicializa la radio
 radio.begin();
 
 // Define el numero de reintentos
 radio.setRetries(15,15);
 
 // Define el canal de radio para broadcast (0 - 127)
 radio.setChannel(30);
 
 // Define el bitrate (usar para tarjetas de bajo bitrate)
 radio.setDataRate(RF24_250KBPS);
 
 // Define el nivel de apmplificador del modulo de radio (RF24_PA_MIN para pruebas, RF24_PA_HIGH para distancias largas)
 radio.setPALevel(RF24_PA_MIN);
 
 // Habilita payloads dinamicos para los paquetes
 radio.enableDynamicPayloads();
}

void setupSensor()
{
  // Inicializa el sensor
  dht.begin();
  // Espera dos segundos antes de realizar la primera medición.
  delay(2000);
  
}

// setup: Funcion de inicializacion de Arduino
void setup()
{ 
  // Inicializa la conexion serial
  Serial.begin(9600);
  Serial.println("Inicializa puerto serial");
  
  // Deshabilita la detection Brown out para evitar consumo de energia
  sleep_bod_disable();
  
  // Configura el modo Sleep
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  
  // Configura el timer Watchdog
  setupWDT();
  
  // Configura el sensor
  setupSensor();
  
  // Configura el modulo de radio
  setupRadio();
}
// loop: Funcion loop de Arduino
void loop()
{
  // Inicializa los datos del sensor
  sensordata[0] = 0;
  sensordata[1] = 0;
  sensordata[2] = 0;

  // Asigna un numero de serie al mensaje
  sensordata[0] = messageCounter;
  
  // Obtiene los datos del sensor
  // Obtiene la Temperatura en Celsius
  sensordata[1] = dht.readTemperature();
  // Obtiene la Humedad
  sensordata[2] = dht.readHumidity();

  
  // Control de errores, valida que se obtuvieron valores para los datos medidos
  if (isnan(sensordata[0]) || isnan(sensordata[1])) {
    Serial.println("Falla al leer el sensor DHT!");
    return;
  }
  
  // Abre un canal de escritura utilizando la radio
  radio.openWritingPipe(pipe);
  
  // Imprime los datos recibidos para validacion
  Serial.print("PacketCounter = ");
  Serial.print(sensordata[0]);
  Serial.print(" Temperatura = ");
  Serial.print(sensordata[1]);
  Serial.print(" Humedad = ");      
  Serial.println(sensordata[2]);

  // Escribe los datos en el canal
  radio.write(sensordata, sizeof(sensordata));
  messageCounter++;
  
  // Entra en modo dormir (Ahorro de energia)
  enterSleep();
}
