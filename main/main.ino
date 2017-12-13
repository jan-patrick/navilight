///////////////////////////////////////////////////////////////////////////////////////
//
//  main code of navilight
//  
//  authors: Tobias Haag, Jaqueline Lechner, Jan Schneider
//
//  date: 12.12.2017
//  
//  description: At current state the code prints out the variables of the MPU-9150.
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

// counter for every time we read out the MPU-9150
long readoutcounter = 0;

void setup(){

  Serial.begin(SERIAL_PORT_SPEED);
  Serial.print("navilight started using MPU-9150 device "); Serial.println(DEVICE_TO_USE);
  Wire.begin();
  MPU.selectDevice(DEVICE_TO_USE);                        // only really necessary if using device 1
  MPU.init(MPU_UPDATE_RATE, MPU_MAG_MIX_GYRO_AND_MAG, MAG_UPDATE_RATE, MPU_LPF_RATE);   // start the MPU

  // initialise LEDs
  pinMode(LED_MAIN, OUTPUT);
  pinMode(LED_LEFT, OUTPUT);
  pinMode(LED_RIGHT, OUTPUT);
}

void loop(){

  MPU.selectDevice(DEVICE_TO_USE);                         // make sure we use the right MPU and as control

  digitalWrite(LED_MAIN, LOW);                             // make sure the LEDs are off while not being used
  digitalWrite(LED_LEFT, LOW); 
  digitalWrite(LED_RIGHT, LOW); 

  if (MPU.read()) {                                        // get the latest data if ready
    readoutcounter++;
    String readouttext = "Readouts: ";
    String readouttextandvariable = readouttext + readoutcounter;
    Serial.println(readouttextandvariable); 
//  MPU.printQuaternion(MPU.m_rawQuaternion);              // print the raw quaternion from the dmp
//  MPU.printVector(MPU.m_rawMag);                         // print the raw mag data
//  MPU.printVector(MPU.m_rawAccel);                       // print the raw accel data
//  MPU.printAngles(MPU.m_dmpEulerPose);                   // the Euler angles from the dmp quaternion
//  MPU.printVector(MPU.m_calAccel);                       // print the calibrated accel data
//  MPU.printVector(MPU.m_calMag);                         // print the calibrated mag data
    MPU.printAngles(MPU.m_fusedEulerPose);                 // print the output of the data fusion
    Serial.println();
  }
}