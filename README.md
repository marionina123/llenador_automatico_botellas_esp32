# Llenador Automático de Botellas – ESP32
### Proyecto Final de Sistemas Embebidos

Este repositorio contiene el desarrollo de un **sistema embebido autónomo** capaz de llenar botellas con un volumen fijo de **500 ml**, utilizando un **ESP32**, sensores industriales, una **máquina de estados (FSM)** y una arquitectura concurrente basada en **FreeRTOS**.  
El diseño replica el comportamiento fundamental de una máquina industrial de llenado, priorizando **seguridad**, **precisión** y **control determinista**.

---

## Descripción general del funcionamiento

1. El sistema inicia en **SISTEMA_DETENIDO**.  
2. El operador presiona **START** para habilitarlo.  
3. El sensor ultrasónico verifica la presencia estable de una botella.  
4. Una vez detectada la botella:
   - El llenado puede iniciar automáticamente (modo AUTO), o  
   - El operador puede iniciarlo mediante el botón **LLENAR** (modo manual).  
5. Durante el llenado:
   - El sensor de flujo mide el volumen mediante pulsos capturados por interrupción.
   - Se supervisa que la botella continúe presente.
   - Se verifica la existencia de flujo y el tiempo máximo permitido.  
6. Al alcanzar los **500 ml**, la bomba se apaga y el sistema pasa a **LLENADO_COMPLETADO**.  
7. Si ocurre una condición anómala (botella retirada, sin flujo, timeout), se ingresa al estado **ERROR_SISTEMA**.  
8. El botón **STOP** detiene la operación en cualquier momento, con prioridad absoluta sobre cualquier otro evento.

---

## Características principales

- Medición de volumen por pulsos del **sensor de flujo YF-S402**.  
- **Detección robusta de botella** mediante sensor ultrasónico con filtrado.  
- Control seguro de la **minibomba de agua** mediante MOSFET.  
- Tres botones de operación:
  - START (habilitación)
  - STOP (detención inmediata)
  - LLENAR (inicio manual del ciclo)
- **Pantalla LCD I2C** para visualización del estado, volumen y errores.  
- Arquitectura modular basada en **FreeRTOS** con múltiples tareas concurrentes.  
- **Estado compartido** protegido con mutex.  
- **Persistencia en NVS** para mantener el total de botellas llenadas y el último error registrado.  
- FSM clara y documentada que garantiza comportamiento determinista.  
- Diseño completamente no bloqueante, orientado a sistemas embebidos de tiempo real.

---

## Arquitectura del software

### Tareas FreeRTOS implementadas

| Tarea           | Función principal                                                                 |
|-----------------|-----------------------------------------------------------------------------------|
| `TaskControl`   | Ejecuta la FSM. Controla bomba, errores y transiciones entre estados.             |
| `TaskDebug`     | Envía información por puerto serie para diagnóstico.                              |
| `TaskBotones`   | Lectura con antirrebote y generación de eventos de botón.                        |
| `TaskUltrasonico` | Detección de botella mediante filtrado temporal.                                |
| `TaskFlujo`     | Conversión de pulsos del sensor a volumen real; lectura vía ISR.                 |
| `TaskPantalla`  | Actualiza el LCD según el estado del sistema y eventos relevantes.                |

### Módulos incluidos

| Módulo                 | Descripción                                                                 |
|------------------------|-----------------------------------------------------------------------------|
| `config_sistema.h`     | Parámetros globales: pines, tiempos, prioridades y umbrales.                |
| `control.*`            | Implementación de la FSM y tareas de control y depuración.                 |
| `estados_sistema.*`    | Estado global compartido, mutex y funciones de acceso seguro.              |
| `persistencia.*`       | Manejo de memoria no volátil (NVS): errores y botellas llenadas.           |
| `sensor_flujo.*`       | ISR y lógica de medición del volumen.                                      |
| `sensor_ultrasonico.*` | Lectura y filtrado de distancia y presencia de botella.                    |
| `botones.*`            | Antirrebote y generación de eventos START, STOP, LLENAR.                   |
| `actuadores.*`         | Control de la bomba mediante MOSFET.                                       |
| `pantalla.*`           | Presentación de información en LCD con bufferización.                      |
| `main.ino`             | Inicialización general y creación de tareas.                               |

---

# Bloques del Proyecto

A continuación se detalla la estructura conceptual del proyecto en bloques de desarrollo.  
Cada bloque presenta su propósito, actividades y criterios de finalización, reflejando exactamente el proceso seguido en este firmware.

---

## Bloque A — Infraestructura del Firmware

**Propósito:**  
Establecer la arquitectura base del software y los mecanismos de concurrencia necesarios para operar múltiples tareas de forma segura en el ESP32.

**Actividades principales:**  
- Configuración inicial de la plataforma ESP32.  
- Definición de la arquitectura modular (sensores, actuadores, FSM, pantalla, persistencia).  
- Diseño del **Estado Compartido**, que centraliza toda la información relevante del sistema.  
- Implementación de un **mutex** para acceso seguro entre tareas concurrentes.  
- Creación de tareas iniciales de FreeRTOS (control, debug, IO).  

**Criterios de finalización:**  
- La ESP32 ejecuta múltiples tareas concurrentes de forma estable.  
- Las tareas acceden al estado mediante operaciones sincronizadas.  
- El flujo general de datos y control del sistema está definido antes de integrar hardware complejo.

---

## Bloque B — Hardware Básico y Control Manual

**Propósito:**  
Integrar las interfaces básicas de entrada y salida, logrando control manual estable y detección confiable de botella.

