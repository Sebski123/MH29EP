#include "MH29EP.h"

extern char *__brkval;
int freeMemory()
{
    char top;
    return &top - __brkval;
}

int cmpfunc(const void *pa, const void *pb)
{
    // Need to cast the void * to int *
    int *a = (int *)pa;
    int *b = (int *)pb;
    // The comparison
    int diffx = a[0] - b[0];
    if (diffx)
    {
        return diffx;
    }
    return a[1] - b[1];
}

MH29EP::MH29EP(uint8_t SDI, uint8_t SCK, uint8_t CS, uint8_t DC, uint8_t RESET, uint8_t BUSY)
{
    data = SDI;
    clock = SCK;
    chipSelect = CS;
    dc = DC;
    res = RESET;
    busy = BUSY;

    pinMode(data, OUTPUT);
    pinMode(clock, OUTPUT);
    CHIP_SELECT_1;
    pinMode(chipSelect, OUTPUT);
    pinMode(dc, OUTPUT);
    pinMode(res, OUTPUT);
    pinMode(busy, INPUT);

    WIDTH = 128;  //128
    HEIGHT = 296; //296
}

void MH29EP::init(mode mode /*=BlackAndRed*/)
{
    color_mode = mode;
    hwReset(); //Electronic paper IC reset

    writeCommand(0x06); //boost soft start, all default values
    writeData(0x17);    //A
    writeData(0x17);    //B
    writeData(0x17);    //C

    writeCommand(0x04); //Power on
    checkBusy();        //waiting for the electronic paper IC to release the idle signal

    writeCommand(0x00);                           //panel setting
    writeData(mode == BlackAndRed ? 0x0f : 0x1f); //LUT from OTP, Scan up, Shift right, Booster on, don't Reset     Color mode based on "mode"

    writeCommand(0x61); //resolution setting
    writeData(WIDTH);
    writeData(HEIGHT >> 8);
    writeData(HEIGHT & 0xff);

    writeCommand(0X50); //VCOM AND DATA INTERVAL SETTING
    writeData(0x77);    //WBmode:VBDF 17|D7 VBDW 97 VBDB 57   WBRmode:VBDF F7 VBDW 77 VBDB 37  VBDR B7
}
void MH29EP::refresh()
{
    writeCommand(0x12); //DISPLAY REFRESH
    delay(2);           //!!!The delay here is necessary, 200uS at least!!!  datasheet page 15
    checkBusy();
}
void MH29EP::sleep()
{
    writeCommand(0X50); //VCOM AND DATA INTERVAL SETTING
    writeData(0xf7);    //WBmode:VBDF 17|D7 VBDW 97 VBDB 57    WBRmode:VBDF F7 VBDW 77 VBDB 37  VBDR B7

    writeCommand(0X02); //power off
    checkBusy();
}
void MH29EP::clear()
{
    unsigned int i;
    writeCommand(0x10); //Transfer old data
    for (i = 0; i < 4736; i++)
    {
        writeData(0xff);
    }

    writeCommand(0x13); //Transfer new data
    for (i = 0; i < 4736; i++)
    {
        writeData(0xff);
    }
}

void MH29EP::showImage(uint8_t image[], color color)
{
    if (color == Red && color_mode == BlackOnly)
    {
        return;
    }
    unsigned int i;
    writeCommand((color == Black && color_mode == BlackAndRed) ? 0x10 : 0x13); //Transfer new data
    for (i = 0; i < 4736; i++)
    {
        writeData(image[i]);
    }
}
void MH29EP::showImage(const uint8_t image[], color color)
{
    if (color == Red && color_mode == BlackOnly)
    {
        return;
    }
    unsigned int i;
    writeCommand((color == Black && color_mode == BlackAndRed) ? 0x10 : 0x13); //Transfer new data
    for (i = 0; i < 4736; i++)
    {
        writeData(pgm_read_byte(&image[i]));
    }
}
void MH29EP::showImage(uint8_t black_image[], uint8_t red_image[])
{
    if (color_mode == BlackOnly)
    {
        return;
    }
    unsigned int i;
    writeCommand(0x10); //Transfer old data
    for (i = 0; i < 4736; i++)
    {
        writeData(black_image[i]);
    }

    writeCommand(0x13); //Transfer new data
    for (i = 0; i < 4736; i++)
    {
        writeData(red_image[i]);
    }
}
void MH29EP::showImage(const uint8_t black_image[], const uint8_t red_image[])
{
    if (color_mode == BlackOnly)
    {
        return;
    }
    unsigned int i;
    writeCommand(0x10); //Transfer old data
    for (i = 0; i < 4736; i++)
    {
        writeData(pgm_read_byte(&black_image[i]));
    }

    writeCommand(0x13); //Transfer new data
    for (i = 0; i < 4736; i++)
    {
        writeData(pgm_read_byte(&red_image[i]));
    }
}

