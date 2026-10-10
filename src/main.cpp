// ============================================================================
//  MAIN (programa principal)
//
//  Que hace este proyecto: es una "casa inteligente" de apoyo para una persona
//  mayor. Vigila varias cosas y avisa con luces y sonidos si algo no esta bien:
//
//    1. PANICO     : un boton de emergencia (la alarma mas importante).
//    2. CAMA       : avisa si la persona lleva demasiado tiempo acostada.
//    3. PULSO      : mide los latidos del corazon y avisa si son muy altos o bajos.
//    4. PROXIMIDAD : avisa si algo se acerca a un sensor... pero solo de noche.
//    5. PASILLO    : enciende la luz del pasillo cuando alguien pasa de noche.
//    6. PUERTA     : un boton abre la puerta con un servomotor.
//
//  Como esta organizado: este archivo es el "director de orquesta". Cada tarea
//  vive en su propio archivo (sensors, alerts, pulse...) y aqui solo se decide
//  cuando llamar a cada una y quien tiene prioridad en el buzzer.
//
//  Como funciona un programa de Arduino (explicado simple):
//    - setup() : se ejecuta UNA vez al encender. Aqui se "prepara" todo.
//    - loop()  : se ejecuta una y otra vez, sin parar, para siempre.
//
//  Regla de oro de este proyecto: NUNCA se usa "delay()" (que congela todo el
//  programa). En su lugar se usa millis(), que dice cuantos milisegundos pasaron
//  desde que se encendio el Arduino, y se compara con la hora anotada antes.
//  Asi el programa nunca se queda "dormido" y siempre atiende las emergencias.
// ============================================================================

#include <Arduino.h>   // Libreria base de Arduino: pinMode, digitalWrite, millis(), Serial, etc.
#include "config.h"    // Pines y constantes del proyecto (todo centralizado aqui)
#include "sensors.h"   // Funciones para leer los sensores (distancia x3, fotocelda, cama)
#include "alerts.h"    // Funciones para el buzzer compartido y el boton de panico
#include "display.h"   // Funciones para controlar los LEDs de aviso (panico y proximidad)
#include "lighting.h"  // Funciones para controlar las luces (pasillo y alarma de cama)
#include "access.h"    // Puerta: servo + boton de apertura
#include "lcd.h"       // Pantalla LCD 16x2
#include "pulse.h"     // Sensor de pulso cardiaco (switch ON/OFF + pantalla + alarma por BPM fuera de rango)

// ----------------------------------------------------------------------------
//  Variables globales (la "memoria" del programa).
//  Por convencion, las variables que guardan el ESTADO del sistema se escriben
//  en MAYUSCULAS.
// ----------------------------------------------------------------------------

// Guarda en que momento (milisegundos desde que arranco el Arduino) se leyeron
// los sensores de proximidad por ultima vez. Junto con millis() en loop(), permite
// esperar un intervalo sin usar delay() (que congelaria todo el programa).
unsigned long ULTIMA_LECTURA_MILLIS = 0;

// MODO DE PRUEBA (DEBUG TEMPORAL): controla cada cuanto se imprimen los mensajes de
// diagnostico en la computadora. Sin esto se imprimirian miles de veces por segundo
// y el monitor serial seria ilegible.
unsigned long ULTIMO_LOG_MILLIS = 0;
constexpr unsigned long INTERVALO_LOG_MS = 500UL; // un mensaje cada medio segundo

// Indica si en este momento hay una alerta de proximidad activa (algun sensor de
// distancia detecto algo cerca y es de noche).
bool ALERTA_ACTIVA = false;

// MODO DE PRUEBA (DEBUG TEMPORAL): ultima distancia leida de cada sensor, guardada
// aqui solo para poder imprimirla sin tener que leer los sensores dos veces.
long DEBUG_DISTANCIA_1 = -1;
long DEBUG_DISTANCIA_2 = -1;
long DEBUG_DISTANCIA_PASILLO = -1;

// Luz del pasillo: se enciende cuando el sensor del pasillo detecta algo cerca, y se
// apaga sola tras un rato sin detectar nada.
bool LUZ_PASILLO_ENCENDIDA = false;                // true = la luz esta prendida
unsigned long ULTIMA_DETECCION_PASILLO_MILLIS = 0; // cuando fue la ultima vez que vio algo

// --- Alarma de cama (hoy con un interruptor que simula el sensor real) ---
bool EN_CAMA = false;                          // true = la persona esta acostada
unsigned long INICIO_EN_CAMA_MILLIS = 0;       // desde cuando esta acostada
bool HUBO_AUSENCIA_CAMA = false;               // true = acaba de dejar de detectarse presion
unsigned long INICIO_AUSENCIA_CAMA_MILLIS = 0; // desde cuando falta la presion
bool ALARMA_CAMA_ACTIVA = false;               // true = la alarma de cama esta sonando

