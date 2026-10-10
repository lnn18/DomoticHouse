#ifndef PULSE_H
#define PULSE_H

// ============================================================================
//  PULSE (pulso cardiaco)
//  Que hace este archivo: mide los latidos del corazon (BPM = latidos por
//  minuto) con un sensor que se pone en la yema del dedo, y avisa si el valor
//  sale de lo normal.
//
//  Como se usa (para la persona):
//    - Switch en OFF: no mide nada.
//    - Switch en ON : mide durante 10 segundos, muestra el resultado en la
//                     pantalla y repite mientras el switch siga en ON.
//    - Si el resultado es muy bajo o muy alto, suena el buzzer.
//
//  El sensor entrega una "onda" que sube y baja con cada latido. El programa
//  la lee muchas veces por segundo y detecta cada subida para contar latidos.
// ============================================================================

#include <Arduino.h>

// Prepara el sensor de pulso y su switch ON/OFF.
// Se llama UNA vez al arrancar, DESPUES de inicializarLcd() (porque muestra
// mensajes en la pantalla).
void inicializarPulso(uint8_t pinSensor, uint8_t pinSwitch);

// Se llama en cada vuelta del programa y lo mas seguido posible mientras se mide
// (el sensor necesita leerse cada ~2 milisegundos).
// Atiende el switch, la medicion de 10 s, la pantalla y la alarma.
// Devuelve true si la alarma de pulso esta activa (el ultimo resultado valido
// estuvo fuera del rango normal).
bool actualizarPulso();

// Devuelve true mientras hay una medicion en curso.
// main.cpp lo usa para pausar tareas lentas (los sensores de distancia y los
// mensajes por la computadora), que le harian perder latidos al sensor.
bool midiendoPulso();

#endif
