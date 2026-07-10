#ifndef LIGHTING_H
#define LIGHTING_H

#include <Arduino.h>

// Configura el pin de una luz (llamar una vez por luz en setup()).
void inicializarLuz(uint8_t pinLuz);

void encenderLuz(uint8_t pinLuz);
void apagarLuz(uint8_t pinLuz);

#endif
