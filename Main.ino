#include <Servo.h>

Servo blindServo;

// Pin definitions
const int tempPin = A0, lightPin = A1, humidityPin = A2, gasPin = A3;
const int rainPin = 2, obstaclePin = 3, servoPin = 9;

// Weights for scoring system (tunable)
const float tempWeight = 0.2, humidityWeight = 0.1, gasWeight = 0.4, lightWeight = 0.3;

void setup() {
  Serial.begin(9600);
  blindServo.attach(servoPin);
  pinMode(rainPin, INPUT_PULLUP);
  pinMode(obstaclePin, INPUT_PULLUP);
}

void loop() {
  // Read sensor data
  float temperature = readTemperature();
  float light = readLight();
  float humidity = readHumidity();
  float gas = readGas();
  bool rain = digitalRead(rainPin) == LOW;
  bool obstacle = digitalRead(obstaclePin) == LOW;

  // Get context-aware angle and explanation
  String mode, explanation;
  int angle = decideBlindAngle(temperature, humidity, gas, light, rain, obstacle, mode, explanation);
  blindServo.write(angle);

  // Output in JSON format for digital dataset collection
  Serial.print("{\"temperature\":"); Serial.print(temperature);
  Serial.print(",\"humidity\":"); Serial.print(humidity);
  Serial.print(",\"gas\":"); Serial.print(gas);
  Serial.print(",\"light\":"); Serial.print(light);
  Serial.print(",\"rain\":"); Serial.print(rain);
  Serial.print(",\"obstacle\":"); Serial.print(obstacle);
  Serial.print(",\"angle\":"); Serial.print(angle);
  Serial.print(",\"mode\":\""); Serial.print(mode);
  Serial.print("\",\"explanation\":\""); Serial.print(explanation);
  Serial.println("\"}");
  
  delay(1000);
}

float readTemperature() {
  float raw = analogRead(tempPin);
  float voltage = raw * 5.0 / 1023.0;
  return (voltage - 0.5) * 100.0;
}

float readLight() {
  float val = analogRead(lightPin);
  return (1.0 - val / 1023.0) * 100.0;
}

float readHumidity() {
  return map(analogRead(humidityPin), 0, 1023, 0, 100);
}

float readGas() {
  return map(analogRead(gasPin), 0, 1023, 0, 100);
}

int decideBlindAngle(float temp, float hum, float gas, float light, bool rain, bool obstacle, String &mode, String &explanation) {
  float score = 0;
  int angle = 90;

  if (obstacle) {
    mode = "Safety Override";
    explanation = "Obstacle detected. Partially closed.";
    return 60;
  }
  if (gas > 90) {
    mode = "Emergency Mode";
    explanation = "Toxic gas level. Fully closed.";
    return 180;
  }
  if (rain) {
    mode = "Rain Mode";
    explanation = "Rain detected. Closing blinds.";
    return 160;
  }

  // Score calculation (can be ML-trained in future)
  score += tempWeight * constrain(temp, 0, 100);
  score += humidityWeight * hum;
  score += gasWeight * gas;
  score += lightWeight * (100.0 - light);  // higher light = lower need to open

  // Convert score to angle (you can fine-tune the mapping range)
  angle = map(score, 0, 100, 180, 0);

  // Decision mode based on context
  if (temp > 35 || hum > 80)
    mode = "Comfort Mode";
  else
    mode = "Eco Mode";

  explanation = "Angle based on weighted score of environmental parameters.";
  return constrain(angle, 0, 180);
}
