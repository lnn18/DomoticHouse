#ifndef LCD_H
#define LCD_H

#include <Arduino.h>

// Inicia la pantalla LCD 16x2 en modo paralelo de 4 bits (llamar una vez en setup()).
// Si la pantalla no esta conectada no se cuelga: simplemente no se ve nada.
void inicializarLcd();

// Escribe "texto" en la fila indicada (0 o 1), completando con espacios hasta el ancho de
// la pantalla. Solo toca la pantalla si el contenido de esa fila cambio respecto a la
// ultima escritura: cada escritura bloquea el loop un instante, y el sensor de pulso
// necesita muestrearse cada ~2ms.
void lcdLinea(uint8_t fila, const char* texto);

#endif
