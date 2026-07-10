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

Mapeo actual (`src/main.cpp` ↔ `diagram.json`):

| Componente | Pin | Constante en código |
|---|---|---|
| Buzzer | 4 | `tonePin` |
| HC-SR04 TRIG | 9 | `trigPin` |
| HC-SR04 ECHO | 10 | `echoPin` |
| 74HC595 SHCP (clock) | 11 | `clockPin` |
| 74HC595 STCP (latch) | 12 | `latchPin` |
| 74HC595 DS (data) | 13 | `dataPin` |

Si se cambia un pin en el código, **actualizar `diagram.json`** (y viceversa). Usar el comando `/pin-check` o el hook `pin-consistency-check.js` para detectar discrepancias automáticamente — son advertencias, no bloqueos, así que revisarlas manualmente antes de simular.

### Al agregar un sensor/actuador nuevo

1. Elegir un pin libre respetando las restricciones de hardware (ver `skill: arduino-embedded-standards`).
2. Declarar la constante del pin en `config.h`.
3. Agregar la parte y la conexión correspondiente en `diagram.json`.
4. Ejecutar `pio run` y volver a simular.
