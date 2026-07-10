#include "alerts.h"

// Variables privadas de este archivo: solo se pueden usar aqui, no desde otros .cpp.
namespace {
constexpr unsigned int FRECUENCIA_ALARMA_HZ = 2000; // Frecuencia del sonido de alarma, en Hercios (mas alto = mas agudo)
bool ALERTA_PREVIA = false; // Recuerda si la alarma ya estaba sonando en la lectura anterior
}

void inicializarAlerta(uint8_t pinBuzzer) {
  pinMode(pinBuzzer, OUTPUT);
  noTone(pinBuzzer);    // Asegura que el buzzer arranca en silencio
  ALERTA_PREVIA = false;
}

void actualizarAlerta(uint8_t pinBuzzer, bool activa) {
  // Si el estado no cambio desde la ultima vez (sigue sonando o sigue en silencio),
  // no hacemos nada: evita reiniciar el tono innecesariamente en cada vuelta del loop.
  if (activa == ALERTA_PREVIA) {
    return; // sin cambio de estado, no reiniciar el tono
  }

  if (activa) {
    tone(pinBuzzer, FRECUENCIA_ALARMA_HZ); // Empieza a sonar el buzzer a la frecuencia definida
  } else {
    noTone(pinBuzzer); // Apaga el sonido del buzzer
  }

  ALERTA_PREVIA = activa; // Actualiza el recuerdo del estado para la proxima comparacion
}
