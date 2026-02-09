#include "./BSP/terazzo/terazzo.h"

Circle c[150]={0};             //圆集合
uint16_t cnum=0;                //圆个数
uint16_t terazzo_COLORS[13]={Pink,LavenderBlush,Thistle,MediumSlateBlue,Lavender,CornflowerBlue,LightSteelBlue,LightCyan,Auqamarin,Khaki,Moccasin,LightSalmon,LightCoral};

uint16_t p_width=320,p_height=480;    //画布宽度




void init_terazzo(void){
    //c={0};
    cnum=0;
    lcd_clear(g_back_color);
}




int terazzo_Random(int n)
{
    srand(SysTick->VAL+n);                      /* 半主机模式下使用time函数会报错，在这里用系统定时器的值替代 */
    return rand() % 1000;
}

int check_circle(Circle *circle){
    for(int i=0;i<cnum;i++){
        int x=circle->x-c[i].x;
        int y=circle->y-c[i].y;
        if((x*x+y*y)<pow(circle->r+c[i].r,2)){
            //printf("0");
            return 0;
        }
    }
    //printf("check_success\n");
    return 1;
}

/*
返回一个不与其它圆重叠的圆，如果返回的圆半径为0，说明找不到这种圆
*/
Circle create_circle(){
    Circle cir;
    int r=0;
    do{
        cir.r=terazzo_Random(r+=10)%45+10;
        cir.x=terazzo_Random(r+=10)%p_width;
        cir.y=terazzo_Random(r+=10)%p_height;
        
    }while(!check_circle(&cir)&&(r<3000));
    if(!check_circle(&cir)){
        //printf("creat_error\n");
        cir.r=0;
    }
    //printf("\n%d %d\n",cir.x,cir.y);
    //printf("creat_success\n");
    return cir;
}


/*
填充指定圆里的多边形，采用逐行扫描填充算法，圆的边界坐标用勾股定理获得。
（Bresenham算法没学会）
按照从左到右、从上到下的方法扫描

注意，本函数的边界判断有很大漏洞。
*/
void terazzo_scan_fill_color(Circle cir,uint16_t color){
    for(int i=0;i<=cir.r;i++){
        if(i<0)i=0;
        if(i>=p_width)i=p_width-1;
        int h =sqrt(cir.r*cir.r-pow(cir.r-i,2)); //计算扫描长度
        uint8_t last_point = 0;
        uint8_t b = 0;
        
        for(int j=cir.y-h;j<cir.y+h;j++){   //左边
            
            int ty = j;
            if(ty<0)ty=0;
            if(ty>=p_height)ty=p_height-1;           //判断是否越界
            
            int tx=cir.x-cir.r+i;
            if(tx<0)tx=0;
            if(tx>=p_width)tx=p_width-1;
            
            if(lcd_read_point(tx,ty)==color){   //扫描到彩色
                if(!last_point){                //上一个点不是彩色，说明这个点是边界
                    b=!b;                       //切换填色
                    last_point=1;
                }
            }else{
                last_point=0;
            }
            if(b){
                lcd_draw_point(cir.x-cir.r+i,j,color);
            }
        }
        
        b = 0;
        last_point = 0;
        
        for(int j=cir.y-h;j<cir.y+h;j++){   //右边
            
            int ty = j;
            if(ty<0)ty=0;
            if(ty>=p_height)ty=p_height-1;           //判断是否越界
            
            int tx=cir.x+cir.r-i;
            if(tx<0)tx=0;
            if(tx>=p_width)tx=p_width-1;
            
            if(lcd_read_point(tx,ty)==color){ //扫描到彩色
                if(!last_point){//上一个点不是彩色，说明这个点是边界
                    b=!b;                //切换填色
                    last_point=1;
                }
                
            }else{
                last_point=0;
            }
            if(b){
                lcd_draw_point(cir.x+cir.r-i,j,color);
            }
        }
        printf("fill,%d\n",h);
    }
    
}

/*
采用向量判断的方式判断点是否在多边形的内部
设A、B、C、D……为多边形的顶点，P为另一点，
若PA+PB+PC+PD+……=0，则P在多边形内部。
nvert:顶点数
vertx/y：存有顶点坐标的数组
*/
uint8_t terazzo_vector_pnpoly(int nvert, uint16_t *vertx, uint16_t *verty, uint16_t testx, uint16_t testy);



