# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

# Casa IoT para Adultos Mayores — Arduino Uno

## Descripción del Proyecto
Sistema domótico de asistencia para personas mayores implementado sobre Arduino Uno.
Desarrollado en VS Code con PlatformIO. Simulación en Wokwi.

Estado actual: el firmware ([src/main.cpp](src/main.cpp), modularizado en `sensors`/`alerts`/`display`/`lighting`) implementa:
- **Proximidad**: 2 sensores ultrasónicos HC-SR04. Cuando cualquiera detecta un objeto a ≤20cm, enciende 2 LEDs (vía registro de desplazamiento 74HC595, usando Q0/Q1) y suena el buzzer.
- **Luz de pasillo**: un sensor PIR enciende una luz al detectar movimiento y la apaga sola tras 15s sin movimiento.
- **Alarma de cama**: un sensor de presión detecta si la persona lleva más de 7 horas seguidas acostada; si es así, enciende una luz de alarma y suena el buzzer con un tono distinto al de proximidad.
- **Botón de pánico**: interrupción en D3 (INT1), tipo interruptor (un pulso activa la alarma, el siguiente la desactiva). Es la alarma de mayor prioridad del sistema: silencia cualquier otra en el buzzer compartido y enciende un LED dedicado (Q2 del 74HC595).

El buzzer y el registro 74HC595 son recursos compartidos entre las distintas alarmas; `main.cpp` arbitra la prioridad (pánico > cama > proximidad) antes de pedir una única frecuencia a `actualizarBuzzer()` / un único patrón de LEDs a `actualizarLeds()`, una vez por vuelta de `loop()`.

## Stack Técnico
- **MCU**: Arduino Uno (ATmega328P, 32KB Flash, 2KB SRAM)
- **IDE**: VS Code + PlatformIO
- **Simulación**: Wokwi (wokwi.toml + diagram.json)
- **Lenguaje**: C++ / Arduino Framework
- **Librerías clave**: (se irán añadiendo)

## Comandos

Requiere la CLI de PlatformIO (`pio`), normalmente instalada con la extensión de PlatformIO para VS Code.

- Compilar: `pio run`
- Subir a placa física: `pio run --target upload`
- Monitor serial: `pio device monitor` (el firmware usa 9600 baudios)
- Limpiar: `pio run --target clean`

No hay configuración de lint ni de tests (el directorio `test/` es scaffolding vacío de PlatformIO).

## Simulación (Wokwi)

El proyecto se simula en Wokwi (extensión de VS Code o wokwi.com):

- [wokwi.toml](wokwi.toml) apunta al binario compilado (`.pio/build/uno/firmware.hex` / `.elf`), así que hay que ejecutar `pio run` antes de iniciar la simulación.
- [diagram.json](diagram.json) define el circuito simulado y el cableado. Los pines asignados en el código deben mantenerse sincronizados con las conexiones declaradas ahí:
  - Buzzer → pin 4 (compartido entre alarma de proximidad y alarma de cama)
  - HC-SR04 sensor 1: TRIG → pin 9, ECHO → pin 10
  - HC-SR04 sensor 2: TRIG → pin 7, ECHO → pin 8
  - 74HC595 SHCP (clock) → pin 11, STCP (latch) → pin 12, DS (data) → pin 13
  - Salidas Q0–Q7 del registro → barra de LEDs (se usan Q0/Q1 para proximidad y Q2 para pánico; el resto quedan siempre apagadas)
  - PIR pasillo: OUT → pin 2; luz de pasillo → pin 6
  - Sensor de presión de cama: SIG → pin A0 (simulado con potenciómetro en Wokwi, ya que no hay pieza FSR nativa); luz de alarma de cama → pin 5
  - Botón de pánico: pin 3 (INT1), `INPUT_PULLUP` + `attachInterrupt(FALLING)`

Si se cambia un pin en el código, actualizar `diagram.json` (y viceversa).

## Restricciones de Hardware
- Solo 6 pines analógicos (A0–A5)
- Solo 14 pines digitales (2–13 usables; 0/1 reservados para Serial)
- Pines PWM: 3, 5, 6, 9, 10, 11
- **D2 y D3 son los únicos pines de interrupción externa (INT0/INT1)**. D2 está en uso por el PIR del pasillo (por sondeo, no por interrupción). D3 lo usa el botón de pánico, vía `attachInterrupt`.
- Sin WiFi nativo — usar módulo ESP8266 (AT commands vía SoftwareSerial) o HC-05 Bluetooth
- Sin RTOS — código secuencial, evitar delay(), usar millis()
- SRAM limitada: evitar String, preferir char[], F() macro para literales

