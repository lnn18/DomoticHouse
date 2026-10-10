#ifndef PULSE_H
#define PULSE_H

#include <Arduino.h>

// Configura el sensor de pulso y su switch ON/OFF (llamar una vez en setup(), despues de
// inicializarLcd()).
void inicializarPulso(uint8_t pinSensor, uint8_t pinSwitch);

// Debe llamarse en cada vuelta de loop() y lo mas seguido posible mientras
// midiendoPulso() sea true (el sensor se muestrea cada ~2ms). Con el switch en OFF no mide;
// en ON mide en ciclos y publica el resultado de cada uno en la LCD y el serial. Devuelve
// true si la alarma de pulso esta activa (ultimo ciclo valido fuera de rango).
bool actualizarPulso();

// true mientras hay un ciclo de medicion en curso. main.cpp lo usa para pausar las
// lecturas que bloquean el loop (ultrasonicos, logs seriales).
bool midiendoPulso();

#endif
