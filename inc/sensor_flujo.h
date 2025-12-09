#ifndef SENSOR_FLUJO_H
#define SENSOR_FLUJO_H

#include <Arduino.h>

void initFlujo();                  // Configura el pin y la interrupcion
void TaskFlujo(void *pvParameters); // Tarea que convierte pulsos en ml

#endif
