#include "access.h"
#include "config.h"
#include <Servo.h> // Libreria que sabe como mover un servomotor

namespace {
// Variables PRIVADAS de este archivo (otros archivos no las pueden tocar).

Servo servoPuerta; // El servomotor que mueve el pestillo de la puerta

bool puertaAbierta = false;              // true = la puerta esta abierta ahora
unsigned long momentoAperturaMillis = 0; // Cuando se abrio (para saber cuando cerrarla)

// --- Datos del boton de apertura manual: INPUT_PULLUP, presionado = LOW ---
uint8_t botonPuertaGpio = 0;                     // En que pin esta conectado
bool botonPuertaConfigurado = false;             // true = ya se preparo en setup()
bool botonPuertaPresionadoEstable = false;       // nivel ya filtrado por el anti-rebote
bool botonPuertaLecturaPrevia = false;           // lo que leimos la vez anterior
unsigned long ultimoCambioBotonPuertaMillis = 0; // cuando cambio la lectura por ultima vez

// Abre la puerta: gira el servo al angulo "abierto" y anota la hora.
void abrirPuerta() {
  servoPuerta.write(ANGULO_PUERTA_ABIERTA);
  puertaAbierta = true;
  momentoAperturaMillis = millis(); // si ya estaba abierta, reinicia el tiempo
  Serial.println(F("Puerta: ABIERTA")); // DEBUG TEMPORAL: mensaje para la computadora
}

// Cierra la puerta: gira el servo al angulo "cerrado".
void cerrarPuerta() {
  servoPuerta.write(ANGULO_PUERTA_CERRADA);
  puertaAbierta = false;
  Serial.println(F("Puerta: cerrada")); // DEBUG TEMPORAL
}
}

// Prepara el servo y deja la puerta cerrada al arrancar.
void inicializarAcceso(uint8_t pinServo) {
  servoPuerta.attach(pinServo); // "attach" = conectar el servo a ese pin
  cerrarPuerta();
}

// Prepara el boton de apertura.
void inicializarBotonPuerta(uint8_t pinBoton) {
  botonPuertaGpio = pinBoton;
  // INPUT_PULLUP: el pin queda en HIGH hasta que el boton lo conecte a tierra (LOW).
  pinMode(pinBoton, INPUT_PULLUP);
  // Nivel de partida: si arranca presionado no cuenta como un pulso nuevo (asi la
  // puerta no se abre sola al encender).
  botonPuertaLecturaPrevia = (digitalRead(pinBoton) == LOW);
  botonPuertaPresionadoEstable = botonPuertaLecturaPrevia;
  botonPuertaConfigurado = true;
}

// Se llama en cada vuelta del programa.
void actualizarAcceso() {
  // Parte 1: si la puerta esta abierta y ya paso su tiempo, se cierra sola, sin
  // pausar el programa (solo comparamos la hora actual con la hora de apertura).
  if (puertaAbierta && (millis() - momentoAperturaMillis >= DURACION_PUERTA_ABIERTA_MS)) {
    cerrarPuerta();
  }

  // Si el boton no se preparo, no hay nada mas que revisar.
  if (!botonPuertaConfigurado) {
    return;
  }

  // Parte 2: revisar el boton. Un pulso valido abre la puerta (o reinicia el tiempo
  // de apertura si ya estaba abierta).
  unsigned long ahora = millis();
  bool lectura = (digitalRead(botonPuertaGpio) == LOW); // LOW = boton presionado
  // Si la lectura cambio, anotamos cuando: puede ser un rebote o un cambio de verdad.
  if (lectura != botonPuertaLecturaPrevia) {
    botonPuertaLecturaPrevia = lectura;
    ultimoCambioBotonPuertaMillis = ahora;
  }
  // Anti-rebote: solo creemos el cambio si la lectura se mantuvo estable un ratito.
  if ((ahora - ultimoCambioBotonPuertaMillis >= DEBOUNCE_BOTON_PUERTA_MS) &&
      (lectura != botonPuertaPresionadoEstable)) {
    botonPuertaPresionadoEstable = lectura;
    if (lectura) { // true = acaba de presionarse (no cuando se suelta)
      Serial.println(F("Puerta: boton de apertura presionado"));
      abrirPuerta();
    }
  }
}
