#include "pantalla.h"
#include "config_sistema.h"
#include "estados_sistema.h"
#include "persistencia.h"

#include <Wire.h>
#include <LiquidCrystal_PCF8574.h>

// Ajusta esta direccion si tu modulo I2C es distinto (0x27 o 0x3F suelen ser los mas comunes)
#define LCD_I2C_ADDR  0x27
#define LCD_COLS      16
#define LCD_ROWS      2

// Objeto global del LCD (PCF8574)
static LiquidCrystal_PCF8574 lcd(LCD_I2C_ADDR);

// Convierte el enum EstadoSistema en un texto corto (max 16 chars)
static const char* textoEstado(EstadoSistema est)
{
  switch (est)
  {
    case SISTEMA_DETENIDO:      return "DETENIDO      ";
    case ESPERANDO_BOTELLA:     return "ESP BOTELLA   ";
    case BOTELLA_DETECTADA:     return "BOTELLA LISTA ";
    case LLENANDO:              return "LLENANDO      ";
    case LLENADO_COMPLETADO:    return "LLENADO OK    ";
    case ERROR_SISTEMA:         return "ERROR         ";
    default:                    return "DESCONOCIDO   ";
  }
}

// Devuelve texto segun error activo
static const char* textoError(const EstadoCompartido &estado)
{
  if (estado.errorBotellaRetirada)
    return "Err: botella   ";
  if (estado.errorSinFlujo)
    return "Err: sin flujo ";
  if (estado.errorTimeout)
    return "Err: timeout   ";

  return "Error sistema  ";
}

// Texto para el ultimo error guardado (persistente)
static const char* textoUltimoError(uint8_t codigo)
{
  switch (codigo)
  {
    case ERROR_CODIGO_BOTELLA:
      return "UltErr: botella";
    case ERROR_CODIGO_SIN_FLUJO:
      return "UltErr: sin flujo";
    case ERROR_CODIGO_TIMEOUT:
      return "UltErr: timeout ";
    case ERROR_CODIGO_NINGUNO:
    default:
      return "Sin ultimo err ";
  }
}


// Inicializacion de la pantalla
void initPantalla()
{
  // ESP32: especificar pines I2C (SDA=21, SCL=22)
  Wire.begin(21, 22);

  // Inicializa el LCD 16x2
  lcd.begin(LCD_COLS, LCD_ROWS);
  // En esta libreria el backlight se maneja con un valor 0..255
  lcd.setBacklight(255);   // maximo brillo

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Llenador 500ml");
  lcd.setCursor(0, 1);
  lcd.print("Iniciando...   ");
}

// Tarea que solo observa el estado compartido y actualiza el LCD
void TaskPantalla(void *pvParameters)
{
  (void) pvParameters;

  Serial.println("TaskPantalla: iniciada");

  const TickType_t periodo = pdMS_TO_TICKS(200); // actualiza cada 200ms

  // Cache para detectar cambios
  EstadoSistema ultimoEstado          = SISTEMA_DETENIDO;
  bool          ultimoModoAutomatico  = true;
  bool          ultimaBotellaPresente = false;
  float         ultimoVolumenMl       = -1.0f;
  bool          ultimoErrRetirada     = false;
  bool          ultimoErrSinFlujo     = false;
  bool          ultimoErrTimeout      = false;
  uint32_t      ultimoTotalBotellas   = 0;
  uint8_t       ultimoErrorCodigo     = ERROR_CODIGO_NINGUNO;


  while (true)
  {
    EstadoCompartido estado = leerEstado();

    bool hayCambio = false;

    if (estado.estadoActual         != ultimoEstado)          hayCambio = true;
    if (estado.modoAutomatico       != ultimoModoAutomatico)  hayCambio = true;
    if (estado.botellaPresente      != ultimaBotellaPresente) hayCambio = true;
    if (fabsf(estado.volumenMedidoMl - ultimoVolumenMl) > 1)  hayCambio = true; // >1 ml
    if (estado.errorBotellaRetirada != ultimoErrRetirada)     hayCambio = true;
    if (estado.errorSinFlujo        != ultimoErrSinFlujo)     hayCambio = true;
    if (estado.errorTimeout         != ultimoErrTimeout)      hayCambio = true;
    if (estado.totalBotellasLlenadas != ultimoTotalBotellas)  hayCambio = true;
    if (estado.ultimoErrorCodigo    != ultimoErrorCodigo)     hayCambio = true;

    if (hayCambio)
    {
      // Actualizar cache
      ultimoEstado          = estado.estadoActual;
      ultimoModoAutomatico  = estado.modoAutomatico;
      ultimaBotellaPresente = estado.botellaPresente;
      ultimoVolumenMl       = estado.volumenMedidoMl;
      ultimoErrRetirada     = estado.errorBotellaRetirada;
      ultimoErrSinFlujo     = estado.errorSinFlujo;
      ultimoErrTimeout      = estado.errorTimeout;
      ultimoTotalBotellas   = estado.totalBotellasLlenadas;
      ultimoErrorCodigo     = estado.ultimoErrorCodigo;


      // ----- Linea 1: modo + estado -----
      lcd.setCursor(0, 0);

      // Modo: "A:" o "M:"
      if (estado.modoAutomatico)
        lcd.print("A:");
      else
        lcd.print("M:");

      const char *txtEst = textoEstado(estado.estadoActual);
      lcd.print(txtEst);

      // Rellenar hasta el final de la linea
      int len1 = 2 + strlen(txtEst);
      for (int i = len1; i < LCD_COLS; i++)
        lcd.print(' ');

      // ----- Linea 2: depende del estado -----
      lcd.setCursor(0, 1);

      if (estado.estadoActual == ERROR_SISTEMA)
      {
        const char *txtErr = textoError(estado);
        lcd.print(txtErr);
        int l2 = strlen(txtErr);
        for (int i = l2; i < LCD_COLS; i++)
          lcd.print(' ');
      }
      else if (estado.estadoActual == LLENANDO)
      {
        char buffer[17];
        snprintf(buffer, sizeof(buffer), "Vol:%4.0fml     ", estado.volumenMedidoMl);
        lcd.print(buffer);
        int l2 = strlen(buffer);
        for (int i = l2; i < LCD_COLS; i++)
          lcd.print(' ');
      }
            else
      {
        // En SISTEMA_DETENIDO, si hay un ultimo error guardado,
        // lo mostramos en la segunda linea
        if (estado.estadoActual == SISTEMA_DETENIDO &&
            estado.ultimoErrorCodigo != ERROR_CODIGO_NINGUNO)
        {
          const char *txtUlt = textoUltimoError(estado.ultimoErrorCodigo);
          lcd.print(txtUlt);
          int l2 = strlen(txtUlt);
          for (int i = l2; i < LCD_COLS; i++)
            lcd.print(' ');
        }
        else
        {
          // Estados "normales": botella presente + total de botellas
          char buffer[17];
          snprintf(buffer, sizeof(buffer),
                   "Bot:%s Tot:%3lu",
                   estado.botellaPresente ? "SI" : "NO",
                   (unsigned long)estado.totalBotellasLlenadas);
          lcd.print(buffer);
          int l2 = strlen(buffer);
          for (int i = l2; i < LCD_COLS; i++)
            lcd.print(' ');
        }
      }
    }
    vTaskDelay(periodo);
  }
}
