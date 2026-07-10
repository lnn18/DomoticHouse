#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino.h>

// Como funciona el 74HC595 (registro de desplazamiento):
// recibe los bits (encendido/apagado) uno por uno por el pin DATOS, sincronizados por
// el pin CLOCK, y cuando termina de recibirlos todos los "publica" de golpe en sus 8
// salidas (Q0 a Q7) al mover el pin LATCH. Asi se controlan hasta 8 LEDs usando solo 3
// pines del Arduino.

// Configura los pines del registro de desplazamiento 74HC595 (llamar una vez en setup()).
void inicializarDisplay(uint8_t pinClock, uint8_t pinLatch, uint8_t pinDatos);

// Actualiza las salidas del registro con el estado de cada luz que controla:
// Q0/Q1 para la deteccion de proximidad, Q2 para la alarma de panico.
// Se llama una sola vez por vuelta de loop() (el shiftOut() manda los 8 bits juntos,
// asi que hace falta conocer el estado de todas las luces antes de enviarlo).
void actualizarLeds(uint8_t pinClock, uint8_t pinLatch, uint8_t pinDatos, bool proximidadActiva, bool panicoActivo);

#endif
