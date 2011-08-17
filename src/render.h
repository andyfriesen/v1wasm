extern char layer0, layer1, layervc, layervc2, layervcwrite, cameratracking, drawparty, drawentities;
extern unsigned char* vcscreen, hookretrace, *vspmask, *vcscreen2, *vcscreen1;
extern int quakex, quakey, quake, screengradient, tileidx[2048], vspspeed;
extern int foregroundlock, xwin1, ywin1;

void animate(int);
void drawmap();
void InitRenderSystem();
