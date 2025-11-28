# Llenador Automático de Botellas – ESP32 🍾
### Proyecto Final de Sistemas Embebidos I (IMT-222)

Este repositorio contiene el desarrollo de un **sistema embebido** capaz de llenar automáticamente botellas con un volumen fijo de **500 ml**, empleando un microcontrolador **ESP32** y una arquitectura de control basada en **FreeRTOS**. El sistema replica de forma simplificada el comportamiento de una máquina industrial de llenado, priorizando la seguridad, la precisión y el control determinista.

---

## Características y Funcionamiento Principal

El sistema garantiza un **llenado preciso de 500 ml** y opera bajo un control determinista basado en una Máquina de Estados Finita (FSM).

### Características Clave
* **Volumen fijo de 500 ml** medido mediante un sensor de flujo.
* **Detección de botella** con sensor ultrasónico y control seguro de la minibomba.
* **Tres botones físicos:** **START** (habilita), **STOP** (detención inmediata), **LLENAR** (inicio manual).
* **Control Concurrente:** Arquitectura basada en **FreeRTOS** y **Temporización no bloqueante**.
* **Precisión:** Uso de **Interrupciones (ISR)** para conteo de pulsos de flujo.

### Ciclo de Operación (Descripción General)
1.  **Inicio:** El sistema entra en **Estado Detenido**. El operador presiona **START** para habilitarlo.
2.  **Espera:** El sistema monitorea la presencia de una botella con el sensor ultrasónico.
3.  **Llenado:** Si se detecta la botella, el llenado puede iniciar **automáticamente** o por comando **LLENAR**.
4.  **Medición y Fin:** Durante el llenado, la ISR cuenta pulsos. Al alcanzar **500 ml**, la bomba se apaga y muestra “Llenado completado”.
5.  **Seguridad/Error:** El botón **STOP** detiene el sistema en cualquier momento. Un problema (botella retirada, falta de flujo, o tiempo excedido) lo mueve al **Estado de Error**.

---

## Tecnologías y Arquitectura del Software

El *firmware* utiliza un enfoque modular y concurrente para manejar las entradas/salidas y la lógica de control.

### Tecnologías Clave
| Componente | Tecnología/Herramienta | Propósito |
| :--- | :--- | :--- |
| **Microcontrolador** | **ESP32** (con soporte dual-core) | Ejecución del *firmware* y FreeRTOS. |
| **Sistema Operativo** | **FreeRTOS** | Arquitectura concurrente, manejo de tareas y protección de datos mediante **Mutex**. |
| **Metodología** | **Máquina de Estados Finita (FSM)** | Control determinista y seguro de la lógica del sistema. |

### Estados del Sistema (FSM)
El comportamiento se organiza mediante los siguientes estados, garantizando un control seguro:
* **SISTEMA DETENIDO**
* **ESPERANDO BOTELLA**
* **BOTELLA DETECTADA**
* **LLENANDO**
* **LLENADO COMPLETADO**
* **ERROR**

---

## Estado Actual del Proyecto (Avances Funcionales)

El desarrollo se encuentra **funcional hasta la Fase 6**, con la integración del *hardware* esencial y la columna vertebral del sistema establecida.

| Fase Completada | Avance Concreto | Concepto Implementado |
| :--- | :--- | :--- |
| **Fase 1-3** | **Arquitectura y Concurrencia** | Estructura modular, **FreeRTOS** y **Mutex** funcional para el manejo seguro del Estado Compartido. |
| **Fase 4** | **Entradas Físicas (Botones)** | Lógica de **debounce** implementada para la lectura estable de los botones (START, STOP, LLENAR). |
| **Fase 5** | **Actuador (Bomba)** | Control de la **Minibomba** mediante la etapa de potencia, activada y desactivada por comandos de botones. |
| **Fase 6** | **Detección de Botella** | Integración del **Sensor Ultrasónico** con filtrado y estabilidad para detectar la presencia de una botella. |

### Próximos Pasos (Pendiente de Implementación)
* **Fase 7:** Integración del sensor de flujo e Interrupciones (ISR).
* **Fase 8 & 9:** Implementación e integración completa de la Máquina de Estados Finita (FSM).
* **Fase 10:** Integración de la Pantalla (UI).

---

## Hardware utilizado

* **ESP32**
* **Minibomba de agua**
* **Sensor de flujo**
* **Sensor ultrasónico**
* **Botones:** START, STOP, LLENAR
* **Pantalla** (LCD)
* **Etapa de potencia para la bomba**
* **Fuente de alimentación**
* **Mangueras y estructura básica**

---

## Estructura del Repositorio
## Estructura inicial del repositorio



```

llenador-automatico-botellas-esp32/

│

├─ src/                     → código fuente del sistema (.ino, )

|

├─ inc/                     → encabezdos (.h)

│

├─ docs/                    → reportes y archivos del proyecto

│

├─ hardware/                → esquemas, diagramas y fotos del montaje

│

├─ img/                     → imágenes del prototipo

│

├─ README.md                → descripción general del proyecto

├─ LICENSE                  → licencia del repositorio

└─ .gitignore               → archivos ignorados por Git

```

---

## Integrantes y Licencia

Este proyecto fue desarrollado en solitario por **Mario Alberto Nina Gallo** como **Ingeniero de Firmware y Arquitecto del Sistema**.

La distribución se rige bajo los términos especificados en el archivo **LICENSE** incluido en este repositorio.
