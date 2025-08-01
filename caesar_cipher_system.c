#include <avr/io.h>
#include <avr/eeprom.h>
#include <util/delay.h>
#include <string.h>

#define F_CPU 16000000UL

// LCD pin definitions (assuming 4-bit mode)
#define LCD_DATA_PORT PORTD
#define LCD_DATA_DDR DDRD
#define LCD_DATA_PIN PIND
#define LCD_CTRL_PORT PORTB
#define LCD_CTRL_DDR DDRB
#define LCD_RS PB0
#define LCD_EN PB1

// Keypad pin definitions (4x4 keypad)
#define KEYPAD_PORT PORTC
#define KEYPAD_DDR DDRC
#define KEYPAD_PIN PINC
#define ROW_MASK 0x0F    // PC0-PC3 for rows
#define COL_MASK 0xF0    // PC4-PC7 for columns

// EEPROM addresses
#define KEY_EEPROM_ADDR 0
#define PLAINTEXT_START_ADDR 10

/**
 * Function to write data to EEPROM
 */
void EEPROMwrite(uint16_t address, uint8_t data) {
    while(EECR & (1<<EEPE));
    EEAR = address;
    EEDR = data;
    EECR |= (1<<EEMPE);
    EECR |= (1<<EEPE);
}

/**
 * Function to read data from EEPROM
 */
uint8_t EEPROMread(uint16_t address) {
    while(EECR & (1<<EEPE));
    EEAR = address;
    EECR |= (1<<EERE);
    return EEDR;
}

/**
 * LCD Functions
 */
void LCD_pulse_enable(void) {
    LCD_CTRL_PORT |= (1 << LCD_EN);
    _delay_us(1);
    LCD_CTRL_PORT &= ~(1 << LCD_EN);
    _delay_us(100);
}

void LCD_send_nibble(uint8_t nibble) {
    LCD_DATA_PORT = (LCD_DATA_PORT & 0x0F) | (nibble & 0xF0);
    LCD_pulse_enable();
}

void LCD_send_command(uint8_t command) {
    LCD_CTRL_PORT &= ~(1 << LCD_RS);  // RS = 0 for command
    LCD_send_nibble(command);
    LCD_send_nibble(command << 4);
    _delay_ms(2);
}

void LCD_send_data(uint8_t data) {
    LCD_CTRL_PORT |= (1 << LCD_RS);   // RS = 1 for data
    LCD_send_nibble(data);
    LCD_send_nibble(data << 4);
    _delay_ms(2);
}

void LCD_init(void) {
    LCD_DATA_DDR |= 0xF0;  // Set upper 4 bits as output
    LCD_CTRL_DDR |= (1 << LCD_RS) | (1 << LCD_EN);
    
    _delay_ms(20);
    LCD_send_nibble(0x30);
    _delay_ms(5);
    LCD_send_nibble(0x30);
    _delay_us(100);
    LCD_send_nibble(0x30);
    _delay_ms(2);
    LCD_send_nibble(0x20);  // 4-bit mode
    
    LCD_send_command(0x28); // 4-bit, 2 lines, 5x8 font
    LCD_send_command(0x0C); // Display on, cursor off
    LCD_send_command(0x06); // Increment cursor
    LCD_send_command(0x01); // Clear display
    _delay_ms(2);
}

void LCD_clear(void) {
    LCD_send_command(0x01);
    _delay_ms(2);
}

void LCD_set_cursor(uint8_t row, uint8_t col) {
    uint8_t address = (row == 0) ? 0x80 + col : 0xC0 + col;
    LCD_send_command(address);
}

void LCD_print_string(char* str) {
    while(*str) {
        LCD_send_data(*str);
        str++;
    }
}

void LCD_print_char(char c) {
    LCD_send_data(c);
}

/**
 * Keypad Functions
 */
void keypad_init(void) {
    KEYPAD_DDR = COL_MASK;  // Columns as output, rows as input
    KEYPAD_PORT = ROW_MASK; // Enable pull-ups for rows
}

char keypad_scan(void) {
    char keypad_layout[4][4] = {
        {'1', '2', '3', 'A'},
        {'4', '5', '6', 'B'},
        {'7', '8', '9', 'C'},
        {'*', '0', '#', 'D'}
    };
    
    for(uint8_t col = 0; col < 4; col++) {
        // Set all columns high, then set current column low
        KEYPAD_PORT |= COL_MASK;
        KEYPAD_PORT &= ~(1 << (col + 4));
        
        _delay_us(10);
        
        // Check each row
        for(uint8_t row = 0; row < 4; row++) {
            if(!(KEYPAD_PIN & (1 << row))) {
                // Key pressed, wait for release
                while(!(KEYPAD_PIN & (1 << row)));
                _delay_ms(50); // Debounce
                return keypad_layout[row][col];
            }
        }
    }
    return 0; // No key pressed
}

char keypad_get_key(void) {
    char key = 0;
    while(key == 0) {
        key = keypad_scan();
        _delay_ms(50);
    }
    return key;
}

/**
 * Caesar Cipher Functions
 */
