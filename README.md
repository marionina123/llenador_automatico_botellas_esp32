# Llenador Automático de Botellas – ESP32  
### Proyecto Final de Sistemas Embebidos

Este repositorio contiene el desarrollo de un **sistema embebido** capaz de llenar automáticamente botellas con un volumen fijo de **500 ml**, empleando un microcontrolador **ESP32**, sensores de propósito industrial y una arquitectura basada en **FreeRTOS**. El sistema replica de forma simplificada el comportamiento de una máquina industrial de llenado, priorizando la seguridad, la precisión y el control determinista.

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

## 🧱 Arquitectura del software

El software está desarrollado en **Arduino IDE**, utilizando una estructura modular y herramientas propias del ESP32:

- **FreeRTOS:** tareas independientes.
- **Interrupciones (ISR):** conteo preciso de los pulsos del sensor de flujo.
- **Temporización no bloqueante:** basada en ticks del sistema.
- **Máquina de estados (FSM):** organiza el comportamiento global del sistema
- **Modularidad:** organización mediante archivos `.ino`, `.cpp` y `.h` para separar lógica, sensores y actuadores.

---

## Estructura inicial del repositorio
llenador_automatico_botellas_esp32
/docs/ → documentación del proyecto
/hardware/ → esquemas, diagramas, fotos del montaje
/src/ → código fuente (FreeRTOS, lógica del sistema)
README.md → descripción del proyecto
LICENSE → licencia del repositorio
.gitignore → archivos ignorados por Git

llenador-automatico-botellas-esp32/
│
├─ src                                      → archivo principal del sistema
│
├─ docs/                                    → reportes y archivos del proyecto
├─ hardware/                                → esquemas, diagramas, fotos del montaje
├─ img/                                     → imágenes del prototipo
│
├─ README.md                                → descripción general del proyecto
├─ LICENSE                                  → licencia del repositorio
└─ .gitignore                               → archivos ignorados por Git

---

## Objetivo del proyecto

Desarrollar un sistema embebido seguro y funcional que simule el comportamiento básico de una máquina industrial de llenado, integrando sensores, actuadores, temporización avanzada, interrupciones y una arquitectura de control basada en FreeRTOS.

---

## Licencia

Este proyecto se distribuye bajo los términos especificados en el archivo **LICENSE** incluido en este repositorio.

---

## Autor

Proyecto desarrollado por **Mario Alberto Nina Gallo** como **proyecto final de la asignatura Sistemas Embebidos I (IMT-222)**.



