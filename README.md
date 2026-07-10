# Casa IoT para Adultos Mayores

Sistema domótico de asistencia para personas mayores implementado sobre **Arduino Uno**, desarrollado en VS Code con **PlatformIO** y simulado con **Wokwi**.

El firmware es no bloqueante (basado en `millis()`, sin `delay()` en el loop principal) y está organizado en módulos por funcionalidad en vez de un único archivo monolítico.

## Funcionalidades implementadas

- **Alarma de proximidad**: 2 sensores ultrasónicos HC-SR04. Cuando cualquiera detecta un objeto a ≤20 cm, enciende 2 LEDs (vía registro de desplazamiento 74HC595, usando las salidas Q0/Q1) y suena el buzzer.
- **Luz de pasillo**: un sensor PIR enciende una luz al detectar movimiento y la apaga sola tras 15 s sin movimiento.
- **Alarma de inmovilidad en cama**: un sensor de presión (FSR, simulado con un potenciómetro en Wokwi) detecta si la persona lleva más de 7 horas seguidas acostada; si es así, enciende una luz de alarma y suena el buzzer con un tono distinto al de proximidad. Una ausencia de presión breve (ej. un giro en la cama) no reinicia el conteo.
- **Botón de pánico**: interrupción en D3 (INT1), sin sondeo. Funciona como interruptor: un pulso activa la alarma, el siguiente la desactiva. Es la alarma de mayor prioridad del sistema — silencia cualquier otra en el buzzer compartido y enciende un LED dedicado (Q2 del 74HC595).

El buzzer y el registro 74HC595 son recursos compartidos entre las distintas alarmas; [src/main.cpp](src/main.cpp) arbitra la prioridad (pánico > cama > proximidad) antes de pedir una única frecuencia a `actualizarBuzzer()` / un único patrón de LEDs a `actualizarLeds()`, una vez por vuelta de `loop()`.

## Estructura del proyecto

```
include/config.h    Constantes de pines y parámetros (umbrales, timeouts, debounce)
src/main.cpp        Punto de entrada: orquesta los módulos con timers millis()
src/sensors.*        Lectura de los 2 HC-SR04, el PIR y el sensor de presión de cama
src/alerts.*          Control del buzzer compartido (actualizarBuzzer) y del botón de pánico
src/display.*         Control de los LEDs vía 74HC595 (proximidad y pánico)
src/lighting.*        Encender/apagar la luz de pasillo y la luz de alarma de cama
platformio.ini        Entorno único `uno` (atmelavr, framework Arduino)
diagram.json           Circuito de Wokwi
wokwi.toml              Configuración del simulador
.claude/                Agentes, skills, comandos y hooks de Claude Code para este proyecto
```

## Conexiones (pines)

| Componente | Pin(es) |
|---|---|
| Buzzer (compartido) | 4 |
| HC-SR04 #1 | TRIG → 9, ECHO → 10 |
| HC-SR04 #2 | TRIG → 7, ECHO → 8 |
| 74HC595 | SHCP (clock) → 11, STCP (latch) → 12, DS (datos) → 13 |
| Barra de LEDs (vía 74HC595) | Q0/Q1 proximidad, Q2 pánico; Q3–Q7 siempre apagadas |
| PIR de pasillo | OUT → 2 |
| Luz de pasillo | 6 |
| Sensor de presión de cama (FSR / potenciómetro) | SIG → A0 |
| Luz de alarma de cama | 5 |
| Botón de pánico | 3 (INT1), `INPUT_PULLUP` + `attachInterrupt(FALLING)` |

Nota: el pin 0/1 (Serial) queda libre por defecto. D2/D3 son los únicos pines de interrupción externa (INT0/INT1) del Uno: D2 lo usa el PIR (por sondeo, no interrupción) y D3 el botón de pánico (por interrupción).

Si se cambia un pin en el código hay que actualizar `diagram.json` (y viceversa); ver la skill `platformio-wokwi-workflow`.

## Requisitos

