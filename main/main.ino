////////////////////////////////////////////////////////////////////////////////////
//
//  main code of navilight
//  
//  authors: Tobias Haag, Jaqueline Lechner, Jan Schneider
//
//  date: 12.12.2017
//  
//  description: At current state the code prints out the variables of the MPU-9150. 
//               The LEDs are basically implemented.
//
//  source: https://github.com/JanPSchneider/navilight
//  
//  devices:
//  - Arduino Mega
//  - MPU-9150
//  - 3 LEDs (+resistors)
//
//  PINs:
//    Arduino Mega  MPU-9150
//  - 3.3V          VCC
//  - GND           GND 
//  - SDA           SDA
//  - SCL           SCL
//
//  - 2             LED_LEFT
//  - 3             LED_MAIN
//  - 4             LED_RIGHT
//


#include <Wire.h>
#include "I2Cdev.h"
#include "MPU9150Lib.h"
#include "CalLib.h"
#include <dmpKey.h>
#include <dmpmap.h>
#include <inv_mpu.h>
#include <inv_mpu_dmp_motion_driver.h>
#include <EEPROM.h>

//  DEVICE_TO_USE selects whether the IMU at address 0x68 (default) or 0x69 is used
//    0 = use the device at 0x68
//    1 = use the device at ox69
#define  DEVICE_TO_USE    0

MPU9150Lib MPU;                                             // the MPU object

//  MPU_UPDATE_RATE defines the rate (in Hz) at which the MPU updates the sensor data and DMP output
#define MPU_UPDATE_RATE  (20)

//  MAG_UPDATE_RATE defines the rate (in Hz) at which the MPU updates the magnetometer data
//  MAG_UPDATE_RATE should be less than or equal to the MPU_UPDATE_RATE
#define MAG_UPDATE_RATE  (10)

//  MPU_MAG_MIX defines the influence that the magnetometer has on the yaw output.
//  The magnetometer itself is quite noisy so some mixing with the gyro yaw can help
//  significantly. Some example values are defined below:
#define  MPU_MAG_MIX_GYRO_ONLY          0                   // just use gyro yaw
#define  MPU_MAG_MIX_MAG_ONLY           1                   // just use magnetometer and no gyro yaw
#define  MPU_MAG_MIX_GYRO_AND_MAG       10                  // a good mix value 
#define  MPU_MAG_MIX_GYRO_AND_SOME_MAG  50                  // mainly gyros with a bit of mag correction 

//  MPU_LPF_RATE is the low pas filter rate and can be between 5 and 188Hz
#define MPU_LPF_RATE   40

//  SERIAL_PORT_SPEED defines the speed to use for the debug serial port
#define  SERIAL_PORT_SPEED  115200

// set the LED-PINs
const int LED_MAIN =  3;                            // constants won't change
const int LED_LEFT =  2;
const int LED_RIGHT =  4;

// status of navilight (0 = navigation off but device is on, 1 = start, 2 = stop, 3 = straight, 4 = left, 5 = right, 6 = false, 7 = warning)
int status = 0;

// counter for every time we read out the MPU-9150
long readoutcounter = 0;

// time variables
unsigned long currentMillis = 0; 
unsigned long previousMillisStart = 0;
unsigned long previousMillisFadeWarning = 0;
unsigned long previousMillisFadeStop = 0;
unsigned long previousMillisFadeFalse = 0;

// intervals
const long startInterval = 500;
const long fadeWarningInterval = 20;
const long fadeStopInterval = 100;
const long fadeFalseInterval = 50;

// fade variables
int ledBrightness = 0;
int ledBrightnessSteps = 5;
String ledFadeState = "GROW";

void setup(){

  Serial.begin(SERIAL_PORT_SPEED);
  Serial.print("navilight started using MPU-9150 device "); Serial.println(DEVICE_TO_USE);
  Wire.begin();
  MPU.selectDevice(DEVICE_TO_USE);                          // only really necessary if using device 1
  MPU.init(MPU_UPDATE_RATE, MPU_MAG_MIX_GYRO_AND_MAG, MAG_UPDATE_RATE, MPU_LPF_RATE);   // start the MPU

  // initialise LEDs
  pinMode(LED_MAIN, OUTPUT);
  pinMode(LED_LEFT, OUTPUT);
  pinMode(LED_RIGHT, OUTPUT);
}

