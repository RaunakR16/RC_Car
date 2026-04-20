// ============================================================
// XCar - ESP32 Full Bluetooth Control Firmware (Core v3.x)
// ============================================================

#include <BluetoothSerial.h>

BluetoothSerial SerialBT;

// ================= MOTOR DRIVER - LEFT =================
#define L_EN1 32
#define L_IN1 33
#define L_IN2 25

#define L_EN2 26
#define L_IN3 27
#define L_IN4 13

// ================= MOTOR DRIVER - RIGHT =================
#define R_EN1 23
#define R_IN1 22
#define R_IN2 21

#define R_EN2 19
#define R_IN3 18
#define R_IN4 17

// ================= LIGHTS & HORN =================
#define HEADLIGHT       5
#define BRAKE_LIGHT     16
#define LEFT_INDICATOR  1
#define RIGHT_INDICATOR 3
#define HORN            4
#define REVERSE_LIGHT   14

// ================= PWM CONFIG =================
#define PWM_FREQ       1000
#define PWM_RESOLUTION 8

// ================= STATE VARIABLES =================
int  valY         = 0;
int  valX         = 0;
bool headlight    = false;
bool horn         = false;
bool brakePressed = false;

// ================= BUFFER =================
char inputBuffer[60];
int  bufferIndex = 0;

// ================= INDICATOR BLINK =================
unsigned long lastBlinkTime = 0;
bool blinkState = false;

// ============================================================
void setup() {

  // ===== MOTOR DIRECTION PINS =====
  pinMode(L_IN1, OUTPUT); pinMode(L_IN2, OUTPUT);
  pinMode(L_IN3, OUTPUT); pinMode(L_IN4, OUTPUT);
  pinMode(R_IN1, OUTPUT); pinMode(R_IN2, OUTPUT);
  pinMode(R_IN3, OUTPUT); pinMode(R_IN4, OUTPUT);

  // ===== PWM SETUP (v3.x) =====
  ledcAttach(L_EN1, PWM_FREQ, PWM_RESOLUTION);
  ledcAttach(L_EN2, PWM_FREQ, PWM_RESOLUTION);
  ledcAttach(R_EN1, PWM_FREQ, PWM_RESOLUTION);
  ledcAttach(R_EN2, PWM_FREQ, PWM_RESOLUTION);

  // ===== LIGHTS & HORN =====
  pinMode(HEADLIGHT,       OUTPUT);
  pinMode(BRAKE_LIGHT,     OUTPUT);
  pinMode(LEFT_INDICATOR,  OUTPUT);
  pinMode(RIGHT_INDICATOR, OUTPUT);
  pinMode(HORN,            OUTPUT);
  pinMode(REVERSE_LIGHT,   OUTPUT);

  // ===== SAFE DEFAULT STATES =====
  stopAllMotors();
  allLightsOff();

  // ===== BLUETOOTH =====
  SerialBT.begin("XCar");
  SerialBT.println("XCar Ready");
}

// ============================================================
void loop() {

  // ===== READ BLUETOOTH DATA =====
  while (SerialBT.available()) {
    char c = SerialBT.read();

    if (c == '\n') {
      inputBuffer[bufferIndex] = '\0';
      if (bufferIndex > 0) {
        parsePacket();
      }
      bufferIndex = 0;
    } else if (bufferIndex < 59) {
      inputBuffer[bufferIndex++] = c;
    }
  }

  // ===== APPLY MOTOR CONTROL =====
  if (brakePressed) {
    stopAllMotors();
  } else {
    driveMotors(valY, valX);
  }

  // ===== HEADLIGHT =====
  digitalWrite(HEADLIGHT, headlight ? HIGH : LOW);

  // ===== HORN =====
  digitalWrite(HORN, horn ? HIGH : LOW);

  // ===== INDICATORS =====
  handleIndicators();
}

