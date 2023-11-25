# PIC18F66K90-code
A collection of my early and basic test code for this higher end 8 bit PIC. 
Just a basis for development...

After coding for the TQFP64 PIC 18F6410 I wanted to try the more modern and powerful 18F66K90. At first glance the pinout
looks similar to the older 64 pin TQFP chip, though there are two important differences. RF0/AN5 has been moved and pin 18 is
now ENVREG controlling the onboard voltage regulator. It must be tied to Vdd to enable 5V operation. Pin 10 has become
VDDCORE/VCAP and for 3.3V operation is connected to Vdd. For 5V supply it is connected only to a 10uF capacitor when using 
the on chip regulator and isolated from Vdd. I modified my 18F6410 TQFP PCB to allow ENVREG to be connected to Vss or Vdd
and pin 10 to be isolated from Vdd if required via solder jumpers, allowing 3.3 or 5V configuration. 

There are other important differences from the older chip. PORTG pins are limited to 2mA, beware directly connected LEDs, I
used high brightness types via 2k on my board. Code for older 18F types is likely to need modification, my examples show some
18F66K90 setup code. PORTG defaults to analogue inputs, for digital input recongiguration is needed. There are important differences
regarding ADC operation including timing setup, voltage reference selection and configuration registers, see my ADC example code.

My first code for the chip was HelloWorld.c, a hello was achieved using basic code as for the 18F6410. 
