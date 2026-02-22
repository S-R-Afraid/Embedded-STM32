#ifndef OLEDUI_H
#define OLEDUI_H

#include "./BSP/OLED/OLED.h"
#include <stdbool.h>
#include "./BSP/KEY/key.h"
#include "./SYSTEM/delay/delay.h"
#include "string.h"


#define  OLED_KEY_NUM       4
#define  OLED_BTIM_Init     btim_tim7_int_init
#define  OLED_TIM_Hander    g_tim7_handle

#define  OLED_DoubleClick_Time  300             //双击时间间隔,每单位0.9ms，详情见OLED_UI_Init()中定时器的初始化
#define  OLED_KeepPress_Time    10              //长按。连按计数达到多少时才会被判断为长按，此项应根据帧率来进行设置




/*当增加或减少按键个数时，请一起修改OLED_KEY_NUM*/
enum OLED_UI_KEY_VALUE{
    KEY_NOPRESS = 0,     //无按下 0
    KEY_LAST = KEY0_PRES,//上一个 1
    KEY_NEXT = KEY1_PRES,//下一个 2
    KEY_OK   = WKUP_PRES,//确定   3
};

/*操作类型，分为按键被按下、单击、双击、长按和释放长按*/
enum OLED_UI_KEY_STATE{
    OLED_Press,
    OLED_Click,
    OLED_DoubleClick,
    OLED_KeepPress,
    OLED_ReleaseKeepPress,
};

typedef void* (*OLED_UI_FUNC)(void* args);//定义通用函数指针

/*函数指针用法
    void* exampleFunc(void* arg) {
        // 示例函数，假设arg是一个包含多个参数的结构体
        struct {
            int num;
            char* str;
        }* params = (struct { int num; char* str; }*)arg;
        printf("Received number: %d, string: %s\n", params->num, params->str);
        return arg; // 返回原始参数
    }

    OLED_UI_FUNC funcPtr = exampleFunc ;
    
    void* result = funcPtr(&params);    //调用指针指向的函数
    printf("Result: %d, string: %s\n", ((struct { int num; char* str; }*)result)->num, ((struct { int num; char* str; }*)result)->str);

*/
;
/*结构体定义--------------------------------------*/
typedef struct{     //UI行结构
    char showstr[100];      //该行要显示的文字
    uint8_t length ;
            /*该行的长度，以8像素为一个单位。一个汉字长度2，一个半角长度1
            不能直接用strlen函数*/
    OLED_UI_FUNC func;  //当该行被选中并按下“确定”时希望执行的函数
    char args[10];         //函数的参数,最后三位用于OLED_UI_Reaction传入调用页行和操作类型
}OLED_UI_LINE;


typedef struct{     //UI页结构
    uint8_t linenum;    //该页有多少行,0开始计数
    OLED_UI_LINE lines[20];//行结构体数组。一页最多20行。不够可以创造一个“下一页”行指向新的一页。
}OLED_UI_PAGE;


/*全局变量定义-----------------*/
extern uint16_t OLED_SHOWING_PAGE;      //当前显示的是哪一页

extern int8_t OLED_SHOEING_FRIST_LINE; //当前处于屏幕第一行的是哪一行

extern int8_t OLED_CHOOSEING_LINE;     //当前被选中的是哪一行

extern bool OLED_HAS_PRESSED_KEY;       //当前是否有按键被按下却仍未被读取

extern uint32_t OLED_PRESSED_KEY_TIMES; //按键连按次数

extern uint8_t OLED_PRESSING_KEY;       //最近一次按下的是哪个键

extern int64_t OLED_LAST_KEYTIME[OLED_KEY_NUM];    //每个按键上一次被按下是在什么时候

extern int32_t OLED_ROLL_COORD;         //滚动字符串当前坐标

extern OLED_UI_PAGE OLED_Pages[10];     //页数组

extern uint16_t OLED_Page_num;          //页个数 从0开始计数

/*函数定义--------------------*/
void OLED_UI_ShowPage(uint16_t page_index, uint8_t line_index);         //设置当前显示页、选中行

uint16_t OLED_UI_AddNewPage();  //添加新页

//设置某一页的某一行
void OLED_UI_SetLine(uint16_t page_index, uint8_t line_index, char* str, OLED_UI_FUNC func, char *args);

uint8_t OLED_UI_ScanKey(void);                      //扫描哪个按键被按下了

uint8_t OLED_UI_ReadKey(void);                      //读取键值并统计连按次数

void OLED_UI_Reaction(uint8_t key,uint8_t state);                 //对按键做出反应，切换选中行、屏幕第一行并重置滚动坐标；或是执行选中行对应的函数。

void OLED_UI_CreateShowData(void);                  //根据全局变量生成显示数据

void OLED_UI_NewFrame(void);                        //统合各个函数形成一个完整的帧逻辑

void OLED_UI_Init(void);                            //初始化定时器（等）



#endif



