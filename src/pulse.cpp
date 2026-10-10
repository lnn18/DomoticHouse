#include "pulse.h"
#include "config.h"
#include "lcd.h"
#include <stdio.h> // snprintf: arma textos con numeros para la pantalla

// ============================================================================
//  Deteccion de latidos propia (sin libreria externa).
//
//  Por que no usamos una libreria: la libreria PulseSensor Playground usa un
//  temporizador interno que choca con el del buzzer (tone()) y con el del servo.
//  Aqui el programa lee el sensor cada MUESTREO_PULSO_MS desde loop(), sin tocar
//  ningun temporizador.
//
//  Algoritmo (el mismo del sketch original de PulseSensor), explicado simple:
//    1. La senal sube y baja con cada latido (una onda).
//    2. Se guarda el punto mas ALTO (PICO) y el mas BAJO (VALLE) de cada onda.
//    3. Se pone una linea (UMBRAL) a mitad de camino entre los dos.
//    4. Cada vez que la senal CRUZA la linea hacia arriba, cuenta un latido.
//    5. El tiempo entre dos latidos (IBI) da los latidos por minuto:
//       BPM = 60000 / IBI (en milisegundos).
//
//  Flujo general: switch OFF = no se mide. Switch ON = ciclos de
//  DURACION_MEDICION_PULSO_MS; al final de cada ciclo se publica el resultado
//  (pantalla + computadora) y, tras una pausa corta, empieza otro ciclo.
// ============================================================================

