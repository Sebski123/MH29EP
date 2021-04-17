#include "BWR_29_lmao.h"
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
    display.init();
    display.showImage(gImage_black1, gImage_red1);
    display.refresh();
    display.sleep(); //Sleep instruction is necessary, please do not delete!!!
    delay(2);
    display.init();
    display.clear();
    display.refresh();
    display.sleep(); //Sleep instruction is necessary, please do not delete!!!
}

void loop()
{
}