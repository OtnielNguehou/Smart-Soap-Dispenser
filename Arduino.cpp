 #include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_VL53L0X.h>


// #define OLED_RESET -1  // Reset pin # (or -1 if sharing Arduino reset pin)


int photoresistorPin = A0;


int pump = 3;
// Adafruit_VL53L0X lox = Adafruit_VL53L0X();


// Adafruit_SSD1306 display(128, 64, &Wire, OLED_RESET);




//we create the pins to connect the legs of the LED to
int PIN_RED   = 9;
int PIN_GREEN = 10;
int PIN_BLUE  = 11;


//variables to hold our color intensities
int R;
int G;
int B;


void setup() {
  Serial.begin(115200);//Begins serial communication that allows you to print in serial monitor
  setupRGB();
  pinMode(pump,OUTPUT);
}


void loop() {
  // oledRun();
  while(PhotoResistorRun() == 1){
    digitalWrite(pump,255/2);
    delay(500);
    digitalWrite(pump,0);
    delay(3000);


   
  }
 


}


int PhotoResistorRun(){
  //the value read from analog A0
  int photoresistorVal = analogRead(photoresistorPin);
  //this maps that value to the pwm range of 0-255(used for fading led)
  int brightness = map(photoresistorVal, 0, 1023, 0, 255);
  //keeping track of variables and for debugging
  Serial.print("A0 reads: ");
  Serial.print(photoresistorVal);
  Serial.print(" Brightness: ");
  Serial.println(brightness);


  //if the value read from the analog A0 is less than the threshold(60)
  //then there's something blocking light
  if(photoresistorVal > 800){
    setColor(0, 255, 0);//set LED to green to indicate something's blocking light
    return 1;


  }
  //else if nothing is blocking light
  else{
    setColor(255, 255, 0);//set the LED to red
    return -1;
  }
 
}
//setting up the RGB LED pins
void setupRGB(){
  //set all three pins to output mode
  pinMode(PIN_RED,   OUTPUT);
  pinMode(PIN_GREEN, OUTPUT);
  pinMode(PIN_BLUE,  OUTPUT);
}
// //this function writes values to the rgb pins
void setColor(int R, int G, int B) {
  analogWrite(PIN_RED,   R);
  analogWrite(PIN_GREEN, G);
  analogWrite(PIN_BLUE,  B);
}


