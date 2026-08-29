#include "sensors.h"
#include "config.h"

// Deja los pines listos: TRIG como salida (el Arduino "habla") y ECHO como entrada (el Arduino "escucha").
void inicializarSensor(uint8_t pinTrig, uint8_t pinEcho) {
  pinMode(pinTrig, OUTPUT);
  pinMode(pinEcho, INPUT);
  digitalWrite(pinTrig, LOW); // Arranca en LOW para asegurar un pulso limpio en la primera medicion
}

long medirDistanciaCm(uint8_t pinTrig, uint8_t pinEcho) {
  // Paso 1: enviamos un pulso muy corto (10 microsegundos) por TRIG.
  // Eso hace que el sensor emita el sonido ultrasonico.
  digitalWrite(pinTrig, HIGH);
  delayMicroseconds(10); // ancho de pulso minimo exigido por el sensor ultrasonico, no es delay() de loop
  digitalWrite(pinTrig, LOW);

  // Paso 2: pulseIn() espera hasta que el pin ECHO se ponga en HIGH y mide cuanto tiempo
  // (en microsegundos) se mantiene asi. Ese tiempo es lo que tardo el sonido en ir hasta
  // el objeto y volver. TIMEOUT_PULSO_US evita quedarse esperando para siempre si no hay
  // ningun objeto en rango.
  unsigned long duracionUs = pulseIn(pinEcho, HIGH, TIMEOUT_PULSO_US);
  if (duracionUs == 0) {
    return -1; // timeout: no hubo eco dentro del rango esperado
  }

  // Paso 3: convertimos el tiempo del eco a distancia.
  // Se divide entre 2 porque el sonido recorre la distancia dos veces (ida y vuelta).
  // Se divide entre 29.1 porque el sonido tarda, aproximadamente, 29.1 microsegundos
  // en recorrer cada centimetro de aire.
  return (long)(duracionUs / 2) / 29.1;
}

namespace {
// Estado interno para el anti-rebote del PIR (DESACTIVADO: el pasillo ahora usa un
// sensor ultrasonico, ver mas abajo).
// bool ULTIMO_ESTADO_PIR = false;
// unsigned long ULTIMO_CAMBIO_PIR_MILLIS = 0;
// bool MOVIMIENTO_ESTABLE = false;

// Mismo patron de anti-rebote, aplicado a la lectura analogica del sensor de presion.
// DESACTIVADO TEMPORALMENTE junto con el resto de la alarma de cama (ver config.h y main.cpp).
// bool ULTIMO_ESTADO_FSR = false;
// unsigned long ULTIMO_CAMBIO_FSR_MILLIS = 0;
// bool PRESION_ESTABLE = false;

// Mismo patron de anti-rebote, aplicado a la lectura analogica de la fotocelda: evita que
// la luz de pasillo y la alarma de proximidad parpadeen si la luz ambiente esta justo en
// el limite del umbral (ej. al amanecer/atardecer).
bool ULTIMO_ESTADO_FOTOCELDA = false;
unsigned long ULTIMO_CAMBIO_FOTOCELDA_MILLIS = 0;
bool ES_DE_NOCHE_ESTABLE = false;
}

// DESACTIVADAS: el pasillo paso de PIR a un 3er sensor ultrasonico (medirDistanciaCm()).
// void inicializarSensorPir(uint8_t pinPir) {
//   pinMode(pinPir, INPUT);
// }
//
// bool detectaMovimiento(uint8_t pinPir) {
//   bool estadoActual = digitalRead(pinPir) == HIGH;
//   unsigned long ahora = millis();
//
//   if (estadoActual != ULTIMO_ESTADO_PIR) {
//     ULTIMO_CAMBIO_PIR_MILLIS = ahora;
//     ULTIMO_ESTADO_PIR = estadoActual;
//   }
//   if (ahora - ULTIMO_CAMBIO_PIR_MILLIS >= DEBOUNCE_PIR_MS) {
//     MOVIMIENTO_ESTABLE = estadoActual;
//   }
//   return MOVIMIENTO_ESTABLE;
// }

// DESACTIVADAS TEMPORALMENTE junto con el resto de la alarma de cama (ver config.h y main.cpp):
// usan PIN_FSR_CAMA / DEBOUNCE_PRESION_CAMA_MS, que quedaron comentadas alli.
// void inicializarSensorPresionCama(uint8_t pinFsr) {
//   pinMode(pinFsr, INPUT);
// }
//
// bool hayPersonaEnCama(uint8_t pinFsr, int umbralAdc) {
//   bool estadoActual = analogRead(pinFsr) >= umbralAdc;
//   unsigned long ahora = millis();
//
//   // El anti-rebote aqui es clave: sin el, un roce o un ruido de lectura que baje
//   // la presion un instante reiniciaria el temporizador de "tiempo en cama" en
//   // main.cpp, y la alarma de 7 horas nunca se dispararia (falla silenciosa).
//   if (estadoActual != ULTIMO_ESTADO_FSR) {
//     ULTIMO_CAMBIO_FSR_MILLIS = ahora;
//     ULTIMO_ESTADO_FSR = estadoActual;
//   }
//   if (ahora - ULTIMO_CAMBIO_FSR_MILLIS >= DEBOUNCE_PRESION_CAMA_MS) {
//     PRESION_ESTABLE = estadoActual;
//   }
//   return PRESION_ESTABLE;
// }

void inicializarFotocelda(uint8_t pinFotocelda) {
  pinMode(pinFotocelda, INPUT);
}

bool esDeNoche(uint8_t pinFotocelda, int umbralAdc) {
  // La fotocelda es una LDR de 2 patas (no un modulo integrado): el divisor de tension se
  // arma a mano con una resistencia fija (~10k) en serie, y este pin lee el punto medio
  // entre ambas (ver diagrama de montaje). A menos luz ambiente el LDR tiene mas
  // resistencia: la lectura analogica sube. Por eso "es de noche" es lectura >= umbral,
  // igual criterio que UMBRAL_PRESION_CAMA_ADC (a mayor lectura, mas se cumple la condicion).
  // UMBRAL_OSCURIDAD_ADC es una estimacion inicial: como cada LDR y resistencia real varian,
  // conviene calibrarlo viendo las lecturas reales por Serial (ver DEBUG en main.cpp) antes
  // de confiar en el valor por defecto.
  bool estadoActual = analogRead(pinFotocelda) >= umbralAdc;
  unsigned long ahora = millis();

  if (estadoActual != ULTIMO_ESTADO_FOTOCELDA) {
    ULTIMO_CAMBIO_FOTOCELDA_MILLIS = ahora;
    ULTIMO_ESTADO_FOTOCELDA = estadoActual;
  }
  if (ahora - ULTIMO_CAMBIO_FOTOCELDA_MILLIS >= DEBOUNCE_FOTOCELDA_MS) {
    ES_DE_NOCHE_ESTABLE = estadoActual;
  }
  return ES_DE_NOCHE_ESTABLE;
}
