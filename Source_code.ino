#include <SoftwareSerial.h>
#include <Servo.h>

SoftwareSerial bluetooth(2, 3); // RX, TX
Servo Myservo;
char command;

// Motor pins
#define MLa 10  // left motor 1st pin
#define MLb 9   // left motor 2nd pin
#define MRa 12  // right motor 1st pin
#define MRb 11  // right motor 2nd pin

// Ultrasonic sensor pins
#define trigPin 6   // Trig Pin Of HC-SR04
#define echoPin 7   // Echo Pin Of HC-SR04

// Buzzer and metal detector pins
#define buzzerPin 4       // Pin for the buzzer
#define metalDetectorPin A0 // Pin for the metal detector
int rightDistance = 0, leftDistance = 0, middleDistance = 0;
long duration, distance;

void setup() {
  Serial.begin(9600);
  bluetooth.begin(9600);
  
  // Motor pins
  pinMode(MLa, OUTPUT);
  pinMode(MLb, OUTPUT);
  pinMode(MRa, OUTPUT);
  pinMode(MRb, OUTPUT);

  // Ultrasonic sensor pins
  pinMode(trigPin, OUTPUT); 
  pinMode(echoPin, INPUT);
  
  // Buzzer and metal detector pins
  pinMode(buzzerPin, OUTPUT);
  pinMode(metalDetectorPin, INPUT);
  
  // Attach servo motor to pin 8
  Myservo.attach(8);
}

void loop() {
  if (bluetooth.available() > 0) {
    command = bluetooth.read();
    Serial.println(command);
  }

  // Check for metal detection
  if (digitalRead(metalDetectorPin) == HIGH) {
    soundBuzzer();  // Sound the buzzer when metal is detected
  }

  switch (command) {
    case 'F': // Move forward with obstacle detection
      moveForward();
      checkAndAvoidObstacle();
      break;

    case 'B': // Move backward
      moveBackward();
      break;

    case 'L': // Turn left
      turnLeft();
      break;

    case 'R': // Turn right
      turnRight();
      break;

    case 'S': // Stop
      stopMotors();
      break;

    case 'V': // Turn on horn
      digitalWrite(buzzerPin, HIGH); // Turn on the buzzer
      break;

    case 'v': // Turn off horn
      digitalWrite(buzzerPin, LOW); // Turn off the buzzer
      break;

    default:
      stopMotors();
      break;
  }
  delay(10); // Small delay to avoid command spamming
}

// Function to move forward
void moveForward() {
  digitalWrite(MLa, LOW);
  digitalWrite(MLb, HIGH);
  digitalWrite(MRa, LOW);
  digitalWrite(MRb, HIGH);
}

// Function to move backward
void moveBackward() {
  digitalWrite(MLa, HIGH);
  digitalWrite(MLb, LOW);
  digitalWrite(MRa, HIGH);
  digitalWrite(MRb, LOW);
}

// Function to turn left
void turnLeft() {
  digitalWrite(MLa, LOW);
  digitalWrite(MLb, LOW);
  digitalWrite(MRa, LOW);
  digitalWrite(MRb, HIGH);
}

// Function to turn right
void turnRight() {
  digitalWrite(MLa, LOW);
  digitalWrite(MLb, HIGH);
  digitalWrite(MRa, LOW);
  digitalWrite(MRb, LOW);
}

// Function to stop all motors
void stopMotors() {
  digitalWrite(MLa, LOW);
  digitalWrite(MLb, LOW);
  digitalWrite(MRa, LOW);
  digitalWrite(MRb, LOW);
}

// Function to check for obstacles using the ultrasonic sensor
long checkObstacle() {
  // Send pulse
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // Receive echo
  duration = pulseIn(echoPin, HIGH);
  distance = duration / 58.2;  // Convert time to distance in cm

  Serial.print("Distance: ");
  Serial.println(distance);

  return distance;
}

// Function to check and avoid obstacle
void checkAndAvoidObstacle() {
  middleDistance = checkObstacle();

  if (middleDistance > 3) { // No obstacle
    Myservo.write(90); // Keep the servo centered
  } 
  else if (middleDistance <= 3 && middleDistance > 0) { // Obstacle detected
    stopMotors(); // Stop moving
    avoidObstacle(); // Perform obstacle avoidance
  }
}

// Function to handle obstacle avoidance
void avoidObstacle() {
  Myservo.write(0);    // Look left
  delay(500);
  leftDistance = checkObstacle();
  
  Myservo.write(180);  // Look right
  delay(500);
  rightDistance = checkObstacle();
  
  Myservo.write(90);   // Look forward again

  moveBackward();  // Move backward
  delay(500);
  stopMotors();

  // Turn in the direction with more space
  if (rightDistance > leftDistance) {
    turnRight(); // Turn right if more space on the right
    delay(500);
  } else if (rightDistance < leftDistance) {
    turnLeft(); // Turn left if more space on the left
    delay(500);
  } else {
    moveBackward(); // Back up if both sides are blocked
    delay(180);
  }
}

// Function to sound the buzzer (for metal detection)
void soundBuzzer() {
  digitalWrite(buzzerPin, HIGH); // Turn on buzzer
  delay(1000);                    // Sound for 1 second
  digitalWrite(buzzerPin, LOW);  // Turn off buzzer
}