# Post-Apocalyptic Morse Code Terminal

A robust bidirectional Morse code communication system designed for survival scenarios where traditional communication infrastructure has failed. This terminal enables encoding of text messages into Morse code audio/visual signals and decoding of button-input Morse code back into readable text.

## Features

- **Bidirectional Communication**: Alternates between encoding and decoding modes
- **Non-blocking Design**: Uses millis() for timing instead of delay() functions
- **Visual & Audio Feedback**: LED and piezo buzzer for signal output
- **LCD Display**: Real-time feedback and message display
- **International Morse Code**: Supports A-Z letters and 0-9 numbers
- **End-of-Message Detection**: Uses AR (· − · − ·) sequence to terminate messages

## Hardware Requirements

### Components List

| Component | Quantity | Specification | Purpose |
|-----------|----------|---------------|---------|
| Arduino Uno | 1 | ATmega328P microcontroller | Main processing unit |
| LCD Display | 1 | 16x2 HD44780 compatible | Message display |
| Push Button | 1 | Momentary contact | Morse code input |
| LED | 1 | 5mm standard LED | Visual signal indicator |
| Piezo Buzzer | 1 | Active buzzer, 800Hz | Audio signal output |
| Resistors | 3 | 220Ω, 10kΩ | LED current limiting, button pull-up |
| Breadboard | 1 | Half-size or larger | Circuit prototyping |
| Jumper Wires | ~20 | Male-to-male | Connections |
| USB Cable | 1 | USB A to B | Power and serial communication |

### Circuit Connections

#### LCD Display (HD44780)
- **VSS** → Ground
- **VDD** → 5V
- **V0** → Ground (or potentiometer for contrast)
- **RS** → Digital Pin 12
- **Enable** → Digital Pin 11
- **D4** → Digital Pin 5
- **D5** → Digital Pin 4
- **D6** → Digital Pin 3
- **D7** → Digital Pin 2
- **A** → 5V (backlight anode)
- **K** → Ground (backlight cathode)

#### LED Circuit
- **Anode** → Digital Pin 13 (built-in LED can also be used)
- **Cathode** → 220Ω resistor → Ground

#### Piezo Buzzer
- **Positive** → Digital Pin 8
- **Negative** → Ground

#### Push Button
- **One terminal** → Digital Pin 7
- **Other terminal** → Ground
- **Pull-up resistor** → 10kΩ between Pin 7 and 5V (or use INPUT_PULLUP)

#### Power
- **5V** → Arduino 5V pin
- **Ground** → Arduino GND pin

## Software Architecture

### System States

The terminal operates in two alternating modes:

1. **Encoding Mode**: Receives text via USB serial and transmits as Morse code
2. **Decoding Mode**: Receives Morse code via button input and displays decoded text

### Timing System

The code uses a non-blocking timing approach with `millis()` to avoid system delays:

```c
// Timing constants based on international Morse standards
#define DOT_DURATION 200        // 200ms for dots
#define DASH_DURATION 600       // 3x dot duration for dashes
#define SYMBOL_GAP 200          // Gap between dots/dashes
#define LETTER_GAP 600          // Gap between letters
#define WORD_GAP 1400           // Gap between words
```

### State Machines

#### Encoding State Machine
- **IDLE**: Waiting for input or ready to transmit next character
- **TRANSMITTING_DOT**: Currently outputting a dot signal
- **TRANSMITTING_DASH**: Currently outputting a dash signal  
- **SYMBOL_PAUSE**: Gap between symbols within a character
- **LETTER_PAUSE**: Gap between characters
- **WORD_PAUSE**: Gap between words

#### Decoding State Machine
- **WAITING_INPUT**: Ready to receive button input
- **BUTTON_PRESSED**: Button currently pressed
- **PROCESSING_INPUT**: Analyzing button press duration
- **DISPLAYING_RESULT**: Showing decoded message

### Morse Code Lookup Tables

The system uses two lookup tables for character conversion:

```c
// Letters A-Z
const char* morseTable[] = {
  ".-", "-...", "-.-.", "-..", ".",      // A-E
  "..-.", "--.", "....", "..", ".---",   // F-J
  // ... etc
};

// Numbers 0-9  
const char* morseNumbers[] = {
  "-----", ".----", "..---", "...--", "....-",  // 0-4
  ".....", "-....", "--...", "---..", "----."   // 5-9
};
```

### Key Functions

#### `activateOutput(bool state)`
Controls both LED and buzzer simultaneously for consistent audio-visual feedback.

#### `charToMorse(char c)` & `morseToChar(String morse)`
Bidirectional conversion between ASCII characters and Morse code strings.

#### `handleEncodingMode()` & `handleDecodingMode()`
Main state machine handlers for each operational mode.

#### `isEndOfMessage(String morse)`
Detects the AR sequence (· − · − ·) to terminate message input.

## Operation Instructions

### Setup
1. Connect all components according to the circuit diagram
2. Upload the `morse_terminal.ino` sketch to the Arduino
3. Open Serial Monitor at 9600 baud rate
4. System starts in Encoding Mode

