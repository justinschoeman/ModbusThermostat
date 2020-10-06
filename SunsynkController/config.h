struct config_s {
  uint16_t start_time; // hhmm start time for this config block -> ends with next config entry
  uint8_t temp; // target temperature (0 to turn off)
  uint8_t soc_min; // minimum battery SOC to activate geyser (disable when less)

  uint8_t boost_temp; // target boost temperature (0 to turn off)
  uint8_t boost_soc_min; // minimum battery soc to enable boost mode
  uint16_t boost_max_drain; // maximum battery current drain in boost mode
};

struct config_s cfgs[] = {
  {
    .start_time = 600,
    .temp = 40,
    .soc_min = 40,
    .boost_temp = 50,
    .boost_soc_min = 50,
    .boost_max_drain = 100, // always boost to 50 if we have enough battery
  },
  {
    .start_time = 800,
    .temp = 40,
    .soc_min = 40,
    .boost_temp = 70,
    .boost_soc_min = 80,
    .boost_max_drain = 30, // max 1.5kw = 1.5kw from solar
  },
  {
    .start_time = 1800,
    .temp = 45,
    .soc_min = 50,
    .boost_temp = 0,
    .boost_soc_min = 80,
    .boost_max_drain = 30, // max 1.5kw = 1.5kw from solar
  },
  {
    .start_time = 2300,
    .temp = 0,
    .soc_min = 40,
    .boost_temp = 0,
    .boost_soc_min = 80,
    .boost_max_drain = 30, // max 1.5kw = 1.5kw from solar
  },
};

#define CFG_CNT (sizeof(cfgs)/sizeof(struct config_s))

void config_run(void) {
}
