#include "lighting.h"

void inicializarLuz(uint8_t pinLuz) {
  pinMode(pinLuz, OUTPUT);
  digitalWrite(pinLuz, LOW);
}

void encenderLuz(uint8_t pinLuz) {
  digitalWrite(pinLuz, HIGH);
}

void apagarLuz(uint8_t pinLuz) {
  digitalWrite(pinLuz, LOW);
}
