/* -- ric: 03/May/98 -- */
struct keyb_map {
    char      pressed;                  // keyboard flags
    short int boundscript;
};           // bound script

extern volatile struct keyb_map key_map[128];   // for recording bound keys
extern char b1, b2, b3, b4;            // four button flags for GamePad
extern char up, down, left, right;     // stick position flags
extern char kb1, kb2, kb3, kb4;        // config keys

void readcontrols();
void initcontrols(char jf);
