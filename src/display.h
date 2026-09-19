#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino.h>

// Los LEDs se controlan directo (sin registro de desplazamiento): antes vivian en el
// 74HC595, pero ese registro se quito para liberar el bus SPI (D11/D12/D13) que ahora
// usa el lector RC522.

// Configura los pines de los LEDs (llamar una vez en setup()).
void inicializarDisplay(uint8_t pinLedPanico, uint8_t pinLedProximidad);

// Enciende o apaga cada LED segun corresponda. El LED de proximidad es el canal visual
// de esa alarma (ver PIN_LED_PROXIMIDAD en config.h): se enciende con las mismas
// condiciones que su buzzer, no con el buzzer de panico.
void actualizarLeds(uint8_t pinLedPanico, bool panicoActivo, uint8_t pinLedProximidad, bool proximidadActiva);

#endif
