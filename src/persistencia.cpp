#include "persistencia.h"
#include <Preferences.h>

// Objeto global para manejar NVS
static Preferences prefs;

// Nombre del "namespace" en NVS para este proyecto.
// Lo puedes cambiar si quieres, pero que sea el mismo siempre.
static const char *NVS_NAMESPACE = "llenador";

// Clave para el total de botellas
static const char *KEY_BOTELLAS = "botellas";

// Clave para el ultimo error
static const char *KEY_ULT_ERROR = "ultError";

void persistenciaInit()
{
  // Modo read/write (false = RW, true = solo lectura)
  prefs.begin(NVS_NAMESPACE, false);
}

uint32_t cargarTotalBotellas()
{
  // Si no existe la clave, devuelve 0
  uint32_t total = prefs.getUInt(KEY_BOTELLAS, 0);
  return total;
}

void guardarTotalBotellas(uint32_t total)
{
  prefs.putUInt(KEY_BOTELLAS, total);
}

uint8_t cargarUltimoError()
{
  // Si no existe la clave, devolvemos ERROR_CODIGO_NINGUNO
  uint32_t cod = prefs.getUInt(KEY_ULT_ERROR, ERROR_CODIGO_NINGUNO);
  // Lo guardamos como uint8_t por claridad; el valor cabe sin problema.
  return (uint8_t)cod;
}

void guardarUltimoError(uint8_t codigo)
{
  // Guardamos como uint32 por simplicidad (NVS trabaja comodo con eso)
  prefs.putUInt(KEY_ULT_ERROR, (uint32_t)codigo);
}

