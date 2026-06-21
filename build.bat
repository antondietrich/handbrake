@rem @echo off

@set MCU=atmega32u4
@set FCPU=16000000ul
@set FUSB=48000000ul
@set PLL=2
@set MCUDEF=__AVR_ATmega32U4__

@set DEBUG=-DDEBUG=1

@rem @avr-gcc -std=gnu99 -g -Os -mmcu=%MCU% -DUSE_LUFA_CONFIG_HEADER -IConfig/ %INCLUDE% %DEFINE% -DF_CPU=%FCPU% -DF_USB=%FCPU% -D%MCUDEF%  -c src/Joystick.c -o bin/joystick.o
@avr-gcc -g -Os -mmcu=%MCU% -DUSE_LUFA_CONFIG_HEADER -IConfig/ %DEBUG% -DF_CPU=%FCPU% -DF_USB=%FCPU% -D%MCUDEF%  -c src/main.cpp -o bin/joystick.o
@rem @avr-gcc -std=gnu99 -g -Os -mmcu=%MCU% -DUSE_LUFA_CONFIG_HEADER -IConfig/ %INCLUDE% %DEFINE% -DF_CPU=%FCPU% -DF_USB=%FCPU% -D%MCUDEF%  -c src/Descriptors.c -o bin/descriptors.o

@rem @avr-gcc -std=gnu99 -g -Os -mmcu=%MCU% -DUSE_LUFA_CONFIG_HEADER -IConfig/ %INCLUDE% %DEFINE% -DF_CPU=%FCPU% -DF_USB=%FCPU% -D%MCUDEF%  -c src/lufa/AVR8/USBController_AVR8.c -o bin/usbcon.o
@rem @avr-gcc -std=gnu99 -g -Os -mmcu=%MCU% -DUSE_LUFA_CONFIG_HEADER -IConfig/ %INCLUDE% %DEFINE% -DF_CPU=%FCPU% -DF_USB=%FCPU% -D%MCUDEF%  -c src/lufa/AVR8/Endpoint_AVR8.c -o bin/endp.o
@rem @avr-gcc -std=gnu99 -g -Os -mmcu=%MCU% -DUSE_LUFA_CONFIG_HEADER -IConfig/ %INCLUDE% -DF_CPU=%FCPU% -DF_USB=%FCPU% -D%MCUDEF%  -c src/lufa/USBTask.c -o bin/usbtask.o
@rem @avr-gcc -std=gnu99 -g -Os -mmcu=%MCU% -DUSE_LUFA_CONFIG_HEADER -IConfig/ %INCLUDE% %DEFINE% -DF_CPU=%FCPU% -DF_USB=%FCPU% -D%MCUDEF%  -c src/lufa/AVR8/EndpointStream_AVR8.c -o bin/endps_a8.o
@rem @avr-gcc -std=gnu99 -g -Os -mmcu=%MCU% -DUSE_LUFA_CONFIG_HEADER -IConfig/ %INCLUDE% %DEFINE% -DF_CPU=%FCPU% -DF_USB=%FCPU% -D%MCUDEF%  -c src/lufa/AVR8/USBInterrupt_AVR8.c -o bin/usbint_a8.o
@rem @avr-gcc -std=gnu99 -g -Os -mmcu=%MCU% -DUSE_LUFA_CONFIG_HEADER -IConfig/ %INCLUDE% %DEFINE% -DF_CPU=%FCPU% -DF_USB=%FCPU% -D%MCUDEF%  -c src/lufa/DeviceStandardReq.c -o bin/dsr.o

@rem @avr-gcc -g -mmcu=%MCU% -o bin/main.elf bin/joystick.o bin/descriptors.o bin/usbcon.o bin/endp.o bin/endps_a8.o bin/usbint_a8.o bin/dsr.o
@avr-gcc -g -mmcu=%MCU% -o bin/main.elf bin/joystick.o
@rem avr-objdump -h -S bin/demo.elf
@avr-objcopy -j .text -j .data -O ihex bin/main.elf bin/main.hex

@goto END

:FAIL
@echo Invalid parameters

:END
@echo Finished