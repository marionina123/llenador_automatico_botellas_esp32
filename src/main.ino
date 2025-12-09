#include <Arduino.h>
#include "config_sistema.h"
#include "estados_sistema.h"
#include "actuadores.h"
#include "botones.h"
#include "sensor_ultrasonico.h"
#include "sensor_flujo.h"
#include "persistencia.h"
#include "pantalla.h"

// Prototipos de tareas
void TaskControl(void *pvParameters);
void TaskDebug(void *pvParameters);

void TaskControl(void *pvParameters)
{
  (void) pvParameters;

  Serial.println("TaskControl: iniciada (FSM)");

  const TickType_t periodo = pdMS_TO_TICKS(100); // 100 ms
  TickType_t ultimoDespertar = xTaskGetTickCount();

  // Variables internas para LLENANDO
  TickType_t tickInicioLlenado = 0;
  float volumenAnterior        = 0.0f;
  uint16_t periodosSinFlujo    = 0;

  // Variables internas para deteccion de botella estable
  TickType_t tickBotellaDetectada = 0;
  bool botellaPrev = false;


  while (true)
  {
    // Leer una copia del estado compartido
    EstadoCompartido estado = leerEstado();

    // Captura de eventos y señales
    bool eventoStart  = estado.botonMarchaPresionado;
    bool eventoStop   = estado.botonParadaPresionado;
    bool eventoLlenar = estado.botonLlenarPresionado;
    bool botella      = estado.botellaPresente;
    float volumenMl   = estado.volumenMedidoMl;

    // Limpiar flags de botones (eventos de un solo uso)
    estado.botonMarchaPresionado  = false;
    estado.botonParadaPresionado  = false;
    estado.botonLlenarPresionado  = false;

    // =====================================================
    //  Cambio de modo AUTO / MANUAL solo en SISTEMA_DETENIDO
    // =====================================================
    if (estado.estadoActual == SISTEMA_DETENIDO && eventoLlenar)
    {
      estado.modoAutomatico = !estado.modoAutomatico;
      Serial.print("FSM: modoAutomatico = ");
      Serial.println(estado.modoAutomatico ? "ON (auto)" : "OFF (manual)");
      eventoLlenar = false; // consumimos el evento aqui
    }

    // ==========================
    // STOP: prioridad absoluta
    // ==========================
    if (eventoStop)
    {
      bombaOff();
      estado.bombaEncendida = false;
      estado.estadoActual   = SISTEMA_DETENIDO;

      Serial.println("FSM: STOP -> SISTEMA_DETENIDO, bomba OFF");
    }
    else
    {
      // ==========================
      // FSM principal
      // ==========================
      switch (estado.estadoActual)
      {
        // --------------------------
        // 1) SISTEMA_DETENIDO
        // --------------------------
        case SISTEMA_DETENIDO:
        {
          // Asegurar bomba apagada
          if (estado.bombaEncendida) {
            bombaOff();
            estado.bombaEncendida = false;
          }

          if (eventoStart)
          {
            // Rearmar sistema
            estado.errorBotellaRetirada = false;
            estado.errorSinFlujo        = false;
            estado.errorTimeout         = false;
            estado.volumenMedidoMl      = 0.0f;

            estado.estadoActual = ESPERANDO_BOTELLA;
            Serial.println("FSM: START -> ESPERANDO_BOTELLA");
          }
          break;
        }

        // --------------------------
        // 2) ESPERANDO_BOTELLA
        // --------------------------
        case ESPERANDO_BOTELLA:
        {
          // Logica de "botella estable"
          TickType_t ahora = xTaskGetTickCount();

          if (botella)
          {
            // Primera vez que vemos botella en este ciclo
            if (!botellaPrev)
            {
              tickBotellaDetectada = ahora;
              botellaPrev = true;
              // Opcional: debug
              // Serial.println("FSM: botella vista, esperando estabilidad...");
            }

            // Tiempo que lleva la botella presente de forma continua
            uint32_t msConBotella = (uint32_t)((ahora - tickBotellaDetectada) * portTICK_PERIOD_MS);

            if (msConBotella >= BOTELLA_ESTABLE_MS)
            {
              estado.estadoActual = BOTELLA_DETECTADA;
              Serial.println("FSM: botella ESTABLE -> BOTELLA_DETECTADA");
            }
          }
          else
          {
            // No hay botella: reset del seguimiento
            botellaPrev = false;
            tickBotellaDetectada = 0;
          }

          break;
        }


        // --------------------------
        // 3) BOTELLA_DETECTADA
        // --------------------------
        case BOTELLA_DETECTADA:
        {
          if (!botella)
          {
            // Se fue la botella antes de llenar -> volvemos a esperar
            estado.estadoActual = ESPERANDO_BOTELLA;
            Serial.println("FSM: botella retirada antes de llenar -> ESPERANDO_BOTELLA");
          }
          else
          {
            if (estado.modoAutomatico)
            {
              // MODO AUTOMATICO:
              // Apenas hay botella estable, iniciamos llenado
              estado.estadoActual    = LLENANDO;
              estado.volumenMedidoMl = 0.0f;

              bombaOn();
              estado.bombaEncendida = true;

              tickInicioLlenado = xTaskGetTickCount();
              volumenAnterior   = 0.0f;
              periodosSinFlujo  = 0;

              Serial.println("FSM: AUTO -> LLENANDO (bomba ON, volumen=0)");
            }
            else if (eventoLlenar)
            {
              // MODO MANUAL:
              // Solo comenzamos a llenar si el operador presiona LLENAR
              estado.estadoActual    = LLENANDO;
              estado.volumenMedidoMl = 0.0f;

              bombaOn();
              estado.bombaEncendida = true;

              tickInicioLlenado = xTaskGetTickCount();
              volumenAnterior   = 0.0f;
              periodosSinFlujo  = 0;

              Serial.println("FSM: MANUAL LLENAR -> LLENANDO (bomba ON, volumen=0)");
            }
          }
          break;
        }

        // --------------------------
        // 4) LLENANDO
        // --------------------------
        case LLENANDO:
        {
          // 4.1) Botella retirada durante llenado -> ERROR
          if (!botella)
          {
            bombaOff();
            estado.bombaEncendida       = false;
            estado.estadoActual         = ERROR_SISTEMA;
            estado.errorBotellaRetirada = true;
            estado.ultimoErrorCodigo    = ERROR_CODIGO_BOTELLA;
            guardarUltimoError(estado.ultimoErrorCodigo);

            Serial.println("FSM: ERROR - botella retirada durante LLENANDO");
            break;
          }

          // 4.2) Timeout de llenado
          TickType_t ahora   = xTaskGetTickCount();
          uint32_t elapsedMs = (uint32_t)((ahora - tickInicioLlenado) * portTICK_PERIOD_MS);

          if (elapsedMs > LLENADO_TIMEOUT_MS)
          {
            bombaOff();
            estado.bombaEncendida = false;
            estado.estadoActual   = ERROR_SISTEMA;
            estado.errorTimeout   = true;
            estado.ultimoErrorCodigo = ERROR_CODIGO_TIMEOUT;
            guardarUltimoError(estado.ultimoErrorCodigo);

            Serial.println("FSM: ERROR - timeout de llenado");
            break;
          }

          // 4.3) Deteccion de "sin flujo"
          // Si el volumen aumenta, reiniciamos contador de sin flujo
          if (volumenMl > (volumenAnterior + 0.1f)) // hysteresis pequeña
          {
            volumenAnterior  = volumenMl;
            periodosSinFlujo = 0;
          }
          else
          {
            // No aumento el volumen en este periodo
            if (periodosSinFlujo < 60000 / FLUJO_PERIOD_MS) // evitar overflow
            {
              periodosSinFlujo++;
            }
          }

          uint32_t tiempoSinFlujoMs = (uint32_t)periodosSinFlujo * FLUJO_PERIOD_MS;

          if (tiempoSinFlujoMs > SIN_FLUJO_UMBRAL_MS)
          {
            bombaOff();
            estado.bombaEncendida = false;
            estado.estadoActual   = ERROR_SISTEMA;
            estado.errorSinFlujo  = true;
            estado.ultimoErrorCodigo = ERROR_CODIGO_SIN_FLUJO;
            guardarUltimoError(estado.ultimoErrorCodigo);

            Serial.println("FSM: ERROR - sin flujo durante LLENANDO");
            break;
          }

          // 4.4) Volumen objetivo alcanzado
          if (volumenMl >= VOLUMEN_OBJETIVO_ML)
          {
            bombaOff();
            estado.bombaEncendida = false;
            estado.estadoActual   = LLENADO_COMPLETADO;

            // Contar botella y guardar en NVS (si tienes persistencia)
            estado.totalBotellasLlenadas++;
            guardarTotalBotellas(estado.totalBotellasLlenadas);

            Serial.println("FSM: llenado completado, bomba OFF");
          }

          break;
        }

        // --------------------------
        // 5) LLENADO_COMPLETADO
        // --------------------------
        case LLENADO_COMPLETADO:
        {
          // Esperamos a que retiren la botella para aceptar otra
          if (!botella)
          {
            estado.estadoActual    = ESPERANDO_BOTELLA;
            estado.volumenMedidoMl = 0.0f;
            Serial.println("FSM: botella retirada tras llenado -> ESPERANDO_BOTELLA");
          }
          break;
        }

        // --------------------------
        // 6) ERROR_SISTEMA
        // --------------------------
        case ERROR_SISTEMA:
        {
          // Aseguramos bomba OFF
          if (estado.bombaEncendida)
          {
            bombaOff();
            estado.bombaEncendida = false;
          }

          // START desde ERROR reintenta armar el sistema
          if (eventoStart)
          {
            estado.errorBotellaRetirada = false;
            estado.errorSinFlujo        = false;
            estado.errorTimeout         = false;
            estado.volumenMedidoMl      = 0.0f;

            estado.estadoActual = ESPERANDO_BOTELLA;
            Serial.println("FSM: START en ERROR -> ESPERANDO_BOTELLA (errores limpiados)");
          }
          break;
        }

        default:
          estado.estadoActual = SISTEMA_DETENIDO;
          break;
      } // switch
    }   // else (no STOP)

    // Guardar cambios en el estado compartido
    escribirEstado(estado);

    // Esperar siguiente ciclo de control
    vTaskDelayUntil(&ultimoDespertar, periodo);
  }
}



