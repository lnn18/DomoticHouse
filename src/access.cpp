#include "access.h"
#include "config.h"
#include <SPI.h>
#include <MFRC522.h>
#include <Servo.h>
#include <string.h> // memcmp(), para comparar UIDs contra UIDS_AUTORIZADOS

// NOTA: MFRC522_SPICLOCK (velocidad del bus SPI hacia el RC522, ver diagnostico en curso)
// se define en platformio.ini como build_flag, NO aca. Un #define en este archivo, antes
// de este #include, solo afecta a access.cpp -- MFRC522.cpp (dentro de la libreria, donde
// realmente se usa la macro en cada PCD_ReadRegister/PCD_WriteRegister) es una unidad de
// traduccion separada que nunca ve un #define puesto aca. Confirmado con evidencia del
// binario compilado (SPCR/SPSR en el .elf seguian correspondiendo a 4MHz pese al #define
// local). build_flags en platformio.ini si aplica a todas las unidades de compilacion,
// incluidas las de lib_deps.

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

// Compara el UID leido contra la lista UIDS_AUTORIZADOS de config.h. Devuelve false si
// el tamano del UID leido no coincide con LONGITUD_UID_BYTES (evita comparar bytes de mas
// o de menos, que podria dar un falso positivo con memcmp en un arreglo de tamano fijo).
bool esUidAutorizado(const MFRC522::Uid &uid) {
  if (uid.size != LONGITUD_UID_BYTES) {
    return false;
  }
  // "int i" en vez de "uint8_t i": con CANTIDAD_UIDS_AUTORIZADOS=0 (sin UIDs cargados
  // todavia), un contador uint8_t hace que "i < 0 UIDs" sea siempre falso de forma
  // estaticamente comprobable, y -Wall/-Wextra/cppcheck lo marcan como warning
  // (comportamiento correcto -- ningun UID pasa -- pero el warning distrae). int evita
  // el warning sin cambiar el comportamiento: el rango cabe de sobra en un int.
  for (int i = 0; i < CANTIDAD_UIDS_AUTORIZADOS; i++) {
    if (memcmp(uid.uidByte, UIDS_AUTORIZADOS[i], LONGITUD_UID_BYTES) == 0) {
      return true;
    }
  }
  return false;
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
  // valido (no necesariamente 0x91/0x92: algunos clones devuelven otros valores, ej. 0x12,
  // pero eso no significa que no funcionen -- lo que importa es que no sea 0x00 ni 0xFF).
  // Si devuelve 0x00 o 0xFF, el modulo no esta respondiendo: revisar VCC (debe ser 3.3V,
  // no 5V), GND, y los 5 pines de datos (SDA/SS, SCK, MOSI, MISO, RST).
  byte version = lectorRfid.PCD_ReadRegister(MFRC522::VersionReg);
  Serial.print(F("RFID: version del chip leida = 0x"));
  Serial.print(version, HEX);
  if (version == 0x00 || version == 0xFF) {
    Serial.println(F(" -> ERROR: el RC522 no responde. Revisar cableado (VCC a 3.3V, no 5V; GND; SS/RST/SCK/MOSI/MISO)"));
  } else {
    Serial.println(F(" -> OK: el RC522 responde correctamente"));
  }

  // DEBUG TEMPORAL (diagnostico antena): la comunicacion SPI (control digital) y la antena
  // RF (TX1/TX2) son dos subsistemas distintos del mismo chip -- que VersionReg responda
  // bien NO garantiza que la antena este irradiando. PCD_Init() ya llama PCD_AntennaOn()
  // internamente, pero en vez de asumir que funciono, se lee el registro real (TxControlReg)
  // para confirmarlo. Los bits Tx1RFEn (bit0) y Tx2RFEn (bit1) deben estar en 1 para que la
  // antena este activa; TxControlReg == 0x83 es el valor esperado tras un PCD_Init() normal
  // (ver datasheet 9.3.2.7). Tambien se imprime la ganancia de recepcion (RxGain).
  byte txControl = lectorRfid.PCD_ReadRegister(MFRC522::TxControlReg);
  Serial.print(F("RFID: TxControlReg = 0x"));
  Serial.print(txControl, HEX);
  Serial.print(F(" (Tx1RFEn="));
  Serial.print((txControl & 0x01) ? F("1") : F("0"));
  Serial.print(F(", Tx2RFEn="));
  Serial.print((txControl & 0x02) ? F("1") : F("0"));
  Serial.println(F(") -> si alguno de los dos es 0, la antena NO esta irradiando campo RF"));

  byte antennaGain = lectorRfid.PCD_GetAntennaGain();
  Serial.print(F("RFID: ganancia de antena (RxGain) = 0x"));
  Serial.println(antennaGain, HEX);

  servoPuerta.attach(pinServo);
  cerrarPuerta();
}

