---
description: Checklist para simular el firmware en Wokwi — verifica build, sincronía de pines y pasos para lanzar la simulación.
---

# Simular en Wokwi

## Qué Hace

1. Verifica que el último `pio run` fue exitoso (si no, sugiere correr `/build` primero).
2. Verifica sincronía de pines entre el código (`src/`, `include/config.h`) y `diagram.json` (usa `scripts/lib/pins.js`, igual que `/pin-check`).
3. Confirma que `wokwi.toml` apunta al binario correcto (`.pio/build/uno/firmware.hex` / `.elf`).
4. Da los pasos para lanzar la simulación.

## Pasos de Simulación

```bash
pio run   # asegura que el binario esté actualizado
```

Luego, abrir la simulación con una de estas opciones:
- Extensión de Wokwi para VS Code (comando "Wokwi: Start Simulator" con `diagram.json` abierto).
- wokwi.com, subiendo `diagram.json` y el binario compilado si se simula fuera del editor.

## Cuándo Usarlo

- Antes de validar visualmente un cambio de comportamiento (nuevo patrón de LEDs, nuevo tono de alarma, etc.).
- Después de modificar `diagram.json` o `wokwi.toml`.
- Como paso previo a subir el firmware a la placa física.

## Relacionado

- Skill: `skills/platformio-wokwi-workflow/`
- Comando: `/pin-check`
- Comando: `/build`
