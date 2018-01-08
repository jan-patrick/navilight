////////////////////////////////////////////////////////////////////////////////////
//
//  main code of navilight
//  
//  authors: Tobias Haag, Jaqueline Lechner, Jan Schneider
//
//  date: 08.01.2018
//  
//  description: At current state the code prints out the variables of the MPU-9150. 
//               The LEDs are basically implemented.
//
//  source: https://github.com/jan-patrick/navilight
//  
//  devices:
//  - Arduino Mega
//  - 3 LEDs (+resistors)
//
//  PINs:
//
//  - 2             LED_LEFT
//  - 3             LED_MAIN
//  - 4             LED_RIGHT
//

//  SERIAL_PORT_SPEED defines the speed to use for the debug serial port
#define  SERIAL_PORT_SPEED  115200

// set the LED-PINs
const int LED_MAIN =  3;                                    // constants won't change
const int LED_LEFT =  2;
const int LED_RIGHT =  4;

const int LEDON = 30;                                      // 150 for testing without resistors
const int LEDOFF = 0;

// status of navilight (0 = navigation off but device is on, 1 = start, 2 = stop, 3 = straight, 4 = left, 5 = right, 6 = false, 7 = warning, 8 = all off)
int status = 0;
int started = false;

// time variables
unsigned long currentMillis = 0; 
unsigned long previousMillisStart = 0;
unsigned long previousMillisFadeWarning = 0;
unsigned long previousMillisFadeStop = 0;
unsigned long previousMillisFadeFalse = 0;

// intervals
const long startInterval = 200;
const long fadeWarningInterval = 20;
const long fadeStopInterval = 100;
const long fadeFalseInterval = 75;

// fade variables
int ledBrightness = 0;
int ledBrightnessSteps = 5;
String ledFadeState = "GROW";

void setup(){

  Serial.begin(SERIAL_PORT_SPEED);
  Serial.println("navilight started");

  // initialise LEDs
  pinMode(LED_MAIN, OUTPUT);
  pinMode(LED_LEFT, OUTPUT);
  pinMode(LED_RIGHT, OUTPUT);
}

void loop(){
// Serial.print("hi");

  currentMillis = millis();                                 // setting current time in milliseconds
 
  // setting status of navilight via serial
  if (Serial.available() > 0){
    status = Serial.parseInt();
    Serial.print("I am in status: ");
    Serial.println(status, DEC);
  }

  switch (status){
    // navigation off but device is on
    case 0:
      ledControl(LEDON, LEDOFF, LEDOFF);
      break;
    // start  
    case 1:
      ledControl(LEDON, LEDON, LEDON);
      //if (currentMillis - previousMillisStart >= startInterval) {
      //  previousMillisStart = currentMillis;                // save the last time the LED state was changed
      //  ledControl(LEDON, LEDOFF, LEDOFF);
      //  status = 3;
      //  Serial.println("I am in status: 3");
      //  Serial.println("Started navigation, please start moving."); 
      //}  
      break;
    // stop
    case 2:
      ledBrightnessSteps = 2;
      if(currentMillis - previousMillisFadeStop >= fadeStopInterval) {
        previousMillisFadeStop = currentMillis;   
        ledAllFade();
      }
      break;
    // straight
    case 3:
      ledControl(LEDON, LEDOFF, LEDOFF);
      break;
    // left
    case 4:
      ledControl(LEDON, LEDON, LEDOFF);
      break;
    // right
    case 5:
      ledControl(LEDON, LEDOFF, LEDON);
      break;
    // false
    case 6:
      if(currentMillis - previousMillisFadeFalse >= fadeFalseInterval) {
        previousMillisFadeFalse = currentMillis;   
        ledLeftRightFade();
      }
      break; 
    // warning
    case 7:
      if(currentMillis - previousMillisFadeWarning >= fadeWarningInterval) {
        previousMillisFadeWarning = currentMillis;   
        ledLeftRightFade();
      }
      break;
    // all off by default           
    default:
      ledControl(LEDOFF, LEDOFF, LEDOFF);                   // make sure the LEDs are off while device is "off"
      break;    
  }
}

// functions for all LED cases (every LED that is not ON / HIGH in function name is off / LOW)
void ledControl(int ledMainStatus, int ledLeftStatus, int ledRightStatus){
  analogWrite(LED_MAIN, ledMainStatus);
  analogWrite(LED_LEFT, ledLeftStatus); 
  analogWrite(LED_RIGHT, ledRightStatus);
}

void ledLeftRightFade(){
  if (ledFadeState == "GROW") {
    ledBrightness += ledBrightnessSteps;
    if (ledBrightness>=LEDON-ledBrightnessSteps) {
      ledFadeState = "SHRINK";
    }
  }else if (ledFadeState == "SHRINK") {
    ledBrightness -= ledBrightnessSteps;
    if (ledBrightness<=ledBrightnessSteps) {
      ledFadeState = "GROW";
    }
  }
  analogWrite(LED_MAIN, LEDON);
  analogWrite(LED_LEFT, ledBrightness); 
  analogWrite(LED_RIGHT, ledBrightness);
}

void ledAllFade(){
  if (ledFadeState == "GROW") {
    ledBrightness += ledBrightnessSteps;
    if (ledBrightness>=LEDON-ledBrightnessSteps) {
      ledFadeState = "SHRINK";
    }
  }else if (ledFadeState == "SHRINK") {
    ledBrightness -= ledBrightnessSteps;
    if (ledBrightness<=LEDON/4) {
      ledFadeState = "GROW";
    }
  }
  analogWrite(LED_MAIN, ledBrightness);
  analogWrite(LED_LEFT, ledBrightness); 
  analogWrite(LED_RIGHT, ledBrightness);
}
