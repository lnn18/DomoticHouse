#include "alerts.h"
#include "config.h"

// Variables privadas de este archivo: solo se pueden usar aqui, no desde otros .cpp.
namespace {
unsigned int FRECUENCIA_PREVIA_HZ = 0; // 0 representa "buzzer en silencio"

// "volatile" porque HUBO_PULSO_PANICO se modifica dentro de una interrupcion (isrBotonPanico)
// y se lee desde loop(): sin volatile, el compilador podria optimizar esa lectura asumiendo
// que el valor nunca cambia fuera del flujo normal del programa.
volatile bool HUBO_PULSO_PANICO = false;
// Arranca ya "vencido" (en el pasado) para que un primer pulso dentro de los primeros
// DEBOUNCE_BOTON_PANICO_MS de vida del programa no quede ignorado por el anti-rebote.
unsigned long ULTIMO_TOGGLE_PANICO_MILLIS = 0UL - DEBOUNCE_BOTON_PANICO_MS;
bool ALARMA_PANICO_ACTIVA_INTERNA = false; // Estado real; se expone solo via actualizarAlarmaPanico()

// El ISR debe ser lo mas corto posible: solo marca que hubo un pulso. Todo el trabajo
// real (anti-rebote, decidir si corresponde alternar la alarma) se hace en loop(),
// nunca dentro de una interrupcion.
void isrBotonPanico() {
  HUBO_PULSO_PANICO = true;
}
}

void inicializarAlerta(uint8_t pinBuzzer) {
  pinMode(pinBuzzer, OUTPUT);
  noTone(pinBuzzer); // Asegura que el buzzer arranca en silencio
  FRECUENCIA_PREVIA_HZ = 0;
}

void actualizarBuzzer(uint8_t pinBuzzer, unsigned int frecuenciaHz) {
  // Si se pide la misma frecuencia que ya estaba sonando (o el mismo silencio),
  // no hacemos nada: evita reiniciar el tono innecesariamente en cada vuelta del loop.
  if (frecuenciaHz == FRECUENCIA_PREVIA_HZ) {
    return;
  }

  if (frecuenciaHz == 0) {
    noTone(pinBuzzer);
  } else {
    tone(pinBuzzer, frecuenciaHz);
  }

  FRECUENCIA_PREVIA_HZ = frecuenciaHz;
}

void inicializarBotonPanico(uint8_t pinBoton) {
  // INPUT_PULLUP: el boton conecta el pin a GND al presionarlo; sin presionar, la
  // resistencia interna mantiene el pin en HIGH (no hace falta resistencia externa).
  pinMode(pinBoton, INPUT_PULLUP);
  // attachInterrupt (no sondeo) para minimizar la latencia de respuesta del boton de panico.
  attachInterrupt(digitalPinToInterrupt(pinBoton), isrBotonPanico, FALLING);
}

bool actualizarAlarmaPanico() {
  // Se lee y se limpia la bandera de entrada, antes de procesarla: asi, si el ISR
  // llega a disparar justo mientras se procesa este pulso, no se pierde (quedaria
  // marcada de nuevo para la proxima vuelta de loop() en vez de perderse en el medio).
  bool huboPulso = HUBO_PULSO_PANICO;
  HUBO_PULSO_PANICO = false;

  if (huboPulso) {
    unsigned long ahora = millis();
    if (ahora - ULTIMO_TOGGLE_PANICO_MILLIS >= DEBOUNCE_BOTON_PANICO_MS) {
      ALARMA_PANICO_ACTIVA_INTERNA = !ALARMA_PANICO_ACTIVA_INTERNA; // toggle: activa/desactiva con cada pulso valido
      ULTIMO_TOGGLE_PANICO_MILLIS = ahora;
    }
    // Si el pulso cayo dentro de la ventana de rebote, no se hace nada mas:
    // eso es justamente lo que descarta los rebotes sin generar toggles de mas.
  }
  return ALARMA_PANICO_ACTIVA_INTERNA;
}
