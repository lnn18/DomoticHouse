#include <Arduino.h>   // Libreria base de Arduino: pinMode, digitalWrite, millis(), Serial, etc.
#include "config.h"    // Pines y constantes del proyecto (todo centralizado aqui)
#include "sensors.h"   // Funciones para leer los sensores (HC-SR04 x3, fotocelda, presion de cama)
#include "alerts.h"    // Funciones para el buzzer compartido y el boton de panico
#include "display.h"   // Funcion para controlar el LED de panico
#include "lighting.h"  // Funciones para controlar la luz del pasillo
#include "access.h"    // Control de acceso a la puerta: RC522 + servo

// Guarda en que momento (milisegundos desde que arranco el Arduino) se leyeron
// los sensores de proximidad por ultima vez. Junto con millis() en loop(), permite
// esperar un intervalo sin usar delay() (que congelaria todo el programa).
unsigned long ULTIMA_LECTURA_MILLIS = 0;

// DEBUG TEMPORAL: controla cada cuanto se imprimen los logs de depuracion (separado de
// INTERVALO_LECTURA_MS, que regula cada cuanto se LEEN los sensores de proximidad). Sin
// esto, los logs de pasillo y fotocelda se imprimirian miles de veces por segundo (una
// vez por vuelta de loop(), que no tiene ningun delay), haciendo el monitor serial ilegible.
unsigned long ULTIMO_LOG_MILLIS = 0;
constexpr unsigned long INTERVALO_LOG_MS = 500UL;

// Indica si en este momento hay una alerta de proximidad activa (algun HC-SR04 detecto algo cerca).
bool ALERTA_ACTIVA = false;

// DEBUG TEMPORAL: ultima distancia leida de cada sensor, guardada aqui solo para poder
// imprimirla en imprimirLogsDebug() sin tener que leer los sensores dos veces.
long DEBUG_DISTANCIA_1 = -1;
long DEBUG_DISTANCIA_2 = -1;
long DEBUG_DISTANCIA_PASILLO = -1;

// Luz del pasillo: se enciende cuando el SRF-05 de pasillo detecta algo cerca, y se
// apaga sola tras un rato sin detectar nada.
bool LUZ_PASILLO_ENCENDIDA = false;
unsigned long ULTIMA_DETECCION_PASILLO_MILLIS = 0;

// --- Alarma de cama: DESACTIVADA TEMPORALMENTE ---
// Toda la logica que usa estas variables (actualizarAlarmaCama) esta comentada mas abajo.
// bool EN_CAMA = false;
// unsigned long INICIO_EN_CAMA_MILLIS = 0;
// bool HUBO_AUSENCIA_CAMA = false;
// unsigned long INICIO_AUSENCIA_CAMA_MILLIS = 0;
// bool ALARMA_CAMA_ACTIVA = false;

// Boton de panico: es la alarma de mayor prioridad de todo el sistema.
bool ALARMA_PANICO_ACTIVA = false;

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
  inicializarSensor(PIN_TRIG_PASILLO, PIN_ECHO_PASILLO); // 3er SRF-05, usado para presencia en el pasillo

  inicializarAlerta(PIN_BUZZER); // Prepara el pin del buzzer
  inicializarDisplay(PIN_LED_PANICO); // Prepara el LED de panico (control directo)

  inicializarLuz(PIN_LUZ_PASILLO);

  inicializarFotocelda(PIN_FOTOCELDA);

  // inicializarSensorPresionCama(PIN_FSR_CAMA); // Alarma de cama desactivada temporalmente
  // inicializarLuz(PIN_LUZ_ALARMA_CAMA);

  inicializarBotonPanico(PIN_BOTON_PANICO);

  inicializarAcceso(PIN_RFID_SS, PIN_RFID_RST, PIN_SERVO_PUERTA);
}

