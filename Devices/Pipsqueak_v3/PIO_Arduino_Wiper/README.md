# Pipsqueak Wiper Program

This simple program clears the virtual EEPROM on a Pipsqueak, rendering it safe to transfer
to a new owner or dispose of.

It's important to wipe Pipsqueak non-volatile memory because that memory contains a private
key that could be used to impersonate the device in the future as well as WiFi credentials
and server addresses that you probably don't want to make public.

## Note: Assumed Pin Assignment

This routine assumes the following pin assignments:

Red Indicator LED: Wemost D1 Mini Pin D7 (GPIO 13)
Green Indicator LED: Wemos D1 Mini Pin D6 (GPIO 12)
Signal Enable: Wemos D1 Mini Pin D1 (GPIO 5)
