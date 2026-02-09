#include "./BSP/Maze/maze.h"


/*
    迷宫生成：generate
        深度优先DFS、递归分割recursion、prim生成算法。

*/

/*迷宫区块*/
uint32_t maze_map[MAP_LENGTH][MAP_WIDTH];
/*迷宫墙壁列表*/
MAZE_WALL wall_list[(MAP_WIDTH-1)*MAP_LENGTH+(MAP_LENGTH-1)*MAP_WIDTH];
int wall_num = 0;

void init_map(){
    memset(maze_map,0,sizeof(maze_map));
    wall_num = 0;
}
void draw_line(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color){
    lcd_draw_line(x1,y1,x2,y2,color);
}
void draw_square(int16_t i, int16_t j, uint32_t color){
    int x = MAP_LX + i*MAP_BLOCK_WIDTH;
    int y = MAP_LY + j*MAP_BLOCK_WIDTH;
    lcd_fill( x+1,  y+1,  x+MAP_BLOCK_WIDTH-1,  y+MAP_BLOCK_WIDTH-1,  color);
}

/*判断数组是否越界*/
uint8_t is_beyond(int16_t x,int16_t y){
    return ( (x>=0) && 
            (x<MAP_LENGTH) && 
            (y>=0) && 
            (y<MAP_WIDTH) );
}

/*在两格之间画一堵墙*/
void draw_wall(MAZE_WALL wall){
    if(abs(wall.i1-wall.i2)>1||abs(wall.j1-wall.j2)>1)return;
    if(wall.i1==wall.i2){//横向排列
        int j = wall.j1>wall.j2?wall.j1:wall.j2;
        int x = MAP_LX + j*MAP_BLOCK_WIDTH;
        int y = MAP_LY + wall.i1*MAP_BLOCK_WIDTH;
        draw_line(x,y,x,y+MAP_BLOCK_WIDTH,WHITE);
    }else if(wall.j1==wall.j2){     //纵向排列
        int i = wall.i1>wall.i2?wall.i1:wall.i2;
        int x = MAP_LX + i*MAP_BLOCK_WIDTH;
        int y = MAP_LY + wall.j1*MAP_BLOCK_WIDTH;
        draw_line(x,y,x+MAP_BLOCK_WIDTH,y,WHITE);
    }
}

/*DFS遍历迷宫，作为生成迷宫函数的递归部分起作用。x，y为递归起点，同时会将墙壁画出来*/
void DFS_ergodic(int16_t x,int16_t y,PRNG_SEED seed){
    //draw_square(x,y,RED);
    lcd_show_num(MAP_LX+x*MAP_BLOCK_WIDTH,MAP_LY + y*MAP_BLOCK_WIDTH,maze_map[x][y],2,12,g_point_color);
    delay_ms(100);
    static int8_t f[4][2] = {{-1,0}, {1,0}, {0,-1}, {0,1}};/*上下左右*/
    uint8_t fs = MyRandom_PRNG_STREAM(0,seed,PRNG_LCG)%4; //随机选择方向起始点
    for(uint8_t i = 0;i<4;i++){
        uint8_t d = (fs + i)%4;     //获得本次的方向
        int16_t tx = x + f[d][0],
                ty = y + f[d][1];
        if( is_beyond(tx,ty) ){  //检测是否越界
            //draw_square(tx,ty,GREEN);
            if(maze_map[tx][ty]==0){  //该块还未被访问过
                maze_map[tx][ty] = maze_map[x][y]+1;    //标记
                //draw_square(tx,ty,RED);
                DFS_ergodic(tx,ty,seed);            //递归访问
            }
//            else{   //该点已经被访问过了，说明应当有一堵墙隔在两点之间
//                //if(maze_map[tx][ty]!=maze_map[x][y]-1)draw_wall((MAZE_WALL){x,y,tx,ty});
//            }
        }
        
    }
}

/*DFS生成迷宫。以x1，y1为起点，x2,y2为终点。请确保两点都在边缘处。*/
void DFS_generate(int16_t x1,int16_t y1,int16_t x2,int16_t y2,PRNG_SEED seed){
    init_map();
    MyRandom_PRNG_STREAM(1,seed,PRNG_LCG); /*初始化随机数引擎*/
    maze_map[x1][y1] = 1;
    DFS_ergodic(x1,y1,seed);
}


/*递归分割法的递归部分
x1、x2，y1、y2是要分割的区域，以格子为单位
Y1  ______________
    |             |
    |             | 
    |             | 
Y2  |_____________|
    X1           X2
x0、y0是分割线的焦点
Y1  ______________
    |      |      |
    |      |      | 
    |______|______| Y0
Y2  |______|______|
    X1    X0     X2

bp1 bp2 bp3 bp4 是打通分割线的坐标，对应左右上下
Y1  ______________
    |      |      |
    |             | bp3
    |___ __|___ __| Y0
Y2  |______|______|
    X1 bp1 X0 bp2  X2

*/

void ergodic_separate(int16_t x1,int16_t y1,int16_t x2,int16_t y2,PRNG_SEED seed){
    if(x1==x2||y1==y2)return;
    int16_t x0 = x1 + MyRandom_PRNG_STREAM(0,seed,PRNG_LCG)%(x2-x1);
    int16_t y0 = y1 + MyRandom_PRNG_STREAM(0,seed,PRNG_LCG)%(y2-y1);
    int16_t bp[4];
    bp[0] = x1 + MyRandom_PRNG_STREAM(0,seed,PRNG_LCG)%(x0-x1),
    bp[1] = x0 + MyRandom_PRNG_STREAM(0,seed,PRNG_LCG)%(x2-x0),
    bp[2] = y1 + MyRandom_PRNG_STREAM(0,seed,PRNG_LCG)%(y0-y1),
    bp[3] = y0 + MyRandom_PRNG_STREAM(0,seed,PRNG_LCG)%(y2-y0);
    uint8_t not_break = MyRandom_PRNG_STREAM(0,seed,PRNG_LCG)%4;
    int16_t local = 0;
    //遍历纵向分割线
    for(int i = 0;i<y2-y1;i++){
        local = y1+i;
        if(local ==bp[not_break]&&not_break>1){//需要打通的地方，跳过
            
        }else{
            wall_list[wall_num++]= (MAZE_WALL){x0,local,x0+1,local};
            draw_wall((MAZE_WALL){x0,local,x0+1,local});
        }
    }
    //遍历横向分割线
    for(int i = 0;i<x2-x1;i++){
        local = x1+i;
        if(local ==bp[not_break]&&not_break<2){
            
        }else{
            wall_list[wall_num++]= (MAZE_WALL){local,y0,local,y0+1};
            draw_wall((MAZE_WALL){local,y0,local,y0+1});
        }
    }
    ergodic_separate(x1,y1,x0,y0,seed);
    ergodic_separate(x0,y1,x2,y0,seed);
    ergodic_separate(x1,y0,x0,y2,seed);
    ergodic_separate(x0,y0,x2,y2,seed);
}



void ergodic_separate_generate(int16_t x1,int16_t y1,int16_t x2,int16_t y2,PRNG_SEED seed){
    init_map();
    MyRandom_PRNG_STREAM(1,seed,PRNG_LCG); /*初始化随机数引擎*/
    ergodic_separate(0,0,MAP_WIDTH,MAP_LENGTH,seed);//开始递归
    draw_square(x1,y1,RED);
    draw_square(x2,y2,RED);
}














