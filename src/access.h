#ifndef ACCESS_H
#define ACCESS_H

#include <Arduino.h>

// Control de acceso a la puerta: lector RFID RC522 + servo del pestillo.
//
// Solo abre la puerta un UID que este en UIDS_AUTORIZADOS (ver config.h). Cualquier otra
// tarjeta/tag leido correctamente se registra por Serial pero NO abre la puerta.

// Configura el RC522 (SPI + pines SS/RST) y el servo de la puerta (llamar una vez en setup()).
void inicializarAcceso(uint8_t pinRfidSs, uint8_t pinRfidRst, uint8_t pinServo);

// Debe llamarse en cada vuelta de loop(). Revisa si hay una tarjeta nueva presente:
// si la hay, abre la puerta. Tambien se encarga de cerrarla sola tras
// DURACION_PUERTA_ABIERTA_MS (no bloqueante, via millis()).
void actualizarAcceso();

#endif