void MH29EP::drawSquare(int x, int y, int w, int h, color color, bool filled)
{
    if (x < 0 || y < 0 || w < 0 || h < 0 || x + w > WIDTH || y + h > HEIGHT)
    {
        return;
    }
    uint16_t xe = (x + w) - 1;
    uint16_t ye = (y + h) - 1;
    uint16_t xs_bx = x / 8;
    uint16_t xe_bx = (xe + 7) / 8;
    writeCommand(0x91); // partial in
    setPartialRamArea(x, y, xe, ye);
    writeCommand(color == Black ? 0x10 : 0x13);
    for (int16_t y1 = y; y1 <= ye; y1++)
    {
        for (int16_t x1 = xs_bx; x1 < xe_bx; x1++)
        {
            if (filled)
            {
                writeData(0x00); // black is 0x00
            }
            else
            {
                if (y1 == y || y1 == ye)
                {
                    writeData(0x00); // top and bottom are black
                }
                else if (x1 == xs_bx)
                {
                    writeData(0x7f); // leftmost bit is black
                }
                else if (x1 == xe_bx - 1)
                {
                    writeData(0xfe); // rightmost bit is black
                }
                else
                {
                    writeData(0xff); // rest is white
                }
            }
        }
    }
    writeCommand(0x92); // partial out
}

