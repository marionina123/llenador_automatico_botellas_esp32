# Llenador Automático de Botellas – ESP32
### Proyecto Final de Sistemas Embebidos

Este repositorio contiene el desarrollo de un **sistema embebido** capaz de llenar automáticamente botellas con un volumen fijo de **500 ml**, empleando un microcontrolador **ESP32**, sensores de propósito industrial y una arquitectura basada en **FreeRTOS**. El sistema replica de forma simplificada el comportamiento de una máquina industrial de llenado, priorizando la seguridad, la precisión y el control determinista.

---

## Descripción general del funcionamiento

1. Al encenderse, el sistema entra en **Estado Detenido**.  
2. El operador presiona **START** y el sistema queda habilitado.
3. El sistema monitorea la presencia de una botella mediante el sensor ultrasónico.
4. Si detecta una botella estable:
   - El llenado puede iniciar **automáticamente**, o
   - El usuario puede iniciarlo presionando **LLENAR**.
5. Durante el llenado:
   - Se cuentan los pulsos del sensor de flujo para medir el volumen.
   - Se verifica que la botella siga presente.
   - Se supervisa que exista flujo adecuado.
6. Cuando se alcanzan **500 ml**:
   - La bomba se apaga.
   - El sistema muestra “Llenado completado”.
7. Si ocurre un problema:
   - Botella retirada  
   - Falta de flujo  
   - Tiempo máximo excedido  
   El sistema pasa a **Estado de Error**.
8. El botón **STOP** detiene el sistema en cualquier momento.

---

## Características principales

- **Volumen fijo de 500 ml** medido mediante un sensor de flujo.
- **Detección de botella** utilizando un sensor ultrasónico.
- **Control seguro de una minibomba** a través de una etapa de potencia.
- **Tres botones físicos de operación**:
  - **START:** habilita el sistema.
  - **STOP:** detiene todo inmediatamente.
  - **LLENAR:** inicia el llenado manual (solo si hay botella).
- **Pantalla de usuario** para mostrar estado y mensajes.
- **Arquitectura basada en FreeRTOS**, con múltiples tareas concurrentes.
- **Interrupciones (ISR)** para conteo preciso de pulsos del sensor de flujo.
- **Temporización no bloqueante**, basada en ticks del sistema.
- **Máquina de estados finita (FSM)** para gestionar todo el flujo del sistema.
- **Comunicación serie** para diagnóstico (solo desarrollador).

---

## Tecnologías Utilizadas

El proyecto utiliza las siguientes tecnologías y herramientas clave, enfocadas en la **programación concurrente** y el **control determinista**:

| Componente | Tecnología/Herramienta | Propósito |
| :--- | :--- | :--- |
| **Microcontrolador** | **ESP32** (con soporte dual-core) | Ejecución del *firmware* y FreeRTOS. |
| **Sistema Operativo** | **FreeRTOS** | Arquitectura concurrente, manejo de tareas en paralelo y protección de datos mediante **Mutex**. |
| **Desarrollo** | **Arduino IDE** (Lenguaje **C/C++**) | Plataforma de desarrollo principal. |
| **Metodología** | **Máquina de Estados Finita (FSM)** | Control determinista y seguro de la lógica del sistema. |
| **Precisión** | **Interrupciones (ISR)** | Conteo de pulsos del sensor de flujo para medición precisa de volumen. |

---

## Integrantes y Roles

Este proyecto fue desarrollado en solitario por **Mario Alberto Nina Gallo**.

| Rol | Responsabilidad |
| :--- | :--- |
| **Ingeniero de Firmware** | Programación completa de las tareas de FreeRTOS, FSM, lógica de control y manejo de sensores/actuadores. |
| **Arquitecto del Sistema** | Diseño de la **Arquitectura Modular** y la implementación del **Estado Compartido** protegido por `Mutex`. |
| **Diseñador de Hardware** | Selección de componentes y diseño del esquema de conexión. |

---

## Estado Actual del Proyecto (Avances Funcionales)

El desarrollo se ha realizado por fases. El *firmware* se encuentra **funcional hasta la Fase 6**, con la integración del *hardware* esencial y la columna vertebral del sistema establecida.

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

- **ESP32**
- **Minibomba de agua**
- **Sensor de flujo**
- **Sensor ultrasónico**
- **Botones:** START, STOP, LLENAR
- **Pantalla** (LCD)
- **Etapa de potencia para la bomba**
- **Fuente de alimentación**
- **Mangueras y estructura básica**

---

## Estados del sistema

El comportamiento se organiza mediante una **máquina de estados**, garantizando un control seguro:

- **SISTEMA DETENIDO**  
- **ESPERANDO BOTELLA**
- **BOTELLA DETECTADA**
- **LLENANDO**
- **LLENADO COMPLETADO**
- **ERROR**

Cada transición depende de eventos como botones, presencia de botella, flujo y tiempo.

---

## Hardware utilizado

- **ESP32**
- **Minibomba de agua**
- **Sensor de flujo**
- **Sensor ultrasónico**
- **Botones:** START, STOP, LLENAR
- **Pantalla** (LCD)
- **Etapa de potencia para la bomba**
- **Fuente de alimentación**
- **Mangueras y estructura básica**

---

## Arquitectura del software

El software está desarrollado en **Arduino IDE**, utilizando una estructura modular y herramientas propias del ESP32:

- **FreeRTOS:** tareas independientes.
- **Interrupciones (ISR):** conteo preciso de los pulsos del sensor de flujo.
- **Temporización no bloqueante:** basada en ticks del sistema.
- **Máquina de estados (FSM):** organiza el comportamiento global del sistema
- **Modularidad:** organización mediante archivos `.ino`, `.cpp` y `.h` para separar lógica, sensores y actuadores.

---

## Estructura inicial del repositorio

```
llenador-automatico-botellas-esp32/
│
├─ src/                     → código fuente del sistema (ino, cpp, h)
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

## Objetivo del proyecto

Desarrollar un sistema embebido seguro y funcional que simule el comportamiento básico de una máquina industrial de llenado, integrando sensores, actuadores, temporización avanzada, interrupciones y una arquitectura de control basada en FreeRTOS.

---

## Licencia

Este proyecto se distribuye bajo los términos especificados en el archivo **LICENSE** incluido en este repositorio.

---

## Autor

Proyecto desarrollado por **Mario Alberto Nina Gallo** como **proyecto final de la asignatura Sistemas Embebidos I (IMT-222)**.



