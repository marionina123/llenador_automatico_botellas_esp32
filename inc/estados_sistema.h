#ifndef ESTADOS_SISTEMA_H
#define ESTADOS_SISTEMA_H

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <stdint.h>

// Estados globales del sistema
enum EstadoSistema
{
  SISTEMA_DETENIDO = 0,
  ESPERANDO_BOTELLA,
  BOTELLA_DETECTADA,
  LLENANDO,
  LLENADO_COMPLETADO,
  ERROR_SISTEMA
};

// Toda la informacion importante del sistema
// Toda la informacion importante del sistema
struct EstadoCompartido
{
  // Botones (eventos o flags)
  bool botonMarchaPresionado;
  bool botonParadaPresionado;
  bool botonLlenarPresionado;

  // Sensores
  bool botellaPresente;
  float volumenMedidoMl;   // volumen acumulado (ml) en el ciclo actual

  // Estado global
  EstadoSistema estadoActual;

  // Estado de la bomba (para debug y coherencia)
  bool bombaEncendida;

  // Modo de llenado
  // true  -> modo automatico (llenado se inicia solo al detectar botella)
  // false -> modo manual (requiere boton LLENAR en BOTELLA_DETECTADA)
  bool modoAutomatico;

  // Errores
  bool errorBotellaRetirada;
  bool errorSinFlujo;
  bool errorTimeout;

  // Codigo del ultimo error (persistente)
  // Usa ERROR_CODIGO_* definido en persistencia.h
  uint8_t ultimoErrorCodigo;

  // Datos persistentes
  uint32_t totalBotellasLlenadas;
};


// Funciones de manejo del estado compartido
void estadoSistemaInit();                          // Inicializa struct + mutex
EstadoCompartido leerEstado();                     // Devuelve una copia segura
void escribirEstado(const EstadoCompartido &nuevo); // Escribe de forma segura

#endif
