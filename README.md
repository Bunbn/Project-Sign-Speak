# Project-Sign-Speak
Sign Speak is a wearable glove that will recognize sign language gestures and output the
corresponding letter and visual to a terminal display. This system architecture consists of two
main components: a wearable glove hardware interface and an output terminal display. The sign
language gestures will be measured using flex resistors attached to each finger on the glove.
These resistors serve as variable resistors that increase in resistance when they are bent. By
wiring the flex resistors in voltage division circuit configuration with normal resistors, we are
able to detect an increase in voltage when a finger is bent. All five finger voltage division
measurements will be inputted into a mux that is driven by an Arduino ESP8266. The mux
allows for the ESP8266 to cycle through and measure each finger voltage value all through a
single analog input pin. These individual finger values will then be packed into 16 bits of data
and sent wirelessly to a different ESP8266 through ESP-NOW. Wireless transmission of data is
an integral part of the Sign Speak design as it allows for freedom of user movement. After
receiving the transmitted data, the ESP8266 receiver will relay the 16 bits to the Nucleo board
using I2C. I2C effectively enables the transmission of data using two I/O pins. The Nucleo board
is responsible for unpacking, interpreting, and outputting the data to a terminal via LPUART. The
Nucleo also contains interrupt functions to detect changes in sign inputs and prints to the
terminal one character at a time if there is no input sign change in 2 seconds. The terminal will
print the letter signed and will show a visual real-time hand sign diagram matching each input. It
will also show previous letters signed so the user can form words. Overall, our wireless glove
interface enables seamless real-time communication between sign language users and
non-signers via a terminal display.
![image](https://github.com/Bunbn/Project-Sign-Speak/assets/75515757/7e0817a3-62c1-4c49-acc6-d9227b8a9ea6)

Most of the code is operated on interrupts, with the main.c file just waiting for interrupt flags to
go off before performing any processes. There are two main interrupt flags that the main looks
for, which are flags for when the I2C data transfer has been completed and another for when the
timer counter has overflowed. When the I2C data is ready, the nucleo manages the data transfer
flag and compares the new data to the previous set of data. If the finger data sets are different, the
timer counter is reset and the finger positions are updated on the serial terminal. When the timer
counter flag goes off, the current held character is printed to the serial terminal and the timer
counter flag is managed.
I2C data is received via interrupts, so the Nucleo can run all of the code in the main loop while
waiting for new I2C data to be received. When I2C data is received, the IQR event handler for
I2C1 goes off and bytes are received in a global variable until the data transfer is complete. Data
is transferred from MSB to LSB, so new transferred bytes are shifted into the I2C data variable,
making the I2C receiver work from anywhere from 1 to 4 Bytes (32 bits) of data.
The timer counter is used to interpret when a hand sign is held. When a sign is continuously held
for two seconds, the counter overflows, setting the timer overflow flag. A timer was used to
process new signs so that new inputs could be checked continuously and the timer can be reset
anytime before the two seconds are up if the hand input is changed. Due to the way the timer
code works, if the sign is held after the two seconds are up, nothing prints, meaning that the next
sign can be thought out without anything undesirable being printed to the terminal.

![Final System Schematic](https://github.com/Bunbn/Project-Sign-Speak/assets/75515757/a93e93fd-4498-46d4-8923-82825fb43b43)
