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
  Serial.println(F("Puerta: ABIERTA")); // DEBUG TEMPORAL
}

void cerrarPuerta() {
  servoPuerta.write(ANGULO_PUERTA_CERRADA);
  puertaAbierta = false;
  Serial.println(F("Puerta: cerrada")); // DEBUG TEMPORAL
}
}

void inicializarAcceso(uint8_t pinRfidSs, uint8_t pinRfidRst, uint8_t pinServo) {
  (void)pinRfidSs;
  (void)pinRfidRst;

  SPI.begin();
  lectorRfid.PCD_Init();

  // DEBUG TEMPORAL: valida el cableado del RC522 (SPI, VCC/GND, SS/RST) SIN necesitar
  // ninguna tarjeta cerca. PCD_ReadRegister(VersionReg) lee un registro de solo lectura
  // del propio chip; si el SPI y el cableado estan bien, devuelve un valor de version
  // conocido (0x91 o 0x92 son las versiones mas comunes del MFRC522). Si devuelve 0x00
  // o 0xFF, el modulo no esta respondiendo: revisar VCC (debe ser 3.3V, no 5V), GND, y
  // los 5 pines de datos (SDA/SS, SCK, MOSI, MISO, RST).
  byte version = lectorRfid.PCD_ReadRegister(MFRC522::VersionReg);
  Serial.print(F("RFID: version del chip leida = 0x"));
  Serial.print(version, HEX);
  if (version == 0x00 || version == 0xFF) {
    Serial.println(F(" -> ERROR: el RC522 no responde. Revisar cableado (VCC a 3.3V, no 5V; GND; SS/RST/SCK/MOSI/MISO)"));
  } else {
    Serial.println(F(" -> OK: el RC522 responde correctamente"));
  }

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

  // DEBUG TEMPORAL: imprime el UID leido (util para armar mas adelante una lista de UIDs
  // autorizados; por ahora cualquier tarjeta leida correctamente abre la puerta).
  Serial.print(F("RFID: tarjeta detectada, UID = "));
  for (byte i = 0; i < lectorRfid.uid.size; i++) {
    Serial.print(lectorRfid.uid.uidByte[i] < 0x10 ? F("0") : F(""));
    Serial.print(lectorRfid.uid.uidByte[i], HEX);
    Serial.print(F(" "));
  }
  Serial.println();

  // Version inicial: cualquier tarjeta leida correctamente abre la puerta.
  abrirPuerta();

  lectorRfid.PICC_HaltA();
  lectorRfid.PCD_StopCrypto1();
}
