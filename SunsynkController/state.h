/*
 *  Maintain separate state engines for each heating alogoritm:
 *  1) power lockout
 *  2) heat1
 *  3) heat2
 *  4) boost1
 *  5) boost2
 *  
 *  Run them all every time, so that their state is up to date when required.
 * 
 *  Finally, set temperature based on state.
 *  If power lockout is disabled, then turn everything off.
 *  Otherwise evaluate all active states and pick the highest temperature
 */

uint8_t st_temp; // target temperature
uint8_t st_temp_min; //target minimum temperature

bool st_lockout; // lockout state
unsigned long st_lockout_ms; // time since last lockout condition

bool st_heat1; // heat1 state
unsigned long st_heat1_ms; // time since last heat1 disable condition

bool st_heat2; // heat2 state
unsigned long st_heat2_ms; // time since last heat2 disable condition

bool st_boost1; // boost1 state
unsigned long st_boost1_ms; // time since last boost1 disable condition
unsigned long st_boost1_cut_ms; // time since last boost1 enable condition

bool st_boost2; // boost2 state
unsigned long st_boost2_ms; // time since last boost2 disable condition
unsigned long st_boost2_cut_ms; // time since last boost2 enable condition

void state_setup(void) {
  // set defaults
  st_temp = 0;
  st_temp_min = 0;

  st_lockout = false;
  st_lockout_ms = millis();
  st_heat1 = false;
  st_heat1_ms = st_lockout_ms;
  st_heat2 = false;
  st_heat2_ms = st_lockout_ms;
  st_boost1 = false;
  st_boost1_ms = st_lockout_ms;
  st_boost1_cut_ms = st_lockout_ms;
  st_boost2 = false;
  st_boost2_ms = st_lockout_ms;
  st_boost2_cut_ms = st_lockout_ms;
}

// run load lockout state engine
void _st_run_load(void) {
  
  // always check power exceded?
  if(sun_load_power > cfg_load_max || sun_inv_power > cfg_load_max) {
    // exceed limit for either load or inverter power - disable state, log time and return
    Serial.println(F("load exceed"));
    st_lockout = false;
    st_lockout_ms = millis(); // keeps track of the last time the power was excessive
    return;
  }
  
  // load not excessive
  if(st_lockout) return; // not lockout - no need to evaluate release...
  
  // otherwise test if restore conditions are met
  if(sun_load_power < cfg_load_restore && sun_inv_power < cfg_load_restore) {
    // restore conditions met - wait for lockout
    if(ms_interval(st_lockout_ms) < cfg_load_lock_ts) {
      Serial.println(F("load lock time"));
      return;
    }
    // lockout interval passed
    Serial.println(F("load lock release"));
    st_lockout = true;
    return;
  }

  // restore conditions not met - kick the lockout timer
  Serial.println(F("load not restore"));
  st_lockout_ms = millis(); // keeps track of the last time the power was excessive
}

// run heat1 state engine
void _st_run_heat1(void) {
  // soc below minimum?
  if(cfg->heat1_soc == 0 || sun_soc < cfg->heat1_soc) {
    Serial.println(F("heat1 exceed/disabled"));
    st_heat1 = false;
    st_heat1_ms = millis();
    return;
  }
  // soc ok
  if(st_heat1) return; // already active - no further processing
  // check for lockout timer
  if(ms_interval(st_heat1_ms) < cfg_heat_lock_ts) {
    Serial.println(F("heat1 lock time"));
    return;
  }
  // lockout interval passed
  Serial.println(F("heat1 lock release"));
  st_heat1 = true;
  return;
}

// run heat2 state engine
void _st_run_heat2(void) {
  // soc below minimum?
  if(cfg->heat2_soc == 0 || sun_soc < cfg->heat2_soc) {
    Serial.println(F("heat2 exceed/disabled"));
    st_heat2 = false;
    st_heat2_ms = millis();
    return;
  }
  // soc ok
  if(st_heat2) return; // already active - no further processing
  // check for lockout timer
  if(ms_interval(st_heat2_ms) < cfg_heat_lock_ts) {
    Serial.println(F("heat2 lock time"));
    return;
  }
  // lockout interval passed
  Serial.println(F("heat2 lock release"));
  st_heat2 = true;
  return;
}

