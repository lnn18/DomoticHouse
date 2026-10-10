#include "pulse.h"
#include "config.h"
#include "lcd.h"
#include <stdio.h>

// Deteccion de latidos propia (sin libreria): PulseSensor Playground v2.1.0 usa siempre un
// timer de hardware en AVR (Timer2 con Servo.h presente), el mismo que tone() del buzzer,
// y define un ISR que choca con Servo.h. Aqui se muestrea cada MUESTREO_PULSO_MS desde
// loop(), sin interrupciones ni timers. Algoritmo: umbral adaptativo entre el pico y el
// valle de la senal, igual al del sketch original de PulseSensor.
//
// Flujo: switch OFF = no se mide. Switch ON = ciclos de DURACION_MEDICION_PULSO_MS; al final
// de cada ciclo se publica el resultado (LCD + serial) y, tras una pausa corta, empieza otro.

namespace {
constexpr unsigned long MUESTREO_PULSO_MS = 2UL;     // El sensor necesita ~500 muestras/s
constexpr unsigned long IBI_MINIMO_MS = 250UL;       // 240 BPM: un latido mas rapido es ruido
constexpr unsigned long IBI_REINICIO_MS = 2500UL;    // 24 BPM: sin latidos en este tiempo, se reinician umbrales
constexpr uint8_t TAMANO_PROMEDIO_IBI = 10;          // BPM = promedio de hasta los ultimos 10 intervalos

enum EstadoPulso : uint8_t { APAGADO, MIDIENDO, PAUSA };
EstadoPulso ESTADO = APAGADO;

// Resultado del ultimo ciclo, para mostrarlo en la segunda fila de la LCD.
enum Resultado : uint8_t { SIN_DATO, NORMAL, BAJO, ALTO, INVALIDO };
Resultado ULTIMO_RESULTADO = SIN_DATO;

uint8_t SENSOR_GPIO = 0;
uint8_t SWITCH_GPIO = 0;
bool SWITCH_ESTABLE_ON = false;            // Nivel ya filtrado por el anti-rebote (true = ON)
bool SWITCH_LECTURA_PREVIA = false;
unsigned long ULTIMO_CAMBIO_SWITCH_MILLIS = 0;

unsigned long INICIO_MEDICION_MILLIS = 0;
unsigned long INICIO_PAUSA_MILLIS = 0;
unsigned long ULTIMO_AVANCE_MILLIS = 0;
unsigned long ULTIMA_MUESTRA_MILLIS = 0;
uint8_t LATIDOS_CONTADOS = 0;
int BPM_ACTUAL = 0;
int ULTIMO_BPM_MOSTRADO = 0;
bool ALARMA_PULSO_ACTIVA_INTERNA = false;

// Estado del detector de latidos
bool EN_LATIDO = false;                    // true mientras la senal esta por encima del umbral
bool PRIMER_LATIDO_VISTO = false;          // el primer latido no tiene intervalo previo valido
unsigned long ULTIMO_LATIDO_MILLIS = 0;
unsigned int IBI_MS = 600;                 // ultimo intervalo entre latidos
int PICO = 512;
int VALLE = 512;
int UMBRAL = 512;
unsigned int IBIS[TAMANO_PROMEDIO_IBI];
uint8_t NUM_IBIS = 0;
uint8_t SIGUIENTE_IBI = 0;

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

// Procesa UNA muestra de la senal (0-1023). Devuelve true si en esta muestra empezo un latido.
bool procesarMuestra(int senal, unsigned long ahora) {
  unsigned long n = ahora - ULTIMO_LATIDO_MILLIS; // tiempo desde el ultimo latido
  bool latido = false;

  // Pico y valle de la onda actual, sin contar la "muesca dicrotica" justo tras un latido.
  if (senal < UMBRAL && n > (IBI_MS / 5UL) * 3UL) {
    if (senal < VALLE) VALLE = senal;
  }
  if (senal > UMBRAL && senal > PICO) {
    PICO = senal;
  }

  if (n > IBI_MINIMO_MS) {
    if (senal > UMBRAL && !EN_LATIDO && n > (IBI_MS / 5UL) * 3UL) {
      EN_LATIDO = true;
      latido = true;
      unsigned long intervalo = ahora - ULTIMO_LATIDO_MILLIS;
      ULTIMO_LATIDO_MILLIS = ahora;

      if (!PRIMER_LATIDO_VISTO) {
        PRIMER_LATIDO_VISTO = true; // sin intervalo previo confiable: solo marca el inicio
      } else {
        IBI_MS = (unsigned int)intervalo;
        IBIS[SIGUIENTE_IBI] = IBI_MS;
        SIGUIENTE_IBI = (SIGUIENTE_IBI + 1) % TAMANO_PROMEDIO_IBI;
        if (NUM_IBIS < TAMANO_PROMEDIO_IBI) NUM_IBIS++;

        unsigned long suma = 0;
        for (uint8_t i = 0; i < NUM_IBIS; i++) suma += IBIS[i];
        unsigned long promedio = suma / NUM_IBIS;
        if (promedio > 0) {
          BPM_ACTUAL = (int)(60000UL / promedio);
        }
      }
    }
  }

  // Fin del latido: se recalcula el umbral a la mitad entre pico y valle.
  if (senal < UMBRAL && EN_LATIDO) {
    EN_LATIDO = false;
    int amplitud = PICO - VALLE;
    UMBRAL = amplitud / 2 + VALLE;
    PICO = UMBRAL;
    VALLE = UMBRAL;
  }

  // Mucho tiempo sin latidos: se vuelve a los valores iniciales (dedo quitado/movido).
  if (n > IBI_REINICIO_MS) {
    UMBRAL = 512;
    PICO = 512;
    VALLE = 512;
    ULTIMO_LATIDO_MILLIS = ahora;
    PRIMER_LATIDO_VISTO = false;
  }

  return latido;
}

// Devuelve el nivel del switch (true = ON/cerrado) ya filtrado por el anti-rebote.
bool switchEncendido(unsigned long ahora) {
  bool lectura = (digitalRead(SWITCH_GPIO) == LOW); // INPUT_PULLUP: cerrado = LOW
  if (lectura != SWITCH_LECTURA_PREVIA) {
    SWITCH_LECTURA_PREVIA = lectura;
    ULTIMO_CAMBIO_SWITCH_MILLIS = ahora;
  }
  if (ahora - ULTIMO_CAMBIO_SWITCH_MILLIS >= DEBOUNCE_SWITCH_PULSO_MS) {
    SWITCH_ESTABLE_ON = lectura;
  }
  return SWITCH_ESTABLE_ON;
}

// --- LCD: fila 0 = estado del switch y avance; fila 1 = resultado del ultimo ciclo ---
void dibujarFilaSwitch(bool encendido, unsigned long segundos) {
  char linea[17];
  if (!encendido) {
    snprintf(linea, sizeof(linea), "SW:OFF  Pulso --");
  } else if (ESTADO == MIDIENDO) {
    snprintf(linea, sizeof(linea), "SW:ON Mide %2lu/%lu", segundos, DURACION_MEDICION_PULSO_MS / 1000UL);
  } else {
    snprintf(linea, sizeof(linea), "SW:ON  Resultado");
  }
  lcdLinea(0, linea);
}

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

void terminarCiclo(unsigned long ahora) {
  ESTADO = PAUSA;
  INICIO_PAUSA_MILLIS = ahora;

  Serial.println(F("========== RESULTADO PULSO =========="));
  if (LATIDOS_CONTADOS < LATIDOS_MINIMOS_MEDICION || BPM_ACTUAL <= 0) {
    // Fail-safe: una lectura invalida NO apaga una alarma ya activa ni se muestra como
    // "normal"; lo mas probable es dedo mal puesto, pero no se puede asegurar.
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
      ALARMA_PULSO_ACTIVA_INTERNA = false;
      Serial.println(F("ESTADO: NORMAL"));
    }
  }
  Serial.println(F("====================================="));

