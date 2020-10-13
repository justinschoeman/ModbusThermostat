// transmit enable (active high)
#define RS485_DE 2
// receive enable (active low)
#define RS485_NRE 3

void rs485_setup(void) {
  // set up RS485 pins
  pinMode(RS485_NRE, OUTPUT);
  pinMode(RS485_DE, OUTPUT);
  // Init in receive mode
  digitalWrite(RS485_NRE, 0);
  digitalWrite(RS485_DE, 0);
}

// rs485 transceiver mode control
void preTransmission() {
  Serial.flush();
  while(Serial.available()) {
    int i = Serial.read();
    Serial.println(i);
  }
  Serial.flush();
  digitalWrite(RS485_NRE, 1);
  digitalWrite(RS485_DE, 1);
}

void postTransmission() {
  digitalWrite(RS485_NRE, 0);
  digitalWrite(RS485_DE, 0);
}
