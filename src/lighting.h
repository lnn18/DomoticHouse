#ifndef LIGHTING_H
#define LIGHTING_H

// ============================================================================
//  LIGHTING (luces)
//  Que hace este archivo: guarda las 3 acciones basicas para manejar una luz:
//  prepararla, encenderla y apagarla.
//
//  Se usa para dos luces del proyecto: la luz del pasillo y la luz de la
//  alarma de la cama. Las dos funcionan igual, por eso comparten este modulo.
// ============================================================================

// "Arduino.h" es la libreria base: trae las palabras que usamos para hablar con
// los pines (pinMode, digitalWrite, HIGH, LOW...).
#include <Arduino.h>

// Prepara una luz. Se llama UNA sola vez, al arrancar (en setup()).
// "pinLuz" es el numero del pin del Arduino donde esta conectada la luz.
void inicializarLuz(uint8_t pinLuz);

// Enciende la luz (le manda corriente al pin).
void encenderLuz(uint8_t pinLuz);

// Apaga la luz (le quita la corriente al pin).
void apagarLuz(uint8_t pinLuz);

#endif
