# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

# Casa IoT para Adultos Mayores — Arduino Uno

## Descripción del Proyecto
Sistema domótico de asistencia para personas mayores implementado sobre Arduino Uno.
Desarrollado en VS Code con PlatformIO. Simulación en Wokwi.

Estado actual: el firmware ([src/main.cpp](src/main.cpp), modularizado en `sensors`/`alerts`/`display`/`lighting`/`access`) implementa:
- **Proximidad**: 2 sensores ultrasónicos SRF-05 (en modo de 4 pines, eléctricamente compatible con HC-SR04; el pin OUT/Mode queda sin conectar). Se leen cada 1s (`INTERVALO_LECTURA_MS`). Cuando cualquiera detecta un objeto a ≤20cm **y además es de noche** (ver fotocelda), suena el buzzer (sin LED propio); de día no reacciona aunque haya algo cerca.
- **Luz de pasillo**: un 3er sensor SRF-05 (mismo modelo que proximidad, reutilizando una unidad disponible) enciende una luz cuando detecta algo a ≤10cm (`UMBRAL_PASILLO_CM`) y la apaga sola tras 15s sin detectar nada — pero solo **de noche**; de día no enciende aunque haya alguien cerca. A diferencia de un PIR (que usaba antes), este sensor detecta presencia por distancia, no movimiento real: no distingue una persona quieta de un objeto fijo dentro de su rango.
- **Fotocelda (LDR)**: en A0, decide si "es de noche" (con anti-rebote de 2s) y condiciona tanto la luz de pasillo como la alarma de proximidad. Reutiliza el pin que dejó libre la alarma de cama desactivada.
- **Alarma de cama**: DESACTIVADA TEMPORALMENTE (comentada en `main.cpp`, `config.h` y `sensors.cpp`, no borrada). Cuando se reactive: un sensor de presión detecta si la persona lleva más de 7 horas seguidas acostada; si es así, enciende una luz de alarma y suena el buzzer con un tono distinto al de proximidad. Nota: al reactivarla habría que mover la fotocelda a otro pin analógico, ya que ambas usan A0.
- **Botón de pánico**: interrupción en D3 (INT1), tipo interruptor (un pulso activa la alarma, el siguiente la desactiva). Es la alarma de mayor prioridad del sistema: silencia cualquier otra en el buzzer compartido y enciende un LED dedicado en A1 (control directo, ya no vía registro de desplazamiento). No depende de la fotocelda — funciona a cualquier hora.
- **Control de acceso (puerta)**: un lector RFID RC522 (bus SPI de hardware: D11/D12/D13) detecta cualquier tarjeta/tag (todavía sin validar contra una lista de UIDs autorizados) y abre un servo durante 20s antes de cerrarlo solo, sin bloquear el `loop()`. No depende de la fotocelda — funciona a cualquier hora.

El buzzer es un recurso compartido entre las distintas alarmas; `main.cpp` arbitra la prioridad (pánico > proximidad, ya que la alarma de cama está desactivada) antes de pedir una única frecuencia a `actualizarBuzzer()`, una vez por vuelta de `loop()`. El registro 74HC595 que antes controlaba los LEDs se eliminó por completo: liberaba el bus SPI que ahora necesita el RC522.

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
  - Buzzer → pin 4
  - Sensor de proximidad 1 (físico: SRF-05, modo 4 pines; simulado en Wokwi con la pieza `wokwi-hc-sr04`, eléctricamente equivalente): TRIG → pin A2, ECHO → pin A3 (**no D9/D10**: ver nota de Timer1 más abajo)
  - Sensor de proximidad 2 (mismo caso): TRIG → pin 7, ECHO → pin 8
  - LED de pánico: control directo (sin registro de desplazamiento) → pin A1
  - Sensor de pasillo (3er SRF-05, mismo caso que los de proximidad): TRIG → pin 2, ECHO → pin 5; luz de pasillo → pin 6
  - Botón de pánico: pin 3 (INT1), `INPUT_PULLUP` + `attachInterrupt(FALLING)`
  - Lector RFID RC522 (pieza nativa `board-mfrc522`): VCC → 3.3V (**no 5V**, daña el módulo), GND → GND, MOSI → pin 11, MISO → pin 12, SCK → pin 13 (bus SPI de hardware, fijo), SDA/SS → A4, RST → A5
  - Servo de la puerta (pieza nativa `wokwi-servo`): señal PWM → pin D9 (vía `Servo.h`, no requiere pin PWM de hardware; **no va en A2** para no chocar con el sensor de proximidad 1), V+ → 5V, GND → GND

  **Conflicto de Timer1 (D9/D10) con `Servo.h`**: en el Uno, `Servo.attach()` reconfigura Timer1 del ATmega328P apenas hay un servo activo, sin importar a qué pin esté atado ese servo (el chip solo tiene un timer disponible para servos, a diferencia del Mega). Esto deja D9/D10 inservibles para `pulseIn()` mientras el servo de la puerta esté activo. Por eso el sensor de proximidad 1 se movió a A2/A3 y el servo pasó a D9 — mantener esta separación si se vuelve a tocar la asignación de pines.
  - Fotocelda: en físico es una LDR de 2 patas (sin polaridad) + una resistencia fija (~10kΩ) armando un divisor de tensión a mano — el nodo entre ambas va a A0, un extremo de la LDR a 5V y un extremo de la resistencia a GND. Wokwi no tiene una pieza de LDR suelta ni simula bien resistencias junto a partes analógicas, así que `diagram.json` la simula con el módulo `wokwi-photoresistor-sensor` (mismo comportamiento eléctrico visto desde AO, pero no es el montaje físico real)

  Desactivado temporalmente (comentado en el código, no borrado del todo — ver `config.h`, `main.cpp`, `sensors.cpp`): sensor de presión de cama (potenciómetro en Wokwi) en A0 (choca con la fotocelda — no puede reactivarse sin mover una de las dos a otro pin), luz de alarma de cama en pin 5. No están en `diagram.json` mientras siguen desactivados.

