#include <Arduino.h>   // Libreria base de Arduino: pinMode, digitalWrite, millis(), Serial, etc.
#include "config.h"    // Pines y constantes del proyecto (todo centralizado aqui)
#include "sensors.h"   // Funciones para leer los sensores de distancia HC-SR04
#include "alerts.h"    // Funciones para controlar el buzzer de alarma
#include "display.h"   // Funciones para controlar los LEDs (registro de desplazamiento 74HC595)

// Guarda en que momento (milisegundos desde que arranco el Arduino) se leyeron
// los sensores por ultima vez. Junto con millis() en loop(), permite esperar un
// intervalo sin usar delay() (que congelaria todo el programa).
unsigned long ULTIMA_LECTURA_MILLIS = 0;

// Indica si en este momento hay una alerta activa (algun sensor detecto algo cerca).
// Es una variable global de estado, por eso va en MAYUSCULAS (convencion del proyecto).
bool ALERTA_ACTIVA = false;

// setup() se ejecuta una sola vez, apenas el Arduino arranca o se reinicia.
// Aqui se prepara todo lo necesario antes de empezar a repetir loop().
void setup() {
  Serial.begin(9600); // Abre el puerto serial (para ver mensajes de depuracion en la PC, a 9600 baudios)

  // Prepara cada sensor ultrasonico: configura sus pines TRIG (emisor) y ECHO (receptor)
  inicializarSensor(PIN_TRIG_1, PIN_ECHO_1);
  inicializarSensor(PIN_TRIG_2, PIN_ECHO_2);

  inicializarAlerta(PIN_BUZZER); // Prepara el pin del buzzer
  inicializarDisplay(PIN_CLOCK_REGISTRO, PIN_LATCH_REGISTRO, PIN_DATOS_REGISTRO); // Prepara el registro 74HC595 (LEDs)
}

// loop() se ejecuta una y otra vez, sin parar, mientras el Arduino este encendido.
// Aqui va la logica que se repite: leer sensores, decidir si hay alerta, y mostrarla en LEDs/buzzer.
void loop() {
  unsigned long ahora = millis(); // Milisegundos transcurridos desde que arranco el Arduino

  // En vez de delay() (que bloquearia todo el programa), comparamos tiempos con millis().
  // Asi el Arduino sigue "vivo" y solo entra aqui a leer los sensores cada INTERVALO_LECTURA_MS.
  if (ahora - ULTIMA_LECTURA_MILLIS >= INTERVALO_LECTURA_MS) {
    ULTIMA_LECTURA_MILLIS = ahora; // Guarda el momento de esta lectura, para la proxima comparacion

    // Mide la distancia (en centimetros) que "ve" cada sensor ultrasonico
    long distancia1 = medirDistanciaCm(PIN_TRIG_1, PIN_ECHO_1);
    long distancia2 = medirDistanciaCm(PIN_TRIG_2, PIN_ECHO_2);

    // Un sensor "detecta algo" si la lectura es valida (>= 0, hubo eco) y esta
    // dentro del umbral configurado (por defecto, 20 cm o menos)
    bool deteccion1 = (distancia1 >= 0) && (distancia1 <= UMBRAL_DETECCION_CM);
    bool deteccion2 = (distancia2 >= 0) && (distancia2 <= UMBRAL_DETECCION_CM);

    // Si CUALQUIERA de los dos sensores detecto algo cerca, se activa la alerta general
    ALERTA_ACTIVA = deteccion1 || deteccion2;

    // Refleja el estado de la alerta en los LEDs y en el sonido del buzzer
    actualizarLeds(PIN_CLOCK_REGISTRO, PIN_LATCH_REGISTRO, PIN_DATOS_REGISTRO, ALERTA_ACTIVA);
    actualizarAlerta(PIN_BUZZER, ALERTA_ACTIVA);
  }
}
