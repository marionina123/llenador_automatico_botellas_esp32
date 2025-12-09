#include "botones.h"
#include "config_sistema.h"
#include "estados_sistema.h"

// Estructura interna para manejar antirrebote de un boton
struct EstadoBotonInterno {
  bool estadoFisicoActual;     // Lectura cruda del pin (HIGH/LOW)
  bool estadoEstable;          // Estado "oficial" luego de debounce
  uint8_t contadorEstabilidad; // Cuantas muestras seguidas lleva igual
};

static EstadoBotonInterno botonMarcha;
static EstadoBotonInterno botonParada;
static EstadoBotonInterno botonLlenar;

void initBotones()
{
  pinMode(PIN_BOTON_MARCHA, INPUT_PULLUP);
  pinMode(PIN_BOTON_PARADA, INPUT_PULLUP);
  pinMode(PIN_BOTON_LLENAR, INPUT_PULLUP);

  // Inicializar estados internos con lectura inicial
  botonMarcha.estadoFisicoActual = digitalRead(PIN_BOTON_MARCHA);
  botonMarcha.estadoEstable      = botonMarcha.estadoFisicoActual;
  botonMarcha.contadorEstabilidad = 0;

  botonParada.estadoFisicoActual = digitalRead(PIN_BOTON_PARADA);
  botonParada.estadoEstable      = botonParada.estadoFisicoActual;
  botonParada.contadorEstabilidad = 0;

  botonLlenar.estadoFisicoActual = digitalRead(PIN_BOTON_LLENAR);
  botonLlenar.estadoEstable      = botonLlenar.estadoFisicoActual;
  botonLlenar.contadorEstabilidad = 0;
}

// Devuelve true si hubo un flanco estable HIGH->LOW (pulsacion)
static bool procesarBoton(EstadoBotonInterno &b, int pin)
{
  bool lectura = digitalRead(pin);  // HIGH o LOW

  if (lectura == b.estadoFisicoActual) {
    // Misma lectura que la anterior: aumentamos contador
    if (b.contadorEstabilidad < 255) {
      b.contadorEstabilidad++;
    }
  } else {
    // Cambio de lectura: reiniciamos contador
    b.estadoFisicoActual = lectura;
    b.contadorEstabilidad = 0;
  }

  // ¿Se mantuvo suficiente tiempo para considerarlo estable?
  if (b.contadorEstabilidad >= BOTONES_SAMPLES_STABLE) {
    b.contadorEstabilidad = BOTONES_SAMPLES_STABLE; // saturamos

    if (b.estadoEstable != b.estadoFisicoActual) {
      bool estadoAnterior = b.estadoEstable;
      b.estadoEstable = b.estadoFisicoActual;

      // Flanco: HIGH -> LOW (boton presionado)
      if (estadoAnterior == HIGH && b.estadoEstable == LOW) {
        return true;
      }
    }
  }

  return false;
}

void TaskBotones(void *pvParameters)
{
  (void) pvParameters;

  Serial.println("TaskBotones: iniciada");

  const TickType_t periodo = pdMS_TO_TICKS(BOTONES_PERIOD_MS);

  while (true) {
    bool eventoMarcha = procesarBoton(botonMarcha, PIN_BOTON_MARCHA);
    bool eventoParada = procesarBoton(botonParada, PIN_BOTON_PARADA);
    bool eventoLlenar = procesarBoton(botonLlenar, PIN_BOTON_LLENAR);

    if (eventoMarcha || eventoParada || eventoLlenar) {
      EstadoCompartido estado = leerEstado();

      if (eventoMarcha) {
        estado.botonMarchaPresionado = true;
        Serial.println("TaskBotones: evento MARCHA");
      }
      if (eventoParada) {
        estado.botonParadaPresionado = true;
        Serial.println("TaskBotones: evento PARADA");
      }
      if (eventoLlenar) {
        estado.botonLlenarPresionado = true;
        Serial.println("TaskBotones: evento LLENAR");
      }

      escribirEstado(estado);
    }

    vTaskDelay(periodo);
  }
}
