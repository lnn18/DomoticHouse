#ifndef LCD_H
#define LCD_H

// ============================================================================
//  LCD (pantalla de texto de 16 columnas x 2 filas)
//  Que hace este archivo: ofrece dos funciones sencillas para escribir texto en
//  la pantalla. Quien la use no necesita saber como se conecta ni como funciona
//  por dentro.
//
//  La pantalla muestra, por ejemplo, el estado del switch del pulso y el
//  resultado de la ultima medicion de pulso (ver pulse.cpp).
// ============================================================================

#include <Arduino.h>

// Prepara la pantalla y la deja en blanco. Se llama UNA sola vez al arrancar.
// Si la pantalla no esta conectada, el programa no se cuelga: simplemente no se ve nada.
void inicializarLcd();

// Escribe un texto en una fila de la pantalla.
//   fila  : 0 = la fila de arriba, 1 = la fila de abajo
//   texto : lo que se quiere mostrar (maximo 16 letras; si es mas corto, se
//           completa con espacios para borrar lo que habia antes)
//
// Detalle importante: si el texto es IGUAL al que ya estaba escrito, no hace
// nada. Escribir en la pantalla tarda un poquito y el sensor de pulso necesita
// que el programa no se demore, asi que solo se escribe cuando algo cambia.
void lcdLinea(uint8_t fila, const char* texto);

#endif
