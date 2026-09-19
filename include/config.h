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

// Sensor ultrasonico 1 (SRF-05, en modo de 4 pines compatible HC-SR04): TRIG emite el
// pulso, ECHO recibe el eco. El pin OUT/Mode del SRF-05 se deja sin conectar.
//
// NO USA D9/D10 a proposito: la libreria Servo.h (ver PIN_SERVO_PUERTA mas abajo)
// reconfigura Timer1 del ATmega328P apenas se hace attach() de CUALQUIER servo, sin
// importar a que pin este atado ese servo -- el Uno solo tiene un timer disponible para
// servos. Esto deja D9/D10 inservibles para pulseIn() mientras el servo de la puerta este
// activo (sintoma observado: Sensor 1 en timeout constante, -1cm, con RFID/servo activos).
// Se mueve el sensor 1 a A2/A3 (pines analogicos usables como digitales, sin relacion con
// Timer1) y el servo pasa a D9 (ver PIN_SERVO_PUERTA), que ya no le hace falta a nadie mas.
constexpr uint8_t PIN_TRIG_1 = A2;
constexpr uint8_t PIN_ECHO_1 = A3;

// Sensor ultrasonico 2 (SRF-05, mismo modo de 4 pines)
constexpr uint8_t PIN_TRIG_2 = 7;
constexpr uint8_t PIN_ECHO_2 = 8;

// LED de panico: antes vivia en el registro 74HC595 (Q2), pero ese registro se quito
// para liberar el bus SPI (D11/D12/D13) que ahora usa el lector RC522. La alarma de
// proximidad ya no tiene LED propio, solo el buzzer (ver actualizarBuzzerCompartido en main.cpp).
constexpr uint8_t PIN_LED_PANICO = A1;

// Distancia (cm) a partir de la cual un sensor considera que "detecto algo".
// Si el objeto esta a esta distancia o mas cerca, se activa la alerta.
constexpr int UMBRAL_DETECCION_CM = 20;

// Tiempo maximo de espera de pulseIn (~515 cm), evita bloqueos largos si no hay eco.
// Si el sonido no vuelve en este tiempo, se asume que no hay ningun objeto cerca.
constexpr unsigned long TIMEOUT_PULSO_US = 30000UL;

// Intervalo no bloqueante entre lecturas de los sensores (en milisegundos).
// No hace falta leer los sensores todo el tiempo; cada 1 s es mas que suficiente
// y deja "aire" al procesador para otras tareas.
constexpr unsigned long INTERVALO_LECTURA_MS = 1000UL;

// --- Sensor de presencia en el pasillo (3er SRF-05, en modo de 4 pines) ---
// El pasillo usaba un PIR (deteccion real de movimiento); se cambio a un SRF-05 para
// reutilizar el mismo modelo de sensor que proximidad (se tenian 3 SRF-05 disponibles,
// solo se usaban 2). Esto cambia la semantica: ya no es "detecto movimiento", sino
// "detecto algo a menos de UMBRAL_PASILLO_CM" -- no distingue una persona quieta de un
// objeto fijo (una pared, un mueble) que quede dentro del rango del sensor.
// PIN_TRIG_PASILLO NO usa D3 a proposito: D2 y D3 son los unicos pines de interrupcion
// externa (INT0/INT1) del Uno, y D3 esta reservado para el boton de panico.
constexpr uint8_t PIN_TRIG_PASILLO = 2;  // Antes usado por el PIR (OUT); ahora TRIG del SRF-05
constexpr uint8_t PIN_ECHO_PASILLO = 5;  // Libre desde que se desactivo la alarma de cama
constexpr uint8_t PIN_LUZ_PASILLO = 6;   // Controla la luz del pasillo (LED/rele)
constexpr int UMBRAL_PASILLO_CM = 10;    // Distancia a partir de la cual se considera "hay alguien"
constexpr unsigned long DURACION_LUZ_PASILLO_MS = 15000UL; // Tiempo que la luz queda encendida tras la ultima deteccion

