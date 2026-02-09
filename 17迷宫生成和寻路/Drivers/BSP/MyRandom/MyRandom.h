#ifndef MY_RANDOM_H
#define MY_RANDOM_H

#include "stdlib.h"
#include "./SYSTEM/sys/sys.h"
#include "./SYSTEM/delay/delay.h"
#include "./BSP/MyCryptography/MyCryptography.h"
#include <math.h>

enum Random_Mode{
    PRNG_LCG,//线性同余
    PRNG_BBS,
    PRNG_MT19937,
};

typedef struct {
    int a,c,m;
    unsigned int seed;
}PRNG_SEED;


/*根据给定的数和系统时钟读数生成随机数*/
unsigned int MyRandom_sand_with_timer(int n);

/*生成伪随机数流，一次生成一个字节长。reset为0时，sand无效；
    当reset为1时，sand有效，表示抛弃上次的种子，设置随机数种子。
    mode指定生成方式。
*/
uint8_t MyRandom_PRNG_STREAM(uint8_t reset,PRNG_SEED seed,enum Random_Mode mode);


/*线性同余生成器*/
unsigned int MyRandom_PRNG_LCG(PRNG_SEED seed);

/*BBS*/
unsigned int MyRandom_PRNG_BBS(PRNG_SEED seed);






/*马特赛特旋转演算法*/



#endif

