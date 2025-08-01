#include <avr/io.h>
#include <avr/eeprom.h>

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