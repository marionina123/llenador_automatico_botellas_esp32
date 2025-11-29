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

// Variables internas ultrasonico
static float ultimaDistanciaCm = 0.0f;
static uint8_t contadorPresente = 0;
static uint8_t contadorAusente  = 0;

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

// Mide distancia en cm usando el sensor ultrasonico
static float medirDistanciaCm()
{
  // Generar pulso de TRIG: LOW 2us, HIGH 10us, LOW
  digitalWrite(US_TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(US_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(US_TRIG, LOW);

  // Medir tiempo del pulso de ECHO en microsegundos
  // timeout de ~25 ms para no bloquear demasiado
  unsigned long duracion = pulseIn(US_ECHO, HIGH, 25000UL);

  if(duracion == 0)
  {
    // No se recibio eco dentro del timeout
    return -1.0f;
  }

  // Distancia en cm:
  // velocidad del sonido ~34300 cm/s
  // distancia = (tiempo_us / 2) * 0.0343
  float distancia = (duracion * 0.0343) / 2.0f;
  return distancia;
}

// ACTUADOR: BOMBA

void bombaOn()
{
  digitalWrite(ACT_BOMBA, HIGH);
  Serial.println("[ACTUADOR] Bomba ON");
}

void bombaOff()
{
  digitalWrite(ACT_BOMBA, LOW);
  Serial.println("[ACTUADOR] Bomba OFF");
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

void procesarUltrasonico()
{
  float d = medirDistanciaCm();
  if(d < 0.0f)
  {
    // Lectura invalida, se puede contar como ausente
    return;
  }

  ultimaDistanciaCm = d;

  bool dentroRango = (d >= DIST_MIN_BOTELLA_CM) && (d <= DIST_MAX_BOTELLA_CM);

  EstadoCompartido est;
  leerEstado(est);

  bool estabaPresente = est.botellaPresente;
  bool hayCambio = false;

  if(dentroRango)
  {
    contadorPresente++;
    contadorAusente = 0;
    if(contadorPresente >= LECTURAS_ESTABLES && !estabaPresente)
    {
      est.botellaPresente = true;
      hayCambio = true;
      contadorPresente = LECTURAS_ESTABLES; // saturar
    }
  }
  else
  {
    contadorAusente++;
    contadorPresente = 0;
    if(contadorAusente >= LECTURAS_ESTABLES && estabaPresente)
    {
      est.botellaPresente = false;
      hayCambio = true;
      contadorAusente = LECTURAS_ESTABLES;
    }
  }

  if(hayCambio)
  {
    escribirEstado(est);
    Serial.print("[ULTRASONICO] Botella: ");
    Serial.print(est.botellaPresente ? "PRESENTE" : "AUSENTE");
    Serial.print("Distancia = ");
    Serial.print(d);
    Serial.println(" cm");
  }
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
    Serial.print(local.botonLlenar);
    Serial.print("Botella =");
    Serial.println(local.botellaPresente ? "SI" : "NO");

    vTaskDelay(pdMS_TO_TICKS(1000)); //tiempo sin bloquear a las demas tareas
  }
}

// Tarea de control, posteriormente llevara FSM
void TaskControl(void *pvParameters)
{
  (void) pvParameters;

  // Configuracion del pin de la bomba como salida
  pinMode(ACT_BOMBA, OUTPUT);
  bombaOff();

  EstadoCompartido local; // copia local de trabajo

  for(;;)
  {
    // Lectura del estado actual
    leerEstado(local);

    // Logica simple de prueba de la bomba
    // Si se presiona PARADA, apagar la bomba y reiniciar estado a SISTEMA_DETENIDO
    if(local.botonParada)
    {
      bombaOff();
      local.estadoActual = SISTEMA_DETENIDO;

      // Limpieza de la flag para no repetir la accion
      local.botonParada = false;
    }

    // Si se presiono MARCHA, encender bomba y cambiar a LLENANDO
    if(local.botonMarcha)
    {
      bombaOn();
      local.estadoActual = LLENANDO;

      // Limpieza de la flag para no repetir la accion
      local.botonMarcha = false;
    }

    // implementacion pendiente para que el boton llenar llene manualmente
    if(local.botonLlenar)
    {
      Serial.println("[Control] Se presiono LLENAR (sin uso todavia)");
      local.botonLlenar = false;
    }

    // Escribir el nuevo estado
    escribirEstado(local);

    vTaskDelay(pdMS_TO_TICKS(50));
  }
}

// Tarea de IO: posteriormente leera botones/sensores
void TaskIO(void *pvParameters)
{
  (void) pvParameters;

  configurarBotones();

  // Configurar pines del ultrasonico
  pinMode(US_TRIG, OUTPUT);
  pinMode(US_ECHO, INPUT); 

  // Asegurar TRIG en LOW al inicio
  digitalWrite(US_TRIG, LOW);

  for(;;)
  {
    procesarLecturaBotones();
    procesarUltrasonico();
    vTaskDelay(pdMS_TO_TICKS(50));
  }
}

// ACTUADORES
// Encendido/apagado de bomba, etc

// SENSORES
// flujo, etc

// PANTALLA
// Actualizacion de mensaje, estado, volumen, etc

// FSM (MAQUINA DE ESTADOS)
// Logica de transicion entre estados principales