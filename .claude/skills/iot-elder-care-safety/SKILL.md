---
description: Patrones de seguridad de dominio para funcionalidades de asistencia a adultos mayores — fail-safe, anti-rebote, redundancia de alarmas y accesibilidad.
---

# Seguridad de Dominio — Asistencia a Adultos Mayores

## Cuándo Usar

Al implementar cualquier ítem de las secciones "Seguridad/Emergencias" o "Confort/Accesibilidad" del roadmap en `CLAUDE.md`: botón de pánico, detector de caídas, alarma sonora, temperatura/humedad, gas/humo, PIR, iluminación, LCD, alarmas de medicamentos.

## Cómo Funciona

### Fail-safe por defecto
Un sensor de seguridad desconectado o con lectura inválida debe llevar el sistema a un estado que **alerte**, no a uno silencioso. Ejemplo: si `pulseIn()` del HC-SR04 retorna 0 (timeout), no asumir "sin obstáculo" silenciosamente si ese sensor forma parte de una función de seguridad — distinguir "sin lectura" de "lectura en cero".

### Anti-rebote (debounce)
Para botones e interruptores (botón de pánico, PIR), usar una ventana de tiempo mínima entre lecturas válidas:

```cpp
const unsigned long DEBOUNCE_MS = 50;
unsigned long ultimoCambioBoton = 0;
int ultimoEstadoBoton = HIGH;

void revisarBotonPanico() {
  int estadoActual = digitalRead(PANICO_PIN);
  if (estadoActual != ultimoEstadoBoton && millis() - ultimoCambioBoton > DEBOUNCE_MS) {
    ultimoCambioBoton = millis();
    ultimoEstadoBoton = estadoActual;
    if (estadoActual == LOW) {
      activarAlarmaPanico();
    }
  }
}
```

Preferir `attachInterrupt` (INT0/INT1) para el botón de pánico en vez de sondeo, para minimizar la latencia de respuesta.

### Redundancia de alarmas
Una alarma crítica no debe depender de un único canal: combinar buzzer (sonido) + LED/barra de LEDs (visual). Si se agrega conectividad (ESP8266/HC-05), considerar un tercer canal (notificación remota) sin que sea el único medio de alerta local.

### Detector de caídas (MPU6050)
- Definir un umbral de aceleración y una ventana de confirmación (p. ej. 2-3 lecturas consecutivas por encima del umbral en menos de 500ms) para reducir falsos positivos sin perder sensibilidad a caídas reales.
- Nunca bloquear el `loop()` esperando estabilizar el sensor: usar `millis()` para espaciar las lecturas I2C.

### Accesibilidad
- LCD: texto grande, alto contraste, mensajes cortos y directos ("ALERTA CAÍDA", no jerga técnica).
- Tonos de buzzer distintos por tipo de evento (frecuencia/patrón distinto para pánico vs. medicamento vs. temperatura alta) para que se puedan distinguir de oído.

## Checklist antes de dar por completa una funcionalidad de seguridad

- [ ] ¿Qué pasa si el sensor se desconecta físicamente?
- [ ] ¿La detección usa interrupción o sondeo con `millis()` (nunca `delay()`)?
- [ ] ¿Hay anti-rebote donde corresponde?
- [ ] ¿La alarma tiene al menos dos canales de salida?
- [ ] ¿El mensaje/tono es distinguible de otras alertas del sistema?

Para la revisión formal de estos criterios usar el agente `iot-safety-reviewer` o el comando `/review`.
