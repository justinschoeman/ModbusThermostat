# ModbusThermostat
Modbus controlled geyser/water heater thermostat control for Arduino

# Introduction

Basically, I need a smart thermostat, which integrates with my solar inverter.  The plan is to heat water as hot as possible when excess solar energy production is available, sustain a minimum comfortable temperature when not, and shut off the element when inverter load is too high or battery too low, etc.

# Geyserwise

I was hoping to reverse engineer the comms on a Geyserwise controller and use that, but it seems obfuscated, so not really worth the trouble. Instead, I have yanked the MCU, and will be plugging an Arduino into the power/control/input signals.

# Modularised

Software will be modular, with plug and playish insterfaces for each piece of hardware, so it should be reusable for any sort of hardware.
