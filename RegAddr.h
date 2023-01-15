#ifndef REG_ADDR
#define REG_ADDR

#define MAX_ELEMENT 10

union counter
{
    char ch;
    unsigned int num;
    unsigned short addr;
    
};

typedef struct reg_t
{
    int value;
    union counter opQ[MAX_ELEMENT];
    char flag[MAX_ELEMENT];    

} REG_T;

typedef struct ureg_t
{
    unsigned int value;
    union counter opQ[MAX_ELEMENT];
    char flag[MAX_ELEMENT];    

} UREG_T;


#define ADDR2INDEX(base, addr) (addr - base)
#endif
