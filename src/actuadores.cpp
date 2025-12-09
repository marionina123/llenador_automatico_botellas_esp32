#include "actuadores.h"
#include "config_sistema.h"

void initActuadores()
{
  pinMode(PIN_BOMBA_MOSFET, OUTPUT);

  // Para un MOSFET N-channel en low-side:
  // - LOW  -> bomba apagada (MOSFET en corte)
  // - HIGH -> bomba encendida (MOSFET saturado)
  digitalWrite(PIN_BOMBA_MOSFET, LOW); // bomba apagada al inicio
}

void bombaOn()
{
  digitalWrite(PIN_BOMBA_MOSFET, HIGH);
}

void bombaOff()
{
  digitalWrite(PIN_BOMBA_MOSFET, LOW);
}
