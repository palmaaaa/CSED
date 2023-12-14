/* 
Name: Quizzer
How it works: The code manages the two Ultrasonic sensor, 3 leds, a buzzer and
the IR receiver in order to let two players (or teams) play a quiz based game
Author: Michele Palma
Date: 04/12/2023
*/
#include <IRremote.h>  // Infrared module library
#include <ezBuzzer.h>  // Buzzer library

// Hexadecimal values for the IR Remote buttons
const uint16_t POWER = 0x45;
const uint16_t BUTTON_0 = 0x16;
const uint16_t BUTTON_1 = 0xC;
const uint16_t BUTTON_2 = 0x18;
const uint16_t BUTTON_3 = 0x5E;
const uint16_t BUTTON_4 = 0x8;
const uint16_t BUTTON_5 = 0x1C;
const uint16_t BUTTON_6 = 0x5A;
const uint16_t BUTTON_7 = 0x42;
const uint16_t BUTTON_8 = 0x52;
const uint16_t BUTTON_9 = 0x4A;
const uint16_t ARROW_UP = 0x9;
const uint16_t ARROW_DOWN = 0x7;
const uint16_t ARROW_L = 0x44;
const uint16_t ARROW_R = 0x43;
const uint16_t PLAY = 0x40;
const uint16_t VOL_UP = 0x46;
const uint16_t VOL_DOWN = 0x15;
const uint16_t EQ = 0x19;
const uint16_t FUNC_STOP = 0x47;
const uint16_t ST_REPT = 0xD;
// ===============================================
const int LED_US1 = 2;  // Led pin for the Ultrasonic sensor for player 1
const int US1_E = 3;    // Ultrasonic Echo Pin
const int US1_T = 4;    // Ultrasonic Trigger Pin

const int LED_US2 = 5;  // Led pin for the Ultrasonic sensor for player 2
const int US2_E = 6;    // Ultrasonic Echo Pin
const int US2_T = 7;    // Ultrasonic Trigger Pin

const int END_GAME_LED = 9;  // Led that lights when the match is over

const int BUZZER = 8;  // Pin for the buzzer
const int MARGIN = 3;  // Tolarance threshold to trigger the reservation for the players

int cm = 0;   // Distance recorded for the first Ultrasonic sensor in a t time instance
int cm2 = 0;  // Distance recorded for the second Ultrasonic sensor in a t time instance

int maxVal = 0;   // Max distance recorded for Ultrasonic sensor 1
int maxVal2 = 0;  // Max distance recorded for Ultrasonic sensor 2

bool p1 = false;    // Boolean to check if player 1 was the first to reserve the answer
bool p2 = false;    // Boolean to check if player 2 was the first to reserve the answer
bool start = true;  // Boolean to setup the first round sensor distances

int action = 0;    // Variable that stores the action relative to the button clicked on the IR Remote
int p1_score = 0;  // Score of player 1
int p2_score = 0;  // Score of player 2

ezBuzzer buzzer(BUZZER);  // Intialize buzzer library object
IRrecv receiver(12);      // Intialize IR Receiver library object

// Setup all the necessary pins and object variables pins
void setup() {
  Serial.begin(9600);
  pinMode(US1_T, OUTPUT);
  pinMode(US1_E, INPUT);
  pinMode(LED_US1, OUTPUT);
  pinMode(US2_T, OUTPUT);
  pinMode(US2_E, INPUT);
  pinMode(LED_US2, OUTPUT);
  pinMode(END_GAME_LED, OUTPUT);
  IrReceiver.begin(12);
}

