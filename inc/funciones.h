// Declaraciones: enums, struct de estado, constantes, prototipos de funciones y tareas

#ifndef FUNCIONES_H
#define FUNCIONES_H

#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h" // para el mutex

// CONFIGURACION GENERAL
// pendiente:
// define pines, constanes, tiempos
// Pines de los botones
#define BTN_MARCHA    14
#define BTN_PARADA    27
#define BTN_LLENAR    26

// Pin de la bomba
#define ACT_BOMBA     23

// Pines del sensor ultrasonico 
#define US_TRIG       18
#define US_ECHO       19

// Tiempo de debounce 
#define DEBOUNCE_TIME_MS  30

// Rango de deteccion de botella (cm)
#define DIST_MIN_BOTELLA_CM   3.0f
#define DIST_MAX_BOTELLA_CM   15.0f
#define LECTURAS_ESTABLES     3

// TIPOS Y ESTADO DEL SISTEMA
enum EstadoSistema
{
  SISTEMA_DETENIDO,
  ESPERANDO_BOTELLA,
  BOTELLA_DETECTADA,
  LLENANDO,
  LLENADO_COMPLETADO,
  ERROR_SISTEMA
};

struct EstadoCompartido
{
  bool botonMarcha;
  bool botonParada;
  bool botonLlenar;
  bool botellaPresente;
  float volumenMedidoMl;
  EstadoSistema estadoActual;
  bool errorBotellaRetirada;
  bool errorSinFlujo;
  bool errorMuchoTiempo;
};

// inicializacion del estado global (crear estado y mutex)
void inicializarEstadoSistema();

// Acceso seguro al estado compartido
void leerEstado(EstadoCompartido &dest);
void escribirEstado(const EstadoCompartido &src);

// PROTOTIPOS DE TAREAS FREERTOS
void TaskDebug(void *pvParameters);
void TaskControl(void *pvParameters);
void TaskIO(void *pvParameters);

// BOTONES
void procesarLecturaBotones();

// ACTUADORES
void bombaOn();
void bombaOff();

// SENSORES
// pendiente: flujo, etc
void procesarUltrasonico();

//PANTALLA
// pendiente: funciones para actualizar interfaz de usuario

// FSM (MAQUINA DE ESTADOS)
// pendiente: funciones para cambiar de estado, procesar eventos, etc

#endif