/*此算法由W. Randolph Franklin提出*/
/*参考链接：https://wrfranklin.org/Research/Short_Notes/pnpoly.html*/
uint8_t terazzo_pnpoly(int nvert, uint16_t *vertx, uint16_t *verty, uint16_t testx, uint16_t testy)
{
  uint16_t i, j, c = 0;
  for (i = 0, j = nvert-1; i < nvert; j = i++) {
    if ( ((verty[i]>testy) != (verty[j]>testy)) &&
    (testx < (vertx[j]-vertx[i]) * (testy-verty[i]) / (verty[j]-verty[i]) + vertx[i]) )
       c = !c;
  }
  return c;
}

/*
采用判断点是否在多边形的内部，从而上色。
nvert:顶点数
vertx/y：存有顶点坐标的数组
*/
void terazzo_vector_fill_color(Circle cir,uint16_t color,uint8_t nvert, uint16_t *vertx, uint16_t *verty){
    for(int i=0;i<=cir.r*cir.r;i++){
        int h =sqrt(cir.r*cir.r-pow(cir.r-i,2)); //计算扫描长度
        for(int j=cir.y-h;j<=cir.y+h;j++){
            
            int ty = j;
            if(ty<0)ty=0;
            if(ty>=p_height)ty=p_height-1;           //判断是否越界
            
            int tx=cir.x+cir.r-i;
            if(tx<0)tx=0;
            if(tx>=p_width)tx=p_width-1;
            
            if(OLED_pnpoly(nvert,(short *)vertx,(short *)verty,tx,j)){
                //printf("");
                lcd_draw_point(tx,j,color);
            }
            
//            tx=cir.x-cir.r+i;
//            if(tx<0)tx=0;
//            if(tx>p_width)tx=p_width;
//            if(OLED_pnpoly(nvert,(short *)vertx,(short *)verty,tx,j)){
//                lcd_draw_point(cir.x-cir.r+i,j,color);
//            }
        }
        
    }
}


/*
在一个给定的圆里选取随机的点，并按指定的颜色连线

内部连线方式：先随机产生3~6个弧度值，并按大小排序。
    然后按照排序顺序生成点坐标并连线。
这样可以确保生成的连线不会交叉。
*/

void link_point(Circle *cir,uint16_t color){
    if(cir->r==0){
        //printf("draw_error\n");
        return;
    }
    
    int zeta_num = terazzo_Random(cir->x+cir->y) % 5 + 3;
    float zeta[zeta_num];
    
    for(int i=0;i<zeta_num;i++){
        zeta[i] = terazzo_Random(i+cir->x+cir->y)%628/100.0f;//生成弧度值
    }
    
    /* 选择排序，小的在前 */
    for(int i=0;i<zeta_num;i++){
        int z=zeta_num-1;
        for(int j=zeta_num-1;j>=i;j--){
            if(zeta[z]>zeta[j]){
                z=j;
            }
        }
        float t=zeta[i];
        zeta[i]=zeta[z];
        zeta[z]=t;
    }
    
    int16_t pointx[zeta_num],pointy[zeta_num];
    /* 生成点坐标 */
    for(int i=0;i<zeta_num;i++){
        pointx[i]=(cir->x-cir->r*cos(zeta[i]));
        pointy[i]=(cir->y-cir->r*sin(zeta[i]));
        if(pointx[i]>=p_width){
            pointx[i]=p_width-1;
        }
        if(pointy[i]>=p_height){
            pointy[i]=p_height-1;
        }
        if(pointx[i]<0){
            pointx[i]=0;
        }
        if(pointy[i]<0){
            pointy[i]=0;
        }
    }
    
    /*把由于屏幕边界问题而产生的多余的点给去掉*/
    //int
    
    
    
    /* 连线 */
    for(int i=0;i<zeta_num;i++){
        lcd_draw_line(pointx[i],pointy[i],pointx[(i+1)%zeta_num],pointy[(i+1)%zeta_num],color);
    }
    terazzo_vector_fill_color(*cir,color,zeta_num,(uint16_t *)pointx,(uint16_t *)pointy);
    //lcd_draw_circle(cir->x,cir->y,cir->r,RED);
    c[cnum++]=*cir;
    //printf("draw：%d\n",cnum);
}


/* 
在指定大小的画布内（尽可能地）画指定数量的多边形
`*/
void update_terazzo(uint16_t width,uint16_t height,uint16_t num){
    p_width=width;
    p_height=height;
    Circle cir;
    for(int i=0;i<num;i++){
        cir=create_circle();
        
        link_point(&cir,terazzo_COLORS[terazzo_Random(i)%13]);
        
    }
    printf("%d\n",cnum);
}










