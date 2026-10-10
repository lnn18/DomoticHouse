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
// para liberar el bus SPI (D11/D12/D13) que ahora usa el lector RC522.
constexpr uint8_t PIN_LED_PANICO = A1;

// LED de proximidad: canal visual para la alarma de los sensores externos (1 y 2), que
// hasta ahora solo tenia buzzer (ver iot-elder-care-safety: una alarma no deberia depender
// de un unico canal). Se enciende/apaga junto con ALERTA_ACTIVA en main.cpp -- mismas
// condiciones que el buzzer de proximidad (objeto cerca, segun el umbral de cada sensor,
// ver UMBRAL_DETECCION_1_CM/UMBRAL_DETECCION_2_CM, Y de noche).
// Va en D10: libre desde que el sensor 1 se movio a A2/A3. Sirve para un LED simple
// (digitalWrite, sin PWM), asi que no le afecta que Servo.h use Timer1 en D9/D10 --
// ese conflicto es solo para pulseIn()/analogWrite(), no para digitalWrite() (ver config.h,
// nota de PIN_TRIG_1, y CLAUDE.md).
constexpr uint8_t PIN_LED_PROXIMIDAD = 10;

// Distancia (cm) a partir de la cual cada sensor de proximidad considera que "detecto
// algo". Si el objeto esta a esta distancia o mas cerca, se activa la alerta. Umbrales
// separados por sensor (antes uno solo, UMBRAL_DETECCION_CM, compartido entre los dos):
// el sensor 2 (D7/D8) quedo a proposito mas sensible (8cm) que el sensor 1 (A2/A3, 20cm)
// a pedido, por su ubicacion/uso especifico en el montaje.
constexpr int UMBRAL_DETECCION_1_CM = 20;
constexpr int UMBRAL_DETECCION_2_CM = 8;

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
constexpr uint8_t PIN_ECHO_PASILLO = 5;  // Con la alarma de cama reactivada, D5 vuelve a ser SOLO de este sensor -- ver PIN_LUZ_ALARMA_CAMA mas abajo (se movio a D9 para no chocar aca)
constexpr uint8_t PIN_LUZ_PASILLO = 6;   // Controla la luz del pasillo (LED/rele)
constexpr int UMBRAL_PASILLO_CM = 10;    // Distancia a partir de la cual se considera "hay alguien"
constexpr unsigned long DURACION_LUZ_PASILLO_MS = 15000UL; // Tiempo que la luz queda encendida tras la ultima deteccion

// --- Sensor de presion en la cama: SIMULADO CON SWITCH (temporal, en lugar del FSR real) ---
// El sensor de presion (FSR) todavia no esta armado fisicamente. Mientras tanto, se simula
// la señal "hay presion / no hay presion" con un interruptor simple, igual patron que el
// boton de panico (INPUT_PULLUP + digitalRead, sin resistencia externa): switch cerrado =
// "hay alguien en la cama", abierto = "cama vacia". El resto de la logica (7 horas seguidas,
// tolerancia de ausencia corta) es identica a la pensada para el FSR real.
//
// PIN_FSR_CAMA NO va en A0 (lo documentado originalmente) porque A0 ya lo usa la fotocelda,
// y NO va en ningun pin digital 2-13 porque estan todos ocupados por otros modulos. Va en
// A4, que en el Uno era PIN_RFID_SS. En el Mega el RFID ya no lo usa (su SS es D53, ver mas
// abajo), asi que A4 es solo de este switch y no hay conflicto al reactivar el acceso.
//
// LIMITACION CONOCIDA (aplica igual con switch que con FSR real, mismo espiritu que
// PIN_BOTON_PANICO): con INPUT_PULLUP, un cable cortado o el switch desconectado se lee
// igual que "cama vacia" (pin en HIGH estable) -- no hay forma de distinguir en firmware
// "switch sano, sin presionar" de "switch desconectado" sin hardware adicional. Con el FSR
// real ademas existe el riesgo inverso ya documentado antes (ver historial): una lectura
// analogica baja por desconexion se confunde con "cama vacia" en silencio. Riesgo pendiente,
// no resuelto en firmware en ninguno de los dos casos.
constexpr uint8_t PIN_FSR_CAMA = A4;         // Switch (temporal) o FSR (definitivo) de presion en la cama
// PIN_LUZ_ALARMA_CAMA NO va en D5 (lo documentado originalmente): D5 ya lo usa
// PIN_ECHO_PASILLO, y con la alarma de cama reactivada ambos pisarian el mismo pin
// (uno como INPUT para leer eco, el otro como OUTPUT para la luz -- se reconfigurarian
// mutuamente cada vez que el otro modulo se ejecuta, corrompiendo la lectura del sensor
// de pasillo). Va en D9, que en el Uno era PIN_SERVO_PUERTA. En el Mega el servo paso a D11
// (ver mas abajo), asi que D9 es solo de esta luz y no hay conflicto con el servo.
constexpr uint8_t PIN_LUZ_ALARMA_CAMA = 9;   // Luz de alarma si se demora en levantarse
constexpr unsigned long DEBOUNCE_PRESION_CAMA_MS = 50UL; // Antirrebote mecanico del switch (mismo valor que DEBOUNCE_BOTON_PANICO_MS; con el FSR real, un antirrebote mas largo -- ver nota abajo -- es mas apropiado para ruido analogico, no mecanico)

