// ============================================
// XCar - Main Bluetooth Control Firmware
// ============================================

#include <SoftwareSerial.h>

SoftwareSerial XCar(0, 1); // RX, TX

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
#define M4_IN4  12

// ---- Lights ----
#define HEADLIGHTS    A2
#define LEFT_IND      A3
#define RIGHT_IND     A4
#define BRAKE_LIGHTS  A5

// ---- Horn ----
#define HORN  2

// ---- State Variables ----
int valY = 0;         // Forward/Backward: -255 to +255
int valX = 0;         // Turn: -255 to +255
bool headlight = false;
bool horn = false;
bool brakePressed = false;

// ---- Indicator blink ----
unsigned long lastBlinkTime = 0;
bool blinkState = false;

String inputBuffer = "";

// ============================================
void setup() {
  Serial.begin(9600);
  XCar.begin(9600);

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
  pinMode(LEFT_IND, OUTPUT);
  pinMode(RIGHT_IND, OUTPUT);
  pinMode(BRAKE_LIGHTS, OUTPUT);
  pinMode(HORN, OUTPUT);

  stopAllMotors();
  allLightsOff();
  Serial.println(F("XCar Ready"));
}

// ============================================
void loop() {
  // Read Bluetooth data
  while (XCar.available()) {
    char c = XCar.read();
    if (c == '\n') {
      parsePacket(inputBuffer);
      inputBuffer = "";
    } else {
      inputBuffer += c;
    }
  }

  // Apply motor control
  if (brakePressed) {
    stopAllMotors();
  } else {
    driveMotors(valY, valX);
  }

  // Headlight
  digitalWrite(HEADLIGHTS, headlight ? HIGH : LOW);

  // Horn
  digitalWrite(HORN, horn ? HIGH : LOW);

  // Indicators blinking
  handleIndicators();
}

// ============================================
// Parse packet: "Y:200,X:-100,H:1,HN:0,B:0"
// ============================================
void parsePacket(String packet) {
  // Extract Y
  int yIdx = packet.indexOf("Y:");
  int xIdx = packet.indexOf(",X:");
  int hIdx = packet.indexOf(",H:");
  int hnIdx = packet.indexOf(",HN:");
  int bIdx = packet.indexOf(",B:");

  if (yIdx == -1 || xIdx == -1 || hIdx == -1 || hnIdx == -1 || bIdx == -1) {
    return; // Invalid packet, ignore
  }

  valY        = packet.substring(yIdx + 2, xIdx).toInt();
  valX        = packet.substring(xIdx + 3, hIdx).toInt();
  headlight   = packet.substring(hIdx + 3, hnIdx).toInt();
  horn        = packet.substring(hnIdx + 4, bIdx).toInt();
  brakePressed = packet.substring(bIdx + 3).toInt();

  // Clamp values
  valY = constrain(valY, -255, 255);
  valX = constrain(valX, -255, 255);
}

// ============================================
// Drive motors using Y and X joystick values
// Differential steering:
// Left  motors = Y + X
// Right motors = Y - X
// ============================================
void driveMotors(int y, int x) {
  int leftSpeed  = constrain(y + x, -255, 255);
  int rightSpeed = constrain(y - x, -255, 255);

  // Left motors (Motor 1 and Motor 2)
  setMotor(M1_EN, M1_IN1, M1_IN2, leftSpeed);
  setMotor(M2_EN, M2_IN3, M2_IN4, leftSpeed);

  // Right motors (Motor 3 and Motor 4)
  setMotor(M3_EN, M3_IN1, M3_IN2, rightSpeed);
  setMotor(M4_EN, M4_IN3, M4_IN4, rightSpeed);

  // Brake lights — on when both joysticks are at zero
  if (y == 0 && x == 0) {
    digitalWrite(BRAKE_LIGHTS, HIGH);
  } else {
    digitalWrite(BRAKE_LIGHTS, LOW);
  }
}

// ============================================
// Set individual motor speed and direction
// ============================================
void setMotor(int enPin, int in1Pin, int in2Pin, int speed) {
  if (speed > 0) {
    analogWrite(enPin, speed);
    digitalWrite(in1Pin, LOW);
    digitalWrite(in2Pin, HIGH);
  } else if (speed < 0) {
    analogWrite(enPin, abs(speed));
    digitalWrite(in1Pin, HIGH);
    digitalWrite(in2Pin, LOW);
  } else {
    analogWrite(enPin, 0);
    digitalWrite(in1Pin, LOW);
    digitalWrite(in2Pin, LOW);
  }
}

// ============================================
// Handle indicator blinking
// ============================================
void handleIndicators() {
  // Only blink when turning, not braking
  if (brakePressed || (valY == 0 && valX == 0)) {
    digitalWrite(LEFT_IND, LOW);
    digitalWrite(RIGHT_IND, LOW);
    return;
  }

  if (valX > 30) {
    // Turning right
    digitalWrite(LEFT_IND, LOW);
    if (millis() - lastBlinkTime >= 500) {
      lastBlinkTime = millis();
      blinkState = !blinkState;
      digitalWrite(RIGHT_IND, blinkState);
    }
  } else if (valX < -30) {
    // Turning left
    digitalWrite(RIGHT_IND, LOW);
    if (millis() - lastBlinkTime >= 500) {
      lastBlinkTime = millis();
      blinkState = !blinkState;
      digitalWrite(LEFT_IND, blinkState);
    }
  } else {
    digitalWrite(LEFT_IND, LOW);
    digitalWrite(RIGHT_IND, LOW);
  }
}

// ============================================
// Helper Functions
// ============================================
void stopAllMotors() {
  analogWrite(M1_EN, 0); analogWrite(M2_EN, 0);
  analogWrite(M3_EN, 0); analogWrite(M4_EN, 0);
  digitalWrite(M1_IN1, LOW); digitalWrite(M1_IN2, LOW);
  digitalWrite(M2_IN3, LOW); digitalWrite(M2_IN4, LOW);
  digitalWrite(M3_IN1, LOW); digitalWrite(M3_IN2, LOW);
  digitalWrite(M4_IN3, LOW); digitalWrite(M4_IN4, LOW);
  digitalWrite(BRAKE_LIGHTS, HIGH);
}

void allLightsOff() {
  digitalWrite(HEADLIGHTS, LOW);
  digitalWrite(LEFT_IND, LOW);
  digitalWrite(RIGHT_IND, LOW);
  digitalWrite(BRAKE_LIGHTS, LOW);
  digitalWrite(HORN, LOW);
}