// Boton de panico: es la alarma de mayor prioridad de todo el sistema.
bool ALARMA_PANICO_ACTIVA = false;

// Pulso cardiaco: true si la ultima medicion salio de [BPM_MIN_NORMAL, BPM_MAX_NORMAL] y
// todavia no se ha corregido (con un ciclo normal o apagando el switch).
bool ALARMA_PULSO_ACTIVA = false;

// ----------------------------------------------------------------------------
//  setup(): se ejecuta UNA sola vez, apenas el Arduino arranca o se reinicia.
//  Aqui se prepara todo lo necesario antes de empezar a repetir loop().
// ----------------------------------------------------------------------------
void setup() {
  Serial.begin(9600); // Abre la comunicacion con la computadora (para ver mensajes), a 9600 baudios

  // Se deja registro de cada arranque: si el Arduino se reinicia (corte de luz, etc.)
  // mientras alguien lleva horas en la cama, el conteo de TIEMPO_MAX_EN_CAMA_MS se
  // pierde y arranca de nuevo desde 0 sin otro aviso. Este mensaje al menos permite
  // notar, revisando el monitor serial, que hubo un reinicio inesperado.
  Serial.println(F("Arduino (re)iniciado"));

  // Prepara cada sensor de distancia: configura sus pines TRIG (emisor) y ECHO (receptor)
  inicializarSensor(PIN_TRIG_1, PIN_ECHO_1);
  inicializarSensor(PIN_TRIG_2, PIN_ECHO_2);
  inicializarSensor(PIN_TRIG_PASILLO, PIN_ECHO_PASILLO); // 3er sensor, usado para presencia en el pasillo

  inicializarAlerta(PIN_BUZZER); // Prepara el buzzer
  inicializarDisplay(PIN_LED_PANICO, PIN_LED_PROXIMIDAD); // Prepara los dos LEDs de aviso

  inicializarLuz(PIN_LUZ_PASILLO); // Prepara la luz del pasillo

  inicializarFotocelda(PIN_FOTOCELDA); // Prepara el sensor de luz (dia/noche)

  inicializarSensorPresionCama(PIN_FSR_CAMA); // Interruptor (temporal, simula el sensor real de la cama)
  inicializarLuz(PIN_LUZ_ALARMA_CAMA);        // Luz de la alarma de cama

  inicializarBotonPanico(PIN_BOTON_PANICO); // Boton de emergencia

  inicializarLcd();                                            // Pantalla de texto
  inicializarPulso(PIN_SENSOR_PULSO, PIN_SWITCH_PULSO);        // Sensor de pulso + su switch

  // Puerta: servo del pestillo + boton de apertura manual.
  inicializarAcceso(PIN_SERVO_PUERTA);
  inicializarBotonPuerta(PIN_BOTON_PUERTA);
}

// ----------------------------------------------------------------------------
//  Alarma de PROXIMIDAD
//  Lee los 2 sensores de distancia y actualiza ALERTA_ACTIVA. Se llama en cada
//  vuelta, pero solo mide una vez cada INTERVALO_LECTURA_MS (1 segundo).
//  (El 3er sensor, el del pasillo, se lee aparte en actualizarLuzPasillo.)
//  De dia (esDeNoche == false) la alarma no se activa: igual que la luz del
//  pasillo, depende de la fotocelda.
// ----------------------------------------------------------------------------
void actualizarProximidad(unsigned long ahora, bool esDeNoche) {
  // Si todavia no paso 1 segundo desde la ultima medicion, no hacemos nada.
  if (ahora - ULTIMA_LECTURA_MILLIS < INTERVALO_LECTURA_MS) {
    return;
  }
  ULTIMA_LECTURA_MILLIS = ahora; // Anotamos cuando medimos

  // Medimos la distancia de cada sensor (en centimetros, o -1 si no hubo eco).
  long distancia1 = medirDistanciaCm(PIN_TRIG_1, PIN_ECHO_1);
  long distancia2 = medirDistanciaCm(PIN_TRIG_2, PIN_ECHO_2);
  DEBUG_DISTANCIA_1 = distancia1; // DEBUG TEMPORAL: guardado para mostrarlo en la computadora
  DEBUG_DISTANCIA_2 = distancia2;

  // "Detecto algo" = hubo una medicion valida (>= 0) Y esta dentro de la distancia de alerta.
  bool deteccion1 = (distancia1 >= 0) && (distancia1 <= UMBRAL_DETECCION_1_CM);
  bool deteccion2 = (distancia2 >= 0) && (distancia2 <= UMBRAL_DETECCION_2_CM);

  // La alerta se activa solo si es de noche Y alguno de los dos sensores detecto algo.
  // (&& significa "y", || significa "o".)
  ALERTA_ACTIVA = esDeNoche && (deteccion1 || deteccion2);
}

