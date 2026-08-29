#ifndef ACCESS_H
#define ACCESS_H

#include <Arduino.h>

// Control de acceso a la puerta: lector RFID RC522 + servo del pestillo.
//
// Version inicial de prueba: CUALQUIER tarjeta/tag valido (leido correctamente por el
// RC522) abre la puerta. Todavia no hay lista de UIDs autorizados -- se agregara como
// paso siguiente, una vez verificado que el lector y el servo funcionan bien juntos.

// Configura el RC522 (SPI + pines SS/RST) y el servo de la puerta (llamar una vez en setup()).
void inicializarAcceso(uint8_t pinRfidSs, uint8_t pinRfidRst, uint8_t pinServo);

// Debe llamarse en cada vuelta de loop(). Revisa si hay una tarjeta nueva presente:
// si la hay, abre la puerta. Tambien se encarga de cerrarla sola tras
// DURACION_PUERTA_ABIERTA_MS (no bloqueante, via millis()).
void actualizarAcceso();

#endif
