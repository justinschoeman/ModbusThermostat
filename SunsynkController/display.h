/*
 * General purpose display module.
 * 
 * Uses u8x8 from U8g2 for broad compatibility.
 * 
 * Choose your display and connect as per mfg/library instructions.
 * 
 * Code is configured for 128x64 display, but can be modified as required.
 */
#include <U8x8lib.h>

U8X8_SH1106_128X64_NONAME_HW_I2C u8x8(/* reset=*/ U8X8_PIN_NONE);

void display_setup(void) {
    u8x8.begin();
    u8x8.setFont(u8x8_font_pressstart2p_r );
    //u8x8.drawString(0, 0, "01234567890123456789");
}

bool disp_error;

void display_error(const __FlashStringHelper  * t) {
  u8x8.clearDisplay();
  Serial.println(t);
  u8x8.setCursor(0, 1);
  u8x8.print(t);
  delay(500);
  disp_error = true;
  // test - hard lockup so we know there was an error
  //while(1) wdt_reset();
}

void display_run(void) {
  char buf[17];
  // clear error display
  if(disp_error) {
    u8x8.clearDisplay();
    disp_error = false;
  }
  // temp bar
  sprintf(buf, "%02u", st_temp_min);
  u8x8.drawString(1, 1, buf);
  if(therm_on) {
    sprintf(buf, "*%02u* ", therm_temp);
  } else {
    sprintf(buf, " %02u  ", therm_temp);
  }
  u8x8.draw2x2String(4, 0, buf);
  sprintf(buf, "%02u", st_temp);
  u8x8.drawString(13, 1, buf);
  // state bar
  u8x8.setCursor(0, 3);
  u8x8.print(F(" PL H1 H2 B1 B2"));
  u8x8.setCursor(0, 2);
  if(!st_lockout) {
    u8x8.print(F(" --"));
  } else {
    u8x8.print(F("   "));
  }
  if(st_heat1) {
    u8x8.print(F(" --"));
  } else {
    u8x8.print(F("   "));
  }
  if(st_heat2) {
    u8x8.print(F(" --"));
  } else {
    u8x8.print(F("   "));
  }
  if(st_boost1) {
    u8x8.print(F(" --"));
  } else {
    u8x8.print(F("   "));
  }
  if(st_boost2) {
    u8x8.print(F(" --"));
  } else {
    u8x8.print(F("   "));
  }
}
