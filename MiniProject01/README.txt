*************************
Miniproject 1
Author: Ross Hansen
*************************

The file mp1.c contains the miniproject main code. The file pwmLib.c contains helper libraries for PWMs. pwmTest is a test wrapper to easily use the pwm libraries for testing.

mp1 turns on and off LEDs based off an interrupt-driven switch input. In one switch direction, the GPIO LED is off and the PWM LED blinks at a frequency of 3 Hz. In the other switch direction, the GPIO LED is turned on and the PWM LED changes frequency to 100 Hz. In either switch configuration the duty cycle of the PWM LED is controlled by the analog input.

The switch is read as an interrupt, triggered on both rising and falling edges. Whenever an edge is triggered, in addition to the LED patterns changing, the I2C temperature sensor is read and the temperature is displayed.

All GPIO pins, I2C parameters, etc. are specified as arguments passed to mp1 at the command line.

--------------------------------------------------------------------------------------------
Usage: mp1 <gpio-pin-switch> <gpio-pin-LED1> <i2c bus> <i2c addr> <pwm-num1> <pwm-num2> <anin-pin>
--------------------------------------------------------------------------------------------

For example, the command "mp1 7 60 3 74 1 0 6" uses GPIO 7 for the switch, GPIO 60 for the LED, the I2C bus is 3, the I2C address is 74 (0x4a), the PWM LED is at ehrpwm.1:A, and the analog input is input 6 (ain6 in Linux)

Requires the pwmLib.c library. Compile with gcc: "gcc mp1.c pwmLib.c -o mp1"



** Requirements **

use at least 1 interupt driven gpio input (it could read a switch)
 - switch input is interrupt-driven
use one gpio output (it could blink an LED)
 - one LED is GPIO
use a PWM output (maybe blink or dim and LED)
 - one LED is PWM
use an i2c device (read a temperature?)
 - reads the temperature from I2C when the switch is pressed/depressed
use at least one analog in (read a voltage?)
 - reads an analog voltage from a potentiometer to control duty cycle
handle a ^C interrupt
 - stops the LED PWM and exits the program
