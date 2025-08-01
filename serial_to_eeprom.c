#include <avr/io.h>
#include <avr/eeprom.h>
#include <util/delay.h>

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
 * Receive a character from UART
 */
char UART_receive(void) {
    // Wait for data to be received
    while(!(UCSR0A & (1<<RXC0)));
    
    // Get and return received data from buffer
    return UDR0;
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

int main(void) {
    char received_char;
    uint16_t eeprom_address = 0;
    
    // Initialize UART
    UART_init();
    
    // Send prompt message
    UART_transmit_string("Enter ASCII sentence (end with \\r): ");
    
    while(1) {
        // Receive character from serial port
        received_char = UART_receive();
        
        // Echo the character back to confirm reception
        UART_transmit(received_char);
        
        // Write character to EEPROM at current address
        EEPROMwrite(eeprom_address, received_char);
        
        // Check if carriage return received (end of sentence)
        if(received_char == '\r') {
            UART_transmit_string("\\nSentence stored in EEPROM!\\n");
            UART_transmit_string("Enter ASCII sentence (end with \\r): ");
            eeprom_address = 0; // Reset address for next sentence
        } else {
            eeprom_address++; // Move to next EEPROM address
            
            // Prevent EEPROM overflow (assuming 1024 bytes EEPROM)
            if(eeprom_address >= 1024) {
                eeprom_address = 0;
            }
        }
    }
    
    return 0;
}