# AVR EEPROM and Caesar Cipher Projects

This repository contains four C programs for AVR microcontrollers (specifically ATmega328P) that demonstrate EEPROM operations and implement a Caesar cipher encryption system.

## Project Structure

1. **eeprom_functions.c** - Basic EEPROM read/write functions
2. **serial_to_eeprom.c** - Serial input to EEPROM storage program
3. **eeprom_to_serial.c** - EEPROM content reader and serial output program
4. **caesar_cipher_system.c** - Complete Caesar cipher encryption system with LCD and keypad

## Hardware Requirements

- AVR microcontroller (ATmega328P recommended)
- 16MHz crystal oscillator
- 16x2 LCD display (HD44780 compatible)
- 4x4 matrix keypad
- UART/USB-to-serial converter for programs 2 and 3
- Pull-up resistors for keypad rows
- Standard LCD connection resistors and potentiometer for contrast

## Pin Connections

### LCD Connections (4-bit mode):
- **Data pins**: PD4-PD7 (LCD D4-D7)
- **RS (Register Select)**: PB0
- **Enable**: PB1
- **VSS, VDD**: Ground and +5V
- **V0**: Contrast control (via potentiometer)

### Keypad Connections (4x4):
- **Rows**: PC0-PC3 (with pull-up resistors)
- **Columns**: PC4-PC7

### Serial Communication:
- **TX**: PD1 (Arduino pin 1)
- **RX**: PD0 (Arduino pin 0)

## Program Descriptions

### 1. EEPROM Functions (eeprom_functions.c)

Basic library functions for EEPROM operations:

- `EEPROMwrite(uint16_t address, uint8_t data)`: Writes a byte to specified EEPROM address
- `EEPROMread(uint16_t address)`: Reads a byte from specified EEPROM address

**Features:**
- Handles EEPROM write completion checking
- Supports full EEPROM address range (0-1023 for ATmega328P)
- Includes proper register setup and timing

### 2. Serial to EEPROM (serial_to_eeprom.c)

Program that receives ASCII sentences via serial port and stores them in EEPROM.

**Features:**
- 9600 baud UART communication
- Receives characters until carriage return (\r)
- Stores each character sequentially in EEPROM
- Provides user feedback via serial echo
- Automatic address wrapping to prevent overflow
- Prompts for new sentences after each completion

**Usage:**
1. Connect serial terminal at 9600 baud
2. Type sentences ending with carriage return
3. Program confirms storage and prompts for next sentence

### 3. EEPROM to Serial (eeprom_to_serial.c)

Program that reads the first 1024 bytes of EEPROM and displays them via serial port in hex dump format.

**Features:**
- Formatted hex dump output with addresses
- ASCII representation of printable characters
- 16 bytes per line formatting
- Complete 1024-byte EEPROM scan
- Non-printable characters shown as dots

**Output Format:**
```
Address  Data (Hex)                    ASCII
-------- ------------------------------- ----------------
0000: 48 65 6C 6C 6F 20 57 6F 72 6C 64 21 0D 00 00 00 | Hello World!....
0010: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 | ................
```

### 4. Caesar Cipher System (caesar_cipher_system.c)

Complete encryption system with LCD interface and keypad input.

**Features:**
- Menu-driven interface on LCD
- Caesar cipher encryption (shift cipher)
- EEPROM storage for encryption key
- Real-time text input via keypad
- 10-character plaintext limit
- Key management (0-25 shift values)
- No PC communication required after programming

**Menu Options:**
1. **Encrypt**: Enter 10-character plaintext and see encrypted result
2. **Change Key**: Modify the Caesar cipher shift value (0-25)

**Keypad Layout:**
```
1 2 3 A
4 5 6 B
7 8 9 C
* 0 # D
```

**Controls:**
- **Numbers (0-9)**: Input characters/key values
- **Letters (A-D)**: Input alphabetic characters
- **# (Hash)**: Enter/Confirm
- **\* (Star)**: Backspace

**Operation Flow:**
1. System displays welcome message
2. Main menu appears with options
3. User selects encrypt or change key
4. For encryption: enter 10 characters, view encrypted result
5. For key change: enter new key (0-25), confirm update
6. Return to main menu

## Compilation Instructions

For AVR-GCC compiler:

```bash
# Compile individual programs
avr-gcc -mmcu=atmega328p -DF_CPU=16000000UL -Os -o program.elf program.c
avr-objcopy -O ihex program.elf program.hex

# Upload to microcontroller (using avrdude)
avrdude -p atmega328p -c usbasp -U flash:w:program.hex
```

## EEPROM Memory Map

- **Address 0**: Caesar cipher key (caesar_cipher_system.c)
- **Addresses 1-9**: Reserved
- **Addresses 10-19**: Plaintext storage (caesar_cipher_system.c)
- **Addresses 20+**: Available for serial input storage (serial_to_eeprom.c)

## Notes

- All programs use 16MHz crystal frequency
- EEPROM has limited write cycles (~100,000), suitable for normal use
- LCD initialization includes proper timing for HD44780 compatibility
- Keypad debouncing implemented for reliable input
- Serial programs use standard 8N1 format at 9600 baud
- Caesar cipher preserves case and ignores non-alphabetic characters

## Troubleshooting

1. **LCD not displaying**: Check connections, contrast setting, and power supply
2. **Keypad not responding**: Verify pull-up resistors on row pins
3. **Serial communication issues**: Confirm baud rate and pin connections
4. **EEPROM data corruption**: Ensure proper write completion checking

## Educational Objectives

These programs demonstrate:
- Low-level EEPROM operations in AVR
- UART serial communication programming
- LCD interfacing in 4-bit mode
- Matrix keypad scanning techniques
- Caesar cipher implementation
- Menu-driven embedded system design
- Memory management in microcontrollers