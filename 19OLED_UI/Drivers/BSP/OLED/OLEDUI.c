#include "./BSP/OLED/OLEDUI.h"
#include "./BSP/TIMER/btim.h"


/*
OLED UI系统
*/




uint16_t OLED_SHOWING_PAGE = 0;         //当前显示的是哪一页

int8_t OLED_SHOEING_FRIST_LINE = 0;     //当前处于屏幕第一行的是哪一行

int8_t OLED_CHOOSEING_LINE = 0;         //当前被选中的是哪一行

bool OLED_HAS_PRESSED_KEY = 0;          //当前是否有按键被按下却仍未被读取

uint32_t OLED_PRESSED_KEY_TIMES = 0;    //按键连按次数

uint8_t OLED_PRESSING_KEY = 0;          //最近一次按下的是哪个键

int64_t OLED_LAST_KEYTIME[OLED_KEY_NUM] = {-1000,-1000,-1000,-1000};           //每个按键上一次被按下是在什么时候

int32_t OLED_ROLL_COORD = 0;            //滚动字符串当前坐标

OLED_UI_PAGE OLED_Pages[10];            //页数组

uint16_t OLED_Page_num = 0;             //页个数,从0开始计数

//int args[10];



/**
 * @brief       高亮一行
 * @param       无
 * @retval      无
 */
void OLED_UI_HighLightLine(uint8_t line_i){
    OLED_ReverseArea(0,0+16*line_i,128,16);
}







/**
 * @brief       根据选中行设置屏幕第一行的行索引
 * @param       无
 * @retval      无
 */
void set_FRIST_LINE_with_CHOOSEING_LINE(void){
    OLED_SHOEING_FRIST_LINE = OLED_CHOOSEING_LINE - 2;
    if(OLED_SHOEING_FRIST_LINE < 0){        //越界限制
        OLED_SHOEING_FRIST_LINE=0;
    }
}

/**
 * @brief       统计字符串的长度，用于行结构体的length
 * @param       无
 * @retval      无
 */
uint32_t OLED_strlrn(char *str){
    int length = 0;
    for(int i = 0;str[i]!='\0';i++){
        if ((str[i] & 0x80) == 0x00)     //第一个字节为0xxxxxxx
            length++;
        else if ((str[i] & 0xE0) == 0xC0)	//第一个字节为110xxxxx
        {
            length+=2;i++;
        }
        else if ((str[i] & 0xF0) == 0xE0)	//第一个字节为1110xxxx
        {
            length+=2;i+=2;
        }
        else if ((str[i] & 0xF8) == 0xF0)	//第一个字节为11110xxx
        {
            length+=2;i+=3;
        }
    }
    return length;
}


/**
 * @brief       设置当前显示页、选中行
 * @param       page_index页索引值，line_index行索引值
 * @retval      无
 */
void OLED_UI_ShowPage(uint16_t page_index, uint8_t line_index){
    OLED_SHOWING_PAGE = page_index;
    OLED_CHOOSEING_LINE = line_index;
    if(OLED_CHOOSEING_LINE > OLED_Pages[OLED_SHOWING_PAGE].linenum){    //检查是否越界
        OLED_CHOOSEING_LINE = OLED_Pages[OLED_SHOWING_PAGE].linenum;
    }
    set_FRIST_LINE_with_CHOOSEING_LINE();               //设置显示范围
}


/**
 * @brief       添加新页
 * @param       无
 * @retval      返回初始化好的页面索引
 */
uint16_t OLED_UI_AddNewPage(){
    OLED_Page_num++;
    OLED_Pages[OLED_Page_num].linenum = 0;

    return OLED_Page_num;
}


/**
 * @brief       设置某一页的某一行
 * @param       页索引、行索引、行显示字符串、行绑定函数指针、参数列表
 * @retval      无
 */
void OLED_UI_SetLine(uint16_t page_index, uint8_t line_index, char* str, OLED_UI_FUNC func, char *args){
    strcpy(OLED_Pages[page_index].lines[line_index].showstr,str);
    OLED_Pages[page_index].lines[line_index].length = OLED_strlrn(str);
    OLED_Pages[page_index].lines[line_index].func = func;
    for(int i=0;i<10;i++){
        OLED_Pages[page_index].lines[line_index].args[i]=args[i];
    }
}

/**
 * @brief       扫描并记录哪个按键被按下了
 * @param       无
 * @retval      键值
 */
uint8_t OLED_UI_ScanKey(void){
    
    uint8_t key_r = key_scan(1);
    OLED_PRESSING_KEY = key_r;
    //if(OLED_PRESSING_KEY!=KEY_NOPRESS)
        OLED_HAS_PRESSED_KEY = 1;
    return key_r;
}

/**
 * @brief       读取键值并判断操作类型、统计连按次数
 * @param       无
 * @retval      键值
 */