// ----------------------------------------------------------------------------
//  Luz del PASILLO
//  Enciende la luz cuando el sensor del pasillo detecta algo a menos de
//  UMBRAL_PASILLO_CM, y la apaga sola tras DURACION_LUZ_PASILLO_MS sin detectar
//  nada. De dia no enciende, aunque haya alguien cerca.
// ----------------------------------------------------------------------------
void actualizarLuzPasillo(unsigned long ahora, bool esDeNoche) {
  long distanciaPasillo = medirDistanciaCm(PIN_TRIG_PASILLO, PIN_ECHO_PASILLO);
  DEBUG_DISTANCIA_PASILLO = distanciaPasillo; // DEBUG TEMPORAL: guardado para mostrarlo
  bool hayAlguienCerca = (distanciaPasillo >= 0) && (distanciaPasillo <= UMBRAL_PASILLO_CM);

  if (esDeNoche && hayAlguienCerca) {
    // Hay alguien: anotamos la hora y, si la luz estaba apagada, la encendemos.
    ULTIMA_DETECCION_PASILLO_MILLIS = ahora;
    if (!LUZ_PASILLO_ENCENDIDA) {
      encenderLuz(PIN_LUZ_PASILLO);
      LUZ_PASILLO_ENCENDIDA = true;
    }
  } else if (LUZ_PASILLO_ENCENDIDA && (!esDeNoche || (ahora - ULTIMA_DETECCION_PASILLO_MILLIS >= DURACION_LUZ_PASILLO_MS))) {
    // No hay nadie y la luz estaba prendida: se apaga si ya es de dia, o si ya paso
    // el tiempo de espera desde la ultima deteccion.
    apagarLuz(PIN_LUZ_PASILLO);
    LUZ_PASILLO_ENCENDIDA = false;
  }
}

// ----------------------------------------------------------------------------
//  Alarma de CAMA
//  Cuenta cuanto tiempo lleva la persona acostada y dispara la alarma (luz y
//  buzzer) pasado TIEMPO_MAX_EN_CAMA_MS.
//  Una ausencia corta (ej. un giro en la cama) NO reinicia el conteo: solo si la
//  ausencia dura mas de TOLERANCIA_AUSENCIA_CAMA_MS se confirma que la persona se
//  levanto.
// ----------------------------------------------------------------------------
void actualizarAlarmaCama(unsigned long ahora) {
  bool presionDetectada = hayPersonaEnCama(PIN_FSR_CAMA); // true = hay alguien acostado

  if (presionDetectada) {
    // Hay alguien en la cama.
    if (!EN_CAMA) {
      // Acaba de acostarse: empezamos a contar desde ahora.
      EN_CAMA = true;
      INICIO_EN_CAMA_MILLIS = ahora;
    }
    HUBO_AUSENCIA_CAMA = false; // Volvio la presion: se cancela cualquier ausencia en curso
  } else if (EN_CAMA) {
    // Estaba acostada y ahora no se detecta presion.
    if (!HUBO_AUSENCIA_CAMA) {
      // Primer instante de ausencia: anotamos desde cuando falta.
      HUBO_AUSENCIA_CAMA = true;
      INICIO_AUSENCIA_CAMA_MILLIS = ahora;
    } else if (ahora - INICIO_AUSENCIA_CAMA_MILLIS >= TOLERANCIA_AUSENCIA_CAMA_MS) {
      // La ausencia ya duro lo suficiente: se confirma que la persona se levanto.
      EN_CAMA = false;
      HUBO_AUSENCIA_CAMA = false;
      ALARMA_CAMA_ACTIVA = false;
    }
  }

  // Si lleva acostada mas del tiempo maximo, se activa la alarma.
  if (EN_CAMA && (ahora - INICIO_EN_CAMA_MILLIS >= TIEMPO_MAX_EN_CAMA_MS)) {
    ALARMA_CAMA_ACTIVA = true;
  }

  // La luz de la alarma sigue al estado de la alarma.
  if (ALARMA_CAMA_ACTIVA) {
    encenderLuz(PIN_LUZ_ALARMA_CAMA);
  } else {
    apagarLuz(PIN_LUZ_ALARMA_CAMA);
  }
}

