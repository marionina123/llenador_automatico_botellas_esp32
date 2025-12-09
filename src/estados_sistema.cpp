#include "estados_sistema.h"
#include "persistencia.h"

// Variables globales internas a este modulo
static EstadoCompartido g_estado;
static SemaphoreHandle_t g_mutexEstado = nullptr;

void estadoSistemaInit()
{
  // inicializar valores por defecto
  g_estado.botonMarchaPresionado  = false;
  g_estado.botonParadaPresionado  = false;
  g_estado.botonLlenarPresionado  = false;

  g_estado.botellaPresente = false;
  g_estado.volumenMedidoMl = 0.0f;

  g_estado.estadoActual = SISTEMA_DETENIDO;

  g_estado.bombaEncendida = false;

  g_estado.modoAutomatico = true; // Por defecto: modo automatico activado

  g_estado.errorBotellaRetirada = false;
  g_estado.errorSinFlujo        = false;
  g_estado.errorTimeout         = false;

  // Cargar desde memoria no volatil (NVS)
  g_estado.totalBotellasLlenadas = cargarTotalBotellas();
  g_estado.ultimoErrorCodigo     = cargarUltimoError();

  // Crear mutex si aun no existe
  if (g_mutexEstado == nullptr)
  {
    g_mutexEstado = xSemaphoreCreateMutex();
  }
}

EstadoCompartido leerEstado()
{
  EstadoCompartido copia;

  if (g_mutexEstado != nullptr)
  {
    if (xSemaphoreTake(g_mutexEstado, portMAX_DELAY) == pdTRUE)
    {
      copia = g_estado; // copia completa
      xSemaphoreGive(g_mutexEstado);
    }
  }
  else
  {
    // Si por algun motivo no hay mutex, se devuelve lo que haya
    copia = g_estado;
  }

  return copia;
}

void escribirEstado(const EstadoCompartido &nuevo)
{
  if (g_mutexEstado != nullptr)
  {
    if (xSemaphoreTake(g_mutexEstado, portMAX_DELAY) == pdTRUE)
    {
      g_estado = nuevo;
      xSemaphoreGive(g_mutexEstado);
    }
  }
  else
  {
    // Fallback: sin mutex (no deberia pasar si se llama a init)
    g_estado = nuevo;
  }
}
