// global comms input/output

int16_t mod_temp = 100; // temperature sensor value
int16_t mod_temp_trg = 45; // target temperature
int16_t mod_temp_trg_min = 43; // hysterisis min temp (defaults to trg - 1/16)

#include "temp_geyserwise.h"

void setup() {
  Serial.begin(9600);
  temp_setup();
}

void loop() {
  temp_read();
  Serial.print(temp_val);
  Serial.print(" ");
  Serial.print(temp_val >> 6);
  Serial.print(" ");
  Serial.print(mod_temp);
  Serial.println("");
}
