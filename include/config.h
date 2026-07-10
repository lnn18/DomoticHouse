#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// Este archivo reune todas las constantes del proyecto: en que pin esta conectado
// cada componente, y algunos parametros de comportamiento (umbrales, tiempos). Asi,
// si algun dia se cambia un cable de lugar, solo hay que tocar este archivo.
//
// Se usa "constexpr" en vez de "#define" porque constexpr respeta tipos (uint8_t,
// int, etc.) y el compilador puede detectar errores que un simple #define no veria.

// Buzzer (altavoz pequeno que suena cuando hay una alerta)
constexpr uint8_t PIN_BUZZER = 4;

// Sensor ultrasonico 1 (HC-SR04): TRIG emite el pulso, ECHO recibe el eco
constexpr uint8_t PIN_TRIG_1 = 9;
constexpr uint8_t PIN_ECHO_1 = 10;

// Sensor ultrasonico 2 (HC-SR04)
constexpr uint8_t PIN_TRIG_2 = 7;
constexpr uint8_t PIN_ECHO_2 = 8;

// Registro de desplazamiento 74HC595 (controla la barra de LEDs usando solo 3 pines)
// Solo se usan Q0 y Q1 (2 LEDs); el resto de salidas quedan siempre apagadas.
constexpr uint8_t PIN_CLOCK_REGISTRO = 11; // Sincroniza el envio de cada bit (SHCP)
constexpr uint8_t PIN_LATCH_REGISTRO = 12; // Ordena "mostrar" los bits ya recibidos (STCP)
constexpr uint8_t PIN_DATOS_REGISTRO = 13; // Por aqui se envian los bits, uno por uno (DS)

// Distancia (cm) a partir de la cual un sensor considera que "detecto algo".
// Si el objeto esta a esta distancia o mas cerca, se activa la alerta.
constexpr int UMBRAL_DETECCION_CM = 20;

// Tiempo maximo de espera de pulseIn (~515 cm), evita bloqueos largos si no hay eco.
// Si el sonido no vuelve en este tiempo, se asume que no hay ningun objeto cerca.
constexpr unsigned long TIMEOUT_PULSO_US = 30000UL;

// Intervalo no bloqueante entre lecturas de los sensores (en milisegundos).
// No hace falta leer los sensores todo el tiempo; cada 100 ms es mas que suficiente
// y deja "aire" al procesador para otras tareas.
constexpr unsigned long INTERVALO_LECTURA_MS = 100UL;

#endif