  dibujarFilaSwitch(true, 0);
  dibujarFilaResultado();
}

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

void inicializarPulso(uint8_t pinSensor, uint8_t pinSwitch) {
  SENSOR_GPIO = pinSensor;
  SWITCH_GPIO = pinSwitch;
  pinMode(pinSwitch, INPUT_PULLUP);
  // El pin analogico no necesita pinMode: analogRead() lo configura.
  SWITCH_LECTURA_PREVIA = (digitalRead(pinSwitch) == LOW);
  SWITCH_ESTABLE_ON = SWITCH_LECTURA_PREVIA;
  dibujarFilaSwitch(false, 0);
  dibujarFilaResultado();
}

bool midiendoPulso() {
  return ESTADO == MIDIENDO;
}

bool actualizarPulso() {
  unsigned long ahora = millis();
  bool encendido = switchEncendido(ahora);

  if (!encendido) {
    if (ESTADO != APAGADO) {
      apagarPulso();
    }
    return ALARMA_PULSO_ACTIVA_INTERNA;
  }

  // Switch en ON.
  if (ESTADO == APAGADO) {
    iniciarCiclo(ahora);
  }

  if (ESTADO == MIDIENDO) {
    if (ahora - ULTIMA_MUESTRA_MILLIS >= MUESTREO_PULSO_MS) {
      ULTIMA_MUESTRA_MILLIS = ahora;
      int senal = analogRead(SENSOR_GPIO);
      bool latido = procesarMuestra(senal, ahora);
      if (latido && LATIDOS_CONTADOS < 255) {
        LATIDOS_CONTADOS++;
      }
      if (DEBUG_PULSO_SENAL) { // DEBUG TEMPORAL
        static uint8_t contadorMuestras = 0;
        if (latido) {
          Serial.print(F("LATIDO bpm="));
          Serial.println(BPM_ACTUAL);
        } else if (++contadorMuestras >= 10) { // 1 de cada 10 muestras = cada 20ms
          contadorMuestras = 0;
          Serial.print(F("Senal:"));
          Serial.print(senal);
          Serial.print(F(" Umbral:"));
          Serial.println(UMBRAL);
        }
      }
    }

    // Avance una vez por segundo (serial y LCD): tiempo transcurrido, latidos y BPM parcial.
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

    if (ahora - INICIO_MEDICION_MILLIS >= DURACION_MEDICION_PULSO_MS) {
      terminarCiclo(ahora);
    }
  } else if (ESTADO == PAUSA) {
    // Ventana corta entre ciclos: midiendoPulso() es false, asi main.cpp deja leer los
    // ultrasonicos. Luego, si el switch sigue en ON, arranca otro ciclo.
    if (ahora - INICIO_PAUSA_MILLIS >= PAUSA_ENTRE_MEDICIONES_MS) {
      iniciarCiclo(ahora);
    }
  }

  return ALARMA_PULSO_ACTIVA_INTERNA;
}
