# ModbusThermostat
Modbus controlled geyser/water heater thermostat control for Arduino

# Introduction

Basically, I need a smart thermostat, which integrates with my solar inverter.  The plan is to heat water as hot as possible when excess solar energy production is available, sustain a minimum comfortable temperature when not, and shut off the element when inverter load is too high or battery too low, etc.

# Geyserwise

I was hoping to reverse engineer the comms on a Geyserwise controller and use that, but it seems obfuscated, so not really worth the trouble. Instead, I have yanked the MCU, and will be plugging an Arduino into the power/control/input signals.

# Modularised

Software will be modular, with plug and playish insterfaces for each piece of hardware, so it should be reusable for any sort of hardware.

# ModBus

Monitoring/control is though 4 Modbus holding registers:

Addr 0: Status bits (read only)
bit 0 = run flag (set after thermostat temperature is first set)
bit 1 = relay/element (set when the relay is on

Addr 1: Temperature (read only)
Temperature in °C

Addr 2: Target temperature (read/write)
Target temperature in °C

Addr 3: Min target temperature (read/write)
Temperature to turn element on again (heats from Min -> Targ and back again).
NOTE: Set to 15/16 of target temperature whenever Target is set - no need to override it, unless you want a different hysteresis level.