// Lee los 2 sensores ultrasonicos de proximidad y actualiza ALERTA_ACTIVA. Se llama cada
// INTERVALO_LECTURA_MS. (El 3er SRF-05, el de pasillo, se lee aparte en actualizarLuzPasillo).
// De dia (esDeNoche == false) la alarma de proximidad no se activa, igual criterio que
// la luz de pasillo (ver actualizarLuzPasillo): ambas dependen de la fotocelda.
void actualizarProximidad(unsigned long ahora, bool esDeNoche) {
  if (ahora - ULTIMA_LECTURA_MILLIS < INTERVALO_LECTURA_MS) {
    return;
  }
  ULTIMA_LECTURA_MILLIS = ahora;

  long distancia1 = medirDistanciaCm(PIN_TRIG_1, PIN_ECHO_1);
  long distancia2 = medirDistanciaCm(PIN_TRIG_2, PIN_ECHO_2);
  DEBUG_DISTANCIA_1 = distancia1; // DEBUG TEMPORAL: guardado para imprimirDebug()
  DEBUG_DISTANCIA_2 = distancia2;

  bool deteccion1 = (distancia1 >= 0) && (distancia1 <= UMBRAL_DETECCION_CM);
  bool deteccion2 = (distancia2 >= 0) && (distancia2 <= UMBRAL_DETECCION_CM);

  ALERTA_ACTIVA = esDeNoche && (deteccion1 || deteccion2);
}

// Enciende la luz del pasillo cuando el SRF-05 de pasillo detecta algo a menos de
// UMBRAL_PASILLO_CM, y la apaga sola tras DURACION_LUZ_PASILLO_MS sin detectar nada.
// De dia (esDeNoche == false) no enciende, aunque haya alguien cerca.
void actualizarLuzPasillo(unsigned long ahora, bool esDeNoche) {
  long distanciaPasillo = medirDistanciaCm(PIN_TRIG_PASILLO, PIN_ECHO_PASILLO);
  DEBUG_DISTANCIA_PASILLO = distanciaPasillo; // DEBUG TEMPORAL: guardado para imprimirDebug()
  bool hayAlguienCerca = (distanciaPasillo >= 0) && (distanciaPasillo <= UMBRAL_PASILLO_CM);

  if (esDeNoche && hayAlguienCerca) {
    ULTIMA_DETECCION_PASILLO_MILLIS = ahora;
    if (!LUZ_PASILLO_ENCENDIDA) {
      encenderLuz(PIN_LUZ_PASILLO);
      LUZ_PASILLO_ENCENDIDA = true;
    }
  } else if (LUZ_PASILLO_ENCENDIDA && (!esDeNoche || (ahora - ULTIMA_DETECCION_PASILLO_MILLIS >= DURACION_LUZ_PASILLO_MS))) {
    apagarLuz(PIN_LUZ_PASILLO);
    LUZ_PASILLO_ENCENDIDA = false;
  }
}

// --- Alarma de cama: DESACTIVADA TEMPORALMENTE ---
// Sigue cuanto tiempo lleva la persona acostada y dispara la alarma (luz) tras TIEMPO_MAX_EN_CAMA_MS.
// Una ausencia de presion corta (ej. un giro en la cama) no reinicia el conteo: solo si la
// ausencia se sostiene mas de TOLERANCIA_AUSENCIA_CAMA_MS se confirma que la persona se levanto.
// void actualizarAlarmaCama(unsigned long ahora) {
//   bool presionDetectada = hayPersonaEnCama(PIN_FSR_CAMA, UMBRAL_PRESION_CAMA_ADC);
//
//   if (presionDetectada) {
//     if (!EN_CAMA) {
//       EN_CAMA = true;
//       INICIO_EN_CAMA_MILLIS = ahora;
//     }
//     HUBO_AUSENCIA_CAMA = false;
//   } else if (EN_CAMA) {
//     if (!HUBO_AUSENCIA_CAMA) {
//       HUBO_AUSENCIA_CAMA = true;
//       INICIO_AUSENCIA_CAMA_MILLIS = ahora;
//     } else if (ahora - INICIO_AUSENCIA_CAMA_MILLIS >= TOLERANCIA_AUSENCIA_CAMA_MS) {
//       // La ausencia ya duro lo suficiente: se confirma que la persona se levanto.
//       EN_CAMA = false;
//       HUBO_AUSENCIA_CAMA = false;
//       ALARMA_CAMA_ACTIVA = false;
//     }
//   }
//
//   if (EN_CAMA && (ahora - INICIO_EN_CAMA_MILLIS >= TIEMPO_MAX_EN_CAMA_MS)) {
//     ALARMA_CAMA_ACTIVA = true;
//   }
//
//   if (ALARMA_CAMA_ACTIVA) {
//     encenderLuz(PIN_LUZ_ALARMA_CAMA);
//   } else {
//     apagarLuz(PIN_LUZ_ALARMA_CAMA);
//   }
// }

