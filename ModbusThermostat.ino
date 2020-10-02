// debug flag = set to true when app can use serial port for debugging
bool dbg = true;

// https://github.com/yaacov/ArduinoModbusSlave
#include <ModbusSlave.h>
#include <avr/wdt.h>

#define MB_SLAVE_ID 1
#define MB_BAUD 9600
#define MB_RS485_NRE_DE 8

// define this to keep the relay off until a temperature write is received
#define PARANOID_OFF
// define this to watchdog reset on comms list
#define PARANOID_COMMS

Modbus slave(Serial, MB_SLAVE_ID, MB_RS485_NRE_DE);

// modbus status flags
// running - set when first temp write is received
#define MB_STATUS_RUN  1U
// element on - set when relay/element is on
#define MB_STATUS_ON   2U

// global comms input/output
uint16_t mb_status = 0; // status bits
uint16_t mb_temp = 100; // temperature sensor value
uint16_t mb_temp_trg = 45; // target temperature
uint16_t mb_temp_trg_min = 43; // hysterisis min temp (defaults to trg - 1/16)

#include "temp_geyserwise.h"
#include "relay_geyserwise.h"

bool _mb_comms;

uint8_t mb_read1(uint16_t address, int8_t b) {
  uint16_t val;
  switch(address) {
    case 0:
      val = mb_status;
      break;
    case 1:
      val = mb_temp;
      break;
    case 2:
      val = mb_temp_trg;
      break;
    case 3:
      val = mb_temp_trg_min;
      break;
    default:
      return 0;
  }
  slave.writeRegisterToBuffer(b, val);
  return 1;
}

uint8_t mb_read(uint8_t fc, uint16_t address, uint16_t length) {
  int i;
  for(i = 0; i < length; i++) {
    if(!mb_read1(address+i, i)) return STATUS_ILLEGAL_DATA_ADDRESS;
  }
  _mb_comms = true;
  return STATUS_OK;
}

uint8_t mb_write1(uint16_t address, uint16_t val) {
  switch(address) {
    case 2:
      mb_temp_trg = val;
      mb_temp_trg_min = val - (val >> 4);
      mb_status |= MB_STATUS_RUN;
      break;
    case 3:
      mb_temp_trg_min = val;
      break;
    default:
      return 0;
  }
  return 1;
}

uint8_t mb_write(uint8_t fc, uint16_t address, uint16_t length) {
  int i;
  for(i = 0; i < length; i++) {
    if(!mb_write1(address+i, slave.readRegisterFromBuffer(i))) return STATUS_ILLEGAL_DATA_ADDRESS;
  }
  _mb_comms = true;
  return STATUS_OK;
}


void setup() {
  // start watchdog
  wdt_enable(WDTO_8S);

  // make sure relay is turned off as early as possible!
  relay_setup();
  Serial.begin(MB_BAUD);
  Serial.println("Starting...");
  slave.cbVector[CB_READ_HOLDING_REGISTERS] = mb_read;
  slave.cbVector[CB_WRITE_HOLDING_REGISTERS] = mb_write;
  slave.begin(MB_BAUD);
  temp_setup();
}

void loop() {
  // read current temperature
  temp_read();
  //return;
  
  // run modbus engine
  dbg = false;
  _mb_comms = false;
  Serial.flush();
  if(!slave.poll()) {
    Serial.flush();
    dbg = true;
  }
  //return;

  // run relay
  // be very paranoid about turning it off
#ifdef PARANOID_OFF
  if(mb_status & MB_STATUS_RUN) {
#endif
  if(mb_temp_trg == 0) {
    // targ == 0 -> always off
    relay_off();
  } else if(mb_temp >= mb_temp_trg) {
    // temp > targ -> always off
    relay_off();
  } else if(mb_temp < mb_temp_trg_min) {
    // temp < min, then we can turn on
    relay_on();
  }
  // untested case is temp >= min and temp < trg - in this case leave state unchanged
#ifdef PARANOID_OFF
  } else {
    // have not yet received a temperature write - keep relay off
    relay_off();
  }
#endif

  // dump debug, if possible
  if(dbg) {
    Serial.print(temp_val);
    Serial.print(" ");
    Serial.print(temp_val >> 6);
    Serial.print(" ");
    Serial.print(mb_temp);
    Serial.println("");
  }

  // bump watchdog
#ifdef PARANOID_COMMS
  if(_mb_comms) // only reset wdt if we successfully processed a modbus command
#endif
  wdt_reset();
}
