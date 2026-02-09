#include "./BSP/MyRandom/MyRandom.h"






/*顺便一说：C语言自带的rand函数就是用线性同余实现的。最大值位32767*/
unsigned int MyRandom_sand_with_timer(int n)
{
    srand(SysTick->VAL+n);                      /* 半主机模式下使用time函数会报错，在这里用系统定时器的值替代 */
    return rand() % 1000;
}


/*线性同余生成器。
基于公式：
            Xn+1 = (aXn+c) mod m
生成。

生成周期T:
混合同余法（c=0）：
如果 c=0，则称为混合同余法。此时，周期 T 可以达到 m。
周期 T 最大为 m，当且仅当 a 和 m 互质，并且 a−1 能被 m 的所有质因数整除，且 c 与 m 互质。

选取参数的总结

    模数m尽可能大，一般大于2**30
    当m选取为2的幂次方时，应该满足a mod 8=5
    当m和a选取都合理时，c需要在与m互质的条件下选取。

最大周期符合的条件：

    m和c互质
    m的所有质因子的积能整除a-1
    如果m是4的倍数，a-1也是
    a,c,seed都比m小
    a,c是正整数
    
缺点：
    当m较大时，较低位的周期往往比整体周期要小很多。
    换言之，我们最好只取高位作为结果。
*/
/*已知部分编译器的取值（a,c,m）:
数据来自https://www.cnblogs.com/xkfz007/archive/2012/03/27/2420154.html
    
    1664525     1013904223      2^32    1
    22695477    1               2^32    2
    1103515245  12345           2^32    3
    134775813   1               2^32    4
    214013      2531011         2^32    5
    16807       0               2^31-1  6
我个人喜欢1、5、6 。
*/
unsigned int MyRandom_PRNG_LCG(PRNG_SEED seed){
    
    return( seed.a*seed.seed + seed.c) % seed.m ;
    
}



/*BBS是利用两个大素数的乘积作为模数，以seed为起点，迭代运算结果的算法

参数选择：
    素数p、q：p % 4 = 3，q % 4 = 3;m = p*q
    种子seed：seed与m互质；

公式：Si+1 = Si^2 % m

输出Si % 2 。本函数输出32位伪随机比特作为结果。

*/

/*参数
在我查到的一篇论文里，作者选择了p/q = 47/67(m=3149);s=7

*/
unsigned int MyRandom_PRNG_BBS(PRNG_SEED seed){
    
    return seed.seed*seed.seed % seed.m ;
    
}


/*分组密码
分为CTR(计数器模式)和OFB(输出反馈模式)，
种子分为两部分：密钥加密值和每次生成都会更新的V值。（V对应加密算法中的明文）
CTR模式下V值每次加一；OFB模式V值为上一次的输出

对于AES-128，种子分为128位密钥和128位V值组成。
(AES-128中128的意思是将明文以128个位为单位分块加密。密钥长度没增加64位，算法就需要额外循环两轮。)

*/












/*生成伪随机数流，一次生成一个字节长。reset为0时，sand无效；
    当reset为1时，sand有效，表示抛弃上次的种子，设置随机数种子。
    mode指定生成方式。
*/
uint8_t MyRandom_PRNG_STREAM(uint8_t reset,PRNG_SEED seed,enum Random_Mode mode){
    static PRNG_SEED last_output  = {0};
    if(reset)last_output = seed;
    if(mode == PRNG_LCG){
        
        last_output.seed = MyRandom_PRNG_LCG(last_output);
        
    }
    else if(mode == PRNG_BBS){
        
        last_output.seed = MyRandom_PRNG_BBS(last_output);
        
    }
    uint8_t out = last_output.seed>>24;//只取前8位
    return out;
}

;















