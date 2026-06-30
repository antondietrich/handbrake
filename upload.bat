@rem @echo off

avrdude -c usbasp-clone -p atmega32u4 -U flash:w:bin/main.hex:i
@rem dump fuses
@rem avrdude -c usbasp -p m32u4 -U lfuse:r:-:h -U hfuse:r:-:h -U efuse:r:-:h