### Encoding Mode
1. Type text (A-Z, 0-9, spaces) into Serial Monitor
2. Press Enter to send
3. Watch LED and listen to buzzer for Morse output
4. System automatically switches to Decoding Mode after transmission

### Decoding Mode
1. Use push button to input Morse code:
   - **Short press** (< 400ms): Dot (·)
   - **Long press** (≥ 400ms): Dash (−)
2. Wait 1 second between characters for auto-decode
3. Send AR sequence (· − · − ·) to end message
4. Decoded message displays on LCD
5. System automatically switches back to Encoding Mode

### Timing Guidelines
- **Dot**: 200ms
- **Dash**: 600ms (3x dot)
- **Symbol gap**: 200ms
- **Letter gap**: 600ms
- **Word gap**: 1400ms
- **Button timeout**: 1000ms

## Morse Code Reference

### Letters
| A | · − | B | − · · · | C | − · − · | D | − · · |
|---|-----|---|---------|---|---------|---|-------|
| E | · | F | · · − · | G | − − · | H | · · · · |
| I | · · | J | · − − − | K | − · − | L | · − · · |
| M | − − | N | − · | O | − − − | P | · − − · |
| Q | − − · − | R | · − · | S | · · · | T | − |
| U | · · − | V | · · · − | W | · − − | X | − · · − |
| Y | − · − − | Z | − − · · |

### Numbers
| 0 | − − − − − | 1 | · − − − − | 2 | · · − − − |
|---|-----------|---|-----------|---|-----------|
| 3 | · · · − − | 4 | · · · · − | 5 | · · · · · |
| 6 | − · · · · | 7 | − − · · · | 8 | − − − · · |
| 9 | − − − − · |

### Special Sequences
- **AR** (End of Message): · − · − ·

## Troubleshooting

### Common Issues

1. **No LCD Display**
   - Check wiring connections
   - Verify contrast setting (V0 pin)
   - Ensure 5V power supply

2. **Button Not Responding**
   - Check button wiring
   - Verify pull-up resistor connection
   - Test button continuity

3. **No Audio/Visual Output**
   - Check LED and buzzer connections
   - Verify pin assignments in code
   - Test components individually

4. **Serial Communication Issues**
   - Ensure 9600 baud rate
   - Check USB cable connection
   - Verify driver installation

### Timing Adjustments

If timing seems off, adjust these constants in the code:
```c
#define DOT_DURATION 200        // Increase for slower transmission
#define BUTTON_TIMEOUT 1000     // Increase for slower input
#define DEBOUNCE_DELAY 50       // Adjust for button responsiveness
```

## Development Timeline

### Phase 1: Hardware Setup (2-3 hours)
- [ ] Assemble circuit on breadboard
- [ ] Test individual components
- [ ] Verify all connections
- [ ] Test basic Arduino functionality

### Phase 2: Core Software Development (4-5 hours)
- [ ] Implement Morse lookup tables
- [ ] Develop non-blocking timing system
- [ ] Create encoding state machine
- [ ] Add serial input handling
- [ ] Test encoding functionality

### Phase 3: Decoding Implementation (3-4 hours)
- [ ] Implement button input handling
- [ ] Create decoding state machine
- [ ] Add timeout detection
- [ ] Implement AR sequence detection
- [ ] Test decoding functionality

### Phase 4: Integration & Testing (2-3 hours)
- [ ] Integrate both modes
- [ ] Add mode switching logic
- [ ] Implement LCD display updates
- [ ] Comprehensive system testing
- [ ] Debug and optimize

### Phase 5: Documentation & Finalization (1-2 hours)
- [ ] Create circuit diagrams
- [ ] Write operation manual
- [ ] Document troubleshooting guide
- [ ] Final code cleanup

**Total Estimated Time: 12-17 hours**

## Future Enhancements

1. **EEPROM Storage**: Save/load preset messages
2. **Speed Control**: Adjustable transmission speed
3. **Extended Character Set**: Support punctuation marks
4. **RF Module**: Wireless transmission capability
5. **Battery Power**: Portable operation
6. **Multiple Channels**: Frequency selection
7. **Message Logging**: Store transmitted/received messages

## Circuit Diagram

```
                    Arduino Uno
                   ┌─────────────┐
                   │             │
    LCD RS  ──────│ 12          │
    LCD E   ──────│ 11          │
                   │             │
    LCD D4  ──────│ 5           │
    LCD D5  ──────│ 4           │
    LCD D6  ──────│ 3           │
    LCD D7  ──────│ 2           │
                   │             │
    LED     ──────│ 13          │
    Buzzer  ──────│ 8           │
    Button  ──────│ 7           │
                   │             │
    5V      ──────│ 5V          │
    GND     ──────│ GND         │
                   │             │
    USB     ──────│ USB         │
                   └─────────────┘
```

This Morse code terminal provides a robust communication solution for post-apocalyptic scenarios, maintaining simplicity while ensuring reliability when conventional communication systems have failed.