#ifndef ACCESS_H
#define ACCESS_H

// ============================================================================
//  ACCESS (puerta)
//  Que hace este archivo: controla la puerta con un servomotor (un motorcito
//  que gira a un angulo exacto y mueve el pestillo) y un boton de apertura.
//
//  Como funciona: al presionar el boton, el servo gira y abre la puerta. Pasado
//  un tiempo (ver DURACION_PUERTA_ABIERTA_MS en config.h) la puerta se cierra sola.
//
//  Aviso: el boton abre SIN pedir autorizacion. Debe quedar del lado de ADENTRO
//  de la puerta; si queda al alcance desde afuera, cualquiera podria entrar.
// ============================================================================

#include <Arduino.h>

// Prepara el servo del pestillo y lo deja en la posicion cerrada.
// Se llama UNA vez al arrancar. "pinServo" es el pin donde va su cable de senal.
void inicializarAcceso(uint8_t pinServo);

// Prepara el boton de apertura. Se llama UNA vez al arrancar.
// El boton se conecta entre el pin y tierra (GND), sin resistencia externa.
void inicializarBotonPuerta(uint8_t pinBoton);

// Se llama en cada vuelta del programa. Hace dos cosas:
//   1) Mira si presionaron el boton (filtrando rebotes) y, si es asi, abre la puerta.
//   2) Cierra la puerta sola cuando termina su tiempo abierta.
// No usa pausas: el programa sigue atendiendo todo lo demas mientras tanto.
void actualizarAcceso();

#endif
