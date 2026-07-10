#include "display.h"

namespace {
// Cada bit representa una salida del registro, de Q7 (izquierda) a Q0 (derecha).
constexpr byte BIT_LEDS_PROXIMIDAD = B00000011; // Q0 y Q1 (2 LEDs)
constexpr byte BIT_LED_PANICO = B00000100;      // Q2 (1 LED)
}

void inicializarDisplay(uint8_t pinClock, uint8_t pinLatch, uint8_t pinDatos) {
  pinMode(pinClock, OUTPUT);
  pinMode(pinLatch, OUTPUT);
  pinMode(pinDatos, OUTPUT);
  actualizarLeds(pinClock, pinLatch, pinDatos, false, false); // Arranca con los LEDs apagados
}

void actualizarLeds(uint8_t pinClock, uint8_t pinLatch, uint8_t pinDatos, bool proximidadActiva, bool panicoActivo) {
  byte patron = B00000000;
  if (proximidadActiva) {
    patron |= BIT_LEDS_PROXIMIDAD;
  }
  if (panicoActivo) {
    patron |= BIT_LED_PANICO;
  }

  // Mientras LATCH esta en LOW, el registro esta "recibiendo" datos y no actualiza
  // todavia lo que muestra en sus salidas.
  digitalWrite(pinLatch, LOW);

  // shiftOut() envia los 8 bits del patron elegido, uno por uno, usando CLOCK para
  // sincronizar cada bit. MSBFIRST significa que se envia primero el bit mas a la
  // izquierda (el mas significativo).
  shiftOut(pinDatos, pinClock, MSBFIRST, patron);

  // Al subir LATCH a HIGH, el registro "publica" de golpe los 8 bits recibidos en sus salidas.
  digitalWrite(pinLatch, HIGH);
}
