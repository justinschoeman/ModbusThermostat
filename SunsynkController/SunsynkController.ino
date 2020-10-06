//https://github.com/4-20ma/ModbusMaster.git
#include <ModbusMaster.h>
#include <avr/wdt.h>

// global config
#define BUZZER_PIN 8
#define SUNSYNK_POLL_MS 1000UL

#define BAT_MIN 49
#define POWER_MAX 7000
//#define POWER_LOCKOUT (1UL * 60UL*1000UL)
#define POWER_LOCKOUT (1UL * 20UL*1000UL)
#define POWER_RESTORE 3000

#include "config.h"
#include "rs485.h"
#include "sunsynk.h"
#include "thermostat.h"
#include "state.h"

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
  Serial.println("Init sunsynk");
  sunsynk_setup();

  // set up transmitter
  Serial.println("Init thermostat");
  therm_setup();

  // set up local timer
  poll_ts = millis();

  // set up local state
  sys_temp = therm_temp; // default to on
  sys_ts = millis();

  // keep watchdog ticking
  wdt_reset();
}

void loop()
{
  // wait for poll interval
  if((millis() - poll_ts) < SUNSYNK_POLL_MS) return;
  poll_ts = millis();

  // read inverter state
  if(!sunsynk_read()) {
    Serial.println("sunsynk read failed");
    return;
  }

  // read thermostat state
  if(!therm_read()) {
    Serial.println("th read failed");
    return;
  }

  // set config from state
  config_run();

  // process next state (safe to do this only on a read... extra 1s latency wont hurt on release)
  state_run();

  // set target temp

  // keep watchdog ticking on every successfull loop
  wdt_reset();
}
