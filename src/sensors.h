#ifndef SENSORS_H
#define SENSORS_H

#include <Arduino.h>

// Como funciona un sensor ultrasonico de 4 pines (SRF-05 en modo compatible HC-SR04):
// tiene un pin TRIG (dispara un pulso de ultrasonido, inaudible para nosotros) y un pin
// ECHO (recibe el eco que rebota en el objeto mas cercano). Midiendo cuanto tiempo tarda
// en volver el eco se calcula la distancia, igual que un radar o un murcielago.
// El SRF-05 tiene un 5to pin (OUT/Mode) para modo de 1 pin; al dejarlo sin conectar,
// se comporta identico al HC-SR04 en modo de 4 pines, por eso el codigo no distingue entre ambos.

// Configura los pines de un sensor ultrasonico de 4 pines (llamar una vez por sensor en setup()).
void inicializarSensor(uint8_t pinTrig, uint8_t pinEcho);

// Dispara el sensor y mide la distancia en centimetros.
// Devuelve -1 si el eco no llego dentro de TIMEOUT_PULSO_US (sin objeto en rango o error de lectura).
long medirDistanciaCm(uint8_t pinTrig, uint8_t pinEcho);

// DESACTIVADAS: el pasillo paso de usar un sensor PIR a un 3er sensor ultrasonico
// (ver PIN_TRIG_PASILLO/PIN_ECHO_PASILLO en config.h y actualizarLuzPasillo en main.cpp,
// que ahora usa medirDistanciaCm() en vez de estas funciones).
// void inicializarSensorPir(uint8_t pinPir);
// bool detectaMovimiento(uint8_t pinPir);

// Sensor de presion de la cama: simulado con un switch digital (INPUT_PULLUP), mismo patron
// que el boton de panico, mientras no este armado el FSR real (ver PIN_FSR_CAMA en config.h).
// Configura el pin del switch/FSR (llamar una vez en setup()).
void inicializarSensorPresionCama(uint8_t pinFsr);

// Indica si hay presion estable sobre la cama, es decir, si la persona esta acostada
// (con anti-rebote interno para no reaccionar a un rebote mecanico del switch, o a ruido
// momentaneo de la lectura una vez que se reemplace por el FSR analogico real).
bool hayPersonaEnCama(uint8_t pinFsr);

// Configura el pin de la fotocelda (LDR, llamar una vez en setup()).
void inicializarFotocelda(uint8_t pinFotocelda);

// Indica si esta oscuro (es de noche) segun la fotocelda, con anti-rebote interno para
// no parpadear si la luz ambiente esta justo en el limite del umbral (ej. al amanecer).
bool esDeNoche(uint8_t pinFotocelda, int umbralAdc);

#endif
