// debug flag = set to true when app can use serial port for debugging
bool dbg = true;

// https://github.com/yaacov/ArduinoModbusSlave
#include <ModbusSlave.h>

#define MB_SLAVE_ID 1
#define MB_BAUD 9600
#define MB_RS485_NRE_DE 8

Modbus slave(Serial, MB_SLAVE_ID, MB_RS485_NRE_DE);

// global comms input/output

#define MOD_STATUS_RUN 1
#define MOD_STATUS_ON 2
uint16_t mb_status = 0; // status bits
uint16_t mb_temp = 100; // temperature sensor value
uint16_t mb_temp_trg = 45; // target temperature
uint16_t mb_temp_trg_min = 43; // hysterisis min temp (defaults to trg - 1/16)

#include "temp_geyserwise.h"

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
  return STATUS_OK;
}

uint8_t mb_write1(uint16_t address, uint16_t val) {
  switch(address) {
    case 2:
      mb_temp_trg = val;
      mb_temp_trg_min = val - (val >> 4);
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
  return STATUS_OK;
}


void setup() {
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
  Serial.flush();
  if(!slave.poll()) {
    Serial.flush();
    dbg = true;
  }
  //return;

  // run relay

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
}