## Arquitectura del Código
- Sin POO compleja (RAM limitada). Módulos como .h/.cpp separados por funcionalidad
- Loop principal no bloqueante — máquina de estados o scheduler simple con millis()
- Un archivo por módulo: sensors.h, alerts.h, display.h, communication.h

## Funcionalidades Planificadas
### Seguridad / Emergencias
- [x] Botón de pánico (interrupción INT1/D3) — toggle: un pulso activa, el siguiente desactiva; prioridad máxima sobre las demás alarmas
- [ ] Detector de caída (MPU6050 vía I2C)
- [x] Alarma sonora (buzzer pasivo) — implementada para proximidad, inmovilidad en cama y pánico
- [x] Alarma de inmovilidad prolongada en cama (sensor de presión, luz + buzzer si pasan 7h seguidas acostado) — no estaba en el roadmap original, se agregó a pedido

### Monitoreo Ambiental
- [ ] Temperatura y humedad (DHT11 o DHT22)
- [ ] Detector de gas/humo (MQ-2)
- [x] Sensor de movimiento PIR (presencia en habitaciones) — implementado para el pasillo

### Confort / Accesibilidad
- [x] Control de iluminación — on/off simple con PIR (pasillo) y con la alarma de cama; falta el dimmer PWM
- [ ] Display LCD 16x2 I2C con info de estado
- [ ] Alarmas de medicamentos (RTC DS3231 + buzzer)

### Conectividad
- [ ] ESP8266 en modo AT para MQTT o HTTP hacia broker/dashboard
- [ ] Alternativa: HC-05 Bluetooth para app móvil cercana

## Convenciones
- Variables globales de estado en mayúsculas: `TEMP_ACTUAL`, `ALERTA_ACTIVA`
- Funciones: camelCase. Archivos: snake_case
- Comentarios en español
- Nunca usar `delay()` — siempre `millis()` con timers no bloqueantes
- Constantes de pines en `config.h`

## Estructura de Archivos
- [src/main.cpp](src/main.cpp) — punto de entrada; orquesta los módulos con temporizadores `millis()` no bloqueantes y arbitra la prioridad del buzzer compartido
- [src/sensors.h](src/sensors.h) / [src/sensors.cpp](src/sensors.cpp) — lectura de los 2 HC-SR04, el PIR y el sensor de presión de cama (con anti-rebote)
- [src/alerts.h](src/alerts.h) / [src/alerts.cpp](src/alerts.cpp) — control del buzzer compartido (`actualizarBuzzer(pin, frecuenciaHz)`) y del botón de pánico (interrupción + anti-rebote + toggle)
- [src/display.h](src/display.h) / [src/display.cpp](src/display.cpp) — control de los LEDs vía 74HC595 (proximidad y pánico)
- [src/lighting.h](src/lighting.h) / [src/lighting.cpp](src/lighting.cpp) — encender/apagar la luz de pasillo y la luz de alarma de cama
- [include/config.h](include/config.h) — constantes de pines y parámetros (umbrales, timeouts, tiempos de debounce)
- [platformio.ini](platformio.ini) — único entorno `uno` (plataforma atmelavr, framework Arduino)
- [diagram.json](diagram.json) — circuito de Wokwi
- [wokwi.toml](wokwi.toml) — configuración del simulador
- `lib/` — librerías propias (vacío por ahora)
- `.pio/` — salida de compilación generada; nunca editarla

## Herramientas de Claude Code

Este proyecto tiene un toolkit propio en [.claude/](.claude/README.md) (agentes, skills, comandos, hooks, reglas y configs MCP). Ver `.claude/README.md` para la estructura completa. Esta sección es la guía de **cuándo usar cada pieza**.

### Agentes (invocados automáticamente por los comandos, o directamente si hace falta)