void actualizarAcceso() {
  // Si la puerta esta abierta, se cierra sola pasado el tiempo, sin bloquear el loop.
  if (puertaAbierta && (millis() - momentoAperturaMillis >= DURACION_PUERTA_ABIERTA_MS)) {
    cerrarPuerta();
  }

  // DEBUG TEMPORAL (diagnostico deteccion de tarjeta): PICC_IsNewCardPresent() colapsa el
  // StatusCode real (OK, TIMEOUT, CRC_WRONG, COLLISION, etc. -- ver MFRC522::StatusCode en
  // MFRC522.h) a un simple true/false, ocultando por que falla exactamente. Se llama en su
  // lugar a PICC_RequestA() directo (lo que PICC_IsNewCardPresent hace internamente) para
  // ver el status real. Se loguea solo cuando NO es "no hay tarjeta" (ni OK ni TIMEOUT), o
  // cada ciertos intentos, para no inundar el monitor serial en cada vuelta de loop().
  //
  // Se replica ademas el mismo reset de registros que hace PICC_IsNewCardPresent() antes
  // de pedir PICC_RequestA() (TxModeReg/RxModeReg a baudrate por defecto, ModWidthReg al
  // ancho de modulacion esperado): en el flujo actual no deberian estar desviados de estos
  // valores (no se autentica ni se leen/escriben bloques MIFARE en ningun punto), pero se
  // replica de todas formas para comparar contra el mismo comportamiento exacto que la
  // libreria usa en su version "normal", sin introducir una diferencia extra al diagnostico.
  lectorRfid.PCD_WriteRegister(MFRC522::TxModeReg, 0x00);
  lectorRfid.PCD_WriteRegister(MFRC522::RxModeReg, 0x00);
  lectorRfid.PCD_WriteRegister(MFRC522::ModWidthReg, 0x26);

  byte bufferATQA[2];
  byte bufferSizeATQA = sizeof(bufferATQA);
  MFRC522::StatusCode statusRequestA = lectorRfid.PICC_RequestA(bufferATQA, &bufferSizeATQA);

  static unsigned long ultimoLogIntentoMillis = 0;
  unsigned long ahoraIntento = millis();
  if (statusRequestA != MFRC522::STATUS_TIMEOUT && ahoraIntento - ultimoLogIntentoMillis >= 300UL) {
    // STATUS_TIMEOUT es el caso normal ("no hay tarjeta en el campo ahora mismo") y se
    // imprimiria en CASI todas las vueltas de loop() -- se omite a proposito para no
    // inundar el monitor. Cualquier otro status (OK, CRC_WRONG, COLLISION, etc.) SI se
    // imprime, con un anti-inundacion simple de 300ms para no saturar si se repite rapido.
    ultimoLogIntentoMillis = ahoraIntento;
    Serial.print(F("RFID: PICC_RequestA() = "));
    Serial.println(lectorRfid.GetStatusCodeName(statusRequestA));
  }

  bool hayTarjetaNueva = (statusRequestA == MFRC522::STATUS_OK || statusRequestA == MFRC522::STATUS_COLLISION);
  if (!hayTarjetaNueva) {
    return;
  }
  if (!lectorRfid.PICC_ReadCardSerial()) {
    Serial.println(F("RFID: PICC_RequestA() detecto algo pero PICC_ReadCardSerial() fallo (anticolision/seleccion)"));
    return;
  }

  // Imprime el UID leido en todos los casos (autorizado o no): sirve tanto para copiar el
  // UID de una tarjeta nueva y agregarla a UIDS_AUTORIZADOS (config.h) como para dejar
  // registro de intentos de acceso rechazados.
  Serial.print(F("RFID: tarjeta detectada, UID = "));
  for (byte i = 0; i < lectorRfid.uid.size; i++) {
    Serial.print(lectorRfid.uid.uidByte[i] < 0x10 ? F("0") : F(""));
    Serial.print(lectorRfid.uid.uidByte[i], HEX);
    Serial.print(F(" "));
  }
  Serial.println();

  if (esUidAutorizado(lectorRfid.uid)) {
    Serial.println(F("RFID: UID autorizado -> abriendo puerta"));
    abrirPuerta();
  } else {
    Serial.println(F("RFID: UID NO autorizado -> acceso denegado"));
  }

  lectorRfid.PICC_HaltA();
  lectorRfid.PCD_StopCrypto1();
}
