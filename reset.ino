#include <math.h>
#include <Wire.h>
#include <BleMouse.h>
#include <MPU9250_asukiaaa.h>

BleMouse bleMouse("ESP32 BLE Mouse");
MPU9250_asukiaaa mpu;

// Button pins
#define LEFT_CLICK_PIN     18
#define RIGHT_CLICK_PIN    19
#define CURSOR_TOGGLE_PIN  23 

float sensitivity = 0.5;
float deadzone    = 0.01;
float alpha       = 0.85;
int   maxStep     = 5;


float smoothX = 0;
float smoothY = 0;

unsigned long lastTime = 0;

bool cursorEnabled = true;
bool lastToggleState = HIGH;

void setup() {
  Serial.begin(115200);
  delay(1000);

  Wire.begin(21, 22);
  Wire.setClock(400000);

  mpu.setWire(&Wire);
  mpu.beginGyro();

  bleMouse.begin();

  pinMode(LEFT_CLICK_PIN, INPUT_PULLUP);
  pinMode(RIGHT_CLICK_PIN, INPUT_PULLUP);
  pinMode(CURSOR_TOGGLE_PIN, INPUT_PULLUP);

  lastTime = millis();
  Serial.println("Smooth MPU9250 BLE Mouse Ready");
}

void loop() {

 
  bool toggleState = digitalRead(CURSOR_TOGGLE_PIN);

  if (toggleState == LOW && lastToggleState == HIGH) {
    cursorEnabled = !cursorEnabled;


    smoothX = 0;
    smoothY = 0;
    lastTime = millis();

    Serial.println(cursorEnabled ? "Cursor ENABLED" : "Cursor DISABLED");
    delay(250); // debounce for TTP223
  }
  lastToggleState = toggleState;

 
  if (!bleMouse.isConnected()) return;


  if (!cursorEnabled) {
    delay(5);
    return;
  }


  unsigned long now = millis();
  float dt = (now - lastTime) / 1000.0;
  lastTime = now;
  if (dt <= 0) return;

  mpu.gyroUpdate();

  float rawX = (mpu.gyroZ()) * (-1);
  float rawY = mpu.gyroY();

  if (fabs(rawX) < deadzone) rawX = 0;
  if (fabs(rawY) < deadzone) rawY = 0;

  smoothX = alpha * smoothX + (1 - alpha) * rawX;
  smoothY = alpha * smoothY + (1 - alpha) * rawY;

  float moveX = smoothX * sensitivity * dt * 60;
  float moveY = smoothY * sensitivity * dt * 60;

  moveX = constrain(moveX, -maxStep, maxStep);
  moveY = constrain(moveY, -maxStep, maxStep);

  if (moveX != 0 || moveY != 0) {
    bleMouse.move((int)-moveX, (int)moveY);
  }

 
  if (digitalRead(LEFT_CLICK_PIN) == LOW)
    bleMouse.press(MOUSE_LEFT);
  else
    bleMouse.release(MOUSE_LEFT);

  if (digitalRead(RIGHT_CLICK_PIN) == LOW)
    bleMouse.press(MOUSE_RIGHT);
  else
    bleMouse.release(MOUSE_RIGHT);

  delay(5);
}