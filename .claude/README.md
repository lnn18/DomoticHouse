# Toolkit Claude Code — Casa IoT para Adultos Mayores

Configuración de Claude Code específica de este proyecto (Arduino Uno + PlatformIO + Wokwi). A diferencia de un plugin instalable (como ECC), este toolkit vive directamente en `.claude/` de este repositorio: no requiere marketplace ni bootstrap de plugin, todos los hooks apuntan a scripts locales por ruta relativa.

## Estructura

| Carpeta | Contenido |
|---|---|
| `agents/` | `arduino-reviewer` (calidad de código embebido), `arduino-build-resolver` (errores de compilación PlatformIO), `iot-safety-reviewer` (seguridad de dominio: pánico, caídas, alarmas) |
| `skills/` | `arduino-embedded-standards`, `platformio-wokwi-workflow`, `iot-elder-care-safety` |
| `commands/` | `/build`, `/review`, `/sim`, `/new-module`, `/pin-check` |
| `rules/` | `arduino-cpp.md`, `security-iot.md`, `project-conventions.md` — siempre cargadas por los agentes |
| `mcp-configs/` | `mcp-servers.json` — set mínimo (context7, github) para copiar a `~/.claude.json` si se necesitan |
| `scripts/hooks/` | Hooks de advertencia (nunca bloquean): sincronía de pines, uso de `delay()`, recordatorio de `pio run` |
| `scripts/lib/` | `pins.js` — utilidad compartida de comparación de pines código↔`diagram.json` |
| `settings.json` | Cablea los hooks y los permisos de `Bash` para comandos `pio` |

## Filosofía

- **Solo advertencia**: ningún hook bloquea `Edit`/`Write`/`Bash`. Todo es feedback informativo pensado para un proyecto educativo/hobby, no para un pipeline de CI estricto.
- **Español**: todo el contenido (agentes, skills, comandos, reglas, mensajes de hooks) está en español, igual que las convenciones ya definidas en `CLAUDE.md` del proyecto (comentarios en español, etc.).
- **Acotado al dominio**: a diferencia de un toolkit genérico, incluye un agente y una skill específicos de **seguridad para adultos mayores** (`iot-safety-reviewer`, `iot-elder-care-safety`) — el valor real de este proyecto no es solo "que compile", sino que las funciones de emergencia sean confiables.

## Cómo extenderlo

Al agregar un sensor/actuador nuevo del roadmap (DHT11/22, MQ-2, PIR, MPU6050, RTC DS3231, ESP8266, HC-05):

1. Usar `/new-module` para el andamiaje del módulo.
2. Si el sensor es parte de una función de seguridad (caídas, pánico, medicamentos), consultar `skill: iot-elder-care-safety` antes de implementar.
3. Si agrega conectividad, consultar `rules/security-iot.md`.
4. Si se necesita documentación de una librería nueva, usar el MCP `context7` (`mcp-configs/mcp-servers.json`).
5. Correr `/build`, luego `/review`, luego `/sim` antes de dar la funcionalidad por terminada.

No hace falta crear un agente nuevo por cada sensor — los 3 agentes actuales (código, build, seguridad de dominio) cubren el ciclo completo. Agregar un agente nuevo solo si surge una categoría de revisión genuinamente distinta (p. ej. si se agrega una app móvil complementaria).
