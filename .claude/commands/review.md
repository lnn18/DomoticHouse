---
description: Revisión completa de código C++ embebido y, si aplica, de seguridad de dominio. Invoca arduino-reviewer y (cuando corresponde) iot-safety-reviewer.
---

# Revisión de Código

Este comando invoca primero al agente **arduino-reviewer** para la calidad de código C++ embebido, y luego al agente **iot-safety-reviewer** si el diff toca funcionalidades de seguridad/emergencia o confort/accesibilidad.

## Qué Hace

1. **Identifica cambios**: `git diff -- '*.cpp' '*.h' '*.hpp'`.
2. **Revisión de código embebido** (`arduino-reviewer`): memoria/SRAM, `delay()` fuera de `setup()`, convenciones de nombres, organización de módulos, análisis estático (`pio check`).
3. **Detecta si aplica seguridad de dominio**: si el diff toca botón de pánico, detector de caídas, alarmas, medicamentos, iluminación o LCD.
4. **Revisión de seguridad de dominio** (`iot-safety-reviewer`), solo si aplica: fail-safe, anti-rebote, redundancia de alarmas, accesibilidad.
5. **Reporte combinado** por severidad (CRÍTICO / ALTO / MEDIO).

## Cuándo Usarlo

- Después de escribir o modificar código.
- Antes de subir a la placa física o compartir el cambio.
- Al revisar una nueva funcionalidad del roadmap (seguridad, monitoreo, confort, conectividad).

## Criterios

| Estado | Condición |
|---|---|
| Aprobar | Sin hallazgos CRÍTICO ni ALTO en ninguno de los dos agentes |
| Advertencia | Solo hallazgos MEDIO |
| Revisar antes de continuar | Hallazgos CRÍTICO o ALTO, especialmente en seguridad de dominio |

## Relacionado

- Agentes: `agents/arduino-reviewer.md`, `agents/iot-safety-reviewer.md`
- Skills: `skills/arduino-embedded-standards/`, `skills/iot-elder-care-safety/`
- Usar `/build` primero si el código aún no compila.
