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

