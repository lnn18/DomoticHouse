#ifndef SENSORS_H
#define SENSORS_H

// ============================================================================
//  SENSORS (sensores)
//  Que hace este archivo: reune las funciones que "leen el mundo":
//    1) Sensores de distancia (ultrasonicos): miden a que distancia esta un objeto.
//    2) Sensor de la cama: dice si hay una persona acostada.
//    3) Fotocelda: dice si es de noche (poca luz) o de dia (mucha luz).
//
//  Idea general: cada funcion lee un sensor y devuelve un dato simple (un
//  numero o un si/no), para que el resto del programa no tenga que saber como
//  esta conectado cada sensor.
// ============================================================================

#include <Arduino.h>

// --- Como funciona un sensor ultrasonico (SRF-05 usado como HC-SR04) ---
// Tiene dos pines importantes:
//   TRIG: dispara un "grito" de ultrasonido (un sonido que no escuchamos).
//   ECHO: escucha el eco que rebota en el objeto mas cercano.
// Midiendo cuanto tarda en volver el eco se calcula la distancia, igual que
// hace un murcielago o un radar.
// El SRF-05 tiene un 5to pin (OUT/Mode); al dejarlo sin conectar se comporta
// igual que un HC-SR04, por eso el codigo no distingue entre los dos.

// Prepara los pines de un sensor ultrasonico (TRIG y ECHO).
// Se llama UNA vez por sensor, al arrancar (en setup()).
void inicializarSensor(uint8_t pinTrig, uint8_t pinEcho);

// Dispara el sensor y mide la distancia en centimetros.
// Devuelve -1 si el eco no llego a tiempo (no hay objeto en rango, o hubo un
// error de lectura). Un valor -1 NO es una distancia: es "no pude medir".
long medirDistanciaCm(uint8_t pinTrig, uint8_t pinEcho);

// --- Funciones DESACTIVADAS (guardadas como referencia) ---
// El pasillo antes usaba un sensor PIR (detecta movimiento) y ahora usa un
// tercer sensor ultrasonico con medirDistanciaCm(). Estas funciones ya no se usan.
// void inicializarSensorPir(uint8_t pinPir);
// bool detectaMovimiento(uint8_t pinPir);

// --- Sensor de la cama ---
// Por ahora es un interruptor comun que SIMULA el sensor de presion real (FSR),
// que todavia no esta armado. Interruptor cerrado = "hay alguien en la cama".
// Prepara el pin del interruptor. Se llama UNA vez al arrancar.
void inicializarSensorPresionCama(uint8_t pinFsr);

// Dice si la persona esta acostada (true) o no (false).
// Incluye un filtro anti-rebote: ignora cambios muy breves (un rebote del
// interruptor o un ruido) para no confundirse.
bool hayPersonaEnCama(uint8_t pinFsr);

// --- Fotocelda (LDR) ---
// Una fotocelda es una resistencia que cambia segun la luz que recibe.
// Prepara el pin de la fotocelda. Se llama UNA vez al arrancar.
void inicializarFotocelda(uint8_t pinFotocelda);

// Dice si es de noche (true) o de dia (false).
//   umbralAdc: numero (0 a 1023) a partir del cual se considera "oscuro".
// Incluye un filtro anti-rebote para que la respuesta no parpadee cuando la luz
// esta justo en el limite (por ejemplo, al amanecer).
bool esDeNoche(uint8_t pinFotocelda, int umbralAdc);

#endif
