#include <Mouse.h>
#include<Wire.h>

int BUTTON_PIN = 11;

bool buttonResetted = false;

int button_set_time = 10000;
int button_click_min = 500;

unsigned long buttonSetDelay;
unsigned long buttonClickDelay;

int buttonClickIteration = 0;
int buttonResetIteration = 0;

const int MPU=0x68; 
float AccX, AccY, AccZ;
float GyroX, GyroY, GyroZ;
float accAngleX, accAngleY, gyroAngleX, gyroAngleY, gyroAngleZ;
float roll, pitch, yaw;
float AccErrorX, AccErrorY, GyroErrorX, GyroErrorY, GyroErrorZ;
float elapsedTime, currentTime, previousTime;
int c = 0;

float prevRoll;
float prevPitch;
float prevYaw;

int rollMovementDiff = 1;
int pitchMovementDiff = 1;
int yawMovementDiff = 1;

void setup() {
  //push button
  
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  Mouse.begin();

  Wire.begin();
  Wire.beginTransmission(MPU);
  Wire.write(0x6B); 
  Wire.write(0);    
  Wire.endTransmission(true);
  Serial.begin(9600);
}

void loop() {
  // === Read acceleromter data === //
  Wire.beginTransmission(MPU);
  Wire.write(0x3B); // Start with register 0x3B (ACCEL_XOUT_H)
  Wire.endTransmission(false);
  Wire.requestFrom(MPU, 6, true); // Read 6 registers total, each axis value is stored in 2 registers
  //For a range of +-2g, we need to divide the raw values by 16384, according to the datasheet
  AccX = (Wire.read() << 8 | Wire.read()) / 16384.0; // X-axis value
  AccY = (Wire.read() << 8 | Wire.read()) / 16384.0; // Y-axis value
  AccZ = (Wire.read() << 8 | Wire.read()) / 16384.0; // Z-axis value
  // Calculating Roll and Pitch from the accelerometer data
  accAngleX = (atan(AccY / sqrt(pow(AccX, 2) + pow(AccZ, 2))) * 180 / PI) - 0.58; // AccErrorX ~(0.58) See the calculate_IMU_error()custom function for more details
  accAngleY = (atan(-1 * AccX / sqrt(pow(AccY, 2) + pow(AccZ, 2))) * 180 / PI) + 1.58; // AccErrorY ~(-1.58)
  
  // === Read gyroscope data === //
  previousTime = currentTime;        // Previous time is stored before the actual time read
  currentTime = millis();            // Current time actual time read
  elapsedTime = (currentTime - previousTime) / 1000; // Divide by 1000 to get seconds
  Wire.beginTransmission(MPU);
  Wire.write(0x43); // Gyro data first register address 0x43
  Wire.endTransmission(false);
  Wire.requestFrom(MPU, 6, true); // Read 4 registers total, each axis value is stored in 2 registers
  GyroX = (Wire.read() << 8 | Wire.read()) / 131.0; // For a 250deg/s range we have to divide first the raw value by 131.0, according to the datasheet
  GyroY = (Wire.read() << 8 | Wire.read()) / 131.0;
  GyroZ = (Wire.read() << 8 | Wire.read()) / 131.0;
  
  // Correct the outputs with the calculated error values
  GyroX = GyroX + 0.56; // GyroErrorX ~(-0.56)
  GyroY = GyroY - 2; // GyroErrorY ~(2)
  GyroZ = GyroZ + 0.79; // GyroErrorZ ~ (-0.8)
  
  // Currently the raw values are in degrees per seconds, deg/s, so we need to multiply by sendonds (s) to get the angle in degrees
  gyroAngleX = gyroAngleX + GyroX * elapsedTime; // deg/s * s = deg
  gyroAngleY = gyroAngleY + GyroY * elapsedTime;
  yaw =  yaw + GyroZ * elapsedTime;
  // Complementary filter - combine acceleromter and gyro angle values
  roll = 0.96 * gyroAngleX + 0.04 * accAngleX;
  pitch = 0.96 * gyroAngleY + 0.04 * accAngleY;
  
  // Print the values on the serial monitor
  Serial.print(roll); //rotate up down
  Serial.print("/");
  Serial.print(pitch); //handle left and right
  Serial.print("/");
  Serial.println(yaw); //point left and right

  if(prevRoll == NULL){
    prevRoll = roll;
  }
  if(prevPitch == NULL){
    prevPitch = pitch;
  }
  if(prevYaw == NULL){
    prevYaw = yaw;
  }

  if(prevRoll != roll){
    if(prevRoll > roll){
      Mouse.move(0, -rollMovementDiff, 0); 
    }
    else{
      Mouse.move(0, rollMovementDiff, 0); 
    }
  }

  /*
  int buttonState;
  buttonState = !(digitalRead(BUTTON_PIN));
  
  if ((millis() - buttonClickDelay) >= button_click_min){
    
    buttonClickDelay = millis();
    
    if(buttonState){
      buttonClickIteration++;
    }
  }

  if ((millis() - buttonSetDelay) >= button_set_time){
    buttonSetDelay = millis();

    if(buttonState){
      buttonResetIteration++;
      //Serial.println(buttonSetDelay);
    }
  }

  if(buttonClickIteration > 5){
    buttonClickIteration = 0;
    Serial.println("button click");
  }

  if(buttonResetIteration > 0){
    buttonResetIteration = 0;
    buttonResetted = true;
    Serial.println("button reset");
  }*/
}
