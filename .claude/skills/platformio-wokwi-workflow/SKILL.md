---
description: Flujo de compilación con PlatformIO y simulación con Wokwi para este proyecto, incluida la regla de sincronía de pines entre el código y diagram.json.
---

# Flujo PlatformIO + Wokwi

## Cuándo Usar

Al compilar, subir a placa, simular, o modificar `platformio.ini`, `wokwi.toml` o `diagram.json`.

## Cómo Funciona

### Ciclo de comandos

```bash
pio run                      # compilar
pio run --target upload      # subir a placa física
pio device monitor           # monitor serial (9600 baudios)
pio run --target clean       # limpiar build
```

### Simulación en Wokwi

1. `wokwi.toml` apunta al binario compilado (`.pio/build/uno/firmware.hex` / `.elf`) — **siempre ejecutar `pio run` antes de iniciar la simulación**, si no el simulador corre el binario viejo.
2. Abrir la simulación desde la extensión de Wokwi en VS Code o en wokwi.com.
3. `diagram.json` define el circuito y el cableado — debe mantenerse sincronizado con las constantes de pines del código.

### Regla de sincronía de pines

Mapeo actual (`include/config.h` ↔ `diagram.json`):

| Componente | Pin | Constante en código |
|---|---|---|
| Buzzer | 4 | `PIN_BUZZER` |
| HC-SR04 sensor 1 TRIG | 9 | `PIN_TRIG_1` |
| HC-SR04 sensor 1 ECHO | 10 | `PIN_ECHO_1` |
| HC-SR04 sensor 2 TRIG | 7 | `PIN_TRIG_2` |
| HC-SR04 sensor 2 ECHO | 8 | `PIN_ECHO_2` |
| 74HC595 SHCP (clock) | 11 | `PIN_CLOCK_REGISTRO` |
| 74HC595 STCP (latch) | 12 | `PIN_LATCH_REGISTRO` |
| 74HC595 DS (data) | 13 | `PIN_DATOS_REGISTRO` |
| PIR pasillo OUT | 2 | `PIN_PIR_PASILLO` |
| Luz de pasillo | 6 | `PIN_LUZ_PASILLO` |
| Sensor de presión de cama (SIG) | A0 | `PIN_FSR_CAMA` |
| Luz de alarma de cama | 5 | `PIN_LUZ_ALARMA_CAMA` |
| Botón de pánico | 3 (INT1) | `PIN_BOTON_PANICO` |

El sensor de presión de la cama se simula en Wokwi con un `wokwi-potentiometer` (no existe una pieza FSR nativa en el catálogo de Wokwi); en hardware real se reemplazaría por un FSR con resistencia pull-down en el mismo pin analógico.

`PIN_LUZ_PASILLO` usa D6 y no D3 a propósito: D2/D3 son los únicos pines de interrupción externa (INT0/INT1). D3 quedó reservado para el botón de pánico (`attachInterrupt(..., FALLING)` con `INPUT_PULLUP`, sin resistencia externa). La luz de pánico no usa un pin nuevo del Uno: reutiliza Q2 del 74HC595, que ya estaba cableado a `ledbar` pero sin usar en el firmware.

Las constantes de pines viven ahora en `include/config.h`, no directamente en `main.cpp`.

Si se cambia un pin en el código, **actualizar `diagram.json`** (y viceversa). Usar el comando `/pin-check` o el hook `pin-consistency-check.js` para detectar discrepancias automáticamente — son advertencias, no bloqueos, así que revisarlas manualmente antes de simular.

### Al agregar un sensor/actuador nuevo

1. Elegir un pin libre respetando las restricciones de hardware (ver `skill: arduino-embedded-standards`).
2. Declarar la constante del pin en `config.h`.
3. Agregar la parte y la conexión correspondiente en `diagram.json`.
4. Ejecutar `pio run` y volver a simular.
