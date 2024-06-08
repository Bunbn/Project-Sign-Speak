/* -----------------------------------------------------------------------------
 * module   : send.ino
 * authors  : Ethan Robson - erobson@calpoly.edu
 *            Grayson Parker - gparke09@calpoly.edu
 *            Alain Kanadijan - aakandj@calpoly.edu
 *            ChatGPT
 * version  : 1
 * date     : 6/2/24
 *
 * Pin connections:
 * Vin- Vout battery pack
 * 3.3V- 3.3V Rail
 * GND- GND battery pack 
 * ADC0- MUX COM
 * GPIO16- MUX A
 * GPIO5- MUX B
 * GPIO4- MUX C
 * -------------------------------------------------------------------------- */
#include <ESP8266WiFi.h> //include ESP8266 library
#include <espnow.h>      //include ESP-NOW library

const int pinD2 = 4;  //sets pin num
const int pinD1 = 5;  //sets pin num
const int pinD0 = 16;  //sets pin num

uint8_t broadcastAddress[] = {0x44, 0x17, 0x93, 0x14, 0xE6, 0x82}; //receiver's MAC address
uint16_t packedValue;
uint16_t myValue;  //uint16_t variable that gets sent
unsigned long lastTime = 0;  //start time  
unsigned long timerDelay = 10;  //time interval between sends

/* -----------------------------------------------------------------------------
 * function : void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus)
 * INs      : mac_addr, sendStatus
 * OUTs     : serial print
 * action   : callback when data is sent
 * authors  : Ethan Robson - erobson@calpoly.edu
 *            Grayson Parker - gparke09@calpoly.edu
 *            Alain Kanadijan - aakandj@calpoly.edu
 * version  : 1
 * date     : 6/2/24
 * -------------------------------------------------------------------------- */
void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) {  
  Serial.print("Last Packet Send Status: ");
  if (sendStatus == 0) {  //check if data was sent
    Serial.println("Delivery success");
  } else {
    Serial.println("Delivery fail");  //error occurs
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
  WiFi.mode(WIFI_STA);  //set to operate as Wi-Fi station
  if (esp_now_init() != 0) {  //initalize ESP_NOW 
    Serial.println("Error initializing ESP-NOW");  //error occurs
    return;
  }
  esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);  //set role to controller
  esp_now_register_send_cb(OnDataSent);  //run callback function when data is sent
  esp_now_add_peer(broadcastAddress, ESP_NOW_ROLE_SLAVE, 1, NULL, 0);  //set role and address to peer
  pinMode(A0, INPUT);  //initialize ADC0 as an input
  pinMode(pinD2, OUTPUT);  // Set D2 as an output
  pinMode(pinD1, OUTPUT);  // Set D1 as an output
  pinMode(pinD0, OUTPUT);  // Set D0 as an output
}

/* -----------------------------------------------------------------------------
 * function : loop()
 * INs      : none
 * OUTs     : ESP_NOW output
 * action   : sends packed value containing finger num and analog value via ESP_NOW
 * authors  : Ethan Robson - erobson@calpoly.edu
 *            Grayson Parker - gparke09@calpoly.edu
 *            Alain Kanadijan - aakandj@calpoly.edu
 * version  : 1
 * date     : 6/2/24
 * -------------------------------------------------------------------------- */
void loop() {
  static uint8_t counter = 0;
  if ((millis() - lastTime) > timerDelay) {  //wait timer delay to send again
    uint8_t fingerNum = counter % 6;  // Generate a cycling finger number from 0 to 4
    counter = (counter + 1) % 6;  // Increment and reset the counter if it reaches 5
    // Set pin states based on fingerNum
    if (fingerNum !=4){  //skip pin 4
      digitalWrite(pinD2, (fingerNum & 0x4) >> 2); // Bit 2
      digitalWrite(pinD1, (fingerNum & 0x2) >> 1); // Bit 1
      digitalWrite(pinD0, fingerNum & 0x1);  // Bit 0
      //digitalWrite(pinD1, LOW); // Set D1 to high
      //digitalWrite(pinD0, LOW); // Set D0 to high
      //digitalWrite(pinD2, HIGH); // Set D1 to high
      delay(10);  //delay for chip
      uint16_t analogValue = analogRead(A0);  //read a 10-bit analog value
      //3 bits of random, 10 bits of analog reading, 3 bits of zero
      if (fingerNum ==5){  //using pin 5 instead of pin 4
        fingerNum =4;  //setting to appear as pin 4
      }
      packedValue = (fingerNum << 13) | (analogValue << 3);
      esp_now_send(broadcastAddress, (uint8_t *)&packedValue, sizeof(packedValue));  //send the packed uint16_t value to peer
      lastTime = millis();  //update send time

      // For debugging
      Serial.print("Finger Value: ");
      Serial.print(fingerNum);
      Serial.print(", Analog Value: ");
      Serial.print(analogValue);
      Serial.print(", Packed Value: ");
      Serial.println(packedValue, BIN);
      }
  }
}
