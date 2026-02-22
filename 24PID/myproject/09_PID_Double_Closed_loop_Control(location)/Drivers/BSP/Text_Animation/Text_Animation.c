#include "BSP/Text_Animation/Text_Animation.h"
#include "./BSP/LCD/lcd.h"


struct Text_Animation text_animation={
    .obfu_chars = {
        
        {{0xE2,0x96,0x91},3},

        
        {{0xE2,0x96,0x92},3},

        
        {{0xE2,0x96,0x93},3},

        
        {{0xE2,0x96,0x96},3},

        
        {{0xE2,0x96,0x97},3},

        
        {{0xE2,0x96,0x98},3},

        
        {{0xE2,0x96,0x99},3},

        
        {{0xE2,0x96,0x9A},3},

        
        {{0xE2,0x96,0x9B},3},

        
        {{0xE2,0x96,0x9C},3},

        
        {{0xE2,0x96,0x9D},3},

        
        {{0xE2,0x96,0x9E},3},

        
        {{0xE2,0x96,0x9E},3},

        
        {{0xE2,0x96,0x9F},3},

    },
    .start_tim = 10,
    .end_tim = 50,
    .disp_time = 100,
    .loop = 1,
    .idx = 0,
    .fram = 97,
    .phrases = {"something funny...\0", "THIS IS A TEST.\0", "你好!我我我\0",},
    .showstr = "123456789",
};

struct task tasks[100];      /*任务列表 */
int task_len = 0;

int text_Random(int n)
{
    static uint16_t t = 0 ;
    t++;
    if(t>=999)t=0;
    srand(SysTick->VAL+n+t);                      /* 半主机模式下使用time函数会报错，在这里用系统定时器的值替代 */
    return rand() % 100;
}

CHAR_LENGTH charlength(char* chr)
{
    CHAR_LENGTH result;
    uint8_t i = 0;
#ifdef LCD_GB2312
    if((chr[0] & 0x80) != 0x00){           /* 是汉字，双字节 */
        if(chr[1]=='\0')return result;                     /* 错误 */
        result.Length = 2;
        result.chr[0] = chr[0];
        result.chr[1] = chr[1];
        result.chr[2] = '\0';
        
    }
    else{
        CharLength = 1;
        result.chr[0]=chr[0];
        result.chr[1] = '\0';
    }
#endif
    
#ifdef LCD_UTF8
/*判断UTF8编码第一个字节的标志位*/
    if ((chr[i] & 0x80) == 0x00)         //第一个字节为0xxxxxxx
    {   
        result.Length = 1;                     //字符为1字节
        result.chr[0] = chr[0];       //将第一个字节写入SingleChar第0个位置，随后i指向下一个字节
        result.chr[1] = '\0';               //为SingleChar添加字符串结束标志位
        //lcd_show_char(x,y,chr[0],size_h,mode,color);
    }   
    else if ((chr[i] & 0xE0) == 0xC0)    //第一个字节为110xxxxx
    {   
        result.Length = 2;	                    //字符为2字节
        result.chr[0] = chr[0];       //将第一个字节写入SingleChar第0个位置，随后i指向下一个字节
        if (chr[1] == '\0') {result.Length=0;return result;}     //意外情况，跳出循环，结束显示
        result.chr[1] = chr[1];       //将第二个字节写入SingleChar第1个位置，随后i指向下一个字节
        result.chr[2] = '\0';               //为SingleChar添加字符串结束标志位
    }
    else if ((chr[i] & 0xF0) == 0xE0)    //第一个字节为1110xxxx
    {
        result.Length = 3;                     //字符为3字节
        result.chr[0] = chr[0];
        if (chr[1] == '\0') {result.Length=0;return result;}
        result.chr[1] = chr[1];
        if (chr[2] == '\0') {result.Length=0;return result;}
        result.chr[2] = chr[2];
        result.chr[3] = '\0';
    }
    else if ((chr[i] & 0xF8) == 0xF0)    //第一个字节为11110xxx
    {
        result.Length = 4;                     //字符为4字节
        result.chr[0] = chr[0];
        if (chr[1] == '\0') {result.Length=0;return result;}
        result.chr[1] = chr[1];
        if (chr[2] == '\0') {result.Length=0;return result;}
        result.chr[2] = chr[2];
        if (chr[3] == '\0') {result.Length=0;return result;}
        result.chr[3] = chr[3];
        result.chr[4] = '\0';
    }

#endif
    return result;
    
}
;


