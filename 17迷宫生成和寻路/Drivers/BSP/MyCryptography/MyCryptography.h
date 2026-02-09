#ifndef MY_CRYPTOGRAPHY_H
#define MY_CRYPTOGRAPHY_H

#include "stdlib.h"
#include "./SYSTEM/sys/sys.h"
#include "./SYSTEM/delay/delay.h"
#include <math.h>
#include <string.h>





/*AES-128加密算法。明文128位（16字节），密钥可以128/192/256位（16/24/32字节）。密文输出在cip里*/
void My_AES_128(uint8_t *key,uint8_t *m,uint8_t *cip);

/*BASE64编码解码,调用之前做好初始化，比如将cip清0*/
void My_BASE_64_Encode(uint8_t *m,uint8_t *cip);
void My_BASE_64_Deccode(uint8_t *cip,uint8_t *m);








#endif