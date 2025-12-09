#ifndef PANTALLA_H
#define PANTALLA_H

#include <Arduino.h>
#include "estados_sistema.h"

// Inicializa el LCD
void initPantalla();

// Tarea FreeRTOS para actualizar la pantalla periodicamente
void TaskPantalla(void *pvParameters);

#endif