// ============================================================
// Parse packet: "Y:200,X:-100,H:1,HN:0,B:0"
// ============================================================
void parsePacket() {
  int yVal = 0, xVal = 0, h = 0, hn = 0, b = 0;

  char* ptr;

  ptr = strstr(inputBuffer, "Y:");
  if (ptr) yVal = atoi(ptr + 2);

  ptr = strstr(inputBuffer, "X:");
  if (ptr) xVal = atoi(ptr + 2);

  // HN must be parsed before H to avoid misreading
  ptr = strstr(inputBuffer, "HN:");
  if (ptr) hn = atoi(ptr + 3);

  ptr = strstr(inputBuffer, "H:");
  if (ptr) h = atoi(ptr + 2);

  ptr = strstr(inputBuffer, "B:");
  if (ptr) b = atoi(ptr + 2);

  valY         = constrain(yVal, -255, 255);
  valX         = constrain(xVal, -255, 255);
  headlight    = (h  == 1);
  horn         = (hn == 1);
  brakePressed = (b  == 1);

  // Minimal debug over Bluetooth
  SerialBT.print("Y:"); SerialBT.print(valY);
  SerialBT.print(" X:"); SerialBT.print(valX);
  SerialBT.print(" H:"); SerialBT.print(headlight);
  SerialBT.print(" HN:"); SerialBT.print(horn);
  SerialBT.print(" B:"); SerialBT.println(brakePressed);
}

// ============================================================
// Drive motors using differential steering
// Left  motors = Y + X
// Right motors = Y - X
// ============================================================
void driveMotors(int y, int x) {
  int leftSpeed  = constrain(y + x, -255, 255);
  int rightSpeed = constrain(y - x, -255, 255);

  setMotor(L_EN1, L_IN1, L_IN2, leftSpeed);
  setMotor(L_EN2, L_IN3, L_IN4, leftSpeed);
  setMotor(R_EN1, R_IN1, R_IN2, rightSpeed);
  setMotor(R_EN2, R_IN3, R_IN4, rightSpeed);

  // ===== BRAKE LIGHT =====
  digitalWrite(BRAKE_LIGHT, (y == 0 && x == 0) ? HIGH : LOW);

  // ===== REVERSE LIGHT =====
  digitalWrite(REVERSE_LIGHT, (y < -10) ? HIGH : LOW);
}

// ============================================================
// Set individual motor speed and direction
// ============================================================
void setMotor(int enPin, int in1, int in2, int speed) {
  if (speed > 0) {
    ledcWrite(enPin, speed);
    digitalWrite(in1, LOW);
    digitalWrite(in2, HIGH);
  } else if (speed < 0) {
    ledcWrite(enPin, abs(speed));
    digitalWrite(in1, HIGH);
    digitalWrite(in2, LOW);
  } else {
    ledcWrite(enPin, 0);
    digitalWrite(in1, LOW);
    digitalWrite(in2, LOW);
  }
}

// ============================================================
// Indicator blinking logic
// ============================================================
void handleIndicators() {
  if (brakePressed || (valY == 0 && valX == 0)) {
    digitalWrite(LEFT_INDICATOR,  LOW);
    digitalWrite(RIGHT_INDICATOR, LOW);
    blinkState = false;
    return;
  }

  if (millis() - lastBlinkTime >= 500) {
    lastBlinkTime = millis();
    blinkState = !blinkState;
  }

  if (valX > 30) {
    digitalWrite(LEFT_INDICATOR,  LOW);
    digitalWrite(RIGHT_INDICATOR, blinkState);
  } else if (valX < -30) {
    digitalWrite(RIGHT_INDICATOR, LOW);
    digitalWrite(LEFT_INDICATOR,  blinkState);
  } else {
    digitalWrite(LEFT_INDICATOR,  LOW);
    digitalWrite(RIGHT_INDICATOR, LOW);
  }
}

// ============================================================
// Stop all motors
// ============================================================
void stopAllMotors() {
  ledcWrite(L_EN1, 0); ledcWrite(L_EN2, 0);
  ledcWrite(R_EN1, 0); ledcWrite(R_EN2, 0);

  digitalWrite(L_IN1, LOW); digitalWrite(L_IN2, LOW);
  digitalWrite(L_IN3, LOW); digitalWrite(L_IN4, LOW);
  digitalWrite(R_IN1, LOW); digitalWrite(R_IN2, LOW);
  digitalWrite(R_IN3, LOW); digitalWrite(R_IN4, LOW);

  digitalWrite(BRAKE_LIGHT,   HIGH);
  digitalWrite(REVERSE_LIGHT, LOW);
}

// ============================================================
// Turn off all lights
// ============================================================
void allLightsOff() {
  digitalWrite(HEADLIGHT,       LOW);
  digitalWrite(BRAKE_LIGHT,     LOW);
  digitalWrite(LEFT_INDICATOR,  LOW);
  digitalWrite(RIGHT_INDICATOR, LOW);
  digitalWrite(HORN,            LOW);
  digitalWrite(REVERSE_LIGHT,   LOW);
}
