# include "funciones.h"

// CONFIGURACION GENERAL


// Configuracion / pines / constantes

// ESTADO GLOBAL DEL SISTEMA
static EstadoCompartido g_estado; // variable global interna a este archivo

void inicializarEstadoSistema()
{
  // Estado inciial seguro
  g_estado.botonMarcha        = false;
  g_estado.botonParada        = false;
  g_estado.botonLlenar        = false;
  g_estado.botellaPresente    = false;
  g_estado.volumenMedidoMl    = 0.0f;
  g_estado.estadoActual       = SISTEMA_DETENIDO;
  // pendiente: crear mutex para acceso seguro
  // posiblemente acceso a las colas de eventos
}

// TAREAS FRRERTOS
// Tarea de debug: solo informa que esta viva cada 2 segundos
void TaskDebug(void *pvParameters)
{
  // Implementacion pendiente
  (void) pvParameters;
  for(;;)
  {
    Serial.println("[TaskDebug] viva");
    vTaskDelay(pdMS_TO_TICKS(2000)); //tiempo sin bloquear a las demas tareas
  }
}

// Tarea de control, posteriormente llevara FSM
void TaskControl(void *pvParameters)
{
  (void) pvParameters;
  for(;;)
  {
    Serial.println("[TaskControl] viva");
    vTaskDelay(pdMS_TO_TICKS(1000)); 
  }
}

// Tarea de IO: posteriormente leera botones/sensores
void TaskIO(void *pvParameters)
{
  (void) pvParameters;
  for(;;)
  {
    Serial.println("[TaskIO] viva");
    vTaskDelay(pdMS_TO_TICKS(1500));
  }
}
// BOTONES
// pendiente logica de lectura antirrebote, etc 

// ACTUADORES
// Encendido/apagado de bomba, etc

// SENSORES
// ultrasonido, flujo, etc

// PANTALLA
// Actualizacion de mensaje, estado, volumen, etc

// FSM (MAQUINA DE ESTADOS)
// Logica de transicion entre estados principales