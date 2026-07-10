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

// --- Sensor PIR de movimiento (luz del pasillo) ---
// PIN_LUZ_PASILLO NO usa D3 a proposito: D2 y D3 son los unicos pines de interrupcion
// externa (INT0/INT1) del Uno, y el roadmap los reserva para el boton de panico
// (ver "Seguridad/Emergencias" en CLAUDE.md). Una luz es una simple salida y no
// necesita ser un pin de interrupcion, asi que se deja libre D3 para el futuro boton.
constexpr uint8_t PIN_PIR_PASILLO = 2;   // Salida digital del PIR (HIGH = detecto movimiento)
constexpr uint8_t PIN_LUZ_PASILLO = 6;   // Controla la luz del pasillo (LED/rele)
constexpr unsigned long DEBOUNCE_PIR_MS = 50UL; // Anti-rebote, igual que un boton
constexpr unsigned long DURACION_LUZ_PASILLO_MS = 15000UL; // Tiempo que la luz queda encendida tras el ultimo movimiento

// --- Sensor de presion en la cama (FSR) ---
// LIMITACION CONOCIDA: si el FSR se desconecta fisicamente, analogRead() puede caer
// por debajo de UMBRAL_PRESION_CAMA_ADC igual que "cama vacia", y el sistema lo
// interpretaria (en silencio) como que la persona se levanto. Distinguir "sensor
// desconectado" de "sin presion" con un solo FSR requiere un circuito adicional
// (ver iot-elder-care-safety); queda documentado como riesgo pendiente, no resuelto
// en firmware.
constexpr uint8_t PIN_FSR_CAMA = A0;         // Lectura analogica de presion
constexpr uint8_t PIN_LUZ_ALARMA_CAMA = 5;   // Luz de alarma si se demora en levantarse
constexpr int UMBRAL_PRESION_CAMA_ADC = 300; // Lectura (0-1023) a partir de la cual se considera "en cama"
constexpr unsigned long DEBOUNCE_PRESION_CAMA_MS = 2000UL; // Filtra ruido momentaneo de la lectura analogica
constexpr unsigned long TOLERANCIA_AUSENCIA_CAMA_MS = 60000UL; // Una ausencia mas corta que esto (ej. un giro en la cama) no cuenta como "se levanto": no reinicia el conteo de 7 horas
constexpr unsigned long TIEMPO_MAX_EN_CAMA_MS = 7UL * 60UL * 60UL * 1000UL; // 7 horas seguidas en cama -> alarma

// --- Frecuencias del buzzer, distintas por tipo de alarma para diferenciarlas de oido ---
constexpr unsigned int FRECUENCIA_ALARMA_PROXIMIDAD_HZ = 2000;
constexpr unsigned int FRECUENCIA_ALARMA_CAMA_HZ = 800;

#endif
