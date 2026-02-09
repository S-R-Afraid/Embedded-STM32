
#include "./BSP/Sandbox/sand.h"


/*
芯片方向：
    X<----------
                |   SDA
                |   SCL
                |   GND
                |   5V
                V Y

但是我们可以对x轴取反，这样就变成了正常的坐标系:

屏幕坐标系：
    ------------->X
    |
    |
    |
    |
    |
    V  Y

经过测量，各个典型角度的读数如下所示（方位余弦值）：

正面朝上平放:    
                x=0.04
                y=-0.01
                z=-1.00

正面朝前竖直放置：
                x=0.04
                y=-1.00
                z=-0.03

正面朝后倒立放置：
                y=1.00

正面向右侧翻，侧立：
                x=1.00
                y=-0.03
                z=0.05

正面向左侧立：
                x=-1.00

正面超前竖立，然后右倾45°左右：
                x=0.71
                y=-0.70
                z=0.08

左倾45°：
                x=-0.71

右倾135°，即旋转180°倒立，在左转45°：
                x=0.73
                y=o.68

左倾135°：
                x=-0.76



*/




BoxAngel boxangel;
SandBlock block1,block2;
int sand_clock_time=SAND_NUM;


int sandclock_Random(int n)
{
    srand(SysTick->VAL+n);                      /* 半主机模式下使用time函数会报错，在这里用系统定时器的值替代 */
    return rand() % 100;
}

/*
通过读取加速度来计算姿态角。
至于陀螺仪谁爱写谁写。
*/
void update_angle(void)
{
    int16_t ax,ay,az, GX, GY, GZ;         //定义用于存放各个数据的变量,A是加速度，G是陀螺仪
	MPU6050_GetData(&ax, &ay, &az, &GX, &GY, &GZ);		//获取MPU6050的数据
    double A =sqrt(ax*ax + ay*ay + az*az);
	boxangel.cosx=ax/A;
	boxangel.cosy=ay/A;
	boxangel.cosz=az/A;//计算方向余弦

}


/*
初始化沙漏块
*/
void init_block(void){
    block1.flag=1;
    block2.flag=2;
    block1.snum=SAND_NUM;
    block2.snum=0;
    
    for(int i=1;i<=BOX_LEN;i++){
        for(int j=1;j<=BOX_LEN;j++){
            block1.broad[i][j]=0;
            block2.broad[i][j]=0;
        }
    }
    for(int i=0;i<=BOX_LEN+1;i++){//将边缘处全部赋值为-1
        block1.broad[i][0]=-1;
        block1.broad[0][i]=-1;
        block1.broad[i][BOX_LEN+1]=-1;
        block1.broad[BOX_LEN+1][i]=-1;
        
        block2.broad[i][0]=-1;
        block2.broad[0][i]=-1;
        block2.broad[i][BOX_LEN+1]=-1;
        block2.broad[BOX_LEN+1][i]=-1;
    }
    int snum=0;
    for(int i=1;i<=BOX_LEN;i++){//放置沙子
        if(snum == SAND_NUM)break;
        for(int j=1;j<=BOX_LEN;j++){
            if(snum == SAND_NUM)break;
            block1.broad[i][j]=1;
            snum++;
        }
    }
}


