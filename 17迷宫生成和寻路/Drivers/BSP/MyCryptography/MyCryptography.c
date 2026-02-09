#include "./BSP/MyCryptography/MyCryptography.h"



const char Base64Code[] = {
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N',
    'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b',
    'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p',
    'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '0', '1', '2', '3',
    '4', '5', '6', '7', '8', '9', '+', '/', '='
};
;


/*base64编解码。
流程：将明文以三个字节为一页，将这三个字节拼接成24位；
    再以6位为一组取出，高位补2个0，得到四个字节的密文。
    将密文在映照表里的字符一一替换，就得到了最终的密文。
若明文不是3的整数倍

*/
void My_BASE_64_Encode(uint8_t *m,uint8_t *cip){
    /*选位*/
    const uint32_t choose_bit[4] = {0xfc0000,0x3f000,0xfc0,0x3f,};
    int length = strlen((char *)m);//原始数据长度
    int page = length/3;    //分组长度
    uint32_t value = 0;
    
    for(int i=0;i<page;i++){
        /*拼接*/
        value = ((uint32_t)m[i*3]<<16)+((uint32_t)m[i*3+1]<<8)+m[i*3+2];
        /*拆分*/
        cip[i*4]=(value<<8)>>26;
        cip[i*4+1]=(value<<14)>>26;
        cip[i*4+2]=(value<<20)>>26;
        cip[i*4+3]=(value<<26)>>26;
    }
    /*处理结尾*/
    value=0;
    for(int i = 0;i<length-page*3;i++){
        value += (uint32_t)(m[page+i]<<(8*(2-i)));
    }
    cip[page*4]=(value<<8)>>26;
    cip[page*4+1]=(value<<14)>>26;
    cip[page*4+2]=(value<<20)>>26;
    cip[page*4+3]=(value<<26)>>26;
    /*替换字符*/
    for(int i = 0;i<page*4+4;i++){
        
    }
}





