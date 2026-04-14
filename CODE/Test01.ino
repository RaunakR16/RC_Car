#include <SoftwareSerial.h>

// ---- Bluetooth HC-05 ----
SoftwareSerial Bluetooth(0, 1);  // RX, TX

// ---- Motor 1 (Left Front) ----
#define M1_EN   3
#define M1_IN1  4
#define M1_IN2  5

// ---- Motor 2 (Left Rear) ----
#define M2_EN   6
#define M2_IN3  7
#define M2_IN4  8

// ---- Motor 3 (Right Front) ----
#define M3_EN   9
#define M3_IN1  11
#define M3_IN2  A0

// ---- Motor 4 (Right Rear) ----
#define M4_EN   10
#define M4_IN3  A1
#define M4_IN4  2

// ---- Lights ----
#define HEADLIGHTS    A2
#define LEFT_IND      A3
#define RIGHT_IND     A4
#define BRAKE_LIGHTS  A5

// ---- Horn ----
#define HORN  12

//------ PWM----
int speed = 100;

void setup() 
{
  Serial.begin(9600);
  Bluetooth.begin(9600);

  pinMode(M1_EN, OUTPUT); 
  pinMode(M1_IN1, OUTPUT); 
  pinMode(M1_IN2, OUTPUT);
  pinMode(M2_EN, OUTPUT); 
  pinMode(M2_IN3, OUTPUT); 
  pinMode(M2_IN4, OUTPUT);
  pinMode(M3_EN, OUTPUT); 
  pinMode(M3_IN1, OUTPUT); 
  pinMode(M3_IN2, OUTPUT);
  pinMode(M4_EN, OUTPUT); 
  pinMode(M4_IN3, OUTPUT); 
  pinMode(M4_IN4, OUTPUT);
  pinMode(HEADLIGHTS, OUTPUT);
  digitalWrite(HEADLIGHTS, LOW);
  pinMode(LEFT_IND, OUTPUT);
  pinMode(RIGHT_IND, OUTPUT);
  pinMode(BRAKE_LIGHTS, OUTPUT);
  pinMode(HORN, OUTPUT);
}


void loop() 
{
  if (Bluetooth.available() > 0) 
  {
    char command = Bluetooth.read();

    if (command == 'H') 
    {
      digitalWrite(HEADLIGHTS, HIGH);
    }
    else if (command == 'O') 
    {
      digitalWrite(HEADLIGHTS, LOW);
    }
    else if (command == 'F')
    {
      moveForward(speed);
    }
    else if (command == 'B')
    {
      moveBackward(speed);
    }
    else if (command == 'L')
    {
      moveLeft(speed);
    }
    else if (command == 'R')
    {
      moveRight(speed);
    }
    else if (command == 'S')
    {
      stopAllMotors();
    }
  }
}



void stopAllMotors() 
{
  analogWrite(M1_EN, speed); 
  digitalWrite(M1_IN1, LOW); digitalWrite(M1_IN2, LOW);

  analogWrite(M2_EN, speed);
  digitalWrite(M2_IN3, LOW); digitalWrite(M2_IN4, LOW);

  analogWrite(M3_EN, speed);
  digitalWrite(M3_IN1, LOW); digitalWrite(M3_IN2, LOW);

  analogWrite(M4_EN, speed);
  digitalWrite(M4_IN3, LOW); digitalWrite(M4_IN4, LOW);

  digitalWrite(BRAKE_LIGHTS, HIGH);
  digitalWrite(LEFT_IND, LOW);
  digitalWrite(RIGHT_IND, LOW);
}

void moveForward(int speed)
{
  analogWrite(M1_EN, speed); 
  digitalWrite(M1_IN1, LOW); digitalWrite(M1_IN2, HIGH);

  analogWrite(M2_EN, speed);
  digitalWrite(M2_IN3, HIGH); digitalWrite(M2_IN4, LOW);

  analogWrite(M3_EN, speed);
  digitalWrite(M3_IN1, LOW); digitalWrite(M3_IN2, HIGH);

  analogWrite(M4_EN, speed);
  digitalWrite(M4_IN3, HIGH); digitalWrite(M4_IN4, LOW);

  digitalWrite(BRAKE_LIGHTS, LOW);
  digitalWrite(LEFT_IND, LOW);
  digitalWrite(RIGHT_IND, LOW);
}

void moveBackward(int speed) 
{
  analogWrite(M1_EN, speed); 
  digitalWrite(M1_IN1, HIGH); digitalWrite(M1_IN2, LOW);

  analogWrite(M2_EN, speed);
  digitalWrite(M2_IN3, LOW); digitalWrite(M2_IN4, HIGH);

  analogWrite(M3_EN, speed);
  digitalWrite(M3_IN1, HIGH); digitalWrite(M3_IN2, LOW);

  analogWrite(M4_EN, speed);
  digitalWrite(M4_IN3, LOW); digitalWrite(M4_IN4, HIGH);

  digitalWrite(BRAKE_LIGHTS, LOW);
  digitalWrite(LEFT_IND, LOW);
  digitalWrite(RIGHT_IND, LOW);
}

void moveLeft(int speed)
{
  analogWrite(M3_EN, speed);
  digitalWrite(M3_IN1, LOW); digitalWrite(M3_IN2, HIGH);

  analogWrite(M4_EN, speed);
  digitalWrite(M4_IN3, HIGH); digitalWrite(M4_IN4, LOW);

  digitalWrite(BRAKE_LIGHTS, LOW);
  digitalWrite(LEFT_IND, HIGH);
  digitalWrite(RIGHT_IND, LOW);
}

void moveRight(int speed)
{
  analogWrite(M1_EN, speed); 
  digitalWrite(M1_IN1, LOW); digitalWrite(M1_IN2, HIGH);

  analogWrite(M2_EN, speed);
  digitalWrite(M2_IN3, HIGH); digitalWrite(M2_IN4, LOW);

  digitalWrite(BRAKE_LIGHTS, LOW);
  digitalWrite(LEFT_IND, LOW);
  digitalWrite(RIGHT_IND, HIGH);
}