/*
更新位置，如果某个沙子的重力方向上没有其它沙子，也不处于底边，则移动。
由于我们把边缘处全部赋值为1了，所以不需要判断哪边是底边，只要判断有没有其它沙子就行了
重力方向：当某个方向与坐标轴的夹角小于60度则将该方向视为重力方向之一。
位移判断有两次：第一次是常规判断；
第二次在第一次判定没有产生位移的情况下产生，并使用更小的重力阈值；
且第二次判断不会产生斜向位移并用随机减小位移概率。
这是为了更好的模拟出倒塌的物理效果而不会出现过于整齐的堆叠

*/
void update_local(SandBlock *block){
    int x = 0;
    int y = 0;
    //int z = 0;//重力方向，1表示是正方向，-1表示反方向;当该方向无重力时为0 。
    int s_i=BOX_LEN,s_j=BOX_LEN;//遍历起点,默认右下角开始

    update_angle();//获取重力方向
    if(boxangel.cosx>0.5){         //重力向右
        x=1;
        s_i=BOX_LEN-1;
    }
    if(boxangel.cosx<-0.5){        //重力向左
        x=-1;
        s_i=1;
    }
    if(boxangel.cosy<-0.5){        //重力向下
        y=1;
        s_i=BOX_LEN-1;
    }
    if(boxangel.cosy>0.5){         //重力向上
        y=-1;
        s_j=1;
    }//判断重力方向并设置循环起点
    
    for(int i=s_i;i>0&&i<=BOX_LEN;i-=(x?x:1)){//当该轴有重力作用时采用对应的遍历方向，否则使用1
        for(int j=s_j;j>0&&j<=BOX_LEN;j-=(y?y:1)){
            if(block->broad[i][j]){
                int ni=i,nj=j;
                if(!block->broad[i+x][j+y]){
                    ni=i+x,nj=j+y;
                }else if(sandclock_Random(i+j)%2==0){          //随机判断先向左还是先向右
                    if(!block->broad[i+x][j]){
                        ni=i+x,nj=j;
                    }else if(!block->broad[i][j+y]){
                        ni=i,nj=j+y;
                    }
                }else{
                    if(!block->broad[i][j+y]){
                        ni=i,nj=j+y;
                    }else if(!block->broad[i+x][j]){
                        ni=i+x,nj=j;
                    }
                }
                if(ni==i&&nj==j){
                    /*如果经过上面的判定没有任何移动则进入第二次判断
                    这次判断是为了更好的模拟物理效果
                    */
                    
                    int nx=x,ny=y;      //判断新的重力方向
                    if(boxangel.cosx>0.2){         //重力向右
                        nx=1;
                    }
                    if(boxangel.cosx<-0.2){        //重力向左
                        nx=-1;
                    }
                    if(boxangel.cosy<-0.2){        //重力向下
                        ny=1;
                    }
                    if(boxangel.cosy>0.2){         //重力向上
                        ny=-1;
                    }
                    if(!block->broad[i+nx][j]&&block->broad[i-nx][j]){//如果重力方向上没有物体且反方向有
                        ni=i+nx,nj=j;
                    }else if(!block->broad[i][j+ny]&&block->broad[i][j-ny]){
                        ni=i,nj=j+ny;
                    }
                    if(sandclock_Random(i+j)%10>6){
                        ni=i,nj=j;
                    }
                    
                }
                block->broad[i][j]=0;
                block->broad[ni][nj]=1;//这两个赋值不能颠倒顺序
            }
        }
    }
}

/*
绘制方块
zx,zy:左上角坐标
*/
void draw_sand(int zx,int zy,uint16_t color){
    //delay_us(10);
    //printf("drawsand:%d,%d\r\n",zx,zy);
    lcd_fill(zx,zy,zx+SAND_LEN,zy+SAND_LEN,color);
    
    
}



/*
绘制沙漏块
zx,zy:左上角的坐标
block:要绘制的块
*/
void draw_sandblock(int zx,int zy,SandBlock *block){
    draw_sand(300,400,WHITE);
    //printf("drawblock\n");
    int length = BOX_LEN*SAND_LEN; //边框长度
//    lcd_fill(zx,zy,zx+length,zy+length,BLACK);//清空绘画区域
    lcd_draw_line(zx-1,zy-1,zx+length+1,zy-1,g_point_color);
    lcd_draw_line(zx-1,zy-1,zx-1,zy+length+1,g_point_color);
    lcd_draw_line(zx+length+1,zy-1,zx+length+1,zy+length+1,g_point_color);
    lcd_draw_line(zx-1,zy+length+1,zx+length+1,zy+length+1,g_point_color);//绘制边框
    for(int i=1;i<=BOX_LEN;i++){
        for(int j=1;j<=BOX_LEN;j++){
            if(block->broad[i][j]==1){
                if(i==BOX_LEN&&j==BOX_LEN){
                    draw_sand(zx+(i-1)*SAND_LEN,zy+(j-1)*SAND_LEN,RED);
                }else{
                    draw_sand(zx+(i-1)*SAND_LEN,zy+(j-1)*SAND_LEN,WHITE);
                }
            }
            else{
                draw_sand(zx+(i-1)*SAND_LEN,zy+(j-1)*SAND_LEN,BLACK);
            }
        }
    }
}


/*
沙子流逝。
会根据重力判断从哪个流向哪个
*/
void sand_clock(void){
    update_angle();
    if((boxangel.cosx>0.5||boxangel.cosy<-0.5)&&(boxangel.cosx>-0.5&&boxangel.cosy<0.5)){
        //从一到二
        if(block1.broad[BOX_LEN][BOX_LEN]==1){
            block1.snum--;
            block2.snum++;
            block1.broad[BOX_LEN][BOX_LEN]=0;
            block2.broad[1][1]=1;
        }
        sand_clock_time=block1.snum;
    }
    if((boxangel.cosx<-0.5||boxangel.cosy>0.5)&&(boxangel.cosx<0.5&&boxangel.cosy>-0.5)){
        //从二到一
        if(block2.broad[1][1]==1){
            block1.snum++;
            block2.snum--;
            block2.broad[1][1]=0;
            block1.broad[BOX_LEN][BOX_LEN]=1;
        }
        sand_clock_time=block2.snum;
    }
}




