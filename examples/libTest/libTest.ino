#include "Ap_29lmao.h"
#include "MH29EP.h"

//IO settings
#define BUSY_Pin 8
#define RES_Pin  9
#define DC_Pin   10
#define CS_Pin   11
#define SCK_Pin  12
#define SDI_Pin  13

MH29EP display = MH29EP(SDI_Pin, SCK_Pin, CS_Pin, DC_Pin, RES_Pin, BUSY_Pin);

void setup()
{
    display.init(BlackAndRed);
    display.showImage(gImage_black1, gImage_red1);
    display.refresh();
    display.sleep(); //EPD_sleep,Sleep instruction is necessary, please do not delete!!!
    delay(6000);
    display.init(BlackAndRed); //EPD init
    display.clear();
    display.drawSquare(10, 10, 50, 50, Black, false);
    display.drawCircle(30, 30, 10, Red, true);
    display.drawLine(100, 100, 200, 100, Red);
    
    for (size_t i = 0; i < 10; i++)
    {
        for (size_t j = 0; j < 10; j++)
        {
            display.drawDot(200 + (5 * i), 10 + (5 * j), Black);
        }
    }

    display.refresh();
    display.sleep(); //EPD_sleep,Sleep instruction is necessary, please do not delete!!!
}

void loop()
{
}