#include "access.h"
#include "config.h"
#include <SPI.h>
#include <MFRC522.h>
#include <Servo.h>

namespace {
MFRC522 lectorRfid(PIN_RFID_SS, PIN_RFID_RST);
Servo servoPuerta;

bool puertaAbierta = false;
unsigned long momentoAperturaMillis = 0;

void abrirPuerta() {
  servoPuerta.write(ANGULO_PUERTA_ABIERTA);
  puertaAbierta = true;
  momentoAperturaMillis = millis();
}

void cerrarPuerta() {
  servoPuerta.write(ANGULO_PUERTA_CERRADA);
  puertaAbierta = false;
}
}

void inicializarAcceso(uint8_t pinRfidSs, uint8_t pinRfidRst, uint8_t pinServo) {
  (void)pinRfidSs;
  (void)pinRfidRst;

  SPI.begin();
  lectorRfid.PCD_Init();

  servoPuerta.attach(pinServo);
  cerrarPuerta();
}

void actualizarAcceso() {
  // Si la puerta esta abierta, se cierra sola pasado el tiempo, sin bloquear el loop.
  if (puertaAbierta && (millis() - momentoAperturaMillis >= DURACION_PUERTA_ABIERTA_MS)) {
    cerrarPuerta();
  }

  // PICC_IsNewCardPresent() + PICC_ReadCardSerial() es el patron estandar de MFRC522
  // para detectar una tarjeta nueva sin bloquear: devuelve false de inmediato si no hay
  // ninguna tarjeta en el campo del lector.
  if (!lectorRfid.PICC_IsNewCardPresent()) {
    return;
  }
  if (!lectorRfid.PICC_ReadCardSerial()) {
    return;
  }

  // Version inicial: cualquier tarjeta leida correctamente abre la puerta.
  abrirPuerta();

  lectorRfid.PICC_HaltA();
  lectorRfid.PCD_StopCrypto1();
}
