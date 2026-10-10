#ifndef ALERTS_H
#define ALERTS_H

// ============================================================================
//  ALERTS (alertas)
//  Que hace este archivo: maneja dos cosas de seguridad:
//    1) El BUZZER: el pequeno altavoz que suena cuando hay una alarma.
//    2) El BOTON DE PANICO: un boton que la persona presiona en una emergencia.
//
//  Como funciona el boton de panico: funciona como un interruptor. Un pulso
//  ACTIVA la alarma y el siguiente pulso la DESACTIVA.
// ============================================================================

#include <Arduino.h>

// Prepara el pin del buzzer y lo deja en silencio. Se llama UNA vez al arrancar.
void inicializarAlerta(uint8_t pinBuzzer);

// Hace sonar el buzzer a una frecuencia (el tono, medido en Hz: mayor numero =
// sonido mas agudo). Si se pide 0, el buzzer se calla.
//
// Importante: el buzzer es UNO SOLO y lo comparten todas las alarmas. Este modulo
// solo obedece: quien decide que alarma suena primero (la prioridad) es main.cpp.
// Tambien evita reiniciar el sonido si se pide el mismo tono que ya suena.
void actualizarBuzzer(uint8_t pinBuzzer, unsigned int frecuenciaHz);

// Prepara el boton de panico. Se llama UNA vez al arrancar.
// El boton usa un pin de "interrupcion": una especie de timbre especial que
// avisa al Arduino al instante, aunque este ocupado haciendo otra cosa.
void inicializarBotonPanico(uint8_t pinBoton);

// Revisa el boton de panico. Se llama en cada vuelta del programa.
// Filtra los "rebotes" (un boton real vibra y manda varios pulsos seguidos al
// presionarlo) y alterna la alarma: un pulso la activa, el siguiente la apaga.
// Devuelve true si la alarma de panico esta activa en este momento.
bool actualizarAlarmaPanico();

#endif
