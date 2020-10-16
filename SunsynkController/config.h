/*
 * This is thw worst part - deciding how to limit the functionality to something practically configurable.
 * 
 * The software has 3 primary aims
 * 1) protect inverter - disconnect geyser whenever load is too high
 * 2) maintain minimum usable temperature
 * 3) store additional energy by heating as hot as safely possible when excess energy is available
 * 4) (additionally - add some time dependent capabilities)
 * 
 * Load protection is the first priority, and runs as a separate process.
 * (NOTE: this relies on the inverter being able to sustain temporary overloads)
 * 
 * cfg_load_max: this is the maximum allowed load - if load exceeds this level then disconnect the geyser
 * cfg_load_restore: if the geyser has been disconnected due to overload, the only re-enable whne below this current
 * cfg_load_lock_ts: keep geyser off until restore conditions have been met for at least this time (ms)
 * 
 * For normal heating modes (X is a number 1, or 2, for two possible sets of options - if activation conditions are good for both, then the highest temp is chosen):
 * 
 * cfg_heatX_soc:  minimum soc for this setting, if battery soc is below, then do not use this setting
 * cfg_heatX_temp: target temperature
 * cfg_heatX_temp_min: target minimum temperature (for hysteresis control - cam be = to temp for 1C hysteresis)
 * 
 * For boost modes, I am primarily using mine for off-grid heating, so it is better to heat the geyser really hot when there is lots of solar power
 * than to reheat it later with battery power.
 * So, there are two ways I want to use this:
 * 1) when there is enough excess power to run the load + geyser and still charge, then boost the geyser as much as possible
 * 2) when battery is really full, and there is still *some* power avaialble, then boost the geyser as much as possible
 * 
 * cfg_boostX_soc: minimum soc for this setting, if battery soc is below, then do not use this setting
 * cfg_boostX_power: activate boost setting when PV_POWER - LOAD_POWER >= this setting
 * cfg_boostX_batI: if battery current exceeds this, then cancel boost mode (can be negative to ensure minimum charge current remains)
 * cfg_boostX_temp: target temperature
 * cfg_boostX_temp_min: target minimum temperature (for hysteresis control - cam be = to temp for 1C hysteresis)
 * 
 * cfg_heat_lock_ts: keep geyser off until new heat/boost conditions have been met for at least this time (ms)
 * 
 * If any soc value is 0, then that option is disabled
 * 
 */

uint16_t cfg_load_max = 7000;
uint16_t cfg_load_restore = 3000;
#if 1
unsigned long cfg_load_lock_ts = 5UL * 60UL * 1000UL;
unsigned long cfg_heat_lock_ts = 10UL * 60UL * 1000UL;
#else
// short timers for testing
unsigned long cfg_load_lock_ts = 20UL * 1000UL;
unsigned long cfg_heat_lock_ts = 30UL * 1000UL;
#endif
unsigned long cfg_boost_cut_ts = 10UL * 1000UL;


struct config_s {
  uint16_t start_hhmm; // hhmm start time for this config block -> ends with next config entry

  uint8_t heat1_soc;
  uint8_t heat1_temp;
  uint8_t heat1_temp_min;

  uint8_t heat2_soc;
  uint8_t heat2_temp;
  uint8_t heat2_temp_min;

  uint8_t boost1_soc;
  int16_t boost1_power;
  int8_t boost1_batI;
  uint8_t boost1_temp;
  uint8_t boost1_temp_min;

  uint8_t boost2_soc;
  int16_t boost2_power;
  int8_t boost2_batI;
  uint8_t boost2_temp;
  uint8_t boost2_temp_min;
};

struct config_s cfgs[] = {
  { // early morning config - prep for morning shower if there is reasonable battery left...
    .start_hhmm = 400,

    // min tolerable shower if min available
    .heat1_soc = 30,
    .heat1_temp = 35,
    .heat1_temp_min = 33,

    // decent shower if lots available
    .heat2_soc = 50,
    .heat2_temp = 45,
    .heat2_temp_min = 43,

    // disable
    .boost1_soc = 0,
    .boost1_power = 0,
    .boost1_batI = 0,
    .boost1_temp = 0,
    .boost1_temp_min = 0,
    .boost2_soc = 0,
    .boost2_power = 0,
    .boost2_batI = 0,
    .boost2_temp = 0,
    .boost2_temp_min = 0,
  },
  { // daytime config
    .start_hhmm = 600,

    // last ditch (maintain 40deg or 40%)
    .heat1_soc = 40,
    .heat1_temp = 40,
    .heat1_temp_min = 40,

    // normal temp (aim for 50deg)
    .heat2_soc = 70,
    .heat2_temp = 50,
    .heat2_temp_min = 49,

    // boost bulk energy (boost hard as long as there any excess power)
    .boost1_soc = 70,
    .boost1_power = 3500,
    .boost1_batI = 0,
    .boost1_temp = 80,
    .boost1_temp_min = 80,

    // top-up boost (boost what we can when battery is very full allow up to 2kW from battery)
    .boost2_soc = 90,
    .boost2_power = -100, // inverter lightly cycles battery at top...
    .boost2_batI = 40,
    .boost2_temp = 80,
    .boost2_temp_min = 80,
  },
  { // evening config - don't expect any boost, maintain comfortable temp for evening showers
    .start_hhmm = 1800,

    // min comfortable shower, preserve at least 70% battery at this time
    .heat1_soc = 70,
    .heat1_temp = 45,
    .heat1_temp_min = 43,

    // disable
    .heat2_soc = 0,
    .heat2_temp = 0,
    .heat2_temp_min = 0,
    .boost1_soc = 0,
    .boost1_power = 0,
    .boost1_batI = 0,
    .boost1_temp = 0,
    .boost1_temp_min = 0,
    .boost2_soc = 0,
    .boost2_power = 0,
    .boost2_batI = 0,
    .boost2_temp = 0,
    .boost2_temp_min = 0,
  },
  { // overnight config - disable
    .start_hhmm = 2300,
  
    // disable
    .heat1_soc = 0,
    .heat1_temp = 0,
    .heat1_temp_min = 0,
    .heat2_soc = 0,
    .heat2_temp = 0,
    .heat2_temp_min = 0,
    .boost1_soc = 0,
    .boost1_power = 0,
    .boost1_batI = 0,
    .boost1_temp = 0,
    .boost1_temp_min = 0,
    .boost2_soc = 0,
    .boost2_power = 0,
    .boost2_batI = 0,
    .boost2_temp = 0,
    .boost2_temp_min = 0,
  },
};

#define CFG_CNT (sizeof(cfgs)/sizeof(struct config_s))

struct config_s * cfg;

// use time of day to select correct config
void config_run(void) {
  int i;

  // test
  //sun_hhmm += 20;
  
  Serial.print(F("Search config for time "));
  Serial.println(sun_hhmm);
  for(i = 0; i < CFG_CNT; i++) {
    if(sun_hhmm >= cfgs[i].start_hhmm) {
      // potentially ours?  check if it should not rather be next
      if(i == (CFG_CNT - 1)) break; // no next - must be us
      if(sun_hhmm < cfgs[i + 1].start_hhmm) break; // not next - must be us
    }
  }
  if(i == CFG_CNT) {
    // completed loop without finding it? must be before first i.e. last...
    cfg = cfgs + (CFG_CNT - 1);
  } else {
    cfg = cfgs + i;
  }
  Serial.print(F("Chose CFG: "));
  Serial.println(cfg->start_hhmm);
}
