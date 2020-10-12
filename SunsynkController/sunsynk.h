// inverter address
#define INV_ADDR 1
#define INV_RX 5
#define INV_TX 6

// shared state
int16_t sun_inv_power;
int16_t sun_load_power;
uint8_t sun_soc;
int16_t sun_batI; // +ve = discharge
uint16_t sun_hhmm;
int16_t sun_pv_power;

// modbus/rs485 interface
ModbusMaster ss485;
SoftwareSerial ssSerial(INV_RX, INV_TX);

// time read countdown
uint8_t sun_cnt = 0;

void ss_pre_tx(void) {
  ssSerial.listen();
}

// setup
void sunsynk_setup()
{
  ssSerial.begin(9600);
  ss485.begin(INV_ADDR, ssSerial);
  ss485.preTransmission(ss_pre_tx);
}

bool sunsynk_read()
{
  uint8_t result;
  int16_t val;
  int i;

  // flush serial before enabling 485 transceiver!
  Serial.flush();
  // read power stats
  result = ss485.readHoldingRegisters(175, 17);
  if(result == ss485.ku8MBSuccess) {
#if 0
    for(i = 0; i < 17; i++) {
      val = ss485.getResponseBuffer(i);
      Serial.print(175+i);
      Serial.print(" ");
      Serial.println(val);
    }
#endif
    sun_inv_power = ss485.getResponseBuffer(175-175);
    sun_load_power = ss485.getResponseBuffer(178-175);
    sun_soc = ss485.getResponseBuffer(184-175);
    sun_pv_power = ss485.getResponseBuffer(186-175);
    sun_pv_power += ss485.getResponseBuffer(187-175);
    sun_pv_power += ss485.getResponseBuffer(188-175);
    sun_pv_power += ss485.getResponseBuffer(189-175);
    sun_batI = ss485.getResponseBuffer(191-175);
    sun_batI /= 100;
  } else {
    Serial.println(F("inverter read failed!"));
    return false;
  }
  // dump stats
  Serial.print(F("Inverter Power: "));
  Serial.println(sun_inv_power);
  Serial.print(F("Load Power: "));
  Serial.println(sun_load_power);
  Serial.print(F("PV Power: "));
  Serial.println(sun_pv_power);
  Serial.print(F("Bat SOC: "));
  Serial.println(sun_soc);
  Serial.print(F("Bat I: "));
  Serial.println(sun_batI);
  Serial.print(F("TOD: "));
  Serial.println(sun_hhmm);

  // read time of day (every 15x)
  if(sun_cnt++) {
    if(sun_cnt >= 15) sun_cnt = 0;
    return true;
  }
  // flush serial before enabling 485 transceiver!
  Serial.flush();
  result = ss485.readHoldingRegisters(23, 2);
  if(result == ss485.ku8MBSuccess) {
#if 0
    for(i = 0; i < 2; i++) {
      val = ss485.getResponseBuffer(i);
      Serial.print(23+i);
      Serial.print(" ");
      Serial.println(val, HEX);
    }
#endif
    sun_hhmm = ss485.getResponseBuffer(0)&0xffU;
    sun_hhmm *= 100U;
    sun_hhmm += ss485.getResponseBuffer(1)>>8;
  } else {
    Serial.println(F("inverter read T0D failed!"));
    return false;
  }
  return true;
}
