#include <Servo.h>

// Pin definitions
const int trigPin   = 10;
const int echoPin   = 11;
const int servoPin  = 9;

const int lowThreshold = 100;
const int highThreshold = 200;
const int clawOpen = 0;
const int clawClosed = 180;

bool isOpen;
bool isLocked;

const int windowSize = 5;
int distanceReadings[windowSize] = {0};
int currentIndex = 0;
int totalReadings = 0;

unsigned long lastStateChangeTime = 0;
const unsigned long debounceDelay = 500;

Servo myServo;

void setup() {
  Serial.begin(9600);
  myServo.attach(servoPin);

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  myServo.write(clawOpen);
  delay(1000);
  
  isOpen = true;
  isLocked = false;
}

void loop() {
  unsigned long currentTime = millis();
  
  int distance = getDistance();
  
  if (distance > 0) {
  	addReading(distance);
  }
  
  int smoothedDistance = getAverage();

  Serial.print("Smoothed distance: ");
  Serial.print(smoothedDistance);
  Serial.println(" cm");

  if (!isLocked) {
    if (smoothedDistance > 0 && smoothedDistance <= lowThreshold && isOpen && currentTime - lastStateChangeTime > debounceDelay) {
      myServo.write(clawClosed);
      isOpen = false;
      lastStateChangeTime = currentTime;
      Serial.println("Claw closed");
    } else if (smoothedDistance > 0 && smoothedDistance > highThreshold && !isLocked) {
      isLocked = true;
      lastStateChangeTime = currentTime;
      Serial.println("Claw locked");
    }
  } else {
    if (smoothedDistance > 0 && smoothedDistance <= lowThreshold && !isOpen && currentTime - lastStateChangeTime > debounceDelay) {
      myServo.write(clawOpen);
      isOpen = true;
      isLocked = false;
      lastStateChangeTime = currentTime;
      Serial.println("Claw opened and unlocked");
    }
  }

  delay(100);
}



int getDistance() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH);

  int distanceCm = duration * 0.0343 / 2;

  if (distanceCm == 0) {
    return -1; 
  }
  
  return distanceCm;
}

void addReading(int newReading) {
  totalReadings -= distanceReadings[currentIndex];  
  distanceReadings[currentIndex] = newReading;     
  totalReadings += newReading;                     
  currentIndex = (currentIndex + 1) % windowSize;  
}

int getAverage() {
	return totalReadings / windowSize;
}