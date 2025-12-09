#include "sensor_ultrasonico.h"
#include "config_sistema.h"
#include "estados_sistema.h"

// -----------------------------------------------------------------------------
// Medicion de distancia (misma idea que tu codigo de prueba)
// -----------------------------------------------------------------------------
static float medirDistanciaCm()
{
  // 1. Limpiar TRIG
  digitalWrite(PIN_ULTRA_TRIG, LOW);
  delayMicroseconds(2);

  // 2. Pulso de disparo 10 us
  digitalWrite(PIN_ULTRA_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_ULTRA_TRIG, LOW);

  // 3. Medir eco (con timeout de seguridad)
  unsigned long duracion = pulseIn(PIN_ULTRA_ECHO, HIGH, 30000UL); // 30 ms timeout

  if (duracion == 0) {
    // No se midio nada (timeout)
    return 999.0f; // distancia muy grande
  }

  // 4. Calcular distancia en cm
  // Distancia = (duracion * 0.0343) / 2;
  float distancia = (duracion * 0.0343f) / 2.0f;
  return distancia;
}

void initSensorUltrasonico()
{
  pinMode(PIN_ULTRA_TRIG, OUTPUT);
  pinMode(PIN_ULTRA_ECHO, INPUT);
  digitalWrite(PIN_ULTRA_TRIG, LOW);
}

// -----------------------------------------------------------------------------
// TaskUltrasonico con LOGICA DE ESTABILIDAD CORRECTA
//  - Mide distancia periodicamente.
//  - Compara con el umbral: d < BOTELLA_DIST_UMBRAL_CM => lectura "hay botella".
//  - Solo cambia botellaPresente cuando esa lectura distinta se repite
//    ULTRA_SAMPLES_STABLE veces seguidas.
// -----------------------------------------------------------------------------
void TaskUltrasonico(void *pvParameters)
{
  (void) pvParameters;

  Serial.println("TaskUltrasonico: iniciada (modo estable)");

  const TickType_t periodo = pdMS_TO_TICKS(ULTRA_PERIOD_MS);

  bool estadoBotellaEstable = false;   // lo que el sistema cree actualmente
  uint8_t contadorCambio = 0;          // cuantas lecturas consecutivas "distintas" llevamos

  while (true) {
    float d = medirDistanciaCm();

    // Lectura instantanea (sin filtro): ¿hay botella debajo del umbral?
    bool lecturaHayBotella = (d < BOTELLA_DIST_UMBRAL_CM);

    // Log continuo para que veas lo que esta midiendo SIEMPRE
    Serial.print("Ultrasonico: dist = ");
    Serial.print(d);
    Serial.print(" cm | lecturaHayBotella = ");
    Serial.println(lecturaHayBotella ? "SI" : "NO");

    // -----------------------
    // Filtro de estabilidad
    // -----------------------
    if (lecturaHayBotella == estadoBotellaEstable) {
      // La lectura coincide con lo que ya creiamos: no hay intento de cambio
      // => reseteamos el contador de "cambio potencial"
      contadorCambio = 0;
    } else {
      // La lectura NO coincide con el estado estable actual:
      // podriamos estar en un cambio real o en ruido

      if (contadorCambio < 255) {
        contadorCambio++;
      }

      if (contadorCambio >= ULTRA_SAMPLES_STABLE) {
        // Llevamos N lecturas seguidas en el nuevo estado
        estadoBotellaEstable = lecturaHayBotella;
        contadorCambio = 0;

        // Actualizar estado compartido solo cuando hay CAMBIO ESTABLE
        EstadoCompartido estado = leerEstado();
        estado.botellaPresente = estadoBotellaEstable;
        escribirEstado(estado);

        Serial.print("Ultrasonico: CAMBIO ESTABLE -> botellaPresente = ");
        Serial.println(estadoBotellaEstable ? "SI" : "NO");
      }
    }

    vTaskDelay(periodo);
  }
}
