#include "./BSP/SNAKE/snake.h"


int snake_length=1;                             /* 当前蛇的长度 */

int game_end = 0;                               /* 游戏结束标志 */

struct Body body[SNAKE_LEN];                    /*身体数组*/

uint8_t food_x=0,food_y=0;                      /* 食物位置 */

int forward=2;                                  /* 前进方向 */

/* 摇杆 */
    uint16_t PS2_xy_value[2];               /* 0x 1y */
    uint8_t PS2_z = 0;                      /* z轴是否被按下 */
    float PS2_x_f = 0,PS2_y_f = 0;          /* xy的相对值，取值0~1 */

void Ini_snake(){                               /* 初始化蛇和食物的位置 */
    OLED_Clear();
    snake_length=1;
    body[0].x=8;
    body[0].y=4;
    PutFood();
    Draw();
    
}

int Ask_body_here(int x, int y)
{
    int i=0;
    for(i=0;i<snake_length;i++){
        if(body[i].x==x&&body[i].y==y)
            return 1;
    }
    return 0;
}

int Random(int n)
{
    srand(SysTick->VAL+n);                      /* 半主机模式下使用time函数会报错，在这里用系统定时器的值替代 */
    return rand() % 100;
}

void PutFood()
{
    int i=0;
    do{
        food_x=Random(i)%(BOARD_W-1);
        food_y=Random(i+1)%(BOARD_H-1);
        i++;
    }while(Ask_body_here(food_x,food_y));
    
}

void Draw()
{
    OLED_Clear();
    DrawBody();        //画身体
    DrawBoard();       //画棋盘
    DrawFood();        //画食物
    
}

void DrawSqure(int x,int y){                    /* 绘制方块 */

    OLED_DrawRectangle(x,y,SNAKE_W,SNAKE_W,1);
}

void DrawBoard(){                               /* 绘制棋盘格子 */

    for(int i=0;i<=BOARD_W;i++){
        OLED_DrawLine(i*SNAKE_W,0,i*SNAKE_W,(BOARD_H)*SNAKE_W);
    }
    for(int i=0;i<=BOARD_H;i++){
        OLED_DrawLine(0,i*SNAKE_W,(BOARD_W)*SNAKE_W,i*SNAKE_W);
    }
}

void DrawBody(){                                /* 绘制身体 */

    for(int i=0;i<snake_length;i++){
        DrawSqure(body[i].x*SNAKE_W,body[i].y*SNAKE_W);
    }
    
}

void DrawFood(){                                /* 绘制食物 */

    int kd = OLED_W/BOARD_W/2;
    int f_x = (food_x*2)*kd, f_y = (food_y*2)*kd;
    OLED_DrawCircle(f_x,f_y,SNAKE_W/2,1);
}

void Forward(int d){                            /* 前进函数，改变头的位置 */
/*    1W上2S下3A左4D右    */
    Update();
    static int lastd=-1;
    do{
        if(lastd!=4&&d==3){
            body[0].x--;//向左
            lastd=d;
        }
        else if(lastd!=2&&d==1){
            body[0].y--;//向上
            lastd=d;
        }
        else if(lastd!=3&&d==4){
            body[0].x++;//向右
            lastd=d;
        }
        else if(lastd!=1&&d==2){
            body[0].y++;//向下
            lastd=d;
        }else{//如果不满足上面条件，则表示玩家按键错误，依照上一次前进的方向继续前进
            d=lastd;
            lastd=-1;
        }
    }while(lastd==-1);

    /* 边界检测 */
    if(body[0].x>BOARD_W-1)body[0].x=0;
    if(body[0].x<0)body[0].x=BOARD_W-1;
    if(body[0].y>BOARD_H-1)body[0].y=0;
    if(body[0].y<0)body[0].y=BOARD_H-1;

}

void Update(void){                          /*更新身体位置*/

    for(int i=snake_length;i>0;i--)
    {
        body[i]=body[i-1];
    }
}

void Increase()
{
    snake_length+=1;

}

int Judge(void){                            /* 判断是否碰到身体，是否吃到食物 */

    int i=0;
    for(i=1;i<snake_length;i++){
        if(body[0].x==body[i].x&&body[0].y==body[i].y){
            return 0;           //碰到身体
        }
    }
    if(body[0].x==food_x&&body[0].y==food_y)
    {
       return 1;                //碰到食物
    }

    return 2;                   //一切正常
}


void GameEnd(void){                         /* 游戏结束，显示分数 */

    game_end=1;
    OLED_Clear();
    OLED_Printf(OLED_W/2-20,OLED_H/2-8,OLED_8X16,"SCORE");
    OLED_Printf(OLED_W/2-12,OLED_H/2+8,OLED_8X16,"%3d",snake_length);
}


