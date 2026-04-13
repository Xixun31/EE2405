#include "mbed.h"

// ---------- SPI PINS  ----------
SPI spi(D11, NC, D13);   // MOSI, MISO (NC), SCK
DigitalOut cs(D10);      // Chip Select

// ---------- MAX7219 Registers ----------
constexpr uint8_t NOOP      = 0x00;
constexpr uint8_t DECODE    = 0x09;
constexpr uint8_t INTENSITY = 0x0A;
constexpr uint8_t SCAN      = 0x0B;
constexpr uint8_t SHUTDOWN  = 0x0C;
constexpr uint8_t TEST      = 0x0F;

// ---------- Display Data ----------
uint8_t pattern[8] = {
    0x18, 0x3C, 0x7E, 0xFF,
    0x3C, 0x3C, 0x3C, 0x3C
};

// ---------- Send Command to MAX7219 ----------
void max7219_write(uint8_t reg, uint8_t value)
{
    cs = 0;
    spi.write(reg);
    spi.write(value);
    cs = 1;
}

// ---------- Clear Display ----------
void clear_display()
{
    for (int i = 1; i <= 8; i++) {
        max7219_write(i, 0x00);
    }
}

// ---------- Initialization ----------
void init_max7219() {

    // SPI configuration
    spi.format(8, 0);        // 8 bits, SPI mode 0
    spi.frequency(1000000);  // 1MHz  
    
    max7219_write(SHUTDOWN, 0x01);   // Wake up all modules (Normal operation)
    max7219_write(DECODE, 0x00);     // Set to No-Decode mode (Direct LED control)
    max7219_write(INTENSITY, 0x03);  // Set brightness (0x00 to 0x0F)
    max7219_write(SCAN, 0x07);  // Scan all 8 rows (0-7)
    max7219_write(TEST, 0x00);// Turn off Display Test mode

    clear_display();
}

int main()
{
    
    init_max7219();

    while (true) {
        for (int i = 0; i < 8; i++) {
            max7219_write(i + 1, pattern[i]);
        }
        ThisThread::sleep_for(500ms);
    }
}

