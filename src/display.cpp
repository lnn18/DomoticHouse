#include "display.h"

void inicializarDisplay(uint8_t pinLedPanico, uint8_t pinLedProximidad) {
  pinMode(pinLedPanico, OUTPUT);
  digitalWrite(pinLedPanico, LOW); // Arranca apagado

  pinMode(pinLedProximidad, OUTPUT);
  digitalWrite(pinLedProximidad, LOW); // Arranca apagado
}

void actualizarLeds(uint8_t pinLedPanico, bool panicoActivo, uint8_t pinLedProximidad, bool proximidadActiva) {
  digitalWrite(pinLedPanico, panicoActivo ? HIGH : LOW);
  digitalWrite(pinLedProximidad, proximidadActiva ? HIGH : LOW);
}
