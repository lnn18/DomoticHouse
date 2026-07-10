---
description: Estándares de C++ embebido para Arduino Uno (ATmega328P) en este proyecto — límites de hardware, patrón millis() no bloqueante, y organización de módulos.
---

# Estándares de Código Embebido — Arduino Uno

## Cuándo Usar

Al escribir o modificar cualquier archivo `.cpp`/`.h` en `src/`, `include/` o `lib/`. También al revisar código (`arduino-reviewer` la referencia).

## Cómo Funciona

### Restricciones de hardware (siempre presentes)
- **32KB Flash / 2KB SRAM** — no hay margen para STL pesado, `String` de Arduino, ni buffers grandes sin justificar.
- **6 pines analógicos** (A0–A5), **14 digitales** (2–13 usables; 0/1 reservados para `Serial`).
- **Pines PWM**: 3, 5, 6, 9, 10, 11. Si una función necesita `analogWrite`, el pin debe ser uno de estos.
- Sin WiFi nativo: conectividad vía módulo ESP8266 (comandos AT por `SoftwareSerial`) o HC-05 Bluetooth.
- Sin RTOS: el código es secuencial y cooperativo.

### Patrón `millis()` no bloqueante

`delay()` está prohibido en `loop()` porque bloquea toda la placa, incluida la detección de una emergencia. Patrón mínimo de temporizador no bloqueante:

```cpp
unsigned long previousMillis = 0;
const unsigned long INTERVALO_MS = 600;

void loop() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= INTERVALO_MS) {
    previousMillis = currentMillis;
    // acción periódica aquí, sin bloquear el resto del loop
  }
  // el resto de las tareas (lectura de botón de pánico, etc.) siguen corriendo cada iteración
}
```

Para varias tareas con distintos períodos, usar una máquina de estados simple o un `struct` por tarea con su propio `previousMillis`, en vez de anidar `if` con `delay()`.

### Memoria: evitar `String`
- Usar `char buffer[N]` de tamaño fijo en vez de `String` para evitar fragmentación de heap.
- Envolver literales largos pasados a `Serial.print`/`lcd.print` con el macro `F()` para mantenerlos en Flash en vez de SRAM: `Serial.print(F("Distancia: "));`.

### Organización de módulos

Un archivo por funcionalidad, no todo en `main.cpp`:

| Módulo | Contenido |
|---|---|
| `config.h` | Todas las constantes de pines y parámetros globales |
| `sensors.h` / `sensors.cpp` | Lectura de HC-SR04, DHT11/22, MQ-2, PIR, MPU6050 |
| `alerts.h` / `alerts.cpp` | Buzzer, patrones de alarma, botón de pánico |
| `display.h` / `display.cpp` | Barra de LEDs (74HC595), LCD I2C |
| `communication.h` / `communication.cpp` | ESP8266 (AT/MQTT), HC-05 Bluetooth |

Cada `.h` debe usar include guards (`#ifndef MODULO_H` / `#define MODULO_H` / `#endif`).

### Convenciones de nombres
- Variables globales de estado: **MAYÚSCULAS** (`TEMP_ACTUAL`, `ALERTA_ACTIVA`).
- Funciones: **camelCase**.
- Archivos: **snake_case**.
- Comentarios: en español, explicando el *porqué* cuando no sea obvio (p. ej. por qué un umbral específico).

## Ejemplos

Ver `src/main.cpp` actual como referencia de lo que **hoy** no cumple el estándar (usa `delay(600)` en `loop()`) — es código de partida pendiente de modularizar, no un ejemplo a copiar.
