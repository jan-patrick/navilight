////////////////////////////////////////////////////////////////////////////////////
//
//  main code of navilight
//  
//  authors: Tobias Haag, Jaqueline Lechner, Jan Schneider
//
//  date: 08.01.2018
//  
//  description: This code shows all states navilight can show.
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

const int LEDON = 20;                                      // 150 for testing without resistors
const int LEDOFF = 0;

// status of navilight (0 = navigation off but device is on, 1 = start, 2 = stop, 3 = straight, 4 = left, 5 = right, 6 = false, 7 = warning, 8 = batterie warning, everything else = all off)
int status = 0;
int started = 0;

// time variables
unsigned long currentMillis = 0; 
unsigned long previousMillisStart = 0;
unsigned long previousMillisFadeWarning = 0;
unsigned long previousMillisFadeStop = 0;
unsigned long previousMillisFadeFalse = 0;
unsigned long previousMillisBatterie = 0;
unsigned long previousMillisFadeLeft = 0;
unsigned long previousMillisFadeRight = 0;

// intervals
const long startInterval = 2000;
const long fadeWarningInterval = 175;
const long fadeStopInterval = 700;
const long fadeFalseInterval = 200;
const long fadeBatterieInterval = 200;
const long fadeLeftInterval = 500;
const long fadeRightInterval = fadeLeftInterval;

// fade variables
int ledBrightness = 0;
int ledLeftBrightness = ledBrightness;
int ledRightBrightness = ledBrightness;
int ledMainBatterieBrightness = LEDON;
int ledBrightnessSteps = 2;
int ledMainBatterieBrightnessSteps = ledBrightnessSteps;
int ledLeftBrightnessSteps = 1;
int ledRightBrightnessSteps = ledLeftBrightnessSteps;
String ledFadeState = "GROW";
String ledMainBatterieFadeState = "SHRINK";
int ledMainBatterieFadecounter = 0; 

void setup(){
  Serial.begin(SERIAL_PORT_SPEED);
  Serial.println("navilight started");

  // initialise LEDs
  pinMode(LED_MAIN, OUTPUT);
  pinMode(LED_LEFT, OUTPUT);
  pinMode(LED_RIGHT, OUTPUT);
}

void loop(){
//Serial.print("hi");

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
      if (currentMillis - previousMillisStart >= startInterval) {
        previousMillisStart = currentMillis;                // save the last time the LED state was changed
        started++;
        if(started>=2){
          started = 0;
          status = 3;
        }else{
          ledControl(LEDON, LEDON, LEDON);
        }                   
      }
      break;
    // stop
    case 2:
      ledBrightnessSteps = 2;
      if(currentMillis - previousMillisFadeStop >= fadeStopInterval) {
        previousMillisFadeStop = currentMillis;   
        ledLeftRightBlinkAsyn();
      }
      break;
    // straight
    case 3:
      ledControl(LEDON, LEDOFF, LEDOFF);
      break;
    // left
    case 4:
      if(currentMillis - previousMillisFadeLeft >= fadeLeftInterval) {
        previousMillisFadeLeft = currentMillis;
        ledLeftFade();
      }  
      break;
    // right
    case 5:
      if(currentMillis - previousMillisFadeRight >= fadeRightInterval) {
        previousMillisFadeRight = currentMillis;
        ledRightFade();
      }
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
        ledLeftRightBlink();
      }
      break;
    // low batterie  
    case 8:
      if(currentMillis - previousMillisBatterie >= fadeBatterieInterval) {
        previousMillisBatterie = currentMillis;   
        ledMainFade();
      }
      break;
    // letting left glow  
    case 44:
      ledControl(LEDON, LEDON, LEDOFF);
      break;  
    // letting right glow  
    case 55:
      ledControl(LEDON, LEDOFF, LEDON);
      break;  
    // all off by default and like status 42         
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
    ledBrightness = LEDON;
    ledFadeState = "SHRINK";
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

void ledLeftRightBlink(){
  analogWrite(LED_MAIN, LEDON);
  if (ledFadeState == "GROW") {
    analogWrite(LED_LEFT, LEDON); 
    analogWrite(LED_RIGHT, LEDON);
    ledFadeState = "SHRINK";
  }else if (ledFadeState == "SHRINK") {
    analogWrite(LED_LEFT, LEDOFF); 
    analogWrite(LED_RIGHT, LEDOFF);
    ledFadeState = "GROW";
  }
}

void ledLeftRightBlinkAsyn(){
  analogWrite(LED_MAIN, LEDON);
  if (ledFadeState == "GROW") {
    analogWrite(LED_LEFT, LEDON); 
    analogWrite(LED_RIGHT, LEDOFF);
    ledFadeState = "SHRINK";
  }else if (ledFadeState == "SHRINK") {
    analogWrite(LED_LEFT, LEDOFF); 
    analogWrite(LED_RIGHT, LEDON);
    ledFadeState = "GROW";
  }
}

void ledMainFade(){
  analogWrite(LED_LEFT, LEDOFF); 
  analogWrite(LED_RIGHT, LEDOFF);
  if(ledMainBatterieFadecounter >= 3){
    analogWrite(LED_MAIN, LEDOFF);
    ledMainBatterieFadecounter = 0;
    status = 42;
  }else if (ledMainBatterieFadeState == "GROW") {
    ledMainBatterieBrightness = LEDON;
    ledMainBatterieFadeState = "SHRINK";
    analogWrite(LED_MAIN, ledMainBatterieBrightness);
  }else if (ledMainBatterieFadeState == "SHRINK") {
    ledMainBatterieBrightness -= ledMainBatterieBrightnessSteps;
    analogWrite(LED_MAIN, ledMainBatterieBrightness);
    if (ledMainBatterieBrightness<=0) {
      ledMainBatterieFadeState = "GROW";
      ledMainBatterieFadecounter++;
    }
  }
}

void ledLeftFade(){
  analogWrite(LED_RIGHT, LEDOFF);
  analogWrite(LED_MAIN, LEDON);
  ledLeftBrightness += ledLeftBrightnessSteps;
  analogWrite(LED_LEFT, ledLeftBrightness);
  if (ledLeftBrightness>=LEDON-ledLeftBrightnessSteps) {
    ledLeftBrightness = 0;
    status = 44;
  }
}

void ledRightFade(){
  analogWrite(LED_LEFT, LEDOFF);
  analogWrite(LED_MAIN, LEDON);
  ledRightBrightness += ledRightBrightnessSteps;
  analogWrite(LED_RIGHT, ledRightBrightness);
  if (ledRightBrightness>=LEDON-ledRightBrightnessSteps) {
    ledRightBrightness = 0;
    status = 55;
  }
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