//http://rosettacode.org/wiki/Bitmap/Midpoint_circle_algorithm
void MH29EP::drawCircle(int x0, int y0, int r, color color, bool filled)
{
    if (x0 < 0 || y0 < 0 || r < 0 || x0 + r > WIDTH || y0 + r > HEIGHT || x0 - r < 0 || y0 - r < 0 || r > 40) //max radius is 40 for memory saving this will still take about 1 kB
    {
        return;
    }
    Serial.println(freeMemory());
    int c = (floor(((sqrt(2) * r) + 0.5)) * 4) + 8;
    Serial.println(c);
    int pixels[c][2];
    Serial.println(freeMemory());
    int count = 0;

    auto f = 1 - r;
    auto ddF_x = 0;
    auto ddF_y = -2 * r;
    auto x = 0;
    auto y = r;

    pixels[count][0] = x0;
    pixels[count++][1] = y0 + r;
    pixels[count][0] = x0;
    pixels[count++][1] = y0 - r;
    pixels[count][0] = x0 + r;
    pixels[count++][1] = y0;
    pixels[count][0] = x0 - r;
    pixels[count++][1] = y0;

    while (x < y)
    {
        if (f >= 0)
        {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x + 1;
        pixels[count][0] = x0 + x;
        pixels[count++][1] = y0 + y;
        pixels[count][0] = x0 - x;
        pixels[count++][1] = y0 + y;
        pixels[count][0] = x0 + x;
        pixels[count++][1] = y0 - y;
        pixels[count][0] = x0 - x;
        pixels[count++][1] = y0 - y;
        pixels[count][0] = x0 + y;
        pixels[count++][1] = y0 + x;
        pixels[count][0] = x0 - y;
        pixels[count++][1] = y0 + x;
        pixels[count][0] = x0 + y;
        pixels[count++][1] = y0 - x;
        pixels[count][0] = x0 - y;
        pixels[count++][1] = y0 - x;
    }
    Serial.println(freeMemory());

    qsort(pixels, c, sizeof(pixels[0]), cmpfunc);
    Serial.println("-----------------------------------------------------------------");

    for (size_t i = 0; i < c; i++)
    {
        drawDot(pixels[i][0], pixels[i][1], color);
    }
}

//http://rosettacode.org/wiki/Bitmap/Bresenham%27s_line_algorithm
void MH29EP::drawLine(int sx, int sy, int ex, int ey, color color)
{
    if (sx < 0 || sy < 0 || ex < sx || ey < sy || ex > WIDTH || ey > HEIGHT)
    {
        return;
    }
    uint16_t xs_bx = sx / 8;
    uint16_t xe_bx = (ex + 7) / 8;
    writeCommand(0x91); // partial in
    uint16_t numOfBytes = setPartialRamArea(sx, sy, ex, ey);
    writeCommand(color == Black ? 0x10 : 0x13);
    if (sy == ey)
    {
        for (size_t i = 0; i < numOfBytes; i++)
        {
            writeData(0x00);
        }
    }
    else if (sx == ex)
    {
        for (size_t i = sy; i < ey; i++)
        {
            writeData(0x7f);
        }
    }
    else
    {
        //TODO: handle diagonal lines
    }
    writeCommand(0x92); // partial out
}
void MH29EP::drawDot(int x, int y, color color)
{

    if (x > WIDTH || y > HEIGHT || x < 0 || y < 0)
    {
        return;
    }
    writeCommand(0x91); // partial in
    setPartialRamArea(x, y, x + 1, y + 1);
    writeCommand(color == Black ? 0x10 : 0x13);
    writeData(0x7f);    // only leftmost bit is on
    writeData(0xff);    // only leftmost bit is on
    writeCommand(0x92); // partial out
}

void MH29EP::spiDelay(uint8_t xrate)
{
    uint8_t i;
    while (xrate)
    {
        for (i = 0; i < 2; i++)
            ;
        xrate--;
    }
}
void MH29EP::spiWrite(uint8_t value)
{
    uint8_t i;
    spiDelay(1);
    for (i = 0; i < 8; i++)
    {
        CLOCK_0;
        spiDelay(1);
        if (value & 0x80)
            DATA_1;
        else
            DATA_0;
        value = (value << 1);
        spiDelay(1);
        delayMicroseconds(1);
        CLOCK_1;
        spiDelay(1);
    }
}
void MH29EP::spiRead(uint8_t &valueOut)
{
    uint8_t i;
    CLOCK_0;
    spiDelay(1);
    for (i = 0; i < 8; i++)
    {
        CLOCK_1;
        valueOut |= (READ_DATA << (7 - i));
        spiDelay(1);
        delayMicroseconds(1);
        CLOCK_0;
        spiDelay(1);
    }
}
void MH29EP::readData()
{
    uint8_t _;
    DATA_INPUT;
    spiDelay(1);
    CHIP_SELECT_0;
    DC_1;
    spiRead(_);
    CHIP_SELECT_1;
    DATA_OUTPUT;
}
void MH29EP::readData(uint8_t &dataOut)
{
    dataOut = 0;
    DATA_INPUT;
    spiDelay(1);
    CHIP_SELECT_0;
    DC_1;
    spiRead(dataOut);
    CHIP_SELECT_1;
    DATA_OUTPUT;
}
void MH29EP::writeData(uint8_t data)
{
    spiDelay(1);
    CHIP_SELECT_0;
    DC_1; // command write
    spiWrite(data);
    CHIP_SELECT_1;
}
void MH29EP::writeCommand(uint8_t command)
{
    spiDelay(1);
    CHIP_SELECT_0;
    DC_0; // command write
    spiWrite(command);
    CHIP_SELECT_1;
}
void MH29EP::hwReset(void)
{
    RST_0;       // Module reset
    delay(1000); //At least 10ms delay
    RST_1;
    delay(1000);
}
void MH29EP::checkBusy(void)
{
    uint8_t busy;
    do
    {
        busy = 0;
        writeCommand(0x71);
        readData(busy);
    } while (!(busy & 0x01));
    delay(200);
}
uint16_t MH29EP::setPartialRamArea(uint16_t x, uint16_t y, uint16_t xe, uint16_t ye)
{
    while (ye <= y)
    {
        ye++; //VRED must be greater than VRST
    }

    x &= 0xFFF8;            // byte boundary, last three bits must be 0
    xe = (xe - 1) | 0x0007; // byte boundary - 1, last three bits must be 1
    writeCommand(0x90);     // partial window
    writeData(x % 256);
    writeData(xe % 256);
    writeData(y / 256);
    writeData(y % 256);
    writeData(ye / 256);
    writeData(ye % 256);
    writeData(0x00);
    return (7 + xe - x) / 8; // number of bytes to transfer per line
}