// Functions that calculates the distance (time in microseconds) registered by a Ultrasonic sensor via its trigger and echo pins
long readUltrasonicDistance(int triggerPin, int echoPin) {
  pinMode(triggerPin, OUTPUT);  // Clear the trigger
  digitalWrite(triggerPin, LOW);
  delayMicroseconds(2);
  // Sets the trigger pin to HIGH state for 10 microseconds
  digitalWrite(triggerPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(triggerPin, LOW);
  pinMode(echoPin, INPUT);
  // Reads the echo pin, and returns the sound wave travel time in microseconds
  return pulseIn(echoPin, HIGH);
}

void loop() {
  buzzer.loop();  // Start buzzer sound

  // Do the preliminary measurements, turn on and off the leds, and beep for 700ms then start the game
  if (start) {
    delay(2000);
    cm = 0.01700 * readUltrasonicDistance(US1_T, US1_E);
    cm2 = 0.01700 * readUltrasonicDistance(US2_T, US2_E);
    maxVal = cm;
    maxVal2 = cm2;
    start = false;
    digitalWrite(LED_US1, HIGH);
    digitalWrite(LED_US2, HIGH);
    delay(500);
    digitalWrite(LED_US1, LOW);
    digitalWrite(LED_US2, LOW);
    buzzer.beep(700);
    delay(700);
  }

  // If no player reserved the answer check who will reserve it first
  if (!p1 && !p2) {
    cm = 0.01700 * readUltrasonicDistance(US1_T, US1_E);
    cm2 = 0.01700 * readUltrasonicDistance(US2_T, US2_E);
    delay(10);

    // If player 2 hans't booked yet and player 1 booked, reserve the round for player 1
    if (!p2 && (maxVal - cm < -MARGIN || maxVal - cm > MARGIN)) {
      digitalWrite(LED_US1, HIGH);
      p1 = true;
    } else {
      digitalWrite(LED_US1, LOW);
    }

    // If player 1 hans't booked yet and player 2 booked, reserve the round for player 2
    if (!p1 && (maxVal2 - cm2 < -MARGIN || maxVal2 - cm2 > MARGIN)) {
      digitalWrite(LED_US2, HIGH);
      p2 = true;
    } else {
      digitalWrite(LED_US2, LOW);
    }
    delay(100);

  } else {

    /* After a round has been played the host can decide if to:
        - terminate the game (power button => action 0)
        - move to next round without assigning any points (next button=> action 3)
        - assign a point to player 1 (button one=> action 1)
        - assign a point to player 2 (button two=> action 2)

        The game will not resume until the host makes a decision
    */
    do {
      read_IRtransmitter_button(irReceive());
    } while (action != 0 && action != 1 && action != 2 && action != 3);


    // Depending on the action, as described earlier, something will happen
    switch (action) {
      case 0:  // Ending the game
        Serial.println(p1_score);
        Serial.println(p2_score);
        //Flicker both leds in order to build suspance on who won
        for (int j = 0; j < 7; j++) {
          if (j % 2 == 0) {
            digitalWrite(LED_US1, HIGH);
            digitalWrite(LED_US2, LOW);
            delay(250);
            digitalWrite(LED_US1, LOW);
            digitalWrite(LED_US2, HIGH);
            delay(250);
          } else {
            digitalWrite(LED_US2, HIGH);
            digitalWrite(LED_US1, LOW);
            delay(250);
            digitalWrite(LED_US2, LOW);
            digitalWrite(LED_US1, HIGH);
            delay(250);
          }
        }
        digitalWrite(LED_US2, LOW);
        digitalWrite(LED_US1, LOW);
        digitalWrite(END_GAME_LED, HIGH);
        // If player one won, flick the light as many times as the points that the p1 scored
        if (p1_score > p2_score) {
          for (int j = 0; j < p1_score; j++) {
            digitalWrite(LED_US1, LOW);
            delay(500);
            digitalWrite(LED_US1, HIGH);
            delay(500);
          }
          digitalWrite(LED_US1, LOW);
          delay(1500);
          digitalWrite(LED_US1, HIGH);

          // If player two won, flick the light as many times as the points that the p2 scored
        } else if (p1_score < p2_score) {
          for (int j = 0; j < p2_score; j++) {
            digitalWrite(LED_US2, LOW);
            delay(500);
            digitalWrite(LED_US2, HIGH);
            delay(500);
          }
          digitalWrite(LED_US2, LOW);
          delay(1500);
          digitalWrite(LED_US2, HIGH);

          // If there's a draw just turn both leds on
        } else {
          digitalWrite(LED_US1, HIGH);
          digitalWrite(LED_US2, HIGH);
        }
        do {
        } while (true);  // End the game by doing an infinite loop
        break;

      case 1:      // If the player who reserved the round didn't answer correctly give the point to the other player
        if (p1) {  // if player 1 failed, then light the other player's led to signal that the point went to him/her
          p2_score++;
          digitalWrite(LED_US1, LOW);
          digitalWrite(LED_US2, HIGH);
          delay(1000);
          digitalWrite(LED_US2, LOW);
        } else {
          p1_score++;
          digitalWrite(LED_US2, LOW);
          digitalWrite(LED_US1, HIGH);
          delay(1000);
          digitalWrite(LED_US1, LOW);
        }
        break;

      case 2:  // Increase score of player who reserved the round
        if (p1) {
          p1_score++;
        } else {
          p2_score++;
        }
        break;


      default: break;
    }

    // Reset the max distance, the player reservation and turn of the leds for the next roun
    p1 = false;
    p2 = false;
    digitalWrite(LED_US1, LOW);
    digitalWrite(LED_US2, LOW);
    maxVal = 0.01700 * readUltrasonicDistance(US1_T, US1_E);
    maxVal2 = 0.01700 * readUltrasonicDistance(US2_T, US2_E);
  }
  read_IRtransmitter_button(irReceive());  // Keep capturing the IR Remote inputs (-1 if no input was made)
  if (action == 0) p1 = true;              // If the host presses the power button then the game will end right away
  delay(50);
}

// Function that read the hexadecimal values received by the IR Receiver and modifies the action corrispondently
void read_IRtransmitter_button(uint16_t b) {
  switch (b) {
    case POWER:
      Serial.println("POWER CLICKED");
      action = 0;
      break;
    case BUTTON_0:
      Serial.println("BUTTON 0 CLICKED");
      action = 1;
      break;
    case BUTTON_1:
      Serial.println("BUTTON 1 CLICKED");
      action = 2;
      break;
    case BUTTON_2:
      Serial.println("BUTTON 2 CLICKED");
      //action = 2;
      break;
    case BUTTON_3: Serial.println("BUTTON 3 CLICKED"); break;
    case BUTTON_4: Serial.println("BUTTON 4 CLICKED"); break;
    case BUTTON_5: Serial.println("BUTTON 5 CLICKED"); break;
    case BUTTON_6: Serial.println("BUTTON 6 CLICKED"); break;
    case BUTTON_7: Serial.println("BUTTON 7 CLICKED"); break;
    case BUTTON_8: Serial.println("BUTTON 8 CLICKED"); break;
    case BUTTON_9: Serial.println("BUTTON 9 CLICKED"); break;
    case ARROW_UP: Serial.println("ARROW UP CLICKED"); break;
    case ARROW_DOWN: Serial.println("ARROW DOWN CLICKED"); break;
    case ARROW_L: Serial.println("ARROW LEFT CLICKED"); break;
    case ARROW_R:
      Serial.println("ARROW RIGHT CLICKED");
      action = 3;
      break;
    case PLAY: Serial.println("PLAY CLICKED"); break;
    case VOL_UP: Serial.println("VOLUME UP CLICKED"); break;
    case VOL_DOWN: Serial.println("VOLUME DOWN CLICKED"); break;
    case EQ: Serial.println("EQ CLICKED"); break;
    case FUNC_STOP: Serial.println("FUNC/STOP CLICKED"); break;
    case ST_REPT: Serial.println("ST/REPT CLICKED"); break;
    default: action = -1; break;
  }
}

// Decodes the signal received and outputs the hexadecimal value
uint16_t irReceive() {
  uint16_t received{ -1 };
  if (IrReceiver.decode()) {
    received = IrReceiver.decodedIRData.command;
    IrReceiver.resume();
  }
  return received;
}
