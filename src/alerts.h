#ifndef ALERTS_H
#define ALERTS_H

#include <Arduino.h>

// Configura el pin del buzzer (llamar una vez en setup()).
void inicializarAlerta(uint8_t pinBuzzer);

// Enciende o apaga el tono de alarma segun el estado de deteccion.
// Solo dispara tone()/noTone() en los cambios de estado (flanco, es decir cuando "activa"
// pasa de true a false o viceversa), no en cada llamada, para no reiniciar el sonido sin necesidad.
void actualizarAlerta(uint8_t pinBuzzer, bool activa);

#endif
