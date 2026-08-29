#include "display.h"

void inicializarDisplay(uint8_t pinLedPanico) {
  pinMode(pinLedPanico, OUTPUT);
  digitalWrite(pinLedPanico, LOW); // Arranca apagado
}

void actualizarLeds(uint8_t pinLedPanico, bool panicoActivo) {
  digitalWrite(pinLedPanico, panicoActivo ? HIGH : LOW);
}
