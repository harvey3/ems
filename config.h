#ifndef CONFIG_HEADER
#define CONFIG_HEADER


#define TABLE_SIZE 60

typedef struct addr_block 
{
    unsigned short addr;
    unsigned short count;

} AddrBlock;

typedef struct addr_table
{
    int cnt;
    int abCnt;
    AddrBlock ab[10];
    unsigned short addr[TABLE_SIZE];
    unsigned short value[TABLE_SIZE];



}AddrTable;


enum STATE
{
      CHECK_REG_NAME=1,

      CHECK_OP,
      CHECK_DONE
};
typedef struct parse_result 
{
    char regName[30];
    union counter opQ[MAX_ELEMENT];
    char flag[MAX_ELEMENT];    

} ParseResult;


extern AddrTable gAirConT;
extern AddrTable gAirConWhMeterT;
extern AddrTable gOffGridWhMeterT;
extern AddrTable gOnGridWhMeterT;
extern AddrTable gPcsT;
extern AddrTable gBmsT;
extern AddrTable gThermoT;
extern AddrTable gSensorT;

int reverseCount(int value, union counter *new_res, char *flag, AddrTable *t);
int count(union counter *new_res, char *flag, AddrTable *t);
void buildAddrTable(AddrTable *t, ParseResult *pr);
int buildAddrBlock(AddrTable *t);
void parseConfig(const char *path);
int lookupAddrTable(AddrTable *t, unsigned short addr);
inline void MsecSleep(int msec);


#endif
