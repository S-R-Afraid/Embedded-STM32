
#include "./BSP/Sandbox/sand.h"
#include "math.h"

#define ABS(x) ((x) > 0 ? (x) : -(x))
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


/* 摇杆 */
uint16_t PS2_xy_value[2];               /* 0x 1y */
uint8_t PS2_z = 0;                      /* z轴是否被按下 */
float PS2_x_f = 0,PS2_y_f = 0;          /* xy的相对值，取值0~1 */

int model=1;
    
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
    if(model){
        boxangel.cosx=PS2_x_f*2;
        boxangel.cosy=-PS2_y_f*2;
    }
    else{
        int16_t ax,ay,az, GX, GY, GZ;         //定义用于存放各个数据的变量,A是加速度，G是陀螺仪
        MPU6050_GetData(&ax, &ay, &az, &GX, &GY, &GZ);		//获取MPU6050的数据
        double A =sqrt(ax*ax + ay*ay + az*az);
        boxangel.cosx=ax/A;
        boxangel.cosy=ay/A;
        boxangel.cosz=az/A;//计算方向余弦
    }
    

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

/*更新方向 0平放，1代表下，2右下，3右，4右上，5上，6左上，7左，8左下
    v是判断阈值
*/
uint8_t update_forword(float v){
    int x,y=0;
   update_angle();//获取重力方向
    if(boxangel.cosx>v){         //重力向右
        x=1;
    }
    if(boxangel.cosx<-v){        //重力向左
        x=-1;
    }
    if(boxangel.cosy<-v){        //重力向下
        y=1;
    }
    if(boxangel.cosy>v){         //重力向上
        y=-1;
    }
    
    if(!x&&!y){//平放
        return 0;
    }
    if(y==1&&x==0){//下
        return 1;
    }
    if(y==1&&x==1){//右下
        return 2;
    }
    if(x==1&&y==0){//右
        return 3;
    }
    if(y==-1&&x==1){//右上
        return 4;
    }
    if(y==-1&&x==0){//上
        return 5;
    }
    if(y==-1&&x==-1){//左上
        return 6;
    }
    if(y==0&&x==-1){//左
        return 7;
    }
    if(y==1&&x==-1){//左下
        return 8;
    }
    return 0;
}

