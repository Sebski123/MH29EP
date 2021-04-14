#include "MH29EP.h"

//IO settings
#define BUSY_Pin 8
#define RES_Pin 9
#define DC_Pin 10
#define CS_Pin 11
#define SCK_Pin 12
#define SDI_Pin 13

MH29EP display = MH29EP(SDI_Pin, SCK_Pin, CS_Pin, DC_Pin, RES_Pin, BUSY_Pin);

void setup()
{
    Serial.begin(115200);
    while (!Serial)
    {
        ;
    }

    display.init();
    readRev();
    //readGlassCheck(); //Always returns 0, maybe i'm looking at the wrong datasheet?
    readStatus();       
    //readVcom();       //Always returns 0, maybe i'm looking at the wrong datasheet?
    readTemp();     //Reading this activates the calibration so we have to wait after
    display.checkBusy();
    display.sleep(); //Sleep instruction is necessary, please do not delete!!!
}

void loop()
{
}

void readRev()
{
    display.writeCommand(0x70);
    uint8_t lut_rev;
    uint8_t chip_rev;
    display.readData(lut_rev);
    display.readData(chip_rev);
    Serial.print("LUT Rev: ");
    Serial.println(String(lut_rev));
    Serial.print("Chip Rev: ");
    Serial.println(String(chip_rev));
}

void readGlassCheck()
{
    display.writeCommand(0x44);
    uint8_t PSTA;
    display.readData(PSTA);
    Serial.print("Panel is ");
    Serial.print(PSTA & 1 ? "not " : "");
    Serial.println("broken");
}

void readTemp()
{
    display.writeCommand(0x40);
    uint8_t temp;
    display.readData(temp);
    Serial.print("Temperature is ");
    Serial.println((int8_t)temp);
}

void readStatus()
{
    display.writeCommand(0x71);
    uint8_t status;
    display.readData(status);
    Serial.print("Status is: ");
    Serial.print(", PTL_Flag: ");
    Serial.print(bool(status & 64));
    Serial.print(", I2C_ERR: ");
    Serial.print(bool(status & 32));
    Serial.print(", I2C_BUSY_N: ");
    Serial.print(bool(status & 16));
    Serial.print(", Data_Flag: ");
    Serial.print(bool(status & 8));
    Serial.print(", PON: ");
    Serial.print(bool(status & 4));
    Serial.print(", POF: ");
    Serial.print(bool(status & 2));
    Serial.print(", BUSY_N: ");
    Serial.println(bool(status & 1));
}

void readVcom()
{
    display.writeCommand(0x81);
    uint8_t vcom;
    display.readData(vcom);
    Serial.print("VCOM voltage is -");
    Serial.print(0.10 + (0.05 * vcom));
    Serial.println("V");
}