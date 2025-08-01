/*
 * Post-Apocalyptic Morse Code Terminal
 * A robust bidirectional Morse communication system
 * 
 * Features:
 * - Encoding mode: Serial input to Morse code output
 * - Decoding mode: Button input to decoded text on LCD
 * - Non-blocking timing system
 * - LED and buzzer feedback
 * 
 * Hardware connections:
 * - LCD: pins 12, 11, 5, 4, 3, 2
 * - LED: pin 13
 * - Buzzer: pin 8
 * - Button: pin 7 (with pull-up resistor)
 * - Serial: USB connection at 9600 baud
 */

#include <LiquidCrystal.h>

// Pin definitions
#define LED_PIN 13
#define BUZZER_PIN 8
#define BUTTON_PIN 7
#define BUZZER_FREQ 800  // 800Hz frequency for piezo buzzer

// Timing constants (in milliseconds)
#define DOT_DURATION 200
#define DASH_DURATION (DOT_DURATION * 3)  // 600ms
#define SYMBOL_GAP (DOT_DURATION)         // 200ms gap between dots/dashes
#define LETTER_GAP (DOT_DURATION * 3)     // 600ms gap between letters
#define WORD_GAP (DOT_DURATION * 7)       // 1400ms gap between words

// Button debounce and timing
#define DEBOUNCE_DELAY 50
#define BUTTON_TIMEOUT 1000  // 1 second timeout for button input

// System states
enum SystemState {
  ENCODING_MODE,
  DECODING_MODE
};

enum EncodingState {
  IDLE,
  TRANSMITTING_DOT,
  TRANSMITTING_DASH,
  SYMBOL_PAUSE,
  LETTER_PAUSE,
  WORD_PAUSE
};

enum DecodingState {
  WAITING_INPUT,
  BUTTON_PRESSED,
  PROCESSING_INPUT,
  DISPLAYING_RESULT
};

// Global variables
SystemState currentSystemState = ENCODING_MODE;
EncodingState encodingState = IDLE;
DecodingState decodingState = WAITING_INPUT;

LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

// Timing variables
unsigned long previousMillis = 0;
unsigned long buttonPressTime = 0;
unsigned long buttonReleaseTime = 0;
bool buttonPressed = false;
bool lastButtonState = HIGH;

// Message buffers
String encodingMessage = "";
String decodingBuffer = "";
String decodedMessage = "";
String currentMorseChar = "";

// Current transmission state
int messageIndex = 0;
int morseIndex = 0;
String currentMorseSequence = "";

// Morse code lookup table
const char* morseTable[] = {
  ".-",    // A
  "-...",  // B
  "-.-.",  // C
  "-..",   // D
  ".",     // E
  "..-.",  // F
  "--.",   // G
  "....",  // H
  "..",    // I
  ".---",  // J
  "-.-",   // K
  ".-..",  // L
  "--",    // M
  "-.",    // N
  "---",   // O
  ".--.",  // P
  "--.-",  // Q
  ".-.",   // R
  "...",   // S
  "-",     // T
  "..-",   // U
  "...-",  // V
  ".--",   // W
  "-..-",  // X
  "-.--",  // Y
  "--.."   // Z
};

const char* morseNumbers[] = {
  "-----", // 0
  ".----", // 1
  "..---", // 2
  "...--", // 3
  "....-", // 4
  ".....", // 5
  "-....", // 6
  "--...", // 7
  "---..", // 8
  "----."  // 9
};

// Function prototypes
void initializeSystem();
void handleEncodingMode();
void handleDecodingMode();
void transmitMorse(char symbol);
void activateOutput(bool state);
void processButtonInput();
String charToMorse(char c);
char morseToChar(String morse);
void displayMessage(String message);
void switchMode();
bool isEndOfMessage(String morse);

void setup() {
  initializeSystem();
}

void loop() {
  unsigned long currentMillis = millis();
  
  if (currentSystemState == ENCODING_MODE) {
    handleEncodingMode();
  } else {
    handleDecodingMode();
  }
}

