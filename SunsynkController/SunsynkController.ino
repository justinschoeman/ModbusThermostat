//https://github.com/4-20ma/ModbusMaster.git
#include <ModbusMaster.h>
#include <avr/wdt.h>

// global config
#define BUZZER_PIN 8
#define POLL_MS 1000UL

// helper function

// overflow resistant interval calculation
unsigned long ms_interval(unsigned long ts) {
  unsigned long ms = millis();
  ms -= ts;
  return ms;
}

#include "rs485.h"
#include "sunsynk.h"
#include "config.h"
#include "state.h"
#include "thermostat.h"

unsigned long poll_ts;


void setup()
{
  // start watchdog
  wdt_enable(WDTO_8S);

  // beep the beeper
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, 1);
  delay(250UL);
  digitalWrite(BUZZER_PIN, 0);
  delay(100UL);
  digitalWrite(BUZZER_PIN, 1);
  delay(250UL);
  digitalWrite(BUZZER_PIN, 0);

  // keep watchdog ticking
  wdt_reset();

  // set up serial port
  Serial.begin(9600);

  // set up rs485
  rs485_setup();

  // set up inverter
  Serial.println(F("Init sunsynk"));
  sunsynk_setup();

  // set up transmitter
  Serial.println(F("Init thermostat"));
  therm_setup();

  // set up local timer
  poll_ts = millis();

  // set up local state
  state_setup();

  // keep watchdog ticking
  wdt_reset();
}

void loop()
{
  // wait for poll interval
  if((millis() - poll_ts) < POLL_MS) return;
  poll_ts = millis();

  // read inverter state
  if(!sunsynk_read()) {
    Serial.println(F("sunsynk read failed"));
    return;
  }

  // read thermostat state
  if(!therm_read()) {
    Serial.println(F("th read failed"));
    return;
  }

  // test
  //sun_hhmm = 700;
  //sun_soc = 50;

  // set config from state
  config_run();

  // process next state (safe to do this only on a read... extra 1s latency wont hurt on release)
  state_run();

  // set target temp
  if(!therm_run()) {
    Serial.println(F("th run failed"));
    return;
  }

  // keep watchdog ticking on every successfull loop
  wdt_reset();
}
