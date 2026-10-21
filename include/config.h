#ifndef CONFIG_H
#define CONFIG_H

// ============================================================================
//  CONFIG (configuracion del proyecto)
//
//  Que hace este archivo: es la "lista de ajustes" de todo el sistema. Aqui
//  se dice, en un solo lugar:
//    - En que pin del Arduino esta conectado cada componente.
//    - Los valores que se pueden ajustar: distancias, tiempos, tonos...
//
//  Por que es util: si algun dia se cambia un cable de lugar o se quiere que
//  una alarma tarde mas, solo hay que cambiar el numero AQUI, sin tocar el resto
//  del programa.
//
//  Glosario rapido (para quien no sabe de electronica):
//    - Pin     : cada "patita" del Arduino donde se conecta un cable.
//    - D2, D3...: pines digitales (solo "encendido" o "apagado").
//    - A0, A1... : pines analogicos (leen valores de 0 a 1023).
//    - GND      : tierra, el cable de "retorno" de la electricidad.
//    - HIGH/LOW : encendido (con corriente) / apagado (sin corriente).
//    - ms       : milisegundos (1000 ms = 1 segundo).
//    - cm       : centimetros.
//    - Hz       : Hertz, el tono de un sonido (mas Hz = mas agudo).
//    - Debounce / anti-rebote: esperar un instante antes de creerle a un boton,
//      porque al presionarlo "vibra" y manda varios pulsos seguidos.
//
//  Se usa "constexpr" en vez de "#define" porque asi cada valor tiene un tipo
//  (numero entero, etc.) y el compilador puede detectar errores.
// ============================================================================

#include <Arduino.h>

// ============================================================================
//  BUZZER (alarma sonora)
// ============================================================================
// Buzzer: el pequeno altavoz que suena cuando hay una alerta.
constexpr uint8_t PIN_BUZZER = 4;

// ============================================================================
//  SENSORES DE DISTANCIA (ultrasonicos SRF-05)
//  Cada sensor tiene dos cables de senal:
//    TRIG = el sensor "grita" (emite un sonido que no escuchamos)
//    ECHO = el sensor "escucha" el eco que rebota en el objeto
//  El pin extra OUT/Mode del SRF-05 se deja sin conectar.
// ============================================================================
// Nota tecnica (historia): estos pines evitan D9/D10 porque en la placa Uno la
// libreria del servo los dejaba inservibles. En el Mega ya no pasa, pero se
// dejaron donde estaban.
constexpr uint8_t PIN_TRIG_1 = A2;
constexpr uint8_t PIN_ECHO_1 = A3;

// Sensor de distancia 2 (mismo modelo y mismo modo).
constexpr uint8_t PIN_TRIG_2 = 7;
constexpr uint8_t PIN_ECHO_2 = 8;

// ============================================================================
//  LEDs DE AVISO
// ============================================================================
// LED de panico: se prende cuando alguien presiona el boton de panico.
// (Control directo; antes vivia en un chip auxiliar que ya se elimino.)
constexpr uint8_t PIN_LED_PANICO = A1;

// LED de proximidad: es el "segundo canal" visual de la alarma de proximidad.
// Se prende bajo las mismas condiciones que el buzzer de proximidad: hay algo
// dentro de la distancia de alerta de cada sensor Y es de noche.
// Asi la alarma no depende de un solo aviso (si no se escucha, igual se ve).
constexpr uint8_t PIN_LED_PROXIMIDAD = 10;

// Distancia de alerta de cada sensor, en centimetros: si un objeto esta a esta
// distancia o mas cerca, se activa la alerta de proximidad.
// Cada sensor tiene la suya: el sensor 2 es mas sensible (8 cm) que el 1 (20 cm)
// por su ubicacion en el montaje.
constexpr int UMBRAL_DETECCION_1_CM = 20;
constexpr int UMBRAL_DETECCION_2_CM = 8;