// run boost1 state engine
void _st_run_boost1(void) {
  // soc below minimum?
  if(cfg->boost1_soc == 0 || sun_soc < cfg->boost1_soc || sun_batI > cfg->boost1_batI) {
    if(st_boost1 && ms_interval(st_boost1_cut_ms) < cfg_boost_cut_ts) {
      Serial.println(F("boost1 exceed timer"));
      return;
    }
    Serial.println(F("boost1 exceed/disabled"));
    st_boost1 = false;
    st_boost1_ms = millis();
    return;
  }
  st_boost1_cut_ms = millis(); // keep track of the last time conditions were good
  // soc ok
  if(st_boost1) return; // already active - no further processing
  // check for restore condition
  //Serial.println(sun_pv_power - sun_load_power);
  if(sun_pv_power > 0 && (sun_pv_power - sun_load_power) > cfg->boost1_power) {
    // excess pv available
    if(ms_interval(st_boost1_ms) < cfg_heat_lock_ts) {
      Serial.println(F("boost1 lock time"));
      return;
    } else {
      // lockout interval passed
      Serial.println(F("boost1 lock release"));
      st_boost1 = true;
      return;
    }
  }
  Serial.println(F("boost1 not restore"));
  st_boost1_ms = millis(); // keeps track of the last time the power was insufficient
}

// run boost2 state engine
void _st_run_boost2(void) {
  // soc below minimum?
  if(cfg->boost2_soc == 0 || sun_soc < cfg->boost2_soc || sun_batI > cfg->boost2_batI) {
    if(st_boost2 && ms_interval(st_boost2_cut_ms) < cfg_boost_cut_ts) {
      Serial.println(F("boost2 exceed timer"));
      return;
    }
    Serial.println(F("boost2 exceed/disabled"));
    st_boost2 = false;
    st_boost2_ms = millis();
    return;
  }
  st_boost2_cut_ms = millis(); // keep track of the last time conditions were good
  // soc ok
  if(st_boost2) return; // already active - no further processing
  // check for restore condition
  if(sun_pv_power > 0 && (sun_pv_power - sun_load_power) > cfg->boost2_power) {
    // excess pv available
    if(ms_interval(st_boost2_ms) < cfg_heat_lock_ts) {
      Serial.println(F("boost2 lock time"));
      return;
    } else {
      // lockout interval passed
      Serial.println(F("boost2 lock release"));
      st_boost2 = true;
      return;
    }
  }
  Serial.println(F("boost2 not restore"));
  st_boost2_ms = millis(); // keeps track of the last time the power was insufficient
}


void state_run(void) {
  // reset target temperature to 0 -> it must be activated by a relevant state if possible
  st_temp = 0;
  st_temp_min = 0;

  // run all state engines
  _st_run_load();
  _st_run_heat1();
  _st_run_heat2();
  _st_run_boost1();
  _st_run_boost2();

  // set target temperature
  // lockout takes priority
  if(!st_lockout) {
    Serial.println(F("load lockout"));
    return;
  }
  // otherwise evaluate all active heating options
  if(st_heat1) {
    Serial.println(F("apply heat1"));
    st_temp = cfg->heat1_temp;
    st_temp_min = cfg->heat1_temp_min;
  }
  if(st_heat2) {
    Serial.println(F("apply heat2"));
    if(cfg->heat2_temp > st_temp) st_temp = cfg->heat2_temp;
    if(cfg->heat2_temp_min > st_temp_min) st_temp_min = cfg->heat2_temp_min;
  }
  if(st_boost1) {
    Serial.println(F("apply boost1"));
    if(cfg->boost1_temp > st_temp) st_temp = cfg->boost1_temp;
    if(cfg->boost1_temp_min > st_temp_min) st_temp_min = cfg->boost1_temp_min;
  }
  if(st_boost2) {
    Serial.println(F("apply boost2"));
    if(cfg->boost2_temp > st_temp) st_temp = cfg->boost2_temp;
    if(cfg->boost2_temp_min > st_temp_min) st_temp_min = cfg->boost2_temp_min;
  }
  // sanity check
  if(st_temp_min == 0 || st_temp_min > st_temp) {
    // fake minimal hysteresis
    st_temp_min = st_temp - (st_temp >> 4);
  }
  // at least 1 degree of hysteresis, no matter what
  if(st_temp_min == st_temp && st_temp > 0) st_temp_min -= 1;
  Serial.print(F("target temp: "));
  Serial.println(st_temp);
  Serial.print(F("target temp_min: "));
  Serial.println(st_temp_min);
}
