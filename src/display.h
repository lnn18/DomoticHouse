#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino.h>

// El LED de panico se controla directo (sin registro de desplazamiento): antes vivia
// en el 74HC595, pero ese registro se quito para liberar el bus SPI (D11/D12/D13) que
// ahora usa el lector RC522. La alarma de proximidad ya no tiene LED propio, solo el
// buzzer (ver actualizarBuzzerCompartido en main.cpp).

// Configura el pin del LED de panico (llamar una vez en setup()).
void inicializarDisplay(uint8_t pinLedPanico);

// Enciende o apaga el LED de panico segun corresponda.
void actualizarLeds(uint8_t pinLedPanico, bool panicoActivo);

#endif
