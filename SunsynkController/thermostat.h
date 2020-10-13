// geyser thermostat address
#define THERM_ADDR 'G'
#define THERM_RX 9
#define THERM_TX 8

// modbus/rs485 interface
ModbusMaster th485;
SoftwareSerial thSerial(THERM_RX, THERM_TX);

// shared state
bool therm_on; // geyser element on?
uint16_t therm_temp; // current temperatire
uint16_t therm_targ; // target temperature
uint16_t therm_targ_min; // target min temperature

void th_pre_tx(void) {
  thSerial.listen();
}

// forward declaration
bool therm_run(void);

void therm_setup()
{
  thSerial.begin(9600);
  th485.begin(THERM_ADDR, thSerial);
  th485.preTransmission(th_pre_tx);
  // turn thermostat off
  // shouldn't really set state variables here, bur...
  st_temp = 0;
  st_temp_min = 0;
  therm_targ = 1; // make sure not zero, so settings are actually sent
  if(!therm_run()) {
    display_error(F("Init therm write failed!"));
    while(1) {} // infinite loop and wait for WDT reset
  }
}

bool therm_read()
{
  uint8_t result;
  int16_t val;
  int i;

  // flush serial before enabling 485 transceiver!
  Serial.flush();
  result = th485.readHoldingRegisters(0, 4);
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
    therm_targ_min = th485.getResponseBuffer(3);
  } else {
    Serial.println(F("therm read failed!"));
    return false;
  }
  // dump stats
  Serial.print(F("Element ON: "));
  Serial.println(therm_on);
  Serial.print(F("Geyser temp: "));
  Serial.println(therm_temp);
  Serial.print(F("Target temp: "));
  Serial.println(therm_targ);
  Serial.print(F("Target min temp: "));
  Serial.println(therm_targ_min);
  return true;
}

// set thermostat target temperature
bool therm_run(void) {
  // don't write if the target state matches what we just read
  if(st_temp == therm_targ && st_temp_min == therm_targ_min) return true;

  // build message
  th485.setTransmitBuffer(0, st_temp);
  th485.setTransmitBuffer(1, st_temp_min);

  // flush serial before enabling 485 transceiver!
  Serial.flush();
  if(th485.writeMultipleRegisters(2, 2) != th485.ku8MBSuccess) {
    Serial.println(F("therm write temps failed!"));
    return false;
  }
  return true;
}
