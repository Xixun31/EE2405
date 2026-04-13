#include "mbed.h"

// SPI Pins: MOSI = D11, MISO = NC (Not Connected), SCK = D13
SPI spi(D11, NC, D13);   
// Chip Select Pin
DigitalOut cs(D10);      

// ---------- MAX7219 Registers  ----------
constexpr uint8_t NOOP      = 0x00;
constexpr uint8_t DECODE    = 0x09;
constexpr uint8_t INTENSITY = 0x0A;
constexpr uint8_t SCAN      = 0x0B;
constexpr uint8_t SHUTDOWN  = 0x0C;
constexpr uint8_t TEST      = 0x0F;

// ---------- Display Data for 8x32 (4 modules) ----------
// Includes: Heart, Smiley, Diamond, and Checkerboard patterns
uint8_t pattern[32] = {
    // Module 1 (Far Left) - Heart
    0x00, 0x66, 0xFF, 0xFF, 0x7E, 0x3C, 0x18, 0x00,
    // Module 2 - Smiley Face
    0x3C, 0x42, 0xA5, 0x81, 0xA5, 0x99, 0x42, 0x3C,
    // Module 3 - Diamond Wave
    0x18, 0x3C, 0x7E, 0xFF, 0x7E, 0x3C, 0x18, 0x00,
    // Module 4 (Far Right) - Checkerboard
    0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55
};

/**
 * Sends the same command and data to all 4 cascaded modules.
 * Used primarily for initialization settings.
 */
void write_all(uint8_t reg, uint8_t data) {
    cs = 0;                 // Select the chip
    for (int i = 0; i < 4; i++) {
        spi.write(reg);     // Send register address
        spi.write(data);    // Send data value
    }
    cs = 1;                 // Deselect the chip to latch data
}

/**
 * Initial configuration for the 8x32 dot matrix display.
 */
void init_max7219() {
    // SPI configuration: 8-bit data, Mode 0 (CPOL=0, CPHA=0)
    spi.format(8, 0);         
    // Set frequency to 1MHz (Safe and stable for MAX7219)
    spi.frequency(1000000);   
    
    write_all(SHUTDOWN, 0x01);   // Wake up all modules (Normal operation)
    write_all(DECODE, 0x00);     // Set to No-Decode mode (Direct LED control)
    write_all(INTENSITY, 0x03);  // Set brightness (0x00 to 0x0F)
    write_all(SCAN , 0x07);  // Scan all 8 rows (0-7)
    write_all(TEST, 0x00);// Turn off Display Test mode
}

int main() {
    // Initialize the hardware
    init_max7219();

    while (true) {
        // Update display line by line (Rows 1 to 8)
        for (int row = 1; row <= 8; row++) {
            cs = 0;
            // For cascaded modules, we send data in the order of Module 4 -> 3 -> 2 -> 1.
            // Data "overflows" from the first module to the last.
            for (int m = 0; m < 4 ; m++) {
                spi.write(row);                         // Select the current row
                spi.write(pattern[m * 8 + row -1]); // Send the specific pixel data
            }
            cs = 1; // Latch the data to the LEDs
        }

        ThisThread::sleep_for(500ms); 
    }
}
