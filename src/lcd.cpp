#include "lcd.h"
#include "config.h"
#include <LiquidCrystal.h>
#include <string.h>

namespace {
// "namespace { ... }" significa: lo que esta aqui adentro es PRIVADO de este archivo.
// Ningun otro archivo del proyecto puede tocarlo por error.

// El objeto que representa la pantalla. Se le dice a que pines esta conectada:
// RS, E (control) y D4 a D7 (los 4 cables de datos). Ver los numeros en config.h.
LiquidCrystal LCD(PIN_LCD_RS, PIN_LCD_E, PIN_LCD_D4, PIN_LCD_D5, PIN_LCD_D6, PIN_LCD_D7);

// Memoria del ultimo texto escrito en cada fila. Sirve para no volver a escribir
// lo mismo (el +1 es el espacio para el caracter que marca el fin del texto).
char CACHE_FILAS[LCD_FILAS][LCD_COLUMNAS + 1];
}

// Prepara la pantalla.
void inicializarLcd() {
  LCD.begin(LCD_COLUMNAS, LCD_FILAS); // Le dice a la pantalla cuantas columnas y filas tiene
  LCD.clear();                        // La deja en blanco
  // Borra la memoria de lo escrito: asi la primera escritura siempre se muestra.
  for (uint8_t f = 0; f < LCD_FILAS; f++) {
    CACHE_FILAS[f][0] = '\0';
  }
}

// Escribe un texto en una fila (explicacion completa en lcd.h).
void lcdLinea(uint8_t fila, const char* texto) {
  // Si piden una fila que no existe, no hacemos nada (proteccion).
  if (fila >= LCD_FILAS) {
    return;
  }

  // Armamos una linea de exactamente 16 letras: copiamos el texto y rellenamos
  // con espacios lo que falte, para que quede borrado lo que habia antes.
  char linea[LCD_COLUMNAS + 1];
  uint8_t i = 0;
  while (i < LCD_COLUMNAS && texto[i] != '\0') {
    linea[i] = texto[i];
    i++;
  }
  while (i < LCD_COLUMNAS) {
    linea[i++] = ' ';
  }
  linea[LCD_COLUMNAS] = '\0'; // Marca de fin de texto

  // Si es igual a lo ultimo que escribimos, no hace falta volver a escribirlo.
  if (strcmp(linea, CACHE_FILAS[fila]) == 0) {
    return;
  }
  LCD.setCursor(0, fila);          // Se para al inicio de la fila
  LCD.print(linea);                // Escribe el texto
  strcpy(CACHE_FILAS[fila], linea); // Recuerda lo escrito
}
