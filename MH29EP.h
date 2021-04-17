/*
    see: https://cursedhardware.github.io/epd-driver-ic/UC8151d.pdf for documentation
    https://cursedhardware.github.io/epd-driver-ic/UC8151c.pdf might also be useful 
*/

#ifndef mh29ep_h
#define mh29ep_h

#include "Arduino.h"

#define DATA_0 digitalWrite(data, LOW)
#define DATA_1 digitalWrite(data, HIGH)

#define DATA_OUTPUT pinMode(data, OUTPUT)
#define DATA_INPUT pinMode(data, INPUT)

#define CLOCK_0 digitalWrite(clock, LOW)
#define CLOCK_1 digitalWrite(clock, HIGH)

#define CHIP_SELECT_0 digitalWrite(chipSelect, LOW)
#define CHIP_SELECT_1 digitalWrite(chipSelect, HIGH)

#define DC_0 digitalWrite(dc, LOW)
#define DC_1 digitalWrite(dc, HIGH)

#define RST_0 digitalWrite(res, LOW)
#define RST_1 digitalWrite(res, HIGH)

#define READ_BUSY digitalRead(busy)
#define READ_DATA digitalRead(data)


enum mode
{
    BlackAndRed,
    BlackOnly
};

enum color
{
    Black,
    Red
};

class MH29EP
{
public:
    uint16_t WIDTH, HEIGHT;
    MH29EP(uint8_t SDI, uint8_t SCK, uint8_t CS, uint8_t DC, uint8_t RESET, uint8_t BUSY);

    void init(mode mode = BlackAndRed);
    void refresh();
    void sleep();
    void clear();

    void showImage(uint8_t black_image[]);
    void showImage(const uint8_t black_image[]);
    void showImage(uint8_t black_image[], uint8_t red_image[]);
    void showImage(const uint8_t black_image[], const uint8_t red_image[]);

    void drawSquare(int x, int y, int w, int h, color, bool filled);
    void drawCircle(int x, int y, int r, color, bool filled);
    void drawLine(int sx, int sy, int ex, int ey, color);
    void drawDot(int x, int y, color color);

    void readData();
    void readData(uint8_t &data);
    void writeData(uint8_t data);
    void writeCommand(uint8_t command);
    void checkBusy(void);

private:
    uint8_t data, clock, chipSelect, dc, res, busy;

    void spiDelay(uint8_t xrate);
    void spiRead(uint8_t &value);
    void spiWrite(uint8_t value);
    void hwReset(void);
    uint16_t setPartialRamArea(uint16_t x, uint16_t y, uint16_t xe, uint16_t ye);
};

#endif