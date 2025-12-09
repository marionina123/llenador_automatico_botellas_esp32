#include "sensor_flujo.h"
#include "config_sistema.h"
#include "estados_sistema.h"

// Contador de pulsos incrementado en la ISR
static volatile uint32_t g_contadorPulsos = 0;

// ISR del sensor de flujo
void IRAM_ATTR isrFlujo()
{
  g_contadorPulsos++;   // Mantenerla SIEMPRE minima
}

void initFlujo()
{
  pinMode(PIN_SENSOR_FLUJO, INPUT);  // Entrada de pulsos (con divisor 5V->3.3V)

  // Interrupcion en flanco de subida (ajustable a RISING/FALLING/CHANGE)
  attachInterrupt(
    digitalPinToInterrupt(PIN_SENSOR_FLUJO),
    isrFlujo,
    RISING
  );
}

void TaskFlujo(void *pvParameters)
{
  (void) pvParameters;

  Serial.println("TaskFlujo: iniciada");

  const TickType_t periodo = pdMS_TO_TICKS(FLUJO_PERIOD_MS);

  while (true) {
    // Copiar y resetear contador de pulsos de forma atomica
    noInterrupts();
    uint32_t pulsos = g_contadorPulsos;
    g_contadorPulsos = 0;
    interrupts();

    // Convertir a volumen (ml) en este intervalo
    float mlIntervalo = pulsos * ML_POR_PULSO;

    // Leer estado compartido, acumular y escribir de vuelta
    EstadoCompartido estado = leerEstado();

    float volumenActual = estado.volumenMedidoMl;
    volumenActual += mlIntervalo;
    estado.volumenMedidoMl = volumenActual;

    escribirEstado(estado);

    // Debug por serie
    Serial.print("TaskFlujo: pulsos = ");
    Serial.print(pulsos);
    Serial.print(" | mlIntervalo = ");
    Serial.print(mlIntervalo);
    Serial.print(" | volumenMedidoMl(acumulado) = ");
    Serial.println(volumenActual);

    vTaskDelay(periodo);
  }
}
