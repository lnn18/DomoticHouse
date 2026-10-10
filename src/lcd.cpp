#include "lcd.h"
#include "config.h"
#include <LiquidCrystal.h>
#include <string.h>

namespace {
// Conexion paralela de 4 bits (ver el cableado de los 16 pines en config.h).
LiquidCrystal LCD(PIN_LCD_RS, PIN_LCD_E, PIN_LCD_D4, PIN_LCD_D5, PIN_LCD_D6, PIN_LCD_D7);

// Ultimo contenido escrito en cada fila (con el terminador), para evitar reescribir lo mismo.
char CACHE_FILAS[LCD_FILAS][LCD_COLUMNAS + 1];
}

void inicializarLcd() {
  LCD.begin(LCD_COLUMNAS, LCD_FILAS);
  LCD.clear();
  for (uint8_t f = 0; f < LCD_FILAS; f++) {
    CACHE_FILAS[f][0] = '\0';
  }
}

void lcdLinea(uint8_t fila, const char* texto) {
  if (fila >= LCD_FILAS) {
    return;
  }

  char linea[LCD_COLUMNAS + 1];
  uint8_t i = 0;
  while (i < LCD_COLUMNAS && texto[i] != '\0') {
    linea[i] = texto[i];
    i++;
  }
  while (i < LCD_COLUMNAS) {
    linea[i++] = ' ';
  }
  linea[LCD_COLUMNAS] = '\0';

  if (strcmp(linea, CACHE_FILAS[fila]) == 0) {
    return;
  }
  LCD.setCursor(0, fila);
  LCD.print(linea);
  strcpy(CACHE_FILAS[fila], linea);
}
