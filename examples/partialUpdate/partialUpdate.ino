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
    display.clear();
    //display.drawCircle(40, 40, 15, Black, false);
    //display.drawCircle(40, 100, 30, Red, false);
    display.drawCircle(60, 200, 5, Black, false);
    display.refresh();
    display.sleep(); //EPD_sleep,Sleep instruction is necessary, please do not delete!!!
    delay(2000);
    display.init();
    display.clear();
    display.refresh();
    display.sleep();
}

void loop()
{
}
