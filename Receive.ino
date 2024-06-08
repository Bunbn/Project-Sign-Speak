/* -----------------------------------------------------------------------------
 * module   : receive.ino
 * author   : Ethan Robson - erobson@calpoly.edu
 *            Grayson Parker - gparke09@calpoly.edu
 *            Alain Kanadijan - aakandj@calpoly.edu
 *            ChatGPT
 * version  : 1
 * date     : 6/2/24
 *
 * Pin connections:
 * 3V- 3V 
 * GND- GND  
 * GPIO4- PB9
 * GPIO5- PB8
 * -------------------------------------------------------------------------- */
 
#include <ESP8266WiFi.h>  //include ESP8266 libary
#include <espnow.h>  //include ESP-NOW libary 
#include <Wire.h>

uint16_t receivedValue;  //uint16_t variable for recieved value
uint8_t test[2]; // init test Bytes
uint8_t temp = 0;
uint8_t status;

/* -----------------------------------------------------------------------------
 * function : OnDataRecv(uint8_t * mac, uint8_t *incomingData, uint8_t len)
 * INs      : mac, incomingData, len
 * OUTs     : serial print
 * action   : callback when data is recieved and transmit data 
              through I2C to NUCLEO
 * authors  : Ethan Robson - erobson@calpoly.edu
 *            Grayson Parker - gparke09@calpoly.edu
 *            Alain Kanadijan - aakandj@calpoly.edu
 * version  : 1
 * date     : 6/2/24
 * -------------------------------------------------------------------------- */
void OnDataRecv(uint8_t * mac, uint8_t *incomingData, uint8_t len) { 
    if (len == sizeof(uint16_t)) {  //check the size of data 
        memcpy(&receivedValue, incomingData, sizeof(receivedValue));  //copy recieved data to receivedValue   
        // Extract randomBits and analogValue
        uint8_t fingerNum = receivedValue >> 13;  //shift right by 13 bits to get the top 3 bits
        uint16_t analogValue = (receivedValue >> 3) & 0x03FF;  //shift right by 3 bits and mask to get the next 10 bits
        // Print extracted values       
        Serial.print("Received uint16_t value: ");
        Serial.println(receivedValue, BIN);  //outputs the received uint16_t value
        Serial.print("Finger Value: ");
        Serial.print(fingerNum);  //print finger number
        Serial.print(" Analog Value: ");
        Serial.println(analogValue);  //print analog value  

        // store 16 bits into 2 Bytes and send via I2C
        test[0] = (receivedValue>>8) & 0xFF; // high byte
        test[1] = receivedValue & 0xFF; // low byte
        Wire.beginTransmission(0x11); // transmit to device number 0x11
        Wire.write(test, 2); // write the test val
        status = Wire.endTransmission(); // stop transmitting 
        Serial.print("I2C Transmission Status: ");
        Serial.println(status);
    } else {
        Serial.println("Received data size does not match uint16_t size!");  //error occurs
    }
}

/* -----------------------------------------------------------------------------
 * function : setup()
 * INs      : none
 * OUTs     : serial print
 * action   : sets up ESP_NOW connection
 * authors  : Ethan Robson - erobson@calpoly.edu
 *            Grayson Parker - gparke09@calpoly.edu
 *            Alain Kanadijan - aakandj@calpoly.edu
 * version  : 1
 * date     : 6/2/24
 * -------------------------------------------------------------------------- */
void setup() {
    Serial.begin(115200);  //set to 115200 baud rate
    Serial.println(WiFi.macAddress());
    WiFi.mode(WIFI_STA);  //set to operate as a Wi-Fi station
    if (esp_now_init() != 0) {  //initalize ESP_NOW
        Serial.println("Error initializing ESP-NOW");  //error occurs
        return;
    } 
    esp_now_set_self_role(ESP_NOW_ROLE_SLAVE);  //set role to slave
    esp_now_register_recv_cb(OnDataRecv);  //run callback function when data is recieved 

    // init I2C bus
    Wire.begin(); // join the I2C bus as a controller
    Wire.setClock(100000);
}

void loop() {
}