namespace {
// Todo lo de este bloque es PRIVADO de este archivo.

// --- Constantes del detector ---
constexpr unsigned long MUESTREO_PULSO_MS = 2UL;  // Leer el sensor cada 2 ms (500 veces por segundo)
constexpr unsigned long IBI_MINIMO_MS = 250UL;    // 240 BPM: un latido mas rapido que esto es ruido
constexpr unsigned long IBI_REINICIO_MS = 2500UL; // 24 BPM: si pasa tanto sin latidos, se reinicia el detector
constexpr uint8_t TAMANO_PROMEDIO_IBI = 10;       // El BPM es el promedio de los ultimos 10 intervalos

// --- Estados posibles del modulo ---
//   APAGADO  : el switch esta en OFF, no se mide
//   MIDIENDO : se esta midiendo un ciclo de 10 s
//   PAUSA    : pausa de 1 s entre ciclos (los sensores de distancia pueden leer)
enum EstadoPulso : uint8_t { APAGADO, MIDIENDO, PAUSA };
EstadoPulso ESTADO = APAGADO;

// --- Resultado del ultimo ciclo (para mostrarlo en la fila 2 de la pantalla) ---
//   SIN_DATO : todavia no hay resultado
//   NORMAL   : BPM dentro del rango
//   BAJO     : BPM por debajo del minimo (alarma)
//   ALTO     : BPM por encima del maximo (alarma)
//   INVALIDO : no se detectaron suficientes latidos (dedo mal puesto)
enum Resultado : uint8_t { SIN_DATO, NORMAL, BAJO, ALTO, INVALIDO };
Resultado ULTIMO_RESULTADO = SIN_DATO;

// --- Datos del sensor y del switch ---
uint8_t SENSOR_GPIO = 0;               // Pin del sensor de pulso
uint8_t SWITCH_GPIO = 0;               // Pin del switch ON/OFF
bool SWITCH_ESTABLE_ON = false;        // Posicion del switch ya filtrada (true = ON)
bool SWITCH_LECTURA_PREVIA = false;    // Ultima lectura "cruda" del switch
unsigned long ULTIMO_CAMBIO_SWITCH_MILLIS = 0; // Cuando cambio por ultima vez

// --- Datos de la medicion en curso ---
unsigned long INICIO_MEDICION_MILLIS = 0; // Cuando empezo el ciclo actual
unsigned long INICIO_PAUSA_MILLIS = 0;    // Cuando empezo la pausa entre ciclos
unsigned long ULTIMO_AVANCE_MILLIS = 0;   // Ultima vez que se mostro el avance (1 vez por segundo)
unsigned long ULTIMA_MUESTRA_MILLIS = 0;  // Ultima vez que se leyo el sensor
uint8_t LATIDOS_CONTADOS = 0;             // Latidos detectados en este ciclo
int BPM_ACTUAL = 0;                       // BPM calculado hasta ahora
int ULTIMO_BPM_MOSTRADO = 0;              // BPM del ultimo ciclo terminado (para la pantalla)
bool ALARMA_PULSO_ACTIVA_INTERNA = false; // true = hay alarma de pulso activa

// --- Estado interno del detector de latidos ---
bool EN_LATIDO = false;                   // true mientras la senal esta por encima de la linea (umbral)
bool PRIMER_LATIDO_VISTO = false;         // El primer latido no tiene un intervalo anterior confiable
unsigned long ULTIMO_LATIDO_MILLIS = 0;   // Momento del ultimo latido
unsigned int IBI_MS = 600;                // Ultimo intervalo entre latidos (milisegundos)
int PICO = 512;                           // Punto mas alto de la onda actual
int VALLE = 512;                          // Punto mas bajo de la onda actual
int UMBRAL = 512;                         // La "linea" que hay que cruzar para contar un latido
unsigned int IBIS[TAMANO_PROMEDIO_IBI];   // Lista de los ultimos intervalos
uint8_t NUM_IBIS = 0;                     // Cuantos intervalos hay guardados
uint8_t SIGUIENTE_IBI = 0;                // En que posicion de la lista guardar el proximo

// Deja el detector en blanco para empezar una medicion nueva.
void reiniciarDetector(unsigned long ahora) {
  EN_LATIDO = false;
  PRIMER_LATIDO_VISTO = false;
  ULTIMO_LATIDO_MILLIS = ahora;
  IBI_MS = 600;
  PICO = 512;
  VALLE = 512;
  UMBRAL = 512;
  NUM_IBIS = 0;
  SIGUIENTE_IBI = 0;
}

// Procesa UNA lectura del sensor (un numero de 0 a 1023).
// Devuelve true si en esta lectura empezo un latido.
bool procesarMuestra(int senal, unsigned long ahora) {
  unsigned long n = ahora - ULTIMO_LATIDO_MILLIS; // Tiempo desde el ultimo latido
  bool latido = false;

  // Paso 1: actualizar el punto mas bajo y el mas alto de la onda. El primer "if"
  // ignora los instantes justo despues de un latido para no confundirse con la
  // pequena bajada que ocurre normalmente (la "muesca dicrotica").
  if (senal < UMBRAL && n > (IBI_MS / 5UL) * 3UL) {
    if (senal < VALLE) VALLE = senal;
  }
  if (senal > UMBRAL && senal > PICO) {
    PICO = senal;
  }

  // Paso 2: detectar un latido nuevo. Debe cumplir: haber pasado el tiempo minimo
  // entre latidos, que la senal supere la linea, y que no estemos ya "dentro" de uno.
  if (n > IBI_MINIMO_MS) {
    if (senal > UMBRAL && !EN_LATIDO && n > (IBI_MS / 5UL) * 3UL) {
      EN_LATIDO = true;
      latido = true;
      unsigned long intervalo = ahora - ULTIMO_LATIDO_MILLIS; // Tiempo entre este latido y el anterior
      ULTIMO_LATIDO_MILLIS = ahora;

      if (!PRIMER_LATIDO_VISTO) {
        // El primer latido solo sirve para empezar a contar el tiempo.
        PRIMER_LATIDO_VISTO = true;
      } else {
        // Guardamos el intervalo en la lista (que da la vuelta cuando se llena).
        IBI_MS = (unsigned int)intervalo;
        IBIS[SIGUIENTE_IBI] = IBI_MS;
        SIGUIENTE_IBI = (SIGUIENTE_IBI + 1) % TAMANO_PROMEDIO_IBI;
        if (NUM_IBIS < TAMANO_PROMEDIO_IBI) NUM_IBIS++;

        // Promedio de los intervalos guardados, y de ahi los latidos por minuto.
        unsigned long suma = 0;
        for (uint8_t i = 0; i < NUM_IBIS; i++) suma += IBIS[i];
        unsigned long promedio = suma / NUM_IBIS;
        if (promedio > 0) {
          BPM_ACTUAL = (int)(60000UL / promedio); // 60000 ms = 1 minuto
        }
      }
    }
  }

  // Paso 3: fin del latido. Cuando la senal vuelve a bajar de la linea, se
  // recalcula la linea a mitad de camino entre el pico y el valle.
  if (senal < UMBRAL && EN_LATIDO) {
    EN_LATIDO = false;
    int amplitud = PICO - VALLE;       // Que tan grande fue la onda
    UMBRAL = amplitud / 2 + VALLE;     // Linea a mitad de camino
    PICO = UMBRAL;
    VALLE = UMBRAL;
  }

  // Paso 4: si pasa mucho tiempo sin latidos (dedo quitado o movido), se vuelve a
  // los valores iniciales para volver a "enganchar" la senal.
  if (n > IBI_REINICIO_MS) {
    UMBRAL = 512;
    PICO = 512;
    VALLE = 512;
    ULTIMO_LATIDO_MILLIS = ahora;
    PRIMER_LATIDO_VISTO = false;
  }

  return latido;
}

// Devuelve la posicion del switch (true = ON/cerrado), ya filtrada de rebotes.
bool switchEncendido(unsigned long ahora) {
  bool lectura = (digitalRead(SWITCH_GPIO) == LOW); // INPUT_PULLUP: cerrado = LOW = ON
  // Si la lectura cambio, anotamos cuando.
  if (lectura != SWITCH_LECTURA_PREVIA) {
    SWITCH_LECTURA_PREVIA = lectura;
    ULTIMO_CAMBIO_SWITCH_MILLIS = ahora;
  }
  // Anti-rebote: solo creemos la nueva posicion si se mantuvo un ratito.
  if (ahora - ULTIMO_CAMBIO_SWITCH_MILLIS >= DEBOUNCE_SWITCH_PULSO_MS) {
    SWITCH_ESTABLE_ON = lectura;
  }
  return SWITCH_ESTABLE_ON;
}

// --- Pantalla LCD: fila 1 = estado del switch y avance; fila 2 = resultado del ultimo ciclo ---

// Dibuja la fila de arriba.
void dibujarFilaSwitch(bool encendido, unsigned long segundos) {
  char linea[17]; // 16 letras + 1 de fin de texto
  if (!encendido) {
    snprintf(linea, sizeof(linea), "SW:OFF  Pulso --");
  } else if (ESTADO == MIDIENDO) {
    snprintf(linea, sizeof(linea), "SW:ON Mide %2lu/%lu", segundos, DURACION_MEDICION_PULSO_MS / 1000UL);
  } else {
    snprintf(linea, sizeof(linea), "SW:ON  Resultado");
  }
  lcdLinea(0, linea);
}

// Dibuja la fila de abajo con el resultado del ultimo ciclo.
void dibujarFilaResultado() {
  char linea[17];
  switch (ULTIMO_RESULTADO) {
    case NORMAL:
      snprintf(linea, sizeof(linea), "BPM:%3d NORMAL", ULTIMO_BPM_MOSTRADO);
      break;
    case BAJO:
      snprintf(linea, sizeof(linea), "BPM:%3d BAJO!!", ULTIMO_BPM_MOSTRADO);
      break;
    case ALTO:
      snprintf(linea, sizeof(linea), "BPM:%3d ALTO!!", ULTIMO_BPM_MOSTRADO);
      break;
    case INVALIDO:
      snprintf(linea, sizeof(linea), "Sin lectura dedo");
      break;
    default:
      snprintf(linea, sizeof(linea), "BPM: --");
      break;
  }
  lcdLinea(1, linea);
}

// Empieza un ciclo de medicion nuevo (pone todo en cero).
void iniciarCiclo(unsigned long ahora) {
  ESTADO = MIDIENDO;
  INICIO_MEDICION_MILLIS = ahora;
  ULTIMA_MUESTRA_MILLIS = ahora;
  ULTIMO_AVANCE_MILLIS = ahora;
  LATIDOS_CONTADOS = 0;
  BPM_ACTUAL = 0;
  reiniciarDetector(ahora);
  Serial.println(F(">>> Pulso: MIDIENDO, manten el dedo quieto en el sensor..."));
  dibujarFilaSwitch(true, 0);
}

// Termina el ciclo: decide si el resultado es normal, bajo, alto o invalido,
// lo muestra y activa la alarma si hace falta.
void terminarCiclo(unsigned long ahora) {
  ESTADO = PAUSA;
  INICIO_PAUSA_MILLIS = ahora;

  Serial.println(F("========== RESULTADO PULSO =========="));
  if (LATIDOS_CONTADOS < LATIDOS_MINIMOS_MEDICION || BPM_ACTUAL <= 0) {
    // Muy pocos latidos: lo mas probable es que el dedo estuviera mal puesto.
    // Por seguridad, una lectura invalida NO apaga una alarma que ya estaba
    // activa, y tampoco se muestra como "normal".
    ULTIMO_RESULTADO = INVALIDO;
    Serial.print(F("MEDICION INVALIDA (latidos detectados: "));
    Serial.print(LATIDOS_CONTADOS);
    Serial.print(F(", minimo "));
    Serial.print(LATIDOS_MINIMOS_MEDICION);
    Serial.println(F("). Revisa la posicion del dedo."));
  } else {
    ULTIMO_BPM_MOSTRADO = BPM_ACTUAL;
    Serial.print(F("BPM: "));
    Serial.print(BPM_ACTUAL);
    Serial.print(F("   (latidos: "));
    Serial.print(LATIDOS_CONTADOS);
    Serial.println(F(")"));
    Serial.print(F("Rango normal: "));
    Serial.print(BPM_MIN_NORMAL);
    Serial.print(F("-"));
    Serial.println(BPM_MAX_NORMAL);
    if (BPM_ACTUAL < BPM_MIN_NORMAL) {
      ULTIMO_RESULTADO = BAJO;
      ALARMA_PULSO_ACTIVA_INTERNA = true;
      Serial.println(F("ESTADO: BAJO -> ALARMA"));
    } else if (BPM_ACTUAL > BPM_MAX_NORMAL) {
      ULTIMO_RESULTADO = ALTO;
      ALARMA_PULSO_ACTIVA_INTERNA = true;
      Serial.println(F("ESTADO: ALTO -> ALARMA"));
    } else {
      ULTIMO_RESULTADO = NORMAL;
      ALARMA_PULSO_ACTIVA_INTERNA = false; // Un resultado normal apaga la alarma
      Serial.println(F("ESTADO: NORMAL"));
    }
  }
  Serial.println(F("====================================="));

  dibujarFilaSwitch(true, 0);
  dibujarFilaResultado();
}

// Detiene todo cuando el switch pasa a OFF: apaga la alarma y limpia la pantalla.
void apagarPulso() {
  ESTADO = APAGADO;
  ALARMA_PULSO_ACTIVA_INTERNA = false;
  ULTIMO_RESULTADO = SIN_DATO;
  ULTIMO_BPM_MOSTRADO = 0;
  Serial.println(F(">>> Pulso: switch OFF, medicion detenida"));
  dibujarFilaSwitch(false, 0);
  dibujarFilaResultado();
}
}

