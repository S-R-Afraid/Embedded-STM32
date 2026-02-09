#include "./BSP/DRAWBUG/DRAWBUG.h"

double t = 0;
double PI = 3.1415926535897932384626433382795028;

uint16_t point[2][10000];

void drawbug(void){
    //lcd_clear(BLACK);
    t += PI / 240;
    for(int i = 0; i<10000; i+=1){
        float x = i ;
        float y = i / 235;
        
        // 计算 k 和 e
        float k = (4 + sin(y * 2 - t) * 3) * cos(x / 29);
        float e = y / 8 - 13;

        // 计算 d
        float d = sqrt(k * k + e * e);

        // 计算 q 和 c
        float q = 3 * sin(k * 2) + 0.3 / k + sin(y / 25) * k * (9 + 4 * sin(e * 9 - d * 3 + t * 2));
        float c = d - t;

        // 计算点的坐标
        float px = (q + 30 * cos(c))*1+160;
        float py = (q * sin(c) + d * 39 - 220)*1;

        // 绘制点
        point[0][i]=px;
        point[1][i]=py;
    }
    lcd_clear(BLACK);
    for(int i = 0; i<10000; i++){
        lcd_draw_point(point[0][i],point[1][i],WHITE);
    }
}
