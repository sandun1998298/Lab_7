#include <avr/io.h>
#include <avr/eeprom.h>
#include <util/delay.h>
#include <stdio.h>

#define F_CPU 16000000UL
#define BAUD 9600
#define UBRR_VALUE ((F_CPU/(16UL*BAUD))-1)

/**
 * Function to write data to EEPROM
 * @param address: EEPROM address (0-1023 for ATmega328P)
 * @param data: 8-bit data to write
 */
void EEPROMwrite(uint16_t address, uint8_t data) {
    // Wait for completion of previous write
    while(EECR & (1<<EEPE));
    
    // Set up address and data registers
    EEAR = address;
    EEDR = data;
    
    // Write logical one to EEMPE
    EECR |= (1<<EEMPE);
    
    // Start eeprom write by setting EEPE
    EECR |= (1<<EEPE);
}

/**
 * Function to read data from EEPROM
 * @param address: EEPROM address (0-1023 for ATmega328P)
 * @return: 8-bit data from EEPROM
 */
uint8_t EEPROMread(uint16_t address) {
    // Wait for completion of previous write
    while(EECR & (1<<EEPE));
    
    // Set up address register
    EEAR = address;
    
    // Start eeprom read by writing EERE
    EECR |= (1<<EERE);
    
    // Return data from data register
    return EEDR;
}

/**
 * Initialize UART for serial communication
 */
void UART_init(void) {
    // Set baud rate
    UBRR0H = (uint8_t)(UBRR_VALUE >> 8);
    UBRR0L = (uint8_t)UBRR_VALUE;
    
    // Enable receiver and transmitter
    UCSR0B = (1<<RXEN0) | (1<<TXEN0);
    
    // Set frame format: 8 data bits, 1 stop bit
    UCSR0C = (1<<UCSZ01) | (1<<UCSZ00);
}

/**
 * Transmit a character via UART
 */
void UART_transmit(char data) {
    // Wait for empty transmit buffer
    while(!(UCSR0A & (1<<UDRE0)));
    
    // Put data into buffer, sends the data
    UDR0 = data;
}

/**
 * Transmit a string via UART
 */
void UART_transmit_string(char* str) {
    while(*str) {
        UART_transmit(*str);
        str++;
    }
}

/**
 * Transmit a hexadecimal value as string
 */
void UART_transmit_hex(uint8_t value) {
    char hex_chars[] = "0123456789ABCDEF";
    UART_transmit(hex_chars[(value >> 4) & 0x0F]);
    UART_transmit(hex_chars[value & 0x0F]);
}

/**
 * Transmit a 16-bit address as hexadecimal string
 */
void UART_transmit_address(uint16_t address) {
    UART_transmit_hex((uint8_t)(address >> 8));
    UART_transmit_hex((uint8_t)(address & 0xFF));
}

int main(void) {
    uint16_t address;
    uint8_t data;
    uint8_t bytes_per_line = 16;
    uint8_t byte_count = 0;
    
    // Initialize UART
    UART_init();
    
    // Send header message
    UART_transmit_string("EEPROM Content Dump (First 1024 bytes):\\r\\n");
    UART_transmit_string("Address  Data (Hex)                    ASCII\\r\\n");
    UART_transmit_string("-------- ------------------------------- ----------------\\r\\n");
    
    // Read and display first 1024 bytes of EEPROM
    for(address = 0; address < 1024; address++) {
        // Print address at the beginning of each line
        if(byte_count == 0) {
            UART_transmit_address(address);
            UART_transmit_string(": ");
        }
        
        // Read data from EEPROM
        data = EEPROMread(address);
        
        // Print hex value
        UART_transmit_hex(data);
        UART_transmit(' ');
        
        byte_count++;
        
        // Print ASCII representation and newline every 16 bytes
        if(byte_count == bytes_per_line) {
            UART_transmit_string(" | ");
            
            // Print ASCII representation
            for(uint16_t i = address - bytes_per_line + 1; i <= address; i++) {
                uint8_t ascii_data = EEPROMread(i);
                if(ascii_data >= 32 && ascii_data <= 126) {
                    UART_transmit(ascii_data); // Printable ASCII
                } else {
                    UART_transmit('.'); // Non-printable character
                }
            }
            
            UART_transmit_string("\\r\\n");
            byte_count = 0;
        }
        
        // Small delay to prevent overwhelming the serial port
        _delay_ms(1);
    }
    
    // If last line is incomplete, fill with spaces and print ASCII
    if(byte_count != 0) {
        // Fill remaining hex positions with spaces
        for(uint8_t i = byte_count; i < bytes_per_line; i++) {
            UART_transmit_string("   ");
        }
        
        UART_transmit_string(" | ");
        
        // Print ASCII for incomplete line
        for(uint16_t i = address - byte_count; i < address; i++) {
            uint8_t ascii_data = EEPROMread(i);
            if(ascii_data >= 32 && ascii_data <= 126) {
                UART_transmit(ascii_data);
            } else {
                UART_transmit('.');
            }
        }
        
        UART_transmit_string("\\r\\n");
    }
    
    UART_transmit_string("\\r\\nEEPROM dump complete!\\r\\n");
    
    // Keep the program running
    while(1) {
        _delay_ms(1000);
    }
    
    return 0;
}