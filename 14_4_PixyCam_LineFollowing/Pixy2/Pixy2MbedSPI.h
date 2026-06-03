#ifndef PIXY2_MBED_SPI_H
#define PIXY2_MBED_SPI_H

#include "mbed.h"
#include <memory>


class PixySerialCompat {
public:
    void begin(int) {}
    void print(const char *s) { printf("%s", s); }
    void println(const char *s) { printf("%s\n", s); }
};

static PixySerialCompat Serial;

inline uint32_t millis()
{
    return (uint32_t)std::chrono::duration_cast<std::chrono::milliseconds>(
               Kernel::Clock::now().time_since_epoch())
        .count();
}

inline void delayMicroseconds(uint32_t us)
{
    wait_us(us);
}

#include "TPixy2.h"

#define PIXY_SPI_CLOCKRATE 2000000

class Link2MbedSPI {
public:
    Link2MbedSPI() : _spi(NULL), _cs(NULL) {}

    void bind(SPI &spi, DigitalOut &cs)
    {
        _spi = &spi;
        _cs = &cs;
    }

    int8_t open(uint32_t arg)
    {
        if (_spi == NULL || _cs == NULL) {
            return PIXY_RESULT_ERROR;
        }
        // Configure SPI: 2MHz, SPI mode 3 (CPOL=1, CPHA=1)
        _spi->frequency(PIXY_SPI_CLOCKRATE);
        _spi->format(8, 3);  // 8 bits, mode 3
        *_cs = 1;  // CS idle high
        return 0;
    }

    void close() {}

    int16_t recv(uint8_t *buf, uint8_t len, uint16_t *cs = NULL)
    {
        if (_spi == NULL || _cs == NULL) {
            return PIXY_RESULT_ERROR;
        }

        if (cs) {
            *cs = 0;
        }

        *_cs = 0;  // CS low
        for (uint8_t i = 0; i < len; i++) {
            buf[i] = _spi->write(0x00);
            if (cs) {
                *cs += buf[i];
            }
        }
        *_cs = 1;  // CS high

        return len;
    }

    int16_t send(uint8_t *buf, uint8_t len)
    {
        if (_spi == NULL || _cs == NULL) {
            return PIXY_RESULT_ERROR;
        }

        *_cs = 0;  // CS low
        for (uint8_t i = 0; i < len; i++) {
            _spi->write(buf[i]);
        }
        *_cs = 1;  // CS high

        return len;
    }

private:
    SPI *_spi;
    DigitalOut *_cs;
};

class Pixy2MbedSPI : public TPixy2<Link2MbedSPI> {
public:
    explicit Pixy2MbedSPI(SPI &spi, DigitalOut &cs) : TPixy2<Link2MbedSPI>()
    {
        this->m_link.bind(spi, cs);
    }

    Pixy2MbedSPI(PinName mosi, PinName miso, PinName sclk, PinName cs) : TPixy2<Link2MbedSPI>()
    {
        _ownedSpi = std::unique_ptr<SPI>(new SPI(mosi, miso, sclk));
        _ownedCs = std::unique_ptr<DigitalOut>(new DigitalOut(cs));
        this->m_link.bind(*_ownedSpi, *_ownedCs);
    }

private:
    std::unique_ptr<SPI> _ownedSpi;
    std::unique_ptr<DigitalOut> _ownedCs;
};

#endif
