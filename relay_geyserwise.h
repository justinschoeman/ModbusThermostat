/*
 *  Geyserwise relay output
 *  
 *  ----------------------------------12V
 *                           |     |
 *                           -     C
 *                   Diode   ^     C Geysser relay coil
 *                           |     C
 *                           |     |
 *                           +-----+
 *                                 |
 *                                 /
 *                              |/
 *  MCU PIN 26 -/\/\/\----------|     2N5551
 *              1k 5%           |\
 *                                \,
 *                                |
 * -------------------------------+-- 0V
 *                              
 */

#define RELAY_PIN 9

void relay_setup(void) {
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);
}

void relay_on(void) {
  digitalWrite(RELAY_PIN, HIGH);
  mb_status |= MB_STATUS_ON;
}

void relay_off(void) {
  digitalWrite(RELAY_PIN, LOW);
  mb_status &= ~MB_STATUS_ON;
}
