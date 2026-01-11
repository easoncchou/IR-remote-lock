#include <IRremote.h>

#define IR_RECEIVE_PIN 9

// What signals IR receiver reads translated into the
//  buttons on the remote
#define IR_0  0x16 
#define IR_1  0x0C
#define IR_2  0x18
#define IR_3  0x5E
#define IR_4  0x08
#define IR_5  0x1C
#define IR_6  0x5A
#define IR_7  0x42
#define IR_8  0x52
#define IR_9  0x4A

// ---------- PASSWORD ----------
const int PASSWORD_LENGTH = 4;
const int password[PASSWORD_LENGTH] = {1, 2, 3, 4};

// ---------- STATE MACHINE ----------
enum State {
  WAITING,
  COLLECTING,
  UNLOCKED
};

State state = WAITING;

// ---------- INPUT BUFFER ----------
int enteredCode[PASSWORD_LENGTH]; // code being entered by user
int index = 0; // which digit of the code we expect next from user

// ---------- TIMING ----------
unsigned long unlockTime = 0; // last time at which lock was unlocked
const unsigned long UNLOCK_DURATION = 3000; // how long the lock stays open after correct entry

// ---------- FUNCTIONS ----------
int irCommandToDigit(uint8_t cmd) {
  // Takes in the IR signal and translates it to which numbered button was pressed
  //  using the hash-defined mappings above.
  switch (cmd) {
    case IR_0: return 0;
    case IR_1: return 1;
    case IR_2: return 2;
    case IR_3: return 3;
    case IR_4: return 4;
    case IR_5: return 5;
    case IR_6: return 6;
    case IR_7: return 7;
    case IR_8: return 8;
    case IR_9: return 9;
    default:   return -1;
  }
}

bool checkPassword() {
  // Check each element of enteredCode[] against our expected password
  // Return false if any point there is a mismatch btwn the two
  for (int i = 0; i < PASSWORD_LENGTH; i++) {
    if (enteredCode[i] != password[i]) {
      return false;
    }
  }
  return true;
}

void resetInput() {
  // Re-lock the lock by changing the state of the machine and
  //  resetting the index (which digit of the entered code we are on)
  index = 0;
  state = WAITING;
  Serial.println("Locked. Enter password:");
}

// ---------- SETUP ----------
void setup() {
  Serial.begin(9600);
  IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK);
  Serial.println("Locked. Enter password:");
}

// ---------- LOOP ----------
void loop() {

  // ---------- AUTO-RELOCK ----------
  if (state == UNLOCKED) {
    if (millis() - unlockTime >= UNLOCK_DURATION) { // if UNLOCK_DURATION has passed since
                                                    //  the last unlock time
      resetInput(); // re-lock
    }
  }

  // ---------- IR INPUT ----------
  if (IrReceiver.decode()) { // we got an IR signal

    // translate the signal into a digit
    int digit = irCommandToDigit(IrReceiver.decodedIRData.command);

    if (digit != -1) {

      switch (state) { // enter state machine

        case WAITING:
          index = 0;
          state = COLLECTING;
          // intentional fall-through

        case COLLECTING:
          enteredCode[index] = digit; // track what number was just pressed
          index++; // increment the index for next number (next loop iteration)

          Serial.print("*"); // indicate that a number was received

          if (index >= PASSWORD_LENGTH) { // if all four digits has been entered,
            Serial.println();

            if (checkPassword()) {
              state = UNLOCKED;
              unlockTime = millis(); // track the starting time of latest unlock
              Serial.println("UNLOCKED!");
            } else {
              Serial.println("WRONG CODE");
              resetInput();
            }
          }
          break;

        case UNLOCKED:
          // Ignore input while unlocked
          break;
      }
    }

    IrReceiver.resume();
  }
}
