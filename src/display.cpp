#include "display.h"

namespace {
// B00000000 y B00000011 son numeros binarios: cada 0/1 representa una salida del
// registro, de Q7 (izquierda) a Q0 (derecha).
constexpr byte PATRON_LEDS_APAGADOS = B00000000;   // Las 8 salidas apagadas
constexpr byte PATRON_LEDS_ENCENDIDOS = B00000011; // Q0 y Q1 encendidas (2 LEDs), el resto apagadas
}

void inicializarDisplay(uint8_t pinClock, uint8_t pinLatch, uint8_t pinDatos) {
  pinMode(pinClock, OUTPUT);
  pinMode(pinLatch, OUTPUT);
  pinMode(pinDatos, OUTPUT);
  actualizarLeds(pinClock, pinLatch, pinDatos, false); // Arranca con los LEDs apagados
}

void actualizarLeds(uint8_t pinClock, uint8_t pinLatch, uint8_t pinDatos, bool encender) {
  // Mientras LATCH esta en LOW, el registro esta "recibiendo" datos y no actualiza
  // todavia lo que muestra en sus salidas.
  digitalWrite(pinLatch, LOW);

  // shiftOut() envia los 8 bits del patron elegido, uno por uno, usando CLOCK para
  // sincronizar cada bit. MSBFIRST significa que se envia primero el bit mas a la
  // izquierda (el mas significativo).
  shiftOut(pinDatos, pinClock, MSBFIRST, encender ? PATRON_LEDS_ENCENDIDOS : PATRON_LEDS_APAGADOS);

  // Al subir LATCH a HIGH, el registro "publica" de golpe los 8 bits recibidos en sus salidas.
  digitalWrite(pinLatch, HIGH);
}
