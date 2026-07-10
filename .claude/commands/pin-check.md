---
description: Compara los pines declarados en el código (src/, include/) contra las conexiones definidas en diagram.json y reporta discrepancias.
---

# Verificación de Pines

## Qué Hace

Ejecuta la utilidad `scripts/lib/pins.js` para:

1. Extraer las asignaciones de pines del código (`int xPin = N;`, `#define X_PIN N`) en `src/**/*.cpp` e `include/**/*.h`.
2. Extraer las conexiones a pines de Arduino (`uno:N`) desde `diagram.json`.
3. Comparar ambos conjuntos y reportar:
   - Pines usados en el código que no aparecen conectados en `diagram.json`.
   - Pines conectados en `diagram.json` que no aparecen referenciados en el código.

## Cómo Ejecutarlo

```bash
node .claude/scripts/lib/pins.js
```

## Cuándo Usarlo

- Después de agregar un sensor/actuador nuevo (`/new-module`).
- Antes de simular en Wokwi (`/sim`).
- Cuando el hook `pin-consistency-check.js` reportó una advertencia y quieres el detalle completo.

## Nota

Esta verificación es una **ayuda heurística**, no un compilador: usa expresiones regulares simples sobre el código fuente. Si el resultado no coincide con lo esperado, verificar manualmente antes de confiar en él ciegamente.

## Relacionado

- Skill: `skills/platformio-wokwi-workflow/` (tabla de mapeo de pines actual)
- Hook: `scripts/hooks/pin-consistency-check.js`