// Tiempo maximo que el Arduino espera el eco del sensor (en microsegundos).
// Si el sonido no vuelve en este tiempo, se asume que no hay ningun objeto cerca.
// (30000 us ~ 5 metros de alcance.)
constexpr unsigned long TIMEOUT_PULSO_US = 30000UL;

// Cada cuanto leer los sensores de proximidad (en milisegundos).
// No hace falta leerlos todo el tiempo; una vez por segundo es suficiente y
// deja tiempo libre al procesador para otras tareas.
constexpr unsigned long INTERVALO_LECTURA_MS = 1000UL;

// ============================================================================
//  LUZ DEL PASILLO (3er sensor de distancia + luz)
//  El sensor mide la distancia: si hay algo cerca, enciende la luz. Ojo: no
//  distingue una persona quieta de un objeto fijo (una pared, un mueble) que
//  quede dentro de su rango.
// ============================================================================
constexpr uint8_t PIN_TRIG_PASILLO = 2;  // TRIG del sensor del pasillo
constexpr uint8_t PIN_ECHO_PASILLO = 5;  // ECHO del sensor del pasillo
constexpr uint8_t PIN_LUZ_PASILLO = 6;   // Controla la luz del pasillo (LED/rele)
constexpr int UMBRAL_PASILLO_CM = 10;    // Distancia a partir de la cual se considera "hay alguien"
constexpr unsigned long DURACION_LUZ_PASILLO_MS = 15000UL; // Tiempo que la luz queda encendida tras la ultima deteccion (15 s)

// ============================================================================
//  ALARMA DE CAMA (avisa si la persona lleva demasiado tiempo acostada)
//
//  Por ahora el sensor de presion real (FSR) NO esta armado. Se simula con un
//  interruptor comun: cerrado = "hay alguien en la cama", abierto = "cama vacia".
//  El resto de la logica es la misma que tendra el sensor real.
//
//  Limitacion conocida: si el cable se corta o el interruptor se desconecta, el
//  sistema lo interpreta como "cama vacia" (no puede distinguirlo sin hardware
//  adicional). Es un riesgo pendiente de resolver.
// ============================================================================
constexpr uint8_t PIN_FSR_CAMA = A4;         // Interruptor (temporal) o sensor de presion (definitivo) de la cama
constexpr uint8_t PIN_LUZ_ALARMA_CAMA = 9;   // Luz de alarma si la persona lleva demasiado tiempo en la cama
// Anti-rebote del interruptor de la cama (50 ms). Con el sensor real FSR se
// recomienda un valor mayor (~2000 ms) porque ese sensor tiene ruido de lectura.
constexpr unsigned long DEBOUNCE_PRESION_CAMA_MS = 50UL;

// VALOR DE PRUEBA (restaurar antes de uso real): una ausencia corta de la cama
// (ej. un giro) no reinicia el conteo; solo se considera que la persona se
// levanto si falta mas de este tiempo. Hoy 3 segundos para probar rapido.
// El valor REAL debe ser 60000UL (60 segundos).
constexpr unsigned long TOLERANCIA_AUSENCIA_CAMA_MS = 3UL * 1000UL; // PRUEBA: 3 segundos (valor real: 60000UL)

// VALOR DE PRUEBA (restaurar antes de uso real): tiempo maximo que la persona
// puede estar acostada antes de que suene la alarma. Hoy 5 segundos para poder
// probarla sin esperar. El valor REAL debe ser 7 horas: 7UL * 60UL * 60UL * 1000UL.
constexpr unsigned long TIEMPO_MAX_EN_CAMA_MS = 5UL * 1000UL; // PRUEBA: 5 segundos (valor real: 7UL * 60UL * 60UL * 1000UL)
//
// Al reemplazar el interruptor por el sensor FSR real habra que: subir
// DEBOUNCE_PRESION_CAMA_MS (~2000 ms), definir un umbral analogico de presion y
// cambiar hayPersonaEnCama() para leer con analogRead() en vez de digitalRead().