// --- Sensor de presion en la cama (FSR) ---
// DESACTIVADO TEMPORALMENTE: toda la logica de alarma de cama esta comentada en main.cpp.
// Se deja esta configuracion intacta para poder reactivarla mas adelante sin tener que
// rehacerla. PIN_LUZ_ALARMA_CAMA (5) queda libre mientras tanto.
//
// LIMITACION CONOCIDA (documentada por si se reactiva): si el FSR se desconecta
// fisicamente, analogRead() puede caer por debajo de UMBRAL_PRESION_CAMA_ADC igual que
// "cama vacia", y el sistema lo interpretaria (en silencio) como que la persona se
// levanto. Distinguir "sensor desconectado" de "sin presion" con un solo FSR requiere
// un circuito adicional (ver iot-elder-care-safety); queda documentado como riesgo
// pendiente, no resuelto en firmware.
// constexpr uint8_t PIN_FSR_CAMA = A0;         // Lectura analogica de presion
// constexpr uint8_t PIN_LUZ_ALARMA_CAMA = 5;   // Luz de alarma si se demora en levantarse
// constexpr int UMBRAL_PRESION_CAMA_ADC = 300; // Lectura (0-1023) a partir de la cual se considera "en cama"
// constexpr unsigned long DEBOUNCE_PRESION_CAMA_MS = 2000UL; // Filtra ruido momentaneo de la lectura analogica
// constexpr unsigned long TOLERANCIA_AUSENCIA_CAMA_MS = 60000UL; // Una ausencia mas corta que esto (ej. un giro en la cama) no cuenta como "se levanto": no reinicia el conteo de 7 horas
// constexpr unsigned long TIEMPO_MAX_EN_CAMA_MS = 7UL * 60UL * 60UL * 1000UL; // 7 horas seguidas en cama -> alarma

// --- Fotocelda (LDR): detecta si es de noche, para restringir la luz de pasillo y la ---
// --- alarma de proximidad a que solo reaccionen en la oscuridad.                     ---
// Reutiliza A0, que quedo libre al desactivar la alarma de cama. Si se reactiva esa
// alarma, la fotocelda deberia moverse a otro pin analogico libre (ver CLAUDE.md).
constexpr uint8_t PIN_FOTOCELDA = A0;
constexpr int UMBRAL_OSCURIDAD_ADC = 500; // Lectura (0-1023) a partir de la cual se considera "de noche"; ajustar segun el LDR y la resistencia usados
constexpr unsigned long DEBOUNCE_FOTOCELDA_MS = 2000UL; // Filtra ruido momentaneo de la lectura analogica (ej. una sombra pasajera)

// --- Boton de panico ---
// Usa D3 (INT1), el pin de interrupcion que se dejo libre a proposito (ver PIN_LUZ_PASILLO
// mas arriba). Es la alarma de mayor prioridad: silencia cualquier otra alarma en el buzzer
// compartido. El boton funciona como interruptor (toggle): un pulso activa la alarma, el
// siguiente la desactiva; asi no depende de que otro componente la apague.
//
// LIMITACION CONOCIDA (igual espiritu que PIN_FSR_CAMA): con INPUT_PULLUP, un cable cortado
// o el boton desconectado se lee igual que "nunca presionado" (el pin queda en HIGH de forma
// estable). No hay forma de distinguir en firmware "boton sano, sin presionar" de "boton
// desconectado" sin hardware adicional (ej. resistencia de deteccion de continuidad, o un
// diseno normalmente cerrado). Riesgo pendiente, no resuelto aqui.
constexpr uint8_t PIN_BOTON_PANICO = 3;
constexpr unsigned long DEBOUNCE_BOTON_PANICO_MS = 50UL;

// --- Frecuencias del buzzer, distintas por tipo de alarma para diferenciarlas de oido ---
// FRECUENCIA_ALARMA_PANICO_HZ es la mas alta y aguda a proposito: debe ser la mas
// facil de reconocer, ya que indica la emergencia de mayor prioridad.
constexpr unsigned int FRECUENCIA_ALARMA_PANICO_HZ = 3000;
constexpr unsigned int FRECUENCIA_ALARMA_PROXIMIDAD_HZ = 2000;
// constexpr unsigned int FRECUENCIA_ALARMA_CAMA_HZ = 800; // Desactivada junto con la alarma de cama

// --- Control de acceso: lector RFID RC522 + servo de la puerta ---
// El RC522 usa el bus SPI de hardware del Uno (MOSI=D11, MISO=D12, SCK=D13, fijos):
// por eso se quito el registro 74HC595, que antes ocupaba esos mismos pines.
// SS y RST si son configurables por software.
constexpr uint8_t PIN_RFID_SS = A4;
constexpr uint8_t PIN_RFID_RST = A5;

// Servo que mueve el pestillo de la puerta. No requiere un pin PWM de hardware:
// la libreria Servo.h genera la señal por software en cualquier pin digital.
// Va en D9 (antes ocupado por el sensor 1, ver PIN_TRIG_1 arriba): el servo igual reconfigura
// Timer1 este donde este, asi que D9 no lo perjudica, y libera A2/A3 para el sensor 1.
constexpr uint8_t PIN_SERVO_PUERTA = 9;
constexpr uint8_t ANGULO_PUERTA_CERRADA = 0;
constexpr uint8_t ANGULO_PUERTA_ABIERTA = 90;
constexpr unsigned long DURACION_PUERTA_ABIERTA_MS = 20000UL; // Tiempo que la puerta queda abierta antes de cerrarse sola

#endif
