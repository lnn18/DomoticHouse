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

// Configura el pin del sensor PIR (llamar una vez en setup()).
void inicializarSensorPir(uint8_t pinPir);

// Indica si hay movimiento estable frente al PIR (con anti-rebote interno).
bool detectaMovimiento(uint8_t pinPir);

// Configura el pin del sensor de presion de la cama (llamar una vez en setup()).
void inicializarSensorPresionCama(uint8_t pinFsr);

// Indica si hay presion estable sobre la cama, es decir, si la persona esta acostada
// (con anti-rebote interno para no reaccionar a ruido momentaneo de la lectura).
bool hayPersonaEnCama(uint8_t pinFsr, int umbralAdc);

#endif