uint8_t OLED_UI_ReadKey(void){
    static uint8_t lastkey = KEY_NOPRESS;
    if(OLED_HAS_PRESSED_KEY){               //有未读取的按键
        
        if(OLED_PRESSING_KEY!=KEY_NOPRESS){ //不是空按键
            
            if(OLED_PRESSING_KEY == lastkey){   //是连按
                
                OLED_PRESSED_KEY_TIMES++;
                
            }else{
                
                lastkey = OLED_PRESSING_KEY;    //记录这次按键
                OLED_UI_Reaction(lastkey,OLED_Press);          //对按键做出反应
                OLED_PRESSED_KEY_TIMES=1;
                
            }
            if(OLED_PRESSED_KEY_TIMES>OLED_KeepPress_Time){ //连按超过一定次数，判定进入连按模式
                
                
                if(OLED_PRESSED_KEY_TIMES%4==0)         //防止读取频率过快导致操作次数过多
                    OLED_UI_Reaction(lastkey,OLED_KeepPress);          //对按键做出反应
                
            }
        }else{                  //是空按键
            if(lastkey!=KEY_NOPRESS){
                if(OLED_PRESSED_KEY_TIMES<OLED_KeepPress_Time){//上一次是有效按键且不是长按，说明本次空按键代表一次短按结束
                    
                    uint32_t nowtime= __HAL_TIM_GetCounter(&OLED_TIM_Hander);   //获取现在时刻
                
                    if(nowtime-OLED_LAST_KEYTIME[lastkey]<OLED_DoubleClick_Time){   //两次间隔不超过双击间隔，判断为双击
                        
                        OLED_UI_Reaction(lastkey,OLED_DoubleClick);          //对按键做出反应
                        
                    }else{      //单击
                        
                        OLED_UI_Reaction(lastkey,OLED_Click);          //对按键做出反应
                        
                    }
                    
                    OLED_LAST_KEYTIME[lastkey] = nowtime ; //记录按下的时间
                        
                }else{      //上一次是长按，说明此时长按结束
                    
                    OLED_UI_Reaction(lastkey,OLED_ReleaseKeepPress);          //对按键做出反应
                    
                }
                
            }
            lastkey = KEY_NOPRESS;
            OLED_PRESSED_KEY_TIMES = 0;
        }
        
    }
    OLED_HAS_PRESSED_KEY = 0;               //清除标志位
    return lastkey;
}




/**
 * @brief       对按键做出反应，切换选中行、屏幕第一行并重置滚动坐标；或是执行选中行对应的函数。
 * @param       键值
 * @retval      无
 */
void OLED_UI_Reaction(uint8_t key,uint8_t state){
    switch(key){
        
        case KEY_NOPRESS:
            break;
        
        case KEY_LAST:  //“上一个”
            if(state!=OLED_Click)OLED_CHOOSEING_LINE--;
                /*切换当前选中行,单击需要一个完整的按下->松开动作，
                由于在按下时就已经触发OLED_Press状态了，所以松开时不必再次触发*/
            if(OLED_CHOOSEING_LINE<0){
                OLED_CHOOSEING_LINE = OLED_Pages[OLED_SHOWING_PAGE].linenum;
            }
            set_FRIST_LINE_with_CHOOSEING_LINE();//设置显示范围
            OLED_ROLL_COORD = 0;        //重置滚动坐标
            break;
        
        case KEY_NEXT:  //“下一个”
            if(state!=OLED_Click)OLED_CHOOSEING_LINE++;          //切换当前选中行
            if(OLED_CHOOSEING_LINE>OLED_Pages[OLED_SHOWING_PAGE].linenum){
                OLED_CHOOSEING_LINE = 0;
            }
            set_FRIST_LINE_with_CHOOSEING_LINE();//设置显示范围
            OLED_ROLL_COORD = 0;        //重置滚动坐标
            break;
        
        case KEY_OK:  //“确定”
        {
            OLED_UI_LINE *nline = &OLED_Pages[OLED_SHOWING_PAGE].lines[OLED_CHOOSEING_LINE];
            nline->args[7]=OLED_SHOWING_PAGE;
            nline->args[8]=OLED_CHOOSEING_LINE;
            nline->args[9]=state;
            nline->func(nline->args);
            break;
        }
    }
    
    
}



/**
 * @brief       根据全局变量生成显示数据
 * @param       无
 * @retval      无
 */
void OLED_UI_CreateShowData(void){
    OLED_UI_PAGE nowpage = OLED_Pages[OLED_SHOWING_PAGE];
    OLED_Clear();
    set_FRIST_LINE_with_CHOOSEING_LINE();               //设置显示范围
    for(int i = 0;i<4;i++){ //显示文字
        uint8_t nowl = OLED_SHOEING_FRIST_LINE+i;
        if(nowl>nowpage.linenum)break;                 //发生溢出，退出循环
        if(nowl==OLED_CHOOSEING_LINE)continue;          //跳过选中行
        OLED_Printf(0,0+16*i,OLED_8X16,nowpage.lines[nowl].showstr);
        //OLED_Printf(0,0+16*i,OLED_8X16,"line");
    }
    uint8_t coord_y = 0+16*(OLED_CHOOSEING_LINE-OLED_SHOEING_FRIST_LINE);
    uint8_t len = nowpage.lines[OLED_CHOOSEING_LINE].length;
    if(len >= 16){   //字符串过长，需要滚动显示
        OLED_Printf(OLED_ROLL_COORD--, coord_y, OLED_8X16, nowpage.lines[OLED_CHOOSEING_LINE].showstr);
        if(OLED_ROLL_COORD<=(-8*len)) OLED_ROLL_COORD = 128 ;
    }else{          //正常显示
        OLED_Printf(0, coord_y, OLED_8X16, nowpage.lines[OLED_CHOOSEING_LINE].showstr);
    }
    OLED_UI_HighLightLine(OLED_CHOOSEING_LINE-OLED_SHOEING_FRIST_LINE);
}


/**
 * @brief       统合各个函数形成一个完整的帧逻辑
 * @param       无
 * @retval      无
 */
void OLED_UI_NewFrame(void){
    OLED_UI_ScanKey();
    OLED_UI_ReadKey();
    OLED_UI_CreateShowData();
    OLED_Update();
    
}

/**
 * @brief       初始化定时器（等）
 * @param       无
 * @retval      无
 */
void OLED_UI_Init(void){
    OLED_BTIM_Init(65535, 65535); /* 1100hz的计数频率，计数1次为0.909ms,溢出时间为58981ms,59s */
    
    
}



