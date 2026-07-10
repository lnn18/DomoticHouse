#include <Arduino.h>   // Libreria base de Arduino: pinMode, digitalWrite, millis(), Serial, etc.
#include "config.h"    // Pines y constantes del proyecto (todo centralizado aqui)
#include "sensors.h"   // Funciones para leer los sensores (HC-SR04, PIR, presion de cama)
#include "alerts.h"    // Funcion para controlar el buzzer compartido
#include "display.h"   // Funciones para controlar los LEDs de proximidad (74HC595)
#include "lighting.h"  // Funciones para controlar la luz del pasillo y la luz de alarma de cama

// Guarda en que momento (milisegundos desde que arranco el Arduino) se leyeron
// los sensores de proximidad por ultima vez. Junto con millis() en loop(), permite
// esperar un intervalo sin usar delay() (que congelaria todo el programa).
unsigned long ULTIMA_LECTURA_MILLIS = 0;

// Indica si en este momento hay una alerta de proximidad activa (algun HC-SR04 detecto algo cerca).
bool ALERTA_ACTIVA = false;

// Luz del pasillo: se enciende con movimiento y se apaga sola tras un rato sin detectar nada.
bool LUZ_PASILLO_ENCENDIDA = false;
unsigned long ULTIMO_MOVIMIENTO_MILLIS = 0;

// Sensor de presion en la cama: mide cuanto tiempo lleva la persona acostada sin levantarse.
// EN_CAMA no se cancela ante cualquier bajon de presion: solo si la ausencia dura mas de
// TOLERANCIA_AUSENCIA_CAMA_MS se confirma que la persona se levanto (ver actualizarAlarmaCama).
bool EN_CAMA = false;
unsigned long INICIO_EN_CAMA_MILLIS = 0;
bool HUBO_AUSENCIA_CAMA = false;
unsigned long INICIO_AUSENCIA_CAMA_MILLIS = 0;
bool ALARMA_CAMA_ACTIVA = false;

// setup() se ejecuta una sola vez, apenas el Arduino arranca o se reinicia.
// Aqui se prepara todo lo necesario antes de empezar a repetir loop().
void setup() {
  Serial.begin(9600); // Abre el puerto serial (para ver mensajes de depuracion en la PC, a 9600 baudios)

  // Se deja registro de cada arranque: si el Arduino se reinicia (corte de luz, etc.)
  // mientras alguien lleva horas en la cama, el conteo de TIEMPO_MAX_EN_CAMA_MS se
  // pierde y arranca de nuevo desde 0 sin otro aviso. Este mensaje al menos permite
  // notar, revisando el monitor serial, que hubo un reinicio inesperado.
  Serial.println(F("Arduino (re)iniciado"));

  // Prepara cada sensor ultrasonico: configura sus pines TRIG (emisor) y ECHO (receptor)
  inicializarSensor(PIN_TRIG_1, PIN_ECHO_1);
  inicializarSensor(PIN_TRIG_2, PIN_ECHO_2);

  inicializarAlerta(PIN_BUZZER); // Prepara el pin del buzzer
  inicializarDisplay(PIN_CLOCK_REGISTRO, PIN_LATCH_REGISTRO, PIN_DATOS_REGISTRO); // Prepara el registro 74HC595 (LEDs)

  inicializarSensorPir(PIN_PIR_PASILLO);
  inicializarLuz(PIN_LUZ_PASILLO);

  inicializarSensorPresionCama(PIN_FSR_CAMA);
  inicializarLuz(PIN_LUZ_ALARMA_CAMA);
}

// Lee los 2 HC-SR04 y actualiza los LEDs de proximidad. Se llama cada INTERVALO_LECTURA_MS.
void actualizarProximidad(unsigned long ahora) {
  if (ahora - ULTIMA_LECTURA_MILLIS < INTERVALO_LECTURA_MS) {
    return;
  }
  ULTIMA_LECTURA_MILLIS = ahora;

  long distancia1 = medirDistanciaCm(PIN_TRIG_1, PIN_ECHO_1);
  long distancia2 = medirDistanciaCm(PIN_TRIG_2, PIN_ECHO_2);

  bool deteccion1 = (distancia1 >= 0) && (distancia1 <= UMBRAL_DETECCION_CM);
  bool deteccion2 = (distancia2 >= 0) && (distancia2 <= UMBRAL_DETECCION_CM);

  ALERTA_ACTIVA = deteccion1 || deteccion2;
  actualizarLeds(PIN_CLOCK_REGISTRO, PIN_LATCH_REGISTRO, PIN_DATOS_REGISTRO, ALERTA_ACTIVA);
}