void TaskDebug(void *pvParameters)
{
  (void) pvParameters;

  Serial.println("TaskDebug: iniciada");

  const TickType_t periodo = pdMS_TO_TICKS(5000); // 1 segundo

  while (true)
  {
    // Leer una copia del estado
    EstadoCompartido estado = leerEstado();

    Serial.print("TaskDebug: estado = ");
    Serial.print(static_cast<int>(estado.estadoActual));

    Serial.print(" | modo = ");
    Serial.print(estado.modoAutomatico ? "AUTO" : "MANUAL");

    Serial.print(" | botella = ");
    Serial.print(estado.botellaPresente ? "SI" : "NO");

    Serial.print(" | volumenMl = ");
    Serial.print(estado.volumenMedidoMl, 1);

    Serial.print(" | bomba = ");
    Serial.print(estado.bombaEncendida ? "ON" : "OFF");

    Serial.print(" | errRetirada = ");
    Serial.print(estado.errorBotellaRetirada ? "1" : "0");

    Serial.print(" | errSinFlujo = ");
    Serial.print(estado.errorSinFlujo ? "1" : "0");

    Serial.print(" | errTimeout = ");
    Serial.print(estado.errorTimeout ? "1" : "0");

    Serial.print(" | botellasTotales = ");
    Serial.println(estado.totalBotellasLlenadas);

    vTaskDelay(periodo);
  }
}