**Actividades principales:**  
- Implementación del antirrebote por software en botones.  
- Generación de eventos (flags) para START, STOP y LLENAR.  
- Control manual de la bomba mediante MOSFET.  
- Integración del sensor ultrasónico para detección de botella con filtrado temporal.

**Criterios de finalización:**  
- Cada pulsación genera un único evento.  
- START enciende la bomba y STOP la apaga sin importar el estado actual.  
- La detección de botella es estable y libre de fluctuaciones erráticas.

---

## Bloque C — Medición de Flujo y Persistencia

**Propósito:**  
Asegurar una medición precisa del volumen mediante ISR y conservar datos relevantes entre reinicios.

**Actividades principales:**

1. **Sensor de flujo (interrupciones):**  
   - Implementación de ISR mínima para conteo de pulsos.  
   - Conversión periódica de pulsos a volumen (ml).  
   - Calibración experimental de `PULSOS_POR_LITRO`.

2. **Persistencia (NVS):**  
   - Almacenamiento del total de botellas llenadas.  
   - Registro del código del último error del sistema.  
   - Guardado al finalizar un ciclo de llenado o al producirse un error crítico.

**Criterios de finalización:**  
- El volumen medido es consistente con el volumen real tras calibración.  
- Los datos persistentes permanecen tras reiniciar el dispositivo.  
- El acceso a NVS no impacta la operación en tiempo real.

---

## Bloque D — Máquina de Estados (FSM) y Sistema de Seguridad

**Propósito:**  
Concentrar toda la lógica del sistema en una FSM clara, robusta y fácil de mantener.

**Actividades principales:**  
- Diseño formal de los estados principales:  
  SISTEMA_DETENIDO, ESPERANDO_BOTELLA, BOTELLA_DETECTADA, LLENANDO, LLENADO_COMPLETADO, ERROR_SISTEMA.  
- Definición de los eventos detonadores:  
  START, STOP, presencia de botella, retirada de botella, volumen alcanzado, timeout, sin flujo.  
- Validación inicial mediante simulación (sin mover actuadores reales).  
- Implementación final con hardware real: control de bomba, detección de errores y seguridad.  
- STOP como comando prioritario en cualquier estado.

**Criterios de finalización:**  
- El sistema realiza un ciclo de llenado completo y se detiene al alcanzar 500 ml.  
- Todas las condiciones de error llevan al estado correspondiente.  
- STOP interrumpe el proceso inmediatamente.  
- La máquina de estados opera de forma predecible y alineada al diseño.

---

## Bloque E — Interfaz de Usuario, Validación y Pulido Final

**Propósito:**  
Proveer una interfaz clara al usuario, realizar pruebas formales y optimizar el comportamiento final del sistema.

**Actividades principales:**  
- Implementación de la interfaz en pantalla LCD, mostrando:  
  - Estado actual.  
  - Volumen actualizado en tiempo real.  
  - Mensajes de error.  
  - Contador de botellas (opcional).  
- Pruebas formales:  
  - Llenado normal.  
  - Botella retirada.  
  - Sin flujo.  
  - Timeout.  
  - STOP desde todos los estados.  
  - Verificación de persistencia.  
- Ajustes finales:  
  - Frecuencias de tareas.  
  - Prioridades.  
  - Reducción de logs.  
  - Documentación de FSM y del flujo entre tareas.

**Criterio de finalización:**  
- El sistema puede demostrarse sin conexión a PC.  
- La interfaz muestra claramente cada transición y condición relevante.  
- Comportamiento estable y repetible.  
- Documentación completa del flujo lógico y estructura del firmware.

---

## Máquina de estados del sistema

El sistema opera mediante una **máquina de estados finita (FSM)**:

- `SISTEMA_DETENIDO`  
- `ESPERANDO_BOTELLA`  
- `BOTELLA_DETECTADA`  
- `LLENANDO`  
- `LLENADO_COMPLETADO`  
- `ERROR_SISTEMA`  

Las transiciones se determinan mediante eventos generados por sensores, botones, temporizadores y condiciones de error.

---

## Persistencia

La memoria no volátil (NVS) almacena:

- Total de botellas llenadas.  
- Código del último error registrado.  

Esto permite recuperar información histórica relevante después de un reinicio.

---

## Mapeo de pines

| Función                         | GPIO | Descripción                                                         |
|---------------------------------|------|---------------------------------------------------------------------|
| Botón START                     | 25   | Entrada digital con pull-up interno.                                |
| Botón STOP                      | 26   | Igual que START.                                                     |
| Botón LLENAR                    | 27   | Igual que START/STOP.                                                |
| Bomba – control MOSFET          | 23   | Salida digital para activar la etapa de potencia.                   |
| Ultrasónico TRIG                | 18   | Pulso de disparo del sensor HC-SR04.                                |
| Ultrasónico ECHO                | 19   | Medición de retorno del HC-SR04.                                    |
| Sensor de flujo (YF-S402)       | 34   | Entrada analógica/digital; captura pulsos por interrupción.         |
| LCD I2C SDA                     | 21   | Línea de comunicación I2C (SDA).                                    |
| LCD I2C SCL                     | 22   | Línea de comunicación I2C (SCL).                                    |

---

## Hardware utilizado

- ESP32 (dual-core, soporte FreeRTOS).  
- Sensor de flujo YF-S402.  
- Sensor ultrasónico HC-SR04.  
- Minibomba de agua y etapa de potencia (MOSFET).  
- Pantalla LCD I2C 16×2.  
- Botones START, STOP, LLENAR.  
- Fuente de alimentación y estructura física del prototipo.

---

## Estructura del repositorio