void set_tasks(struct Text_Animation text){//设置任务,比较两个字符串的不同字符,并记录下来,用于后面动画显示%text.end_tim
	static int r =0;
	int new_len = strlen(text.phrases[text.idx]);
    int old_len = strlen(text.showstr);
    task_len = 0;//重置任务数量
    int oi = 0,ni = 0;
    CHAR_LENGTH old_singlechar[100];
    CHAR_LENGTH new_singlechar[100];
    
    printf("old_len:%d,new:%d\n",old_len,new_len);
    
    for(int i = 0;text.showstr[i]!='\0';){//将旧字符串拆分成单个字符集
        CHAR_LENGTH re=charlength(&text.showstr[i]);
        if(re.Length&&(strcmp(re.chr,"\0")!=0)){
            old_singlechar[oi++] = re ;
            i+=re.Length;
            
            printf("OLDStr:%s\n",re.chr);
            
        }
        else{
            break;
        }
    }
    
    printf("OLDSin:%d\n",oi);
    
    for(int i = oi ;i < 100 ;i++){//将剩下的部分全部填上\0
        old_singlechar[i].chr[0] = '\0';
        old_singlechar[i].Length = 1;
    }
    
    for(int i = 0;i<new_len;){//将新字符串拆分成单个字符集
        CHAR_LENGTH re=charlength(&text.phrases[text.idx][i]);
        if(re.Length){
            new_singlechar[ni++] = re ;
            i+=re.Length;
            
            printf("newStr:%s\n",re.chr);
            
        }else{
            break;
        }
        
    }
    
    printf("newSin:%d\n",ni);
    
    for(int i = oi ;i < 100 ;i++){//将剩下的部分全部填上\0
        new_singlechar[i].chr[0] = '\0';
        new_singlechar[i].Length = 1;
    }
    
    int lenth = ni > oi ? ni : oi;
    
    
    for(int i=0;i<lenth;i++){

        if(strcmp(old_singlechar[i].chr ,new_singlechar[i].chr)!=0){

            tasks[i].from = old_singlechar[i];

            tasks[i].to = new_singlechar[i];

            tasks[i].start = text_Random(r++) % 10 ;
            
            tasks[i].end = text_Random(r++) % 50;
            
        }else{
            tasks[i].from = old_singlechar[i];
            tasks[i].to = new_singlechar[i];
//            tasks[i].start = 0;
//            tasks[i].end = 0;
        }
        
        
    }
    
    for(int i=0;i<lenth;i++){
        tasks[i].start = text_Random(r++) % 10 ;
        tasks[i].end = text_Random(r++) % 50;
        printf("tasks:f:%s,t:%s,s:%d,e:%d\n",tasks[i].from.chr,tasks[i].to.chr,tasks[i].start,tasks[i].end);
    }
    task_len = lenth;
    
    for(int i = lenth; i < 100 ; i ++ ){
        tasks[i].from.chr[0] = '\0';
        tasks[i].from.Length = 1;
        tasks[i].to.chr[0] = '\0';
        tasks[i].to.Length = 1;
        tasks[i].start = 0;
        tasks[i].end = 0;
    }
    
    
    
    
    for(int i=0;i<=lenth;i++){
        printf("tasks:f:%s,t:%s,s:%d,e:%d\n",tasks[i].from.chr,tasks[i].to.chr,tasks[i].start,tasks[i].end);
        
    }
}


void update_showstr(struct Text_Animation *text){//更新显示字符串
    
    CHAR_LENGTH new_str[100];
    //text->fram++;//更新帧数
    if(text->fram > text->disp_time){//如果超过显示时间，则切换到下一句话
        text->fram = 0;
        for(int i=strlen(text->phrases[text->idx]);i<100;i++){
            text->showstr[i] =' ';
        }
        do{
            text->idx++;
        }while(text->phrases[text->idx][0] == '\0' && text->idx < PHRASES_NUM);
        if(text->idx >= PHRASES_NUM){
            if(text->loop){
                text->idx = 0;
            }else{
                //text->idx--;
                return;
            }
        }
        set_tasks(*text);
    }
    text->fram++;//更新帧数
//    int new_len = strlen(text->phrases[text->idx]);
//    int old_len = strlen(text->showstr);
//    int lenth = new_len > old_len ? new_len : old_len;
    
    int length = 0;
    int si=0;
    while(text->showstr[si]!='\0'){
        new_str[length] = charlength(&text->showstr[si]);
        si+=new_str[length].Length;
        length++;
    }
    new_str[length].chr[0]='\0';
    new_str[length].Length = 1;
    
//    for(int i=0;i<100;i++){
//        new_str[i] = text->showstr[i];//复制当前显示字符串
//    }
    
    
    si=0;//字符串用索引
    for(int i=0;i<task_len;i++){//遍历任务数组，如果任务开始时间在当前帧数之前，且当前帧数在任务结束时间之前，则显示乱码字符
        if(tasks[i].start <= text->fram && text->fram <= tasks[i].end){
            if(text_Random(i+text->fram)<70){//替换为另一个乱码
                
                new_str[i]=text->obfu_chars[text_Random(i+text->fram+si)%OBFU_NUM];
                
                
            }else{//保持不变
                
                
            }
            
        }else if(tasks[i].end < text->fram){//替换成目标字符
            
            new_str[i]=tasks[i].to;
            
        }else{//什么都不做，跳过这一个
            
            
            
        }
        si++;
    }
    new_str[si].chr[0]='\0';
    new_str[si].Length = 1;
    
    
    int ssi=0;//转录字符串
    for(int i=0;i<si;i++){
        //printf("%s",);
        for(int j =0;j<new_str[i].Length;j++){
            text->showstr[ssi++]=new_str[i].chr[j];
        }
    }
    printf("show:%s\n",text->showstr);
    
}













