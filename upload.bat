@rem @echo off

avrdude -c usbasp-clone -p atmega32u4 -U flash:w:bin/main.hex:i
