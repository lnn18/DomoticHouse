#include "display.h"

// Prepara los dos LEDs para poder encenderlos y apagarlos.
void inicializarDisplay(uint8_t pinLedPanico, uint8_t pinLedProximidad) {
  // OUTPUT = "salida": el Arduino manda corriente por el pin para prender el LED.
  pinMode(pinLedPanico, OUTPUT);
  digitalWrite(pinLedPanico, LOW); // LOW = sin corriente: arranca apagado

  pinMode(pinLedProximidad, OUTPUT);
  digitalWrite(pinLedProximidad, LOW); // Arranca apagado
}

// Pone cada LED en el estado que le toca. Se llama en cada vuelta del programa.
void actualizarLeds(uint8_t pinLedPanico, bool panicoActivo, uint8_t pinLedProximidad, bool proximidadActiva) {
  // "condicion ? A : B" se lee: "si se cumple la condicion, usa A; si no, usa B".
  // Aqui: si el panico esta activo manda HIGH (prende), si no manda LOW (apaga).
  digitalWrite(pinLedPanico, panicoActivo ? HIGH : LOW);
  digitalWrite(pinLedProximidad, proximidadActiva ? HIGH : LOW);
}
