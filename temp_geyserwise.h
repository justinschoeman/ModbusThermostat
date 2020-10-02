/*
 * Geyserwise input/conditioning:
 * 
 *    ------- 5V (PIN 5 MCU)
 *       |
 *       /
 *       \ 10k NTC
 *       /
 *       \
 *       |      1k 5%
 *       + ----/\/\/\-----+ ---------> PIN 32 (MCU)
 *       |                |
 *       /              -----
 *       \              -----
 *       / 3k 1%          |   10uF ?
 *       \                |
 *       |                |
 *    --------------------------- 0V (PIN 1 MCU)
 */

// analog pin for temp input
#define TEMP_PIN A0
// Thermistor R@25C
#define TEMP_R 10000.0
// Thermistor B val
#define TEMP_B 4200.0
// Kelvin offset
#define TEMP_K 273.15
// 25C in Kelvin
#define TEMP_REF (TEMP_K + 25.0)

// set up temperature module
void temp_setup(void) {
  // default pin config is good
  //pinMode(TEMP_PIN, INPUT);
}

// smoothed temp voltage (initialise to max temp, so no spurious relay activations while temperature is stabilising)
uint16_t temp_val = 0xffffU;
int16_t temp_c = 100;

// internal function to calculate temp from ADC val
void _temp_calc(void) {
  uint16_t t = temp_val >> 6;
  double v = t;
  // to voltage
  v *= (5.0/1023.0);
  // to current through 3k resistor
  double i = v / 3000.0;
  // resistance
  double r = (5.0 - v)/i;
  // calculate temp from r
  double c = (TEMP_B * TEMP_REF);
  c /= TEMP_B + (TEMP_REF * log(r / TEMP_R));
  c -= TEMP_K;
  if(c <= 0) {
    mb_temp = 0;
  } else if(c >= 999.0) {
    mb_temp = 999;
  } else {
    mb_temp = c;
  }
}

// read/smoothe temp and update calculation when it changes
void temp_read(void) {
  uint16_t tmp = analogRead(TEMP_PIN);
  uint16_t old_temp_val = temp_val;
  temp_val -= temp_val >> 6;
  temp_val += tmp;
  if(temp_val != old_temp_val) _temp_calc();
}