char caesar_encrypt_char(char c, uint8_t key) {
    if(c >= 'A' && c <= 'Z') {
        return ((c - 'A' + key) % 26) + 'A';
    } else if(c >= 'a' && c <= 'z') {
        return ((c - 'a' + key) % 26) + 'a';
    }
    return c; // Non-alphabetic characters remain unchanged
}

void caesar_encrypt_string(char* plaintext, char* ciphertext, uint8_t key) {
    for(int i = 0; i < strlen(plaintext); i++) {
        ciphertext[i] = caesar_encrypt_char(plaintext[i], key);
    }
    ciphertext[strlen(plaintext)] = '\0';
}

/**
 * Menu and Input Functions
 */
uint8_t get_menu_choice(void) {
    LCD_clear();
    LCD_set_cursor(0, 0);
    LCD_print_string("1-Encrypt");
    LCD_set_cursor(1, 0);
    LCD_print_string("2-Change Key");
    
    char choice;
    do {
        choice = keypad_get_key();
    } while(choice != '1' && choice != '2');
    
    return (choice == '1') ? 1 : 2;
}

uint8_t get_caesar_key(void) {
    LCD_clear();
    LCD_set_cursor(0, 0);
    LCD_print_string("Enter Key (0-25):");
    LCD_set_cursor(1, 0);
    
    char key_str[3] = {0};
    uint8_t key_index = 0;
    char key;
    
    do {
        key = keypad_get_key();
        if(key >= '0' && key <= '9' && key_index < 2) {
            key_str[key_index++] = key;
            LCD_print_char(key);
        } else if(key == '#' && key_index > 0) {
            // Enter key pressed
            break;
        } else if(key == '*' && key_index > 0) {
            // Backspace
            key_index--;
            key_str[key_index] = '\0';
            LCD_set_cursor(1, 0);
            LCD_print_string("   "); // Clear line
            LCD_set_cursor(1, 0);
            for(int i = 0; i < key_index; i++) {
                LCD_print_char(key_str[i]);
            }
        }
    } while(1);
    
    uint8_t caesar_key = 0;
    for(int i = 0; i < key_index; i++) {
        caesar_key = caesar_key * 10 + (key_str[i] - '0');
    }
    
    return caesar_key % 26; // Ensure key is in valid range
}

void get_plaintext(char* plaintext) {
    LCD_clear();
    LCD_set_cursor(0, 0);
    LCD_print_string("Enter Text (10ch):");
    LCD_set_cursor(1, 0);
    
    char text_index = 0;
    char key;
    
    while(text_index < 10) {
        key = keypad_get_key();
        
        if((key >= 'A' && key <= 'D') || (key >= '0' && key <= '9')) {
            // Convert keypad input to letters/numbers
            if(key >= '0' && key <= '9') {
                plaintext[text_index] = key;
            } else {
                // A=A, B=B, C=C, D=D for simplicity
                plaintext[text_index] = key;
            }
            LCD_print_char(plaintext[text_index]);
            text_index++;
        } else if(key == '*' && text_index > 0) {
            // Backspace
            text_index--;
            plaintext[text_index] = '\0';
            LCD_set_cursor(1, text_index);
            LCD_print_char(' ');
            LCD_set_cursor(1, text_index);
        } else if(key == '#') {
            // Enter - pad with spaces if needed
            while(text_index < 10) {
                plaintext[text_index++] = ' ';
            }
        }
    }
    plaintext[10] = '\0';
}

void display_encrypted_text(char* ciphertext) {
    LCD_clear();
    LCD_set_cursor(0, 0);
    LCD_print_string("Encrypted:");
    LCD_set_cursor(1, 0);
    LCD_print_string(ciphertext);
    
    // Wait for any key to continue
    keypad_get_key();
}

/**
 * Main Program
 */
int main(void) {
    uint8_t caesar_key;
    char plaintext[11];
    char ciphertext[11];
    uint8_t choice;
    
    // Initialize peripherals
    LCD_init();
    keypad_init();
    
    // Load existing key from EEPROM or set default
    caesar_key = EEPROMread(KEY_EEPROM_ADDR);
    if(caesar_key > 25) {
        caesar_key = 3; // Default key
        EEPROMwrite(KEY_EEPROM_ADDR, caesar_key);
    }
    
    // Welcome message
    LCD_clear();
    LCD_set_cursor(0, 0);
    LCD_print_string("Caesar Cipher");
    LCD_set_cursor(1, 0);
    LCD_print_string("System Ready");
    _delay_ms(2000);
    
    while(1) {
        choice = get_menu_choice();
        
        switch(choice) {
            case 1: // Encrypt
                get_plaintext(plaintext);
                caesar_encrypt_string(plaintext, ciphertext, caesar_key);
                
                // Store plaintext in EEPROM
                for(int i = 0; i < 10; i++) {
                    EEPROMwrite(PLAINTEXT_START_ADDR + i, plaintext[i]);
                }
                
                display_encrypted_text(ciphertext);
                break;
                
            case 2: // Change key
                caesar_key = get_caesar_key();
                EEPROMwrite(KEY_EEPROM_ADDR, caesar_key);
                
                LCD_clear();
                LCD_set_cursor(0, 0);
                LCD_print_string("Key Updated!");
                _delay_ms(1500);
                break;
        }
        
        _delay_ms(500);
    }
    
    return 0;
}