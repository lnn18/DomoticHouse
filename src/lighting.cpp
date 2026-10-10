#include "lighting.h"

// Prepara el pin de una luz.
void inicializarLuz(uint8_t pinLuz) {
  // OUTPUT = "salida": el Arduino va a MANDAR corriente por este pin (no a leerla).
  pinMode(pinLuz, OUTPUT);
  // LOW = sin corriente. La luz arranca apagada para que no se prenda sola al conectar.
  digitalWrite(pinLuz, LOW);
}

// Enciende la luz.
void encenderLuz(uint8_t pinLuz) {
  // HIGH = con corriente (5 voltios): la luz se prende.
  digitalWrite(pinLuz, HIGH);
}

// Apaga la luz.
void apagarLuz(uint8_t pinLuz) {
  // LOW = sin corriente: la luz se apaga.
  digitalWrite(pinLuz, LOW);
}