void initializeSystem() {
  // Initialize pins
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  
  // Initialize LCD
  lcd.begin(16, 2);
  lcd.clear();
  lcd.print("Morse Terminal");
  lcd.setCursor(0, 1);
  lcd.print("Encoding Mode");
  
  // Initialize serial
  Serial.begin(9600);
  Serial.println("Post-Apocalyptic Morse Terminal Ready");
  Serial.println("Send text for encoding...");
  
  // Turn off outputs initially
  activateOutput(false);
  
  previousMillis = millis();
}

void handleEncodingMode() {
  unsigned long currentMillis = millis();
  
  // Check for incoming serial data
  if (Serial.available() > 0 && encodingState == IDLE) {
    String input = Serial.readString();
    input.trim();
    input.toUpperCase();
    
    // Filter to only printable characters (A-Z, 0-9, space)
    encodingMessage = "";
    for (int i = 0; i < input.length(); i++) {
      char c = input.charAt(i);
      if ((c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == ' ') {
        encodingMessage += c;
      }
    }
    
    if (encodingMessage.length() > 0) {
      Serial.println("Encoding: " + encodingMessage);
      messageIndex = 0;
      morseIndex = 0;
      encodingState = IDLE;
      previousMillis = currentMillis;
    }
  }
  
  // Handle transmission state machine
  if (encodingMessage.length() > 0) {
    switch (encodingState) {
      case IDLE:
        if (messageIndex < encodingMessage.length()) {
          char currentChar = encodingMessage.charAt(messageIndex);
          if (currentChar == ' ') {
            // Start word gap
            encodingState = WORD_PAUSE;
            previousMillis = currentMillis;
          } else {
            // Get morse sequence for character
            currentMorseSequence = charToMorse(currentChar);
            if (currentMorseSequence.length() > 0) {
              morseIndex = 0;
              // Start transmitting first symbol
              if (currentMorseSequence.charAt(morseIndex) == '.') {
                encodingState = TRANSMITTING_DOT;
              } else {
                encodingState = TRANSMITTING_DASH;
              }
              activateOutput(true);
              previousMillis = currentMillis;
            } else {
              messageIndex++;
            }
          }
        } else {
          // Message complete, switch to decoding mode
          encodingMessage = "";
          switchMode();
        }
        break;
        
      case TRANSMITTING_DOT:
        if (currentMillis - previousMillis >= DOT_DURATION) {
          activateOutput(false);
          encodingState = SYMBOL_PAUSE;
          previousMillis = currentMillis;
        }
        break;
        
      case TRANSMITTING_DASH:
        if (currentMillis - previousMillis >= DASH_DURATION) {
          activateOutput(false);
          encodingState = SYMBOL_PAUSE;
          previousMillis = currentMillis;
        }
        break;
        
      case SYMBOL_PAUSE:
        if (currentMillis - previousMillis >= SYMBOL_GAP) {
          morseIndex++;
          if (morseIndex < currentMorseSequence.length()) {
            // More symbols in current character
            if (currentMorseSequence.charAt(morseIndex) == '.') {
              encodingState = TRANSMITTING_DOT;
            } else {
              encodingState = TRANSMITTING_DASH;
            }
            activateOutput(true);
            previousMillis = currentMillis;
          } else {
            // Character complete, start letter gap
            encodingState = LETTER_PAUSE;
            previousMillis = currentMillis;
          }
        }
        break;
        
      case LETTER_PAUSE:
        if (currentMillis - previousMillis >= LETTER_GAP) {
          messageIndex++;
          encodingState = IDLE;
        }
        break;
        
      case WORD_PAUSE:
        if (currentMillis - previousMillis >= WORD_GAP) {
          messageIndex++;
          encodingState = IDLE;
        }
        break;
    }
  }
}

void handleDecodingMode() {
  unsigned long currentMillis = millis();
  
  // Handle button input
  bool currentButtonState = digitalRead(BUTTON_PIN);
  
  // Button press detection with debouncing
  if (currentButtonState != lastButtonState) {
    if (currentMillis - buttonReleaseTime > DEBOUNCE_DELAY) {
      if (currentButtonState == LOW && !buttonPressed) {
        // Button just pressed
        buttonPressed = true;
        buttonPressTime = currentMillis;
        activateOutput(true);
      } else if (currentButtonState == HIGH && buttonPressed) {
        // Button just released
        buttonPressed = false;
        buttonReleaseTime = currentMillis;
        activateOutput(false);
        
        // Determine if it was a dot or dash
        unsigned long pressDuration = buttonReleaseTime - buttonPressTime;
        if (pressDuration < (DOT_DURATION + DASH_DURATION) / 2) {
          currentMorseChar += ".";
        } else {
          currentMorseChar += "-";
        }
        
        Serial.print("Input: ");
        Serial.println(currentMorseChar);
      }
    }
    lastButtonState = currentButtonState;
  }
  
  // Check for end of character (timeout) or end of message
  if (!buttonPressed && currentMorseChar.length() > 0) {
    if (currentMillis - buttonReleaseTime > BUTTON_TIMEOUT) {
      // Timeout - end of character
      if (isEndOfMessage(currentMorseChar)) {
        // End of message detected (AR sequence)
        displayMessage(decodedMessage);
        Serial.println("Message received: " + decodedMessage);
        decodedMessage = "";
        switchMode();
      } else {
        // Decode character
        char decodedChar = morseToChar(currentMorseChar);
        if (decodedChar != '\0') {
          decodedMessage += decodedChar;
          lcd.setCursor(0, 1);
          lcd.print("Input: " + String(decodedChar) + "        ");
        }
      }
      currentMorseChar = "";
    }
  }
}

void activateOutput(bool state) {
  digitalWrite(LED_PIN, state ? HIGH : LOW);
  if (state) {
    tone(BUZZER_PIN, BUZZER_FREQ);
  } else {
    noTone(BUZZER_PIN);
  }
}

String charToMorse(char c) {
  if (c >= 'A' && c <= 'Z') {
    return String(morseTable[c - 'A']);
  } else if (c >= '0' && c <= '9') {
    return String(morseNumbers[c - '0']);
  }
  return "";
}

char morseToChar(String morse) {
  // Check letters
  for (int i = 0; i < 26; i++) {
    if (morse.equals(String(morseTable[i]))) {
      return 'A' + i;
    }
  }
  
  // Check numbers
  for (int i = 0; i < 10; i++) {
    if (morse.equals(String(morseNumbers[i]))) {
      return '0' + i;
    }
  }
  
  return '\0';  // Unknown morse code
}

bool isEndOfMessage(String morse) {
  // Check for AR sequence: ".-.-."
  return morse.equals(".-.-.");
}

void displayMessage(String message) {
  lcd.clear();
  lcd.print("Message:");
  lcd.setCursor(0, 1);
  if (message.length() <= 16) {
    lcd.print(message);
  } else {
    lcd.print(message.substring(0, 16));
  }
  delay(3000);  // Show message for 3 seconds
}

void switchMode() {
  if (currentSystemState == ENCODING_MODE) {
    currentSystemState = DECODING_MODE;
    decodingState = WAITING_INPUT;
    lcd.clear();
    lcd.print("Decoding Mode");
    lcd.setCursor(0, 1);
    lcd.print("Press button...");
    Serial.println("Switched to Decoding Mode");
    Serial.println("Use button for Morse input. End with AR (.-.-.)");
  } else {
    currentSystemState = ENCODING_MODE;
    encodingState = IDLE;
    lcd.clear();
    lcd.print("Encoding Mode");
    lcd.setCursor(0, 1);
    lcd.print("Send text...");
    Serial.println("Switched to Encoding Mode");
    Serial.println("Send text for encoding...");
  }
  
  // Reset state variables
  encodingMessage = "";
  decodedMessage = "";
  currentMorseChar = "";
  messageIndex = 0;
  morseIndex = 0;
  buttonPressed = false;
  activateOutput(false);
}