| Agente | Cuándo usarlo |
|---|---|
| `arduino-reviewer` | Después de escribir o modificar cualquier `.cpp`/`.h`, antes de compilar o de dar por terminado un cambio. Revisa memoria/SRAM, `delay()` fuera de `setup()`, convenciones de nombres. Se invoca vía `/review`. |
| `arduino-build-resolver` | Cuando `pio run` falla. Arregla errores de compilación/enlazado con cambios mínimos, un error a la vez. Se invoca vía `/build`. |
| `iot-safety-reviewer` | Solo cuando el cambio toca botón de pánico, detector de caídas, alarmas o alarmas de medicamentos (roadmap "Seguridad/Emergencias" y "Confort/Accesibilidad"). Revisa fail-safe, anti-rebote y redundancia de alarmas — no aplica a cambios puramente cosméticos o de monitoreo ambiental. Se invoca vía `/review` cuando corresponde. |

### Skills (consultarlas *antes* de escribir el código, no después)

| Skill | Cuándo consultarla |
|---|---|
| `arduino-embedded-standards` | Antes de escribir cualquier `.cpp`/`.h` — restricciones de hardware, patrón `millis()` no bloqueante, organización de módulos. Es la base de todo el firmware. |
| `platformio-wokwi-workflow` | Al compilar, subir a placa, simular, o antes de tocar `diagram.json`/`wokwi.toml`/`platformio.ini`. |
| `iot-elder-care-safety` | **Antes** (no después) de implementar cualquier funcionalidad de seguridad/emergencia o confort/accesibilidad del roadmap: fail-safe, debounce, redundancia de alarmas, checklist de seguridad. |

### Comandos (los que se ejecutan explícitamente con `/`)

| Comando | Cuándo usarlo |
|---|---|
| `/new-module` | Al empezar una funcionalidad nueva del roadmap — crea el andamiaje `.h`/`.cpp` del módulo antes de escribir la lógica. |
| `/build` | Después de escribir código y antes de simular/subir; o directamente cuando `pio run` falla. |
| `/review` | Después de que el código compila, antes de comitear, subir a la placa, o dar la funcionalidad por terminada. |
| `/pin-check` | Después de agregar un sensor/actuador nuevo (pin nuevo), o cuando el hook `pin-consistency-check.js` avisó una posible discrepancia y quieres el detalle. |
| `/sim` | Antes de simular en Wokwi, o inmediatamente después de modificar `diagram.json`. |

Orden típico de un cambio: `/new-module` → código → `/build` → `/pin-check` (si agregaste un pin) → `/review` → `/sim`.

### Reglas (siempre cargadas, no se invocan manualmente)

| Regla | Cuándo se aplica |
|---|---|
| `rules/arduino-cpp.md` | Siempre, a todo código C++ del firmware (nunca `delay()`, nunca `String`, organización en módulos). |
| `rules/project-conventions.md` | Siempre — nombres (MAYÚSCULAS para estado global, camelCase, snake_case), comentarios en español, pines en `config.h`. |
| `rules/security-iot.md` | Se vuelve relevante en cuanto el proyecto toque conectividad: módulo ESP8266 (AT/MQTT), HC-05 Bluetooth, o cualquier credencial. No aplica mientras el firmware sea solo sensores/actuadores locales. |

### MCP (opcionales — copiar a `~/.claude.json` solo si se necesitan)

| Servidor MCP | Cuándo activarlo |
|---|---|
| `context7` | Al agregar una librería nueva a `platformio.ini` (DHT, MPU6050, RTC DS3231, etc.) y necesitar su documentación antes de integrarla. |
| `github` | Solo si el repo tiene un remoto en GitHub y hace falta crear/gestionar issues o PRs desde Claude Code. No hace falta para trabajar solo en local. |

Ninguno se activa solo: hay que copiarlos a la config global (`mcp-configs/mcp-servers.json` trae los valores listos para pegar).

### Hooks (automáticos, disparan solos, nunca bloquean)

| Hook | Cuándo se dispara | Qué hace |
|---|---|---|
| `pin-consistency-check.js` | Automático, tras cada `Edit`/`Write` sobre un `.cpp`/`.h` | Avisa si el pin usado en el código no está conectado en `diagram.json` (o viceversa). |
| `no-delay-guard.js` | Automático, tras cada `Edit`/`Write` sobre un `.cpp`/`.h` | Avisa si se introdujo `delay()` fuera de `setup()`. |
| `pio-build-reminder.js` | Automático, al final de cada respuesta (`Stop`) | Avisa si se editó firmware/config sin haber corrido `pio run` después. |

Los tres son solo informativos — ninguno impide continuar trabajando; sirven como recordatorio, no como gate.
