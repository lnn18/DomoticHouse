#ifndef SENSORS_H
#define SENSORS_H

#include <Arduino.h>

// Como funciona un sensor HC-SR04:
// tiene un pin TRIG (dispara un pulso de ultrasonido, inaudible para nosotros) y un pin
// ECHO (recibe el eco que rebota en el objeto mas cercano). Midiendo cuanto tiempo tarda
// en volver el eco se calcula la distancia, igual que un radar o un murcielago.

// Configura los pines de un sensor HC-SR04 (llamar una vez por sensor en setup()).
void inicializarSensor(uint8_t pinTrig, uint8_t pinEcho);

// Dispara el sensor y mide la distancia en centimetros.
// Devuelve -1 si el eco no llego dentro de TIMEOUT_PULSO_US (sin objeto en rango o error de lectura).
long medirDistanciaCm(uint8_t pinTrig, uint8_t pinEcho);

#endif