void loop(){

  currentMillis = millis();                                 // setting current time in milliseconds

  MPU.selectDevice(DEVICE_TO_USE);                          // make sure we use the right MPU and as control
 
  // setting status of navilight via serial
  if (Serial.available() > 0){
    status = Serial.parseInt();
    Serial.print("I am in status: ");
    Serial.println(status, DEC);
  }

  switch (status){
    // navigation off but device is on
    case 0:
      ledMainOn();
      break;
    // start  
    case 1:
      if (currentMillis - previousMillisStart >= startInterval) {
        previousMillisStart = currentMillis;                // save the last time the LED state was changed
        if(LED_LEFT==LOW){
          ledAllOn();
        }else if(LED_LEFT==HIGH){
          ledMainOn();
          status = 3;
          Serial.println("I am in status: 3");
          Serial.println("Started navigation, please start moving.");
        }  
      }  
      break;
    // stop
    case 2:
      if(currentMillis - previousMillisFadeStop >= fadeStopInterval) {
        previousMillisFadeStop = currentMillis;   
        ledAllFade();
      }
      break;
    // straight
    case 3:
      ledMainOn();
      break;
    // left
    case 4:
      ledMainLelftOn();
      break;
    // right
    case 5:
      ledMainRightOn();
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
    default:
      ledAllOff();                                          // make sure the LEDs are off while device is "off"
      break;    
  }

  if (MPU.read()) {                                         // get the latest data if ready
    readoutMPU();
  }
}



///////////////
// functions //
///////////////

// function to read out the MPU-9150
void readoutMPU() {
  readoutcounter++;
  String readouttext = "Readouts: ";
  String readouttextandvariable = readouttext + readoutcounter;
  Serial.println(readouttextandvariable); 
//  MPU.printQuaternion(MPU.m_rawQuaternion);               // print the raw quaternion from the dmp
//  MPU.printVector(MPU.m_rawMag);                          // print the raw mag data
//  MPU.printVector(MPU.m_rawAccel);                        // print the raw accel data
//  MPU.printAngles(MPU.m_dmpEulerPose);                    // the Euler angles from the dmp quaternion
//  MPU.printVector(MPU.m_calAccel);                        // print the calibrated accel data
//  MPU.printVector(MPU.m_calMag);                          // print the calibrated mag data
  MPU.printAngles(MPU.m_fusedEulerPose);                  // print the output of the data fusion
  Serial.println();
}

// functions for all LED cases (every LED that is not ON / HIGH in function name is off / LOW)
void ledAllOff(){
  digitalWrite(LED_MAIN, LOW);
  digitalWrite(LED_LEFT, LOW); 
  digitalWrite(LED_RIGHT, LOW);
}

void ledAllOn(){
  digitalWrite(LED_MAIN, HIGH);
  digitalWrite(LED_LEFT, HIGH); 
  digitalWrite(LED_RIGHT, HIGH);
}

void ledMainOn(){
  digitalWrite(LED_MAIN, HIGH);
  digitalWrite(LED_LEFT, LOW); 
  digitalWrite(LED_RIGHT, LOW);
}

void ledMainLelftOn(){
  digitalWrite(LED_MAIN, HIGH);
  digitalWrite(LED_LEFT, HIGH); 
  digitalWrite(LED_RIGHT, LOW);
}

void ledMainRightOn(){
  digitalWrite(LED_MAIN, HIGH);
  digitalWrite(LED_LEFT, LOW); 
  digitalWrite(LED_RIGHT, HIGH);
}

void ledLeftRightOn(){
  digitalWrite(LED_MAIN, LOW);
  digitalWrite(LED_LEFT, HIGH); 
  digitalWrite(LED_RIGHT, HIGH);
}

void ledLeftRightFade(){
  if (ledFadeState == "GROW") {
    ledBrightness += ledBrightnessSteps;
    if (ledBrightness>=255-ledBrightnessSteps) {
      ledFadeState = "SHRINK";
    }
  }else if (ledFadeState == "SHRINK") {
    ledBrightness -= ledBrightnessSteps;
    if (ledBrightness<=ledBrightnessSteps) {
      ledFadeState = "GROW";
    }
  }
  digitalWrite(LED_MAIN, HIGH);
  digitalWrite(LED_LEFT, ledBrightness); 
  digitalWrite(LED_RIGHT, ledBrightness);
}

void ledAllFade(){
  if (ledFadeState == "GROW") {
    ledBrightness += ledBrightnessSteps;
    if (ledBrightness>=255-ledBrightnessSteps) {
      ledFadeState = "SHRINK";
    }
  }else if (ledFadeState == "SHRINK") {
    ledBrightness -= ledBrightnessSteps;
    if (ledBrightness<=ledBrightnessSteps) {
      ledFadeState = "GROW";
    }
  }
  digitalWrite(LED_MAIN, HIGH);
  digitalWrite(LED_LEFT, ledBrightness); 
  digitalWrite(LED_RIGHT, ledBrightness);
}