// El LED de panico se controla directo (ya no hay registro de desplazamiento compartido).
void actualizarLedsCompartidos() {
  actualizarLeds(PIN_LED_PANICO, ALARMA_PANICO_ACTIVA);
}

// El buzzer es un recurso compartido: panico > proximidad (la alarma de cama esta
// desactivada temporalmente, ver ALARMA_CAMA_ACTIVA mas arriba).
// Se decide una sola frecuencia por vuelta de loop() antes de pedirla.
void actualizarBuzzerCompartido() {
  unsigned int frecuenciaBuzzer = 0;
  if (ALARMA_PANICO_ACTIVA) {
    frecuenciaBuzzer = FRECUENCIA_ALARMA_PANICO_HZ;
  // } else if (/* ALARMA_CAMA_ACTIVA, desactivada temporalmente */ false) {
  //   frecuenciaBuzzer = FRECUENCIA_ALARMA_CAMA_HZ;
  } else if (ALERTA_ACTIVA) {
    frecuenciaBuzzer = FRECUENCIA_ALARMA_PROXIMIDAD_HZ;
  }
  actualizarBuzzer(PIN_BUZZER, frecuenciaBuzzer);
}

// loop() se ejecuta una y otra vez, sin parar, mientras el Arduino este encendido.
// Se mantiene corto a proposito: cada tarea vive en su propia funcion arriba.
//
// El boton de panico se procesa PRIMERO, antes de actualizarProximidad()/actualizarLuzPasillo():
// ambas pueden bloquear brevemente (hasta ~60ms cada una, ver TIMEOUT_PULSO_US en config.h)
// esperando el eco de los sensores ultrasonicos, y la alarma de mayor prioridad del sistema
// no debe esperar a que esas lecturas terminen para reflejarse en el buzzer y el LED.
// DEBUG TEMPORAL: imprime en un solo bloque legible el estado de los sensores de lectura
// continua (proximidad, pasillo, fotocelda). Los eventos puntuales (panico, RFID, puerta)
// se imprimen aparte, en el momento en que ocurren (ver alerts.cpp y access.cpp), porque
// no tiene sentido esperar este intervalo para mostrarlos.
void imprimirLogsDebug(bool esDeNoche) {
  if (millis() - ULTIMO_LOG_MILLIS < INTERVALO_LOG_MS) {
    return;
  }
  ULTIMO_LOG_MILLIS = millis();

  Serial.print(F("Sensor 1: "));
  Serial.print(DEBUG_DISTANCIA_1);
  Serial.print(F(" cm | Sensor 2: "));
  Serial.print(DEBUG_DISTANCIA_2);
  Serial.print(F(" cm | Pasillo: "));
  Serial.print(DEBUG_DISTANCIA_PASILLO);
  Serial.print(F(" cm | Fotocelda ADC: "));
  Serial.print(analogRead(PIN_FOTOCELDA));
  Serial.print(F(" | De noche: "));
  Serial.print(esDeNoche ? F("SI") : F("no"));
  Serial.print(F(" | Alerta proximidad: "));
  Serial.print(ALERTA_ACTIVA ? F("SI") : F("no"));
  Serial.print(F(" | Luz pasillo: "));
  Serial.println(LUZ_PASILLO_ENCENDIDA ? F("encendida") : F("apagada"));
}

void loop() {
  unsigned long ahora = millis(); // Milisegundos transcurridos desde que arranco el Arduino

  // Se lee una sola vez por vuelta de loop() y se comparte: tanto la luz de pasillo como
  // la alarma de proximidad dependen de si es de noche.
  bool esDeNocheAhora = esDeNoche(PIN_FOTOCELDA, UMBRAL_OSCURIDAD_ADC);

  ALARMA_PANICO_ACTIVA = actualizarAlarmaPanico();
  actualizarProximidad(ahora, esDeNocheAhora);
  actualizarLuzPasillo(ahora, esDeNocheAhora);
  // actualizarAlarmaCama(ahora); // Alarma de cama desactivada temporalmente
  actualizarAcceso();

  actualizarLedsCompartidos();
  actualizarBuzzerCompartido();

  imprimirLogsDebug(esDeNocheAhora); // DEBUG TEMPORAL
}
