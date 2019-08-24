# Digital System Design
We have created the system using HCS12 microcontroller with C programming language. The system converts series 
of analog data to a digital data. We interpreted the data. I have created a Embedded/digital system that have five modes. 
# First mode
is to control the servo motor face angle with a potentiometer. Where the potentiometer was the input and the servo is the output.
# Second mode 
is to control the servo with the analog temperature sensor data of the room. This mode required Analog to Digital Converter (ADC). 
# Third mode 
is to control the servo using the light intensity of the room.
# Fourth mode 
is to be, an auto mode, where we can choose the input of the analog signal of our choice.
# Last mode 
is the Serial communication mode. So the input is coming from a Serial Communication Interface (SCI), We used putty to send commands. 
The system interpreted the command and executed to control the position of the servo.

In this project I have used many of the embedded system characteristics such as Timer module that incorporates input capture (This capability allowed me to measure the frequency, the period, and the duty cycle of an unknown signal ), output compare (allowed me 
to make a copy of the main timer, add a delay to this copy, and store the sum to an output compare register. This capability was 
used to create a delay, generate a digital waveform, trigger an action on a signal pin, and so on.), real-time interrupt, and 
counting capability or counters
• Pulse-width modulation function for easy waveform generation 
• Analog-to-digital converter 
• Temperature sensor 
• Serial I/O interface such as UART, SPI, I2C