// ----------------------------------------------------------------------------
//  LEDs de aviso
//  El LED de panico sigue a la alarma de panico. El LED de proximidad sigue a
//  ALERTA_ACTIVA: las mismas condiciones que su buzzer (objeto cerca Y es de
//  noche), sin relacion con el panico.
// ----------------------------------------------------------------------------
void actualizarLedsCompartidos() {
  actualizarLeds(PIN_LED_PANICO, ALARMA_PANICO_ACTIVA, PIN_LED_PROXIMIDAD, ALERTA_ACTIVA);
}

// ----------------------------------------------------------------------------
//  BUZZER compartido
//  El buzzer es UNO SOLO y varias alarmas lo quieren usar. Si suenan varias a la
//  vez, gana la mas importante, en este orden:
//     panico > cama > pulso > proximidad
//  Panico es la emergencia inmediata. Cama y pulso fuera de rango son problemas de
//  salud que duran, por eso van antes que proximidad (un objeto que pasa cerca).
//  Se decide UN solo tono por vuelta del programa y se le pide al buzzer.
// ----------------------------------------------------------------------------
void actualizarBuzzerCompartido() {
  unsigned int frecuenciaBuzzer = 0; // 0 = silencio
  if (ALARMA_PANICO_ACTIVA) {
    frecuenciaBuzzer = FRECUENCIA_ALARMA_PANICO_HZ;
  } else if (ALARMA_CAMA_ACTIVA) {
    frecuenciaBuzzer = FRECUENCIA_ALARMA_CAMA_HZ;
  } else if (ALARMA_PULSO_ACTIVA) {
    frecuenciaBuzzer = FRECUENCIA_ALARMA_PULSO_HZ;
  } else if (ALERTA_ACTIVA) {
    frecuenciaBuzzer = FRECUENCIA_ALARMA_PROXIMIDAD_HZ;
  }
  actualizarBuzzer(PIN_BUZZER, frecuenciaBuzzer);
}

// ----------------------------------------------------------------------------
//  MODO DE PRUEBA (DEBUG TEMPORAL)
//  Imprime en la computadora, en una sola linea legible, el estado de los
//  sensores de lectura continua (proximidad, pasillo, fotocelda). Los eventos
//  puntuales (panico, puerta) se imprimen aparte, en el momento en que ocurren
//  (ver alerts.cpp y access.cpp).
// ----------------------------------------------------------------------------
void imprimirLogsDebug(bool esDeNoche) {
  // Solo imprime una vez cada INTERVALO_LOG_MS.
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

// ----------------------------------------------------------------------------
//  loop(): se ejecuta una y otra vez, sin parar, mientras el Arduino este encendido.
//  Se mantiene corto a proposito: cada tarea vive en su propia funcion arriba.
//
//  El boton de panico se procesa PRIMERO, antes que las lecturas de distancia:
//  esas lecturas pueden demorarse unos milisegundos esperando el eco, y la alarma
//  mas importante del sistema no debe esperar.
// ----------------------------------------------------------------------------
void loop() {
  unsigned long ahora = millis(); // Milisegundos transcurridos desde que arranco el Arduino

  // Se lee una sola vez por vuelta y se comparte: tanto la luz de pasillo como
  // la alarma de proximidad dependen de si es de noche.
  bool esDeNocheAhora = esDeNoche(PIN_FOTOCELDA, UMBRAL_OSCURIDAD_ADC);

  // 1) Las alarmas mas importantes primero.
  ALARMA_PANICO_ACTIVA = actualizarAlarmaPanico();
  ALARMA_PULSO_ACTIVA = actualizarPulso();

  // 2) Sensores de distancia. Mientras se mide el pulso (ciclos cortos de 10 s) se
  //    pausan: sus lecturas se demoran hasta ~60 ms y el sensor de pulso necesita
  //    leerse cada ~2 ms. El panico (por interrupcion) y la cama (sin esperas)
  //    siguen activos. Costo asumido: durante esos segundos la alarma de proximidad
  //    y la luz de pasillo no reaccionan.
  if (!midiendoPulso()) {
    actualizarProximidad(ahora, esDeNocheAhora);
    actualizarLuzPasillo(ahora, esDeNocheAhora);
  }

  // 3) Cama y puerta.
  actualizarAlarmaCama(ahora);
  actualizarAcceso(); // boton de apertura + cierre automatico de la puerta

  // 4) Con el estado de todas las alarmas ya calculado, se actualizan los avisos.
  actualizarLedsCompartidos();
  actualizarBuzzerCompartido();

  // 5) Mensajes de diagnostico a la computadora (tambien pausados mientras se
  //    mide el pulso, porque escribirlos demora el programa).
  if (!midiendoPulso()) {
    imprimirLogsDebug(esDeNocheAhora); // DEBUG TEMPORAL
  }
}
