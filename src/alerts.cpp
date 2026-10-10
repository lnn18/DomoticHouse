#include "alerts.h"
#include "config.h"

// Variables PRIVADAS de este archivo: solo se pueden usar aqui, no desde otros .cpp.
namespace {
// Guarda el tono que esta sonando ahora mismo. 0 significa "buzzer en silencio".
unsigned int FRECUENCIA_PREVIA_HZ = 0;

// "volatile" porque HUBO_PULSO_PANICO la modifica una interrupcion (isrBotonPanico)
// y la lee loop(): sin volatile, el compilador podria asumir que el valor nunca
// cambia por su cuenta y "olvidarse" de revisarlo.
// Es una bandera: true = "el boton fue presionado, falta procesarlo".
volatile bool HUBO_PULSO_PANICO = false;

// Momento (en milisegundos) del ultimo cambio de la alarma de panico. Arranca
// "en el pasado" para que el primer pulso, aunque llegue muy rapido tras el
// encendido, no sea ignorado por el filtro anti-rebote.
unsigned long ULTIMO_TOGGLE_PANICO_MILLIS = 0UL - DEBOUNCE_BOTON_PANICO_MS;

// El estado real de la alarma: true = activa, false = desactivada. Se expone
// hacia afuera solo a traves de actualizarAlarmaPanico().
bool ALARMA_PANICO_ACTIVA_INTERNA = false;

// ISR = "rutina de servicio de interrupcion": es lo que el Arduino ejecuta al
// instante cuando se presiona el boton. Debe ser lo mas CORTA posible: solo
// levanta la bandera. Todo el trabajo de verdad se hace despues, en loop().
void isrBotonPanico() {
  HUBO_PULSO_PANICO = true;
}
}

// Prepara el buzzer.
void inicializarAlerta(uint8_t pinBuzzer) {
  pinMode(pinBuzzer, OUTPUT);
  noTone(pinBuzzer); // Se asegura de que arranque en silencio
  FRECUENCIA_PREVIA_HZ = 0;
}

// Hace sonar (o callar) el buzzer.
void actualizarBuzzer(uint8_t pinBuzzer, unsigned int frecuenciaHz) {
  // Si piden el mismo tono que ya suena (o el mismo silencio), no hacemos nada:
  // asi evitamos cortar y reiniciar el sonido en cada vuelta del programa.
  if (frecuenciaHz == FRECUENCIA_PREVIA_HZ) {
    return;
  }

  if (frecuenciaHz == 0) {
    noTone(pinBuzzer); // Silencio
  } else {
    tone(pinBuzzer, frecuenciaHz); // Suena al tono pedido
  }

  FRECUENCIA_PREVIA_HZ = frecuenciaHz; // Recordamos que tono quedo sonando
}

// Prepara el boton de panico.
void inicializarBotonPanico(uint8_t pinBoton) {
  // INPUT_PULLUP: el boton conecta el pin a tierra (GND) al presionarlo; sin
  // presionar, una resistencia interna del Arduino mantiene el pin en HIGH. Por
  // eso no hace falta ninguna resistencia externa.
  pinMode(pinBoton, INPUT_PULLUP);
  // Le dice al Arduino: "cuando este pin pase de HIGH a LOW (FALLING = al presionar),
  // ejecuta isrBotonPanico al instante". Es mas rapido que ir preguntando una y otra vez.
  attachInterrupt(digitalPinToInterrupt(pinBoton), isrBotonPanico, FALLING);
}

// Revisa si el boton fue presionado y actualiza la alarma.
bool actualizarAlarmaPanico() {
  // Copiamos la bandera y la bajamos de inmediato. Asi, si el boton se presiona
  // justo mientras procesamos, ese pulso no se pierde: la bandera quedara en true
  // otra vez para la proxima vuelta.
  bool huboPulso = HUBO_PULSO_PANICO;
  HUBO_PULSO_PANICO = false;

  if (huboPulso) {
    unsigned long ahora = millis(); // millis() = milisegundos desde que se encendio el Arduino
    // Anti-rebote: un boton real "vibra" y manda varios pulsos en pocos milisegundos.
    // Solo aceptamos el pulso si paso suficiente tiempo desde el ultimo valido.
    if (ahora - ULTIMO_TOGGLE_PANICO_MILLIS >= DEBOUNCE_BOTON_PANICO_MS) {
      ALARMA_PANICO_ACTIVA_INTERNA = !ALARMA_PANICO_ACTIVA_INTERNA; // "!" invierte: activa<->desactivada
      ULTIMO_TOGGLE_PANICO_MILLIS = ahora;
      // DEBUG TEMPORAL: se escribe un mensaje solo cuando cambia el estado.
      Serial.println(ALARMA_PANICO_ACTIVA_INTERNA ? F("Panico: ACTIVADO") : F("Panico: desactivado"));
    }
    // Si el pulso llego dentro de la ventana de rebote, no se hace nada: eso
    // es justamente lo que descarta los rebotes.
  }
  return ALARMA_PANICO_ACTIVA_INTERNA;
}