// Prepara el sensor y el switch.
void inicializarPulso(uint8_t pinSensor, uint8_t pinSwitch) {
  SENSOR_GPIO = pinSensor;
  SWITCH_GPIO = pinSwitch;
  pinMode(pinSwitch, INPUT_PULLUP); // El switch conecta el pin a tierra al cerrarse (ON)
  // El pin analogico del sensor no necesita pinMode: analogRead() lo configura solo.
  // Posicion de partida: si arranca en ON, eso no cuenta como un cambio nuevo.
  SWITCH_LECTURA_PREVIA = (digitalRead(pinSwitch) == LOW);
  SWITCH_ESTABLE_ON = SWITCH_LECTURA_PREVIA;
  dibujarFilaSwitch(false, 0);
  dibujarFilaResultado();
}

// Dice si hay una medicion en curso.
bool midiendoPulso() {
  return ESTADO == MIDIENDO;
}

// Se llama en cada vuelta del programa. Aqui ocurre todo el flujo de la medicion.
bool actualizarPulso() {
  unsigned long ahora = millis();
  bool encendido = switchEncendido(ahora);

  // Caso 1: switch en OFF. Si veniamos midiendo, se detiene todo.
  if (!encendido) {
    if (ESTADO != APAGADO) {
      apagarPulso();
    }
    return ALARMA_PULSO_ACTIVA_INTERNA;
  }

  // Switch en ON. Si estaba apagado, arranca el primer ciclo.
  if (ESTADO == APAGADO) {
    iniciarCiclo(ahora);
  }

  if (ESTADO == MIDIENDO) {
    // Caso 2: midiendo. Cada 2 ms leemos el sensor y buscamos latidos.
    if (ahora - ULTIMA_MUESTRA_MILLIS >= MUESTREO_PULSO_MS) {
      ULTIMA_MUESTRA_MILLIS = ahora;
      int senal = analogRead(SENSOR_GPIO); // Lee el sensor: un numero de 0 a 1023
      bool latido = procesarMuestra(senal, ahora);
      if (latido && LATIDOS_CONTADOS < 255) {
        LATIDOS_CONTADOS++;
      }
      if (DEBUG_PULSO_SENAL) { // DEBUG TEMPORAL: muestra la senal cruda para calibrar
        static uint8_t contadorMuestras = 0;
        if (latido) {
          Serial.print(F("LATIDO bpm="));
          Serial.println(BPM_ACTUAL);
        } else if (++contadorMuestras >= 10) { // 1 de cada 10 muestras = cada 20 ms
          contadorMuestras = 0;
          Serial.print(F("Senal:"));
          Serial.print(senal);
          Serial.print(F(" Umbral:"));
          Serial.println(UMBRAL);
        }
      }
    }

    // Una vez por segundo: se muestra el avance (en la computadora y en la pantalla).
    if (ahora - ULTIMO_AVANCE_MILLIS >= 1000UL) {
      ULTIMO_AVANCE_MILLIS = ahora;
      unsigned long segundos = (ahora - INICIO_MEDICION_MILLIS) / 1000UL;
      Serial.print(F("  midiendo "));
      Serial.print(segundos);
      Serial.print(F("/"));
      Serial.print(DURACION_MEDICION_PULSO_MS / 1000UL);
      Serial.print(F(" s | latidos: "));
      Serial.print(LATIDOS_CONTADOS);
      Serial.print(F(" | BPM parcial: "));
      Serial.println(BPM_ACTUAL);
      dibujarFilaSwitch(true, segundos);
    }

    // Si se cumplio el tiempo del ciclo, se termina y se publica el resultado.
    if (ahora - INICIO_MEDICION_MILLIS >= DURACION_MEDICION_PULSO_MS) {
      terminarCiclo(ahora);
    }
  } else if (ESTADO == PAUSA) {
    // Caso 3: pausa corta entre ciclos. Mientras dura, midiendoPulso() es false y
    // main.cpp deja que los sensores de distancia lean. Luego, si el switch sigue
    // en ON, arranca otro ciclo.
    if (ahora - INICIO_PAUSA_MILLIS >= PAUSA_ENTRE_MEDICIONES_MS) {
      iniciarCiclo(ahora);
    }
  }

  return ALARMA_PULSO_ACTIVA_INTERNA;
}