// DEBUG TEMPORAL (prueba de banco): 3 segundos en vez de 60, para poder "reiniciar" la
// prueba rapido con solo abrir el switch, en vez de esperar 1 minuto real cada vez.
// IMPORTANTE: restaurar a 60000UL antes de dar el sistema por terminado para uso real --
// con un valor tan chico, una ausencia breve (ej. alguien se da vuelta en la cama, en el
// uso real) ya cuenta como "se levanto" y reinicia el conteo de horas, que es exactamente
// lo que TOLERANCIA_AUSENCIA_CAMA_MS existe para evitar.
constexpr unsigned long TOLERANCIA_AUSENCIA_CAMA_MS = 3UL * 1000UL; // PRUEBA: 3 segundos (valor real: 60000UL)

// DEBUG TEMPORAL (prueba de banco): 5 segundos en vez de 7 horas, para poder probar la
// alarma de cama sin esperar el tiempo real. IMPORTANTE: restaurar a
// "7UL * 60UL * 60UL * 1000UL" (7 horas) antes de dar el sistema por terminado para uso real.
constexpr unsigned long TIEMPO_MAX_EN_CAMA_MS = 5UL * 1000UL; // PRUEBA: 5 segundos (valor real: 7UL * 60UL * 60UL * 1000UL)
//
// NOTA para cuando se reemplace el switch por el FSR real: volver DEBOUNCE_PRESION_CAMA_MS
// a un valor mayor (2000UL en el diseño original) -- un FSR analogico tiene ruido de lectura
// continuo, muy distinto al rebote mecanico de un switch, y necesita mas tiempo de filtrado.
// Tambien habria que reintroducir UMBRAL_PRESION_CAMA_ADC (ya no aplica con un switch digital
// simple) y cambiar hayPersonaEnCama() de digitalRead() a analogRead() con ese umbral.

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
constexpr unsigned int FRECUENCIA_ALARMA_CAMA_HZ = 800;

// --- Sensor de pulso cardiaco (PulseSensor / Electronilab SNB004), bajo demanda ---
// Solo disponible en el Mega (A8 no existe en el Uno). Bajo demanda, controlado por un
// switch de 2 posiciones en PIN_SWITCH_PULSO: con el switch en OFF no se mide; en ON se mide
// en ciclos de DURACION_MEDICION_PULSO_MS, mostrando el resultado en la LCD al final de
// cada ciclo, hasta que se vuelva a OFF. Si el BPM de un ciclo sale de [BPM_MIN_NORMAL,
// BPM_MAX_NORMAL] suena el buzzer hasta que un ciclo posterior sea normal o el switch
// pase a OFF. Prioridad en el buzzer: panico > cama > pulso > proximidad (ver
// actualizarBuzzerCompartido en main.cpp).
// Durante la medicion main.cpp pausa los ultrasonicos y los logs seriales: bloquean el loop
// decenas de ms y el sensor necesita muestrearse cada ~2ms. Entre ciclos hay una pausa
// corta (PAUSA_ENTRE_MEDICIONES_MS) en la que los ultrasonicos si leen.
constexpr uint8_t PIN_SENSOR_PULSO = A8;   // Cable morado (senal analogica); rojo a 5V, negro a GND
constexpr uint8_t PIN_SWITCH_PULSO = 12;   // Switch de 2 posiciones a GND, INPUT_PULLUP (igual que el de la cama): cerrado = ON
constexpr unsigned long DEBOUNCE_SWITCH_PULSO_MS = 50UL;
constexpr unsigned long DURACION_MEDICION_PULSO_MS = 20000UL; // Duracion de cada ciclo de medicion (y cada cuanto se actualiza la LCD)
constexpr unsigned long PAUSA_ENTRE_MEDICIONES_MS = 1000UL;   // Ventana entre ciclos donde los ultrasonicos pueden leer
constexpr uint8_t LATIDOS_MINIMOS_MEDICION = 5;               // Menos latidos que esto = medicion invalida (dedo mal puesto), no se alarma
constexpr int BPM_MIN_NORMAL = 50;         // Por debajo de esto se considera bradicardia -> alarma
constexpr int BPM_MAX_NORMAL = 120;        // Por encima de esto se considera taquicardia -> alarma
constexpr unsigned int FRECUENCIA_ALARMA_PULSO_HZ = 1200; // Distinta de panico (3000), proximidad (2000) y cama (800)
// DEBUG TEMPORAL (calibracion): imprime la senal cruda (0-1023) cada 20ms durante la
// medicion y una linea por cada latido detectado. Es mucho texto: dejar en false salvo
// que haga falta calibrar el sensor (el avance por segundo y el resultado final se
// imprimen siempre).
constexpr bool DEBUG_PULSO_SENAL = false;