// Enciende la luz del pasillo con movimiento y la apaga sola tras DURACION_LUZ_PASILLO_MS sin detectar nada.
void actualizarLuzPasillo(unsigned long ahora) {
  if (detectaMovimiento(PIN_PIR_PASILLO)) {
    ULTIMO_MOVIMIENTO_MILLIS = ahora;
    if (!LUZ_PASILLO_ENCENDIDA) {
      encenderLuz(PIN_LUZ_PASILLO);
      LUZ_PASILLO_ENCENDIDA = true;
    }
  } else if (LUZ_PASILLO_ENCENDIDA && (ahora - ULTIMO_MOVIMIENTO_MILLIS >= DURACION_LUZ_PASILLO_MS)) {
    apagarLuz(PIN_LUZ_PASILLO);
    LUZ_PASILLO_ENCENDIDA = false;
  }
}

// Sigue cuanto tiempo lleva la persona acostada y dispara la alarma (luz) tras TIEMPO_MAX_EN_CAMA_MS.
// Una ausencia de presion corta (ej. un giro en la cama) no reinicia el conteo: solo si la
// ausencia se sostiene mas de TOLERANCIA_AUSENCIA_CAMA_MS se confirma que la persona se levanto.
void actualizarAlarmaCama(unsigned long ahora) {
  bool presionDetectada = hayPersonaEnCama(PIN_FSR_CAMA, UMBRAL_PRESION_CAMA_ADC);

  if (presionDetectada) {
    if (!EN_CAMA) {
      EN_CAMA = true;
      INICIO_EN_CAMA_MILLIS = ahora;
    }
    HUBO_AUSENCIA_CAMA = false;
  } else if (EN_CAMA) {
    if (!HUBO_AUSENCIA_CAMA) {
      HUBO_AUSENCIA_CAMA = true;
      INICIO_AUSENCIA_CAMA_MILLIS = ahora;
    } else if (ahora - INICIO_AUSENCIA_CAMA_MILLIS >= TOLERANCIA_AUSENCIA_CAMA_MS) {
      // La ausencia ya duro lo suficiente: se confirma que la persona se levanto.
      EN_CAMA = false;
      HUBO_AUSENCIA_CAMA = false;
      ALARMA_CAMA_ACTIVA = false;
    }
  }

  if (EN_CAMA && (ahora - INICIO_EN_CAMA_MILLIS >= TIEMPO_MAX_EN_CAMA_MS)) {
    ALARMA_CAMA_ACTIVA = true;
  }

  if (ALARMA_CAMA_ACTIVA) {
    encenderLuz(PIN_LUZ_ALARMA_CAMA);
  } else {
    apagarLuz(PIN_LUZ_ALARMA_CAMA);
  }
}

// El buzzer es un unico recurso compartido: la alarma de cama tiene prioridad sobre
// el aviso de proximidad, asi que aqui se decide una sola frecuencia por vuelta de loop().
void actualizarBuzzerCompartido() {
  unsigned int frecuenciaBuzzer = 0;
  if (ALARMA_CAMA_ACTIVA) {
    frecuenciaBuzzer = FRECUENCIA_ALARMA_CAMA_HZ;
  } else if (ALERTA_ACTIVA) {
    frecuenciaBuzzer = FRECUENCIA_ALARMA_PROXIMIDAD_HZ;
  }
  actualizarBuzzer(PIN_BUZZER, frecuenciaBuzzer);
}

// loop() se ejecuta una y otra vez, sin parar, mientras el Arduino este encendido.
// Se mantiene corto a proposito: cada tarea vive en su propia funcion arriba.
void loop() {
  unsigned long ahora = millis(); // Milisegundos transcurridos desde que arranco el Arduino

  actualizarProximidad(ahora);
  actualizarLuzPasillo(ahora);
  actualizarAlarmaCama(ahora);
  actualizarBuzzerCompartido();
}