void setup()
{
  Serial.begin(SERIAL_BAUDRATE);
  delay(1000);
  Serial.println();
  Serial.println("Sistema arrancando (Bloque B)");

  // Inicializar NVS (persistencia)
  persistenciaInit();

  // Inicializar estado compartido y mutex
  estadoSistemaInit();

  // Inicializar hardware
  initActuadores();
  initBotones();
  initSensorUltrasonico();
  initFlujo();
  initPantalla();

  // Creacion de tareas
  xTaskCreatePinnedToCore(
    TaskControl,
    "TaskControl",
    TASK_CONTROL_STACK_SIZE,
    nullptr,
    TASK_CONTROL_PRIORITY,
    nullptr,
    TASK_CORE_ID
  );

  xTaskCreatePinnedToCore(
    TaskDebug,
    "TaskDebug",
    TASK_DEBUG_STACK_SIZE,
    nullptr,
    TASK_DEBUG_PRIORITY,
    nullptr,
    TASK_CORE_ID
  );

  xTaskCreatePinnedToCore(
    TaskBotones,
    "TaskBotones",
    TASK_BUTTONS_STACK_SIZE,
    nullptr,
    TASK_BUTTONS_PRIORITY,
    nullptr,
    TASK_CORE_ID
  );

  xTaskCreatePinnedToCore(
    TaskUltrasonico,
    "TaskUltrasonico",
    TASK_ULTRA_STACK_SIZE,
    nullptr,
    TASK_ULTRA_PRIORITY,
    nullptr,
    TASK_CORE_ID
  );

  xTaskCreatePinnedToCore(
    TaskFlujo,
    "TaskFlujo",
    TASK_FLUJO_STACK_SIZE,
    nullptr,
    TASK_FLUJO_PRIORITY,
    nullptr,
    TASK_CORE_ID
  );

    xTaskCreatePinnedToCore(
    TaskPantalla,
    "TaskPantalla",
    4096,
    nullptr,
    1,      // prioridad baja, similar a Debug
    nullptr,
    1
  );


  Serial.println("Tareas creadas. Entrando a loop() pasivo");
}

void loop()
{
  // Dejar que FreeRTOS maneje todo
  vTaskDelay(portMAX_DELAY);
}