Si se cambia un pin en el código, actualizar `diagram.json` (y viceversa).

## Restricciones de Hardware
- Solo 6 pines analógicos (A0–A5)
- Solo 14 pines digitales (2–13 usables; 0/1 reservados para Serial)
- Pines PWM: 3, 5, 6, 9, 10, 11
- **D2 y D3 son los únicos pines de interrupción externa (INT0/INT1)**. D2 está en uso por el TRIG del sensor de pasillo (por sondeo, no por interrupción). D3 lo usa el botón de pánico, vía `attachInterrupt`.
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
- [x] Alarma sonora (buzzer pasivo) — implementada para proximidad y pánico (inmovilidad en cama desactivada temporalmente)
- [~] Alarma de inmovilidad prolongada en cama (sensor de presión, luz + buzzer si pasan 7h seguidas acostado) — DESACTIVADA TEMPORALMENTE (comentada, no borrada, ver `config.h`/`main.cpp`/`sensors.cpp`); no estaba en el roadmap original, se había agregado a pedido

### Monitoreo Ambiental
- [ ] Temperatura y humedad (DHT11 o DHT22)
- [ ] Detector de gas/humo (MQ-2)
- [~] Sensor de movimiento PIR (presencia en habitaciones) — REEMPLAZADO por un 3er sensor ultrasónico SRF-05 en el pasillo (detecta presencia por distancia ≤10cm, no movimiento real; ver "Luz de pasillo" arriba). El PIR ya no está en uso, sus funciones quedaron comentadas en `sensors.h`/`sensors.cpp`

### Confort / Accesibilidad
- [x] Control de iluminación — on/off simple con el sensor de pasillo (SRF-05); falta el dimmer PWM
- [ ] Display LCD 16x2 I2C con info de estado
- [ ] Alarmas de medicamentos (RTC DS3231 + buzzer)
- [~] Control de acceso a la puerta (lector RFID RC522 + servo) — no estaba en el roadmap original, se agregó a pedido. Implementado sin validación de UID todavía: cualquier tarjeta/tag leído abre la puerta 20s y se cierra sola. Falta agregar una lista de UIDs autorizados antes de considerarlo terminado para uso real.

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
- [src/sensors.h](src/sensors.h) / [src/sensors.cpp](src/sensors.cpp) — lectura de los 3 sensores ultrasónicos SRF-05 (proximidad x2 + pasillo) y la fotocelda (con anti-rebote); las funciones del PIR (ya no en uso) y del sensor de presión de cama están comentadas
- [src/alerts.h](src/alerts.h) / [src/alerts.cpp](src/alerts.cpp) — control del buzzer compartido (`actualizarBuzzer(pin, frecuenciaHz)`) y del botón de pánico (interrupción + anti-rebote + toggle)
- [src/display.h](src/display.h) / [src/display.cpp](src/display.cpp) — control directo del LED de pánico (`digitalWrite`, ya no vía 74HC595)
- [src/lighting.h](src/lighting.h) / [src/lighting.cpp](src/lighting.cpp) — encender/apagar la luz de pasillo
- [src/access.h](src/access.h) / [src/access.cpp](src/access.cpp) — control de acceso a la puerta: lector RFID RC522 (librería `MFRC522`) + servo del pestillo (librería `Servo`), no bloqueante
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
