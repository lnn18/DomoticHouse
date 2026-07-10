#ifndef ALERTS_H
#define ALERTS_H

#include <Arduino.h>

// Configura el pin del buzzer (llamar una vez en setup()).
void inicializarAlerta(uint8_t pinBuzzer);

// Pone el buzzer a sonar a "frecuenciaHz" (0 = silencio).
// Solo reinicia el tono cuando la frecuencia pedida cambia respecto a la anterior,
// para no cortar el sonido innecesariamente en cada vuelta del loop.
//
// El buzzer es un unico recurso compartido: quien decide que frecuencia pedir en
// cada momento (y con que prioridad entre distintas alarmas) es responsabilidad
// de quien llama a esta funcion (ver main.cpp), no de este modulo.
void actualizarBuzzer(uint8_t pinBuzzer, unsigned int frecuenciaHz);

#endif
