---
description: Compila el firmware con PlatformIO e itera arreglando errores con cambios mínimos. Invoca al agente arduino-build-resolver.
---

# Build de PlatformIO

Este comando invoca al agente **arduino-build-resolver** para compilar el firmware y arreglar errores de forma incremental.

## Qué Hace

1. Ejecuta `pio run` y captura la salida.
2. Si hay errores, los agrupa por archivo y los ordena por severidad.
3. Arregla un error a la vez con el cambio mínimo necesario.
4. Vuelve a ejecutar `pio run` tras cada cambio para verificar.
5. Reporta un resumen final: errores arreglados, archivos modificados, estado del build.

## Cuándo Usarlo

- Tras escribir o modificar código en `src/`/`include/`/`lib/`.
- Antes de simular en Wokwi (`/sim`) o subir a la placa física.
- Cuando `pio run` falla y quieres que se arregle incrementalmente en vez de manualmente.

## Comandos Ejecutados

```bash
pio run
pio run -v          # si se necesita más detalle
pio check --skip-packages
```

## Relacionado

- Agente: `agents/arduino-build-resolver.md`
- Skill: `skills/platformio-wokwi-workflow/`
- Siguiente paso recomendado: `/review` antes de dar el cambio por terminado.