// ============================================================================
//  FOTOCELDA (sensor de luz: decide si es de dia o de noche)
//  Una fotocelda (LDR) cambia su resistencia con la luz. De noche la lectura
//  SUBE. La luz de pasillo y la alarma de proximidad solo funcionan de noche.
// ============================================================================
constexpr uint8_t PIN_FOTOCELDA = A0;
constexpr int UMBRAL_OSCURIDAD_ADC = 500; // Lectura (0-1023) a partir de la cual se considera "de noche"; ajustar segun el LDR y la resistencia usados
constexpr unsigned long DEBOUNCE_FOTOCELDA_MS = 2000UL; // Filtra ruido momentaneo de la lectura (ej. una sombra pasajera): 2 s

// ============================================================================
//  BOTON DE PANICO (la alarma de MAYOR prioridad del sistema)
//  Funciona como interruptor: un pulso activa la alarma y el siguiente la
//  desactiva. Silencia cualquier otra alarma en el buzzer.
//  Usa el pin D3 porque es uno de los dos pines con "interrupcion" (avisa al
//  Arduino al instante).
//
//  Limitacion conocida: si el cable se corta, el sistema lo interpreta como
//  "boton sin presionar". No se puede distinguir sin hardware adicional.
// ============================================================================
constexpr uint8_t PIN_BOTON_PANICO = 3;
constexpr unsigned long DEBOUNCE_BOTON_PANICO_MS = 50UL; // Anti-rebote: 50 ms

// ============================================================================
//  TONOS DEL BUZZER (uno distinto por tipo de alarma, para reconocerlas de oido)
//  El de panico es el mas agudo a proposito: es la emergencia mas importante.
// ============================================================================
constexpr unsigned int FRECUENCIA_ALARMA_PANICO_HZ = 3000;
constexpr unsigned int FRECUENCIA_ALARMA_PROXIMIDAD_HZ = 2000;
constexpr unsigned int FRECUENCIA_ALARMA_CAMA_HZ = 800;

// ============================================================================
//  SENSOR DE PULSO CARDIACO (mide los latidos por minuto, BPM)
//  Se activa con un switch de 2 posiciones: OFF = no mide; ON = mide en ciclos
//  de 10 segundos y muestra el resultado en la pantalla LCD al final de cada
//  ciclo, mientras siga en ON.
//  Si el resultado sale del rango normal (BPM_MIN_NORMAL a BPM_MAX_NORMAL) suena
//  el buzzer hasta que un ciclo posterior sea normal o el switch pase a OFF.
//
//  Prioridad del buzzer: panico > cama > pulso > proximidad.
//  Solo funciona en el Arduino Mega (el pin A8 no existe en el Uno).
//
//  Mientras se mide, el programa pausa los sensores de distancia y los mensajes
//  a la computadora, porque lo demorarian y el sensor de pulso necesita leerse
//  cada ~2 ms. Entre ciclos hay una pausa corta donde si leen.
// ============================================================================
constexpr uint8_t PIN_SENSOR_PULSO = A8;   // Cable morado (senal analogica); rojo a 5V, negro a GND
constexpr uint8_t PIN_SWITCH_PULSO = 12;   // Switch de 2 posiciones a GND: cerrado = ON (igual que el de la cama)
constexpr unsigned long DEBOUNCE_SWITCH_PULSO_MS = 50UL;      // Anti-rebote del switch: 50 ms
constexpr unsigned long DURACION_MEDICION_PULSO_MS = 10000UL; // Duracion de cada ciclo de medicion (y cada cuanto se actualiza la pantalla): 10 s
constexpr unsigned long PAUSA_ENTRE_MEDICIONES_MS = 1000UL;   // Pausa entre ciclos donde los sensores de distancia pueden leer: 1 s
constexpr uint8_t LATIDOS_MINIMOS_MEDICION = 5;               // Menos latidos que esto = medicion invalida (dedo mal puesto), no se alarma
constexpr int BPM_MIN_NORMAL = 60;         // Por debajo de esto hay BRADICARDIA (pulso demasiado lento) -> alarma
constexpr int BPM_MAX_NORMAL = 100;        // Por encima de esto hay TAQUICARDIA (pulso demasiado rapido) -> alarma
constexpr int BPM_MAX_VALIDO = 200;        // Por encima de esto la lectura es ruido del sensor (se cuentan latidos de mas), no un pulso real: se descarta como invalida
constexpr unsigned int FRECUENCIA_ALARMA_PULSO_HZ = 1200; // Tono propio: distinto de panico (3000), proximidad (2000) y cama (800)
// MODO DE CALIBRACION (dejar en false): si se pone en true, imprime en la
// computadora la senal cruda del sensor cada 20 ms y una linea por latido. Es
// mucho texto; solo sirve para ajustar el sensor. El avance por segundo y el
// resultado final se imprimen siempre.
constexpr bool DEBUG_PULSO_SENAL = false;

