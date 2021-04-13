#include "MH29EP.h"

MH29EP::MH29EP(uint8_t SDI, uint8_t SCK, uint8_t CS, uint8_t DC, uint8_t RESET, uint8_t BUSY)
{
    data = SDI;
    clock = SCK;
    chipSelect = CS;
    dc = DC;
    res = RESET;
    busy = BUSY;

    pinMode(data, OUTPUT);
    pinMode(data, OUTPUT);
    pinMode(clock, OUTPUT);
    pinMode(chipSelect, OUTPUT);
    pinMode(dc, OUTPUT);
    pinMode(res, OUTPUT);
    pinMode(busy, INPUT);

    HRES = 0x80;       //128
    VRES_byte1 = 0x01; //296
    VRES_byte2 = 0x28;
}

void MH29EP::init(mode mode)
{
    hwReset(); //Electronic paper IC reset

    writeCommand(0x06);  //boost soft start
    writeData(0x17); //A
    writeData(0x17); //B
    writeData(0x17); //C

    writeCommand(0x04); //Power on
    checkBusy();        //waiting for the electronic paper IC to release the idle signal

    writeCommand(0x00);  //panel setting
    writeData(0x0f); //LUT from OTP£¬128x296
    writeData(0x0d); //VCOM to 0V fast

    writeCommand(0x61); //resolution setting
    writeData(HRES);
    writeData(VRES_byte1);
    writeData(VRES_byte2);

    writeCommand(0X50);  //VCOM AND DATA INTERVAL SETTING
    writeData(0x77); //WBmode:VBDF 17|D7 VBDW 97 VBDB 57   WBRmode:VBDF F7 VBDW 77 VBDB 37  VBDR B7
}
void MH29EP::refresh()
{
    writeCommand(0x12); //DISPLAY REFRESH
    delay(100);  //!!!The delay here is necessary, 200uS at least!!!
    checkBusy();
}
void MH29EP::sleep()
{
    writeCommand(0X50);  //VCOM AND DATA INTERVAL SETTING
    writeData(0xf7); //WBmode:VBDF 17|D7 VBDW 97 VBDB 57    WBRmode:VBDF F7 VBDW 77 VBDB 37  VBDR B7

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

void MH29EP::showImage(uint8_t black_image[])
{
}
void MH29EP::showImage(const uint8_t black_image[])
{
}
void MH29EP::showImage(uint8_t black_image[], uint8_t red_image[])
{
}
void MH29EP::showImage(const uint8_t black_image[], const uint8_t red_image[])
{
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

void MH29EP::drawSquare(int x, int y, int w, int h, color, bool filled)
{
}
void MH29EP::drawCircle(int x, int y, int r, color, bool filled)
{
}
void MH29EP::drawLine(int sx, int sy, int ex, int ey, color)
{
}
void MH29EP::drawDot(int x, int y, color color)
{
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
void MH29EP::readData(uint8_t &dataOut)
{
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
    RST_0;          // Module reset
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