int time();
void delay(int ms);
int timer_init();
int timer_close();
extern unsigned hooktimer;
extern unsigned char an, tickctr, sec, min, hr;

void setTimerCount(int value);
void decTimerCount();
void incTimerCount();
int getTimerCount();

int getVcTimer();
void setVcTimer(int value);