// ============================================================================
//  PANTALLA LCD 16x2 (16 columnas x 2 filas de texto)
//  Se conecta en modo "paralelo de 4 bits": usa los pines de datos D4 a D7 de
//  la pantalla (los D0 a D3 quedan sin conectar), mas RS y E de control.
//  RW va a GND porque solo se escribe en ella.
//
//  Cableado de los 16 pines de la pantalla:
//    1 VSS -> GND
//    2 VDD -> 5V
//    3 V0  -> contraste: cursor (pata central) de un potenciometro de 10k
//             conectado entre 5V y GND
//    4 RS  -> PIN_LCD_RS      5 RW -> GND      6 E -> PIN_LCD_E
//    7 a 10 (D0-D3) -> sin conectar
//    11 D4-> PIN_LCD_D4      12 D5 -> PIN_LCD_D5
//    13 D6 -> PIN_LCD_D6      14 D7 -> PIN_LCD_D7
//    15 A (luz de fondo +) -> 5V con una resistencia de 220 ohm
//    16 K (luz de fondo -) -> GND
// ============================================================================
constexpr uint8_t PIN_LCD_RS = 22;
constexpr uint8_t PIN_LCD_E = 23;
constexpr uint8_t PIN_LCD_D4 = 24;
constexpr uint8_t PIN_LCD_D5 = 25;
constexpr uint8_t PIN_LCD_D6 = 26;
constexpr uint8_t PIN_LCD_D7 = 27;
constexpr uint8_t LCD_COLUMNAS = 16; // Cantidad de letras por fila
constexpr uint8_t LCD_FILAS = 2;     // Cantidad de filas

// ============================================================================
//  PUERTA (boton de apertura + servomotor del pestillo)
//
//  Boton de apertura manual: pulsador entre el pin y GND, pensado para el lado
//  de ADENTRO. Abre sin pedir autorizacion: si queda accesible desde afuera,
//  cualquiera puede entrar.
// ============================================================================
constexpr uint8_t PIN_BOTON_PUERTA = 28;
constexpr unsigned long DEBOUNCE_BOTON_PUERTA_MS = 50UL; // Anti-rebote del boton: 50 ms

// Servo que mueve el pestillo de la puerta.
// Cableado: senal (cable naranja/amarillo) -> D11 | rojo -> 5V | marron/negro -> GND.
// Aviso: un servo pequeno puede pedir picos de ~500 mA al moverse. Si se alimenta
// del 5V del Arduino por USB, la placa puede reiniciarse. Lo seguro es una fuente
// de 5V aparte con GND comun, o al menos un capacitor de 470 uF entre 5V y GND
// cerca del servo.
constexpr uint8_t PIN_SERVO_PUERTA = 11;
constexpr uint8_t ANGULO_PUERTA_CERRADA = 0;   // Angulo del servo con la puerta cerrada (grados)
constexpr uint8_t ANGULO_PUERTA_ABIERTA = 90;  // Angulo del servo con la puerta abierta (grados)
constexpr unsigned long DURACION_PUERTA_ABIERTA_MS = 5000UL; // Tiempo que la puerta queda abierta antes de cerrarse sola (5 s)

#endif
