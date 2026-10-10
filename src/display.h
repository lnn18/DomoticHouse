#ifndef DISPLAY_H
#define DISPLAY_H

// ============================================================================
//  DISPLAY (luces de aviso, LEDs)
//  Que hace este archivo: maneja los DOS LEDs de aviso del sistema:
//    - LED de panico: se prende cuando alguien presiona el boton de panico.
//    - LED de proximidad: se prende cuando algo se acerca a un sensor de noche.
//
//  Cada LED es un "segundo canal" de su alarma: ademas del sonido del buzzer, la
//  persona tambien VE el aviso. Asi, si no escucha, igual se entera.
//
//  Nota: la pantalla de texto (LCD) NO esta aqui, esta en lcd.h.
// ============================================================================

#include <Arduino.h>

// Prepara los pines de los dos LEDs. Se llama UNA sola vez al arrancar (setup()).
void inicializarDisplay(uint8_t pinLedPanico, uint8_t pinLedProximidad);

// Enciende o apaga cada LED segun corresponda:
//   panicoActivo      -> true = prende el LED de panico, false = lo apaga
//   proximidadActiva  -> true = prende el LED de proximidad, false = lo apaga
// El LED de proximidad se prende con las mismas condiciones que el buzzer de
// proximidad (algo cerca Y es de noche), no con el boton de panico.
void actualizarLeds(uint8_t pinLedPanico, bool panicoActivo, uint8_t pinLedProximidad, bool proximidadActiva);

#endif
