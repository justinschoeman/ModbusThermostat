// geyser thermostat address
#define RS485_THERM_ADDR 'G'

// modbus/rs485 interface
ModbusMaster th485;

// shared state
bool therm_on; // geyser element on?
uint16_t therm_temp; // current temperatire
uint16_t therm_targ; // target temperature

void therm_setup()
{
  th485.begin(RS485_THERM_ADDR, Serial);
  th485.preTransmission(preTransmission);
  th485.postTransmission(postTransmission);
}

bool therm_read()
{
  uint8_t result;
  int16_t val;
  int i;

  // flush serial before enabling 485 transceiver!
  Serial.flush();
  result = th485.readHoldingRegisters(0, 3); // ignore hysterisis...
  if(result == th485.ku8MBSuccess) {
#if 0    
    for(i = 0; i < 3; i++) {
      val = th485.getResponseBuffer(i);
      Serial.print(i);
      Serial.print(" ");
      Serial.println(val);
    }
#endif    
    therm_on = th485.getResponseBuffer(0) & 2;
    therm_temp = th485.getResponseBuffer(1);
    therm_targ = th485.getResponseBuffer(2);
  } else {
    Serial.println("therm read failed!");
    return false;
  }
  // modbus slave library does not do interframe timing - manually delay for frame timer
  delay(100);
  // dump stats
  Serial.print("Element ON: ");
  Serial.println(therm_on);
  Serial.print("Geyser temp: ");
  Serial.println(therm_temp);
  Serial.print("Target temp: ");
  Serial.println(therm_targ);
  return true;
}

// set thermostat target temperature
bool therm_set_temp(uint8_t temp) {
  // don't write if the target state matches what we just read
  if(temp == therm_targ) return true;
  
  // flush serial before enabling 485 transceiver!
  Serial.flush();
  if(th485.writeSingleRegister(2, temp) != th485.ku8MBSuccess) {
    Serial.println("therm write failed!");
    return false;
  }
  // modbus slave library does not do interframe timing - manually delay for frame timer
  delay(100);
  return true;
}
