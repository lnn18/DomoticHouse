#include "alerts.h"

// Variable privada de este archivo: solo se puede usar aqui, no desde otros .cpp.
namespace {
unsigned int FRECUENCIA_PREVIA_HZ = 0; // 0 representa "buzzer en silencio"
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
