# include "funciones.h"

// CONFIGURACION GENERAL


// Configuracion / pines / constantes
void configurarBotones()
{
  pinMode(BTN_MARCHA, INPUT_PULLUP);
  pinMode(BTN_PARADA, INPUT_PULLUP);
  pinMode(BTN_LLENAR, INPUT_PULLUP);
}

// Variables internas antirrebote
static bool lastMarcha = false, stableMarcha = false;
static bool lastParada = false, stableParada = false;
static bool lastLlenar = false, stableLlenar = false;

static uint32_t lastTimeMarcha = 0;
static uint32_t lastTimeParada = 0;
static uint32_t lastTimeLlenar = 0;

void procesarLecturaBotones()
{
  // Lectura cruda (invertida porque INPUT_PULLUP: LOW = presionado)
  bool rawMarcha = (digitalRead(BTN_MARCHA) == LOW);
  bool rawParada = (digitalRead(BTN_PARADA) == LOW);
  bool rawLlenar = (digitalRead(BTN_LLENAR) == LOW);

  uint32_t ahora = millis();

  // Debounce MARCHA
  if(rawMarcha != lastMarcha)
  {
    lastTimeMarcha = ahora;
    lastMarcha = rawMarcha;
  }

  if((ahora - lastTimeMarcha) > DEBOUNCE_TIME_MS)
  {
    if(stableMarcha != rawMarcha)
    {
      stableMarcha = rawMarcha;

      if(stableMarcha)
      {
        Serial.println("[BOTON] MARCHA presionado");

        EstadoCompartido est;
        leerEstado(est);
        est.botonMarcha = true;
        escribirEstado(est);
      }
    }
  }

  // Debounce PARADA
  if(rawParada != lastParada)
  {
    lastTimeParada = ahora;
    lastParada = rawParada;
  }

  if((ahora - lastTimeParada) > DEBOUNCE_TIME_MS)
  {
    if(stableParada != rawParada)
    {
      stableParada = rawParada;

      if(stableParada)
      {
        Serial.println("[BOTON] PARADA presionado");

        EstadoCompartido est;
        leerEstado(est);
        est.botonParada = true;
        escribirEstado(est);
      }
    }
  }

  // Debounce Llenar
  if(rawLlenar != lastLlenar)
  {
    lastTimeLlenar = ahora;
    lastLlenar = rawLlenar;
  }

  if((ahora - lastTimeLlenar) > DEBOUNCE_TIME_MS)
  {
    if(stableLlenar != rawLlenar)
    {
      stableLlenar = rawLlenar;

      if(stableLlenar)
      {
        Serial.println("[BOTON] LLENAR presionado");

        EstadoCompartido est;
        leerEstado(est);
        est.botonLlenar = true;
        escribirEstado(est);
      }
    }
  } 
}
// ESTADO GLOBAL DEL SISTEMA
static EstadoCompartido g_estado; // variable global interna a este archivo

// Mutex para proteger el acceso a g_estado
static SemaphoreHandle_t g_mutexEstado = nullptr;
void inicializarEstadoSistema()
{
  // Creacion del mutex si aun no existe
  if(g_mutexEstado == nullptr)
  {
    g_mutexEstado = xSemaphoreCreateMutex();
  }
  // Estado inciial seguro
  g_estado.botonMarcha          = false;
  g_estado.botonParada          = false;
  g_estado.botonLlenar          = false;
  g_estado.botellaPresente      = false;
  g_estado.volumenMedidoMl      = 0.0f;
  g_estado.estadoActual         = SISTEMA_DETENIDO;
  g_estado.errorBotellaRetirada = false;
  g_estado.errorSinFlujo        = false;
  g_estado.errorMuchoTiempo     = false;
}



// Lee una copia del estado protegido por el mutex
void leerEstado(EstadoCompartido &dest)
{
  if(g_mutexEstado == nullptr)
  {
    // Si por alguna razon no existe el mutex, se devuelve tal
    dest = g_estado;
    return;
  }

  if(xSemaphoreTake(g_mutexEstado, portMAX_DELAY) == pdTRUE)
  {
    dest = g_estado; // copia completa
    xSemaphoreGive(g_mutexEstado);
  }
}

// Escribe el estado completo protegido por el mutex
void escribirEstado(const EstadoCompartido &src)
{
  if(g_mutexEstado == nullptr)
  {
    g_estado = src;
    return;
  }

  if(xSemaphoreTake(g_mutexEstado, portMAX_DELAY) == pdTRUE)
  {
    g_estado = src; // copia completa
    xSemaphoreGive(g_mutexEstado);
  }
}

// TAREAS FRRERTOS

// Tarea de debug: solo informa que esta viva cada 2 segundos
void TaskDebug(void *pvParameters)
{
  (void) pvParameters;

  EstadoCompartido local;

  for(;;)
  {
    leerEstado(local);

    Serial.println("[Debugg] Estado actual: ");

    switch(local.estadoActual)
    {
      case SISTEMA_DETENIDO:
        Serial.println("SISTEMA_DETENIDO");
        break;

      case ESPERANDO_BOTELLA:
        Serial.println("ESPERANDO_BOTELLA");
        break;

      case BOTELLA_DETECTADA:
        Serial.println("BOTELLA_DETECTADA");
        break;

      case LLENANDO:
        Serial.println("LLENANDO");
        break;

      case LLENADO_COMPLETADO:
        Serial.println("LLENADO_COMPLETADO");
        break;

      case ERROR_SISTEMA:
        Serial.println("ERROR_SISTEMA");
        break;
    }

    Serial.print("Marcha=");
    Serial.print(local.botonMarcha);
    Serial.print("Parada=");
    Serial.print(local.botonParada);
    Serial.print("llenar=");
    Serial.println(local.botonLlenar);

    vTaskDelay(pdMS_TO_TICKS(1000)); //tiempo sin bloquear a las demas tareas
  }
}

// Tarea de control, posteriormente llevara FSM
void TaskControl(void *pvParameters)
{
  (void) pvParameters;

  EstadoCompartido local; // copia local de trabajo

  for(;;)
  {
    // Lectura del estado actual
    leerEstado(local);

    // Cambio de estado solo a modo de prueba
    switch(local.estadoActual)
    {
      case SISTEMA_DETENIDO:
        local.estadoActual = ESPERANDO_BOTELLA;
        break;

      case ESPERANDO_BOTELLA:
        local.estadoActual = LLENANDO;
        break;

      case LLENANDO:
        local.estadoActual = LLENADO_COMPLETADO;
        break;
      
      case LLENADO_COMPLETADO:
        local.estadoActual = ERROR_SISTEMA;
        break;
      
      case ERROR_SISTEMA:
        local.estadoActual = SISTEMA_DETENIDO;
        break;
    }

    // Escribir el nuevo estado
    escribirEstado(local);

    vTaskDelay(pdMS_TO_TICKS(2000));
  }
}

// Tarea de IO: posteriormente leera botones/sensores
void TaskIO(void *pvParameters)
{
  (void) pvParameters;

  configurarBotones();

  for(;;)
  {
    procesarLecturaBotones();
    vTaskDelay(pdMS_TO_TICKS(20));
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