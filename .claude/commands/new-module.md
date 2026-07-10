---
description: Crea el andamiaje de un nuevo módulo (.h + .cpp) siguiendo la convención de un archivo por funcionalidad del proyecto.
---

# Crear Nuevo Módulo

## Qué Hace

Dado el nombre de una funcionalidad (p. ej. `fall_detector`, `medication_alarm`, `lighting`), crea:

- `include/<nombre>.h` — con include guard, prototipos de funciones públicas en camelCase, comentarios en español.
- `src/<nombre>.cpp` — con la implementación, usando `millis()` para cualquier temporización (nunca `delay()`).

## Plantilla de `.h`

```cpp
#ifndef <NOMBRE>_H
#define <NOMBRE>_H

#include <Arduino.h>

// Inicializa el módulo (pinMode, estado inicial, etc.)
void <nombre>Setup();

// Debe llamarse en cada iteración del loop() principal; no bloqueante
void <nombre>Update();

#endif
```

## Plantilla de `.cpp`

```cpp
#include "<nombre>.h"
#include "config.h"

// Estado del módulo (MAYÚSCULAS si es global compartido)
static unsigned long ultimaActualizacion = 0;

void <nombre>Setup() {
  // configurar pines, estado inicial
}

void <nombre>Update() {
  unsigned long ahora = millis();
  if (ahora - ultimaActualizacion >= INTERVALO_MS) {
    ultimaActualizacion = ahora;
    // lógica periódica no bloqueante
  }
}
```

## Después de Crear el Módulo

1. Registrar cualquier pin nuevo en `config.h` (constante centralizada, no dispersa).
2. Llamar a `<nombre>Setup()` desde `setup()` en `main.cpp` y `<nombre>Update()` desde `loop()`.
3. Si el módulo usa un pin nuevo, actualizar `diagram.json` — usar `/pin-check` para verificar sincronía.
4. Ejecutar `/build` para confirmar que compila.

## Relacionado

- Skill: `skills/arduino-embedded-standards/` (organización de módulos)
- Comando: `/pin-check`
