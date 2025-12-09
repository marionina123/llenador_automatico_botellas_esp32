#ifndef CONFIG_SISTEMA_H
#define CONFIG_SISTEMA_H

// =======================
//  Serial
// =======================
#define SERIAL_BAUDRATE           115200

// =======================
//  FreeRTOS - Task config
// =======================
#define TASK_CONTROL_STACK_SIZE   4096
#define TASK_DEBUG_STACK_SIZE     4096
#define TASK_BUTTONS_STACK_SIZE   4096
#define TASK_ULTRA_STACK_SIZE     4096
// Tarea de flujo
#define TASK_FLUJO_STACK_SIZE     4096
#define TASK_FLUJO_PRIORITY       2


#define TASK_CONTROL_PRIORITY     2
#define TASK_DEBUG_PRIORITY       1
#define TASK_BUTTONS_PRIORITY     2
#define TASK_ULTRA_PRIORITY       1

// Core donde correrán las tareas
#define TASK_CORE_ID              1

// =======================
//  Pines de botones
// =======================
// Lógica: INPUT_PULLUP -> reposo = HIGH, pulsado = LOW
#define PIN_BOTON_MARCHA          25   // START
#define PIN_BOTON_PARADA          26   // STOP
#define PIN_BOTON_LLENAR          27   // LLENAR

// Debounce de botones
#define BOTONES_PERIOD_MS         10    // Periodo de muestreo (ms)
#define BOTONES_SAMPLES_STABLE    5     // 5 muestras * 10 ms = 50 ms

// =======================
//  Bomba / MOSFET
// =======================
// N-channel MOSFET en configuración low-side: HIGH = bomba ON, LOW = OFF
#define PIN_BOMBA_MOSFET          23

// Sensor de flujo YF-S402
#define PIN_SENSOR_FLUJO          34   // entrada de pulsos (usar con divisor 5V->3.3V)

// =======================*/

//  Sensor ultrasónico HC-SR04
// =======================
#define PIN_ULTRA_TRIG            18
#define PIN_ULTRA_ECHO            19

// Distancia para considerar "botella presente" (cm)
#define BOTELLA_DIST_UMBRAL_CM    10.0f

// Filtro de estabilidad para la botella
#define ULTRA_PERIOD_MS           100   // cada 100 ms mide
#define ULTRA_SAMPLES_STABLE      3     // 3 lecturas estables

// =======================
//  Pines reservados para futuros bloques
// =======================
// Sensor de flujo YF-S402 (entrada de pulsos, usará interrupción)
#define PIN_SENSOR_FLUJO          34

// I2C para LCD (Bloque E)
#define PIN_I2C_SDA_LCD           21
#define PIN_I2C_SCL_LCD           22

// =======================
//  Parametros del sensor de flujo
// =======================

// Valor aproximado; se ajustara en la calibracion.
// Reemplaza este numero cuando hagas la calibracion real.
#define PULSOS_POR_LITRO          16818.0f   // EJEMPLO, AJUSTAR LUEGO
// PPL_nuevo = PPL_viejo * (V_medido_ml / V_real_ml)

// Derivado:
#define ML_POR_PULSO              (1000.0f / PULSOS_POR_LITRO)

// Periodo de la tarea de flujo (cada cuanto convierte pulsos a ml)
#define FLUJO_PERIOD_MS           500      // cada 500 ms

// =======================
//  Parametros de la FSM de llenado
// =======================

// Volumen objetivo por botella (ml)
#define VOLUMEN_OBJETIVO_ML       500.0f

// Timeout maximo de llenado (ms) - ej. 30 segundos
#define LLENADO_TIMEOUT_MS        30000

// Tiempo maximo sin flujo (ms) durante LLENANDO antes de considerar error
#define SIN_FLUJO_UMBRAL_MS       5000

// Tiempo que la botella debe estar presente de forma continua
// en ESPERANDO_BOTELLA para considerarla "estable" (ms)
#define BOTELLA_ESTABLE_MS        800

#endif
