include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_VL53L0X.h>
#include "bitmaps.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <UrlEncode.h>
// OLED display reset pin (-1 means sharing ESP32 reset pin)
#define OLED_RESET -1


// Create sensor and display objects
Adafruit_VL53L0X lox = Adafruit_VL53L0X();
Adafruit_SSD1306 display(128, 64, &Wire, OLED_RESET);


// int photoresistorPin = 35;  // ESP32 analog pin (34, 35, 36, 39 are ADC pins)


// RGB LED pins (use PWM-capable pins)
  int PIN_RED   = 32;
  int PIN_GREEN = 33;
  int PIN_BLUE  = 34;


// Laser pin
int laserPin = 36;


// PWM channels for RGB LED (ESP32 has 16 PWM channels: 0-15)
  const int PWM_CHANNEL_RED   = 0;
  const int PWM_CHANNEL_GREEN = 1;
  const int PWM_CHANNEL_BLUE  = 2;
  const int PWM_CHANNEL_LASER = 3;


// PWM settings
  const int pwmFreq = 5000;        // 5kHz frequency
  const int pwmResolution = 8;     // 8-bit resolution (0-255)


// Color intensity variables
  int R;
  int G;
  int B;


//wifi credentials
  char* ssid = "VT Open WiFi";


//API configuration with Callmebot
  String phone = "Enter Number Here";
  String apiKey = “Enter APIkey Here";
void setup() {
  // Start serial communication at 115200 baud for debugging
  Serial.begin(115200);
 
  wifiSetup();
  LaserSetup();
  setupRGB();
  oledSetup();
}


void loop() {
  oledRun();
  delay(200);
}


void setColor(int R, int G, int B) {
  // Write PWM values to RGB LED pins using ESP32 function
  ledcWrite(PIN_RED, R);
  ledcWrite(PIN_GREEN, G);
  ledcWrite(PIN_BLUE, B);
}


/*The function below beasiccaly connects us to the wifi(VT Open WiFi) to allow us to send
messages on whatsapp using a bot*/
void wifiSetup(){
  //begins connection attemp using credentials
  WiFi.begin(ssid);
  Serial.println("Connecting to WiFi");


  //this checks if the connections was succelful using the status
  //method which returns WL_CONNECTED if so
  while(WiFi.status() != WL_CONNECTED){
    Serial.print(".");
  }


  Serial.println("Connected to the WiFi network"); // Print wifi connect message


 
}


/*THe function below is actually used to send the message*/
void sendMessage(String message){
  //check if the wifi is still connected
  if(WiFi.status() == WL_CONNECTED){
    //creates an instance of the HTTPCLient class to handle
    //HTTP requests
    HTTPClient http;
    //API request URL is contructed using information above
    String url = "https://api.callmebot.com/whatsapp.php?phone=" + phone +
                 "&apikey=" + apiKey +
                 "&text=" + urlEncode(message);//urlEncode converts everything to url safe format


  //initializes HTTP connection with the contructed URL
  http.begin(url);


  //makes the actual request and stres the response cede(200=success,4xx=clienterror,..
  //5xx=server error)
  int httpResponseCode = http.GET();
  if(httpResponseCode == 200){
    Serial.println("Message sent successfully");
  }
  else {
    Serial.println("Error Sensding Message: ");
    Serial.println(httpResponseCode);
  }
  http.end();
  }
}


//RGB LED Setup
void setupRGB() {
  // Attach PWM channels to RGB LED pins for ESP32
  ledcAttach(PIN_RED,   pwmFreq, pwmResolution);
  ledcAttach(PIN_GREEN, pwmFreq, pwmResolution);
  ledcAttach(PIN_BLUE,  pwmFreq, pwmResolution);
}


/*Below we use functions from the AdaFruit library to configure and ensure
the laser sensor is connected to the I2c Bus*/
void LaserSetup() {
  // Configure PWM for laser pin on ESP32
  ledcAttach(laserPin, pwmFreq, pwmResolution);
 
  // Check if VL53L0X sensor is connected
  if (!lox.begin()) {
    Serial.println("Failed to find VL53L0X sensor!");
  } else {  
    Serial.println("VL53L0X ready!");
  }
}




/*Setting up the Oled Dispaly*/
void oledSetup() {
  // Initialize I2C communication (default ESP32 pins: SDA=21, SCL=22)
  Wire.begin();
 
  // Initialize OLED display with I2C address 0x3D
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
}


/*Writing to the Oled Dsplay*/
void oledRun() {
  //soapLevel stores a mapping for the laser reading from 150-18(Full-low) and 0-100%
  int soapLevel = map(LaserRun(),150,18,0,100);
  Serial.print(" Percentage: ");
  Serial.print(soapLevel);
  Serial.println("%");
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(2);
  display.setCursor(0, 25);
  display.print("Soap Level   ");
  display.print(soapLevel);
  display.println("%");
  display.display();
  /*Below, if the soap level is low, we call the sendMessage() function
  to send a message through whatsapp to notify the user.*/
  if(soapLevel == 96){
    sendMessage("!!You're Running out of Soap!!: " + String(soapLevel) + "%");
    //print to the serial monitor to confirm the message is sent
    Serial.print("!!You're Running out of Soap!!: ");
    Serial.print(soapLevel);
    Serial.println("%");
  }
}


/*The function below uses functions from the Adafruit Library to actually measures distance
using the laser sensor it prints out the distance to the serial monitor*/
int LaserRun() {
  // Create structure to store measurement data
  VL53L0X_RangingMeasurementData_t measure;
 
  // Perform distance measurement
  lox.rangingTest(&measure, false);
 
  // Extract distance in millimeters
  int distance = measure.RangeMilliMeter;
 
  // Map distance (0-2000mm) to brightness (0-255)
  // Closer object = brighter laser
  int brightness = map(distance, 0, 2000, 0, 255);
 
  // Check if measurement is valid (RangeStatus: 0=valid, 4=out of range)
  if (measure.RangeStatus != 4) {
    // Write PWM value to laser using ESP32 function
    ledcWrite(laserPin, brightness);
   
    // Print distance to Serial Monitor
    Serial.print("Distance (mm): ");
    Serial.print(distance);
   
    return distance;
  } else {
    Serial.println("Out of range");
    return -1;
  }
}
