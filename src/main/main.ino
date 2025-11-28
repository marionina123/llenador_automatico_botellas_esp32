#include "funciones.h"

void setup() 
{
  Serial.begin(115200);
  delay(500); //pequenio respiro para que el puerto se estabilice

  Serial.println();
  Serial.println("Boot Llenador Automatico ESP32");

  // Inicializar estructura de estado y cualquier cosa global
  inicializarEstadoSistema();

  // CREACION DE TAREAS FREERTOS
  // Tarea de debug
  xTaskCreatePinnedToCore(
    TaskDebug,            // Funcion que implementa la tarea
    "TaskDebug",          // Nombre (solo para debug)
    4096,                 // Tamanio de stack en palabras
    nullptr,              // pvParameters (no se usan, asi que nullptr)
    1,                    // Prioridad (1 = baja)
    nullptr,              // Handle de la tarea (no necesaria aun)
    1                     // Core: 0 o 1 (ESP32 tiene 2 cores)
  );

  // Tarea de control (mayor prioridad por mayor importancia)
  xTaskCreatePinnedToCore(
    TaskControl,
    "TaskControl",
    4096,
    nullptr,
    2,                    // prioridad mayor
    nullptr,
    1
  );

  // Tarea de IO (mandada a otro core)
  xTaskCreatePinnedToCore(
    TaskIO,
    "TaskIO",
    4096,
    nullptr,
    1,
    nullptr,
    0                     // core 0
  );
}

void loop() 
{
}