// --- Pantalla LCD 16x2 en paralelo, modo de 4 bits (HD44780 sin modulo I2C, 16 pines) ---
// Pines D4-D7 de la pantalla (los D0-D3 quedan sin conectar), RS y E. RW va a GND (solo se
// escribe). Se usa D22-D27 del Mega, bloque contiguo libre, y no los pines bajos (D2-D13)
// que ya ocupa el resto del sistema.
// Cableado fisico de los 16 pines de la pantalla:
//   1 VSS -> GND | 2 VDD -> 5V | 3 V0 -> contraste (cursor de un potenciometro de 10k entre
//   5V y GND) | 4 RS -> PIN_LCD_RS | 5 RW -> GND | 6 E -> PIN_LCD_E | 7-10 D0-D3 -> sin conectar
//   11 D4 -> PIN_LCD_D4 | 12 D5 -> PIN_LCD_D5 | 13 D6 -> PIN_LCD_D6 | 14 D7 -> PIN_LCD_D7
//   15 A (luz de fondo +) -> 5V con una resistencia de 220 ohm | 16 K (luz de fondo -) -> GND
constexpr uint8_t PIN_LCD_RS = 22;
constexpr uint8_t PIN_LCD_E = 23;
constexpr uint8_t PIN_LCD_D4 = 24;
constexpr uint8_t PIN_LCD_D5 = 25;
constexpr uint8_t PIN_LCD_D6 = 26;
constexpr uint8_t PIN_LCD_D7 = 27;
constexpr uint8_t LCD_COLUMNAS = 16;
constexpr uint8_t LCD_FILAS = 2;

// --- Control de acceso: lector RFID RC522 + servo de la puerta ---
// ACTIVO en el Mega. (En el Uno estuvo pausado: A4/D9 se reutilizaron para la cama.)
//
// El RC522 usa el bus SPI de HARDWARE del Mega, con pines fijos que NO se pueden cambiar:
//   MOSI = D51 | MISO = D50 | SCK = D52 | SS de hardware = D53 (debe quedar como salida)
// SS (SDA en la placa) y RST si son configurables por software.
//
// ALIMENTACION: el RC522 es de 3.3V. Conectarlo a 5V lo daña. El Mega tiene un pin 3.3V.
//   RC522:  SDA -> D53 | SCK -> D52 | MOSI -> D51 | MISO -> D50 | IRQ -> sin conectar
//           GND -> GND | RST -> D49 | 3.3V -> 3.3V del Mega
constexpr uint8_t PIN_RFID_SS = 53;
constexpr uint8_t PIN_RFID_RST = 49;

// UIDs de tarjetas/tags autorizados a abrir la puerta. Cada UID es un arreglo de bytes de
// tamano fijo (los RFID de 13.56MHz mas comunes -- MIFARE Classic/Ultralight -- usan UID
// de 4 bytes; si tu tarjeta da un UID de 7 bytes, ajustar LONGITUD_UID_BYTES).
//
// COMO OBTENER EL UID REAL: subir el firmware tal cual esta (con la lista vacia, ver
// CANTIDAD_UIDS_AUTORIZADOS = 0 mas abajo), abrir el monitor serial (pio device monitor,
// 9600 baudios) y acercar la tarjeta al RC522. La linea "RFID: tarjeta detectada, UID = .."
// imprime los bytes en hex (ej: "DE AD BE EF") -- copiar esos valores aca, agregar una fila
// a UIDS_AUTORIZADOS, subir el UID de CANTIDAD_UIDS_AUTORIZADOS, y volver a subir el firmware.
constexpr uint8_t LONGITUD_UID_BYTES = 4;
constexpr uint8_t CANTIDAD_UIDS_AUTORIZADOS = 0;
constexpr uint8_t UIDS_AUTORIZADOS[][LONGITUD_UID_BYTES] = {
  // Ejemplo (comentado) de como agregar un UID una vez obtenido del monitor serial:
  // { 0xDE, 0xAD, 0xBE, 0xEF },
  { 0x00, 0x00, 0x00, 0x00 }, // fila de relleno: necesaria para que el arreglo no quede vacio en C++; CANTIDAD_UIDS_AUTORIZADOS=0 hace que access.cpp la ignore
};

// Servo que mueve el pestillo de la puerta. Libreria Servo.h: genera la señal por software
// en cualquier pin digital; en el Mega usa Timer5 (solo afecta al PWM de D44-D46, que no se
// usan), asi que ya no hay conflicto con pulseIn()/tone() como en el Uno.
// Cableado: señal (naranja/amarillo) -> D11 | rojo -> 5V | marron/negro -> GND.
// Un servo SG90 puede pedir picos de ~500mA al moverse: alimentarlo del mismo 5V del Mega
// por USB puede reiniciar la placa. Lo seguro es una fuente de 5V aparte con GND comun, o
// al menos un capacitor de 470uF entre 5V y GND cerca del servo.
constexpr uint8_t PIN_SERVO_PUERTA = 11;
constexpr uint8_t ANGULO_PUERTA_CERRADA = 0;
constexpr uint8_t ANGULO_PUERTA_ABIERTA = 90;
constexpr unsigned long DURACION_PUERTA_ABIERTA_MS = 20000UL; // Tiempo que la puerta queda abierta antes de cerrarse sola

#endif
