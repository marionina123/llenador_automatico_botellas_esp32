#ifndef PERSISTENCIA_H
#define PERSISTENCIA_H

#include <Arduino.h>

// Inicializa el acceso a NVS (memoria no volatil del ESP32)
void persistenciaInit();

// Devuelve el total de botellas llenadas guardado en memoria.
// Si no hay nada guardado, devuelve 0.
uint32_t cargarTotalBotellas();

// Guarda el total de botellas llenadas en memoria.
void guardarTotalBotellas(uint32_t total);

// Codigos para el ultimo error guardado
#define ERROR_CODIGO_NINGUNO        0
#define ERROR_CODIGO_BOTELLA        1
#define ERROR_CODIGO_SIN_FLUJO      2
#define ERROR_CODIGO_TIMEOUT        3

// Devuelve el codigo del ultimo error registrado en memoria.
// Si no hay nada guardado, devuelve ERROR_CODIGO_NINGUNO.
uint8_t cargarUltimoError();

// Guarda el codigo del ultimo error en memoria.
void guardarUltimoError(uint8_t codigo);

#endif
