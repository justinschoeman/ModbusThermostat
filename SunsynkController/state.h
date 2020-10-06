uint8_t sys_temp;
unsigned long sys_ts;

void state_run(void) {
  // process sys state
  if(sun_load_power > POWER_MAX || sun_inv_power > POWER_MAX || sun_soc < BAT_MIN) {
    // exceed limit
    Serial.println("POWER TOO HIGH - SHUT OFF LOAD");
    therm_set_temp(0); // set shutoff state
    sys_ts = millis(); // ts keeps track of the last time the power was excessive
    return;
  }
  // power OK - wait for lockout time before clearing
  //if(tx_state) return; // already on? no worries
  // not on?  must wait for restore conditions
  if(sun_load_power < POWER_RESTORE && sun_inv_power < POWER_RESTORE && sun_soc > BAT_MIN) {
    // restore conditions met - wait for lockout
    if((millis() - sys_ts) < POWER_LOCKOUT) {
      Serial.println("LOCKOUT");
      return;
    }
    Serial.println("LOCKOUT OVER - TURN ON POWER");
    therm_set_temp(30);
    return;
  }
  // restore conditions not met - kick the lockout timer
  Serial.println("RESTORE NOT MET");
  sys_ts = millis(); // ts keeps track of the last time the power was excessive
}