void update_local(SandBlock *block){
    int x = 0;
    int y = 0;
    //int z = 0;//重力方向，1表示是正方向，-1表示反方向;当该方向无重力时为0 。
    int s_i=BOX_LEN,s_j=BOX_LEN;//遍历起点,默认右下角开始

    update_angle();//获取重力方向
     // 1. 计算重力在菱形网格上的分量
    // ---------------------------------------------------------
    // 物理向下 (屏幕下方): 对应网格 (i增加, j增加) 
    if (boxangel.cosy < -0.5) {
        x += 1;
        y += 1;
    }
    // 物理向上 (屏幕上方): 对应网格 (i减小, j减小) 
    else if (boxangel.cosy > 0.5) {
        x -= 1;
        y -= 1;
    }

    // 物理向右 (屏幕右方): 对应网格 (i增加, j减小)
    if (boxangel.cosx > 0.5) {
        x += 1;
        y -= 1;
    }
    // 物理向左 (屏幕左方): 对应网格 (i减小, j增加)
    else if (boxangel.cosx < -0.5) {
        x -= 1;
        y += 1;
    }

    // 2. 限制步长范围 (Clamp)
    // 防止斜向重力叠加导致步长变成2
    if (x > 1)  x = 1;
    if (x < -1) x = -1;
    if (y > 1)  y = 1;
    if (y < -1) y = -1;

    // 3. 确定遍历起点 (s_i, s_j)
    if (x > 0)      s_i = BOX_LEN - 1;
    else            s_i = 1;           // 没位移时默认从头开始

    if (y > 0)      s_j = BOX_LEN - 1; 
    else            s_j = 1;
    
    
    
    //判断重力方向并设置循环起点
    
    for(int i=s_i;i>0&&i<=BOX_LEN;i-=(x?x:-1)){//当该轴有重力作用时采用对应的遍历方向，否则使用1
        for(int j=s_j;j>0&&j<=BOX_LEN;j-=(y?y:-1)){
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
                    
                    int nx=x,ny=y;
                    if (boxangel.cosy < -0.2) {
                        nx += 1;
                        ny += 1;
                    } 
                    else if (boxangel.cosy > 0.2) {
                        nx -= 1;
                        ny -= 1;
                    }
                    if (boxangel.cosx > 0.2) {
                        nx += 1;
                        ny -= 1;
                    }
                    else if (boxangel.cosx < -0.2) {
                        nx -= 1;
                        ny += 1;
                    }
                    if (nx > 1)  nx = 1;
                    if (nx < -1) nx = -1;
                    if (ny > 1)  ny = 1;
                    if (ny < -1) ny = -1;
                    
                    if(!block->broad[i+nx][j]&&block->broad[i-nx][j]){//如果重力方向上没有物体且反方向有
                        ni=i+nx,nj=j;
                    }else if(!block->broad[i][j+ny]&&block->broad[i][j-ny]){
                        ni=i,nj=j+ny;
                    }
                    if(sandclock_Random(i+j)%10>6){//随机决定到底要不要移动
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
 * 绘制菱形沙粒函数
 * 增加了边界检查，防止负坐标导致的乱码/满屏线条
 */
void draw_sand(int cx, int cy, uint16_t color) {
    // 将半径设为 SAND_LEN 的一半，这样大小跟原来的方块差不多
    // 如果觉得沙粒太小，可以改为 int r = SAND_LEN;
    int r = (SAND_LEN > 1) ? (SAND_LEN / 2) : 1; 

    // 逐行绘制
    for (int dy = -r; dy <= r; dy++) {
        // 计算当前行的半宽
        int w = r - abs(dy);
        
        // 计算起点和终点
        int x_start = cx - w;
        int x_end   = cx + w;
        int y_pos   = cy + dy;

        // --- 核心修复：边界裁剪 (Clipping) ---
        // 1. 如果 Y 轴超屏，直接跳过
        if (y_pos < 0 || y_pos >= SSD_VER_RESOLUTION) continue;

        // 2. 如果 X 轴完全在屏幕外，跳过
        if (x_end < 0 || x_start >= SSD_HOR_RESOLUTION) continue;

        // 3. 限制 X 轴在屏幕范围内 (防止负数变成65535)
        if (x_start < 0) x_start = 0;
        if (x_end >= SSD_HOR_RESOLUTION) x_end = SSD_HOR_RESOLUTION - 1;

        // 绘制
        if (x_end >= x_start) {
            lcd_draw_line(x_start, y_pos, x_end, y_pos, color);
        }
    }
}

/*
 * 绘制旋转沙漏
 * zx: 请务必传入屏幕宽度的中间值（例如 120），否则会画到屏幕外面！
 * zy: 距离顶部的距离
 */
void draw_sandblock(int zx, int zy, SandBlock *block) {
    // 步长：两个格子的中心距离
    // 为了紧密堆叠，步长应等于 2倍半径 + 缝隙(0或1)
    // 这里设为 SAND_LEN 即可（假设上面的半径是 SAND_LEN/2）
    int step = SAND_LEN; 
    
    // 1. 绘制边框
    // 计算四个顶点的逻辑位置 (i, j)
    // 顶点1 (Top): 1, 1
    // 顶点2 (Right): N, 1
    // 顶点3 (Bottom): N, N
    // 顶点4 (Left): 1, N
    
    // 使用简单的相对坐标计算屏幕坐标
    // 旋转公式：ScreenX = zx + (i - j) * step
    //           ScreenY = zy + (i + j) * step / 2  <-- 除以2是为了压扁一点，形成透视感；不除以2则是标准45度
    
    // 这里使用标准的 45度 坐标映射 (x = x-y, y = x+y)
    // 预先计算四个角的屏幕坐标
    int x_top = zx; 
    int y_top = zy-3;
    
    int x_right  = zx + (BOX_LEN * step)+3;
    int y_right  = zy + (BOX_LEN * step);
    
    int x_bottom = zx; 
    int y_bottom = zy + (2 * BOX_LEN * step)+3;
    
    int x_left   = zx - (BOX_LEN * step)-3;
    int y_left   = zy + (BOX_LEN * step);

    // 绘制外框 
    // 注意：这里的边框是连接网格中心的，为了美观外扩一点，这里先画核心骨架
    lcd_draw_line(x_top, y_top, x_right, y_right, g_point_color);
    lcd_draw_line(x_right, y_right, x_bottom, y_bottom, g_point_color);
    lcd_draw_line(x_bottom, y_bottom, x_left, y_left, g_point_color);
    lcd_draw_line(x_left, y_left, x_top, y_top, g_point_color);

    // 2. 绘制沙粒
    for (int i = 1; i <= BOX_LEN; i++) {
        for (int j = 1; j <= BOX_LEN; j++) {
            
            // 核心坐标变换算法
            // (i-1) 和 (j-1) 是为了让 (1,1) 在原点
            int grid_i = i - 1;
            int grid_j = j - 1;

            // X轴：i 增加向右，j 增加向左
            int sx = zx + (grid_i - grid_j) * step;
            
            // Y轴：i 增加向下，j 增加向下
            int sy = zy + (grid_i + grid_j) * step;

            if (block->broad[i][j] == 1) {
                uint16_t c = (i == BOX_LEN && j == BOX_LEN) ? RED : WHITE;
                draw_sand(sx, sy, c);
            } else {
                draw_sand(sx, sy, BLACK);
            }
        }
    }
}






/*
沙子流逝。
会根据重力判断从哪个流向哪个
*/
void sand_clock(void){
    int f=update_forword(0.5);
    if(f==1||f==2||f==8){
        //从一到二
        if(block1.broad[BOX_LEN][BOX_LEN]==1){
            block1.snum--;
            block2.snum++;
            block1.broad[BOX_LEN][BOX_LEN]=0;
            block2.broad[1][1]=1;
        }
        sand_clock_time=block1.snum;
    }
    if(f==4||f==5||f==6){
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