- [PlatformIO](https://platformio.org/) (CLI `pio`, normalmente vía la extensión de VS Code)
- Extensión de Wokwi para VS Code, o [wokwi.com](https://wokwi.com), para simular sin hardware físico

## Comandos

```sh
pio run                     # Compilar
pio run --target upload     # Subir a una placa Arduino Uno física
pio device monitor          # Monitor serial (9600 baudios)
pio run --target clean      # Limpiar artefactos de compilación
```

No hay configuración de lint ni de tests automatizados (el directorio `test/` es scaffolding vacío de PlatformIO).

## Simulación en Wokwi

1. `pio run` (genera `.pio/build/uno/firmware.hex`/`.elf`, que `wokwi.toml` referencia).
2. Abrir la simulación con la extensión de Wokwi para VS Code (o subir `diagram.json` a wokwi.com).
3. Verificar que los pines de `diagram.json` sigan sincronizados con `include/config.h`.

## Restricciones de hardware (Arduino Uno / ATmega328P)

- 32 KB de Flash, 2 KB de SRAM
- 6 pines analógicos (A0–A5)
- 14 pines digitales (2–13 usables; 0/1 reservados para Serial)
- Pines PWM: 3, 5, 6, 9, 10, 11
- Sin WiFi/Bluetooth nativo
- Sin RTOS: código secuencial no bloqueante, sin `delay()` en `loop()`
- SRAM limitada: se evita `String`, se usan `char[]` y el macro `F()` para literales

## Limitaciones conocidas

- **Sensor de cama / botón de pánico desconectados**: con el cableado actual, un cable cortado o un sensor desconectado se lee igual que "sin presión" / "botón no presionado" — el sistema no distingue una falla de hardware de un estado normal, y no lo reporta. Distinguirlo requeriría circuitería adicional (documentado en `include/config.h`).
- **Reinicio del Arduino durante la alarma de cama**: si el Arduino se reinicia (corte de luz, etc.) mientras alguien lleva horas acostado, el conteo de 7 horas se pierde y arranca de nuevo desde 0, sin más aviso que un mensaje en el monitor serial al arrancar.
- **Alarma de pánico por toggle**: varios pulsos seguidos por confusión pueden dejar la alarma en el estado contrario al deseado; no hay retroalimentación distinta entre activar y desactivar.

## Roadmap

### Seguridad / Emergencias
- [x] Botón de pánico (interrupción INT1/D3) — toggle, prioridad máxima sobre las demás alarmas
- [ ] Detector de caída (MPU6050 vía I2C)
- [x] Alarma sonora (buzzer pasivo) — proximidad, inmovilidad en cama y pánico
- [x] Alarma de inmovilidad prolongada en cama (7h)

### Monitoreo ambiental
- [ ] Temperatura y humedad (DHT11/DHT22)
- [ ] Detector de gas/humo (MQ-2)
- [x] Sensor de movimiento PIR (pasillo)

### Confort / Accesibilidad
- [x] Control de iluminación on/off (PIR de pasillo, alarma de cama) — falta el dimmer PWM
- [ ] Display LCD 16x2 I2C con info de estado
- [ ] Alarmas de medicamentos (RTC DS3231 + buzzer)

### Conectividad
- [ ] ESP8266 en modo AT para MQTT/HTTP hacia un broker/dashboard
- [ ] Alternativa: HC-05 Bluetooth para app móvil cercana

## Convenciones de código

- Variables globales de estado en MAYÚSCULAS (`ALERTA_ACTIVA`, `EN_CAMA`)
- Funciones en camelCase, archivos en snake_case
- Comentarios en español
- Nunca `delay()` en `loop()` — siempre `millis()` con temporizadores no bloqueantes
- Constantes de pines centralizadas en `config.h`

Ver [CLAUDE.md](CLAUDE.md) para el detalle completo de arquitectura, convenciones y el toolkit de Claude Code (`.claude/`) configurado para este proyecto.

## Licencia

MIT — ver [LICENSE](LICENSE).
