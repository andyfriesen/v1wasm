// entity.c
// All entity-ish code.
// Copyright (C)1997 BJ Eirich

#include <stdio.h>
#include <stdlib.h>
#include "engine.h"
#include "vc.h"
void err(const char* ermsg);
int ObstructionAt(int tx, int ty);
void ProcessEntity(int);
int random(int, int);

// ============================ Data ============================

struct chrrec {
    char fname[13]                       __attribute__ ((packed));
};

struct chrrec chrlist[100];           // CHR list
int entities;                         // number of active entities
unsigned char nummovescripts = 0;     // number of movement scripts
int msofstbl[100];                    // movement script offset table
char* msbuf;                          // movement script data buffer
char movesuccess;                     // if call to MoveXX() was successful

// ============================ Code ============================

int EntityAt(int ex, int ey) {
    int i;

    for (i = 5; i < entities; i++)
        if (ex == party[i].cx && ey == party[i].cy) {
            return i;
        }

    return 0;
}

int AnyEntityAt(int ex, int ey) {
    int i, st;

    if (autoent) {
        st = 5;
    } else {
        st = 0;
    }

    for (i = st; i < entities; i++)
        if (ex == party[i].cx && ey == party[i].cy) {
            return i + 1;
        }

    return 0;
}

void MoveRight(int i) {
    int tx, ty;

    tx = party[i].cx + 1;
    ty = party[i].cy;
    if (!party[i].obsmode && (ObstructionAt(tx, ty) || (i > 4 && AnyEntityAt(tx, ty)))) {
        movesuccess = 0;
        return;
    }
    party[i].x++;
    party[i].facing = 2;
    party[i].moving = 3;
    party[i].movcnt = 15;
    party[i].cx++;
    movesuccess = 1;
}

void MoveLeft(int i) {
    int tx, ty;

    tx = party[i].cx - 1;
    ty = party[i].cy;
    if (!party[i].obsmode && (ObstructionAt(tx, ty) || (i > 4 && AnyEntityAt(tx, ty)))) {
        movesuccess = 0;
        return;
    }
    party[i].x--;
    party[i].facing = 3;
    party[i].moving = 4;
    party[i].movcnt = 15;
    party[i].cx--;
    movesuccess = 1;
}

void MoveUp(int i) {
    int tx, ty;

    tx = party[i].cx;
    ty = party[i].cy - 1;
    if (!party[i].obsmode && (ObstructionAt(tx, ty) || (i > 4 && AnyEntityAt(tx, ty)))) {
        movesuccess = 0;
        return;
    }
    party[i].y--;
    party[i].facing = 1;
    party[i].moving = 2;
    party[i].movcnt = 15;
    party[i].cy--;
    movesuccess = 1;
}

void MoveDown(int i) {
    int tx, ty;

    tx = party[i].cx;
    ty = party[i].cy + 1;
    if (!party[i].obsmode && (ObstructionAt(tx, ty) || (i > 4 && AnyEntityAt(tx, ty)))) {
        movesuccess = 0;
        return;
    }
    party[i].y++;
    party[i].facing = 0;
    party[i].moving = 1;
    party[i].movcnt = 15;
    party[i].cy++;
    movesuccess = 1;
}

int Zone(int cx, int cy) {
    return mapp[((cy * xsize) + cx)] >> 1;
}

void ProcessSpeedAdjEntity(int i) {
    if (party[i].speed < 4) {
        switch (party[i].speed) {
        case 1:
            if (party[i].speedct < 3) {
                party[i].speedct++;
                return;
            }
        case 2:
            if (party[i].speedct < 2) {
                party[i].speedct++;
                return;
            }
        case 3:
            if (party[i].speedct < 1) {
                party[i].speedct++;
                return;
            }
        }
    }
    if (party[i].speed < 5) {
        ProcessEntity(i);
    }
    switch (party[i].speed) {
    case 5:
        ProcessEntity(i);
        ProcessEntity(i);
        return;
    case 6:
        ProcessEntity(i);
        ProcessEntity(i);
        ProcessEntity(i);
        return;
    case 7:
        ProcessEntity(i);
        ProcessEntity(i);
        ProcessEntity(i);
        ProcessEntity(i);
        return;
    }
}

void Wander1(int i) {
    if (!party[i].data1) {
        party[i].data2 = random(0, 3);
        party[i].data1 = party[i].step + 1;
    }
    if (party[i].data1 == 1) {
        party[i].delayct++;
        if (party[i].delayct >= party[i].delay) {
            party[i].data1 = 0;
        }
        return;
    }
    if (party[i].data1 > 1) {
        switch (party[i].data2) {
        case 0:
            MoveUp(i);
            break;
        case 1:
            MoveDown(i);
            break;
        case 2:
            MoveLeft(i);
            break;
        case 3:
            MoveRight(i);
            break;
        }
        party[i].data1--;
        if (party[i].data1 == 1) {
            party[i].delayct = 0;
        }
    }
}

void Wander2(int i) {
    if (!party[i].data1) {
        party[i].data2 = random(0, 3);
        party[i].data1 = party[i].step + 1;
    }
    if (party[i].data1 == 1) {
        party[i].delayct++;
        if (party[i].delayct >= party[i].delay) {
            party[i].data1 = 0;
        }
        return;
    }
    if (party[i].data1 > 1) {
        switch (party[i].data2) {
        case 0:
            if (party[i].cy > party[i].y1) {
                MoveUp(i);
            }
            break;
        case 1:
            if (party[i].cy < party[i].y2) {
                MoveDown(i);
            }
            break;
        case 2:
            if (party[i].cx > party[i].x1) {
                MoveLeft(i);
            }
            break;
        case 3:
            if (party[i].cx < party[i].x2) {
                MoveRight(i);
            }
            break;
        }
        party[i].data1--;
        if (party[i].data1 == 1) {
            party[i].delayct = 0;
        }
    }
}

void Wander3(int i) {
    if (!party[i].data1) {
        party[i].data2 = random(0, 3);
        party[i].data1 = party[i].step + 1;
    }
    if (party[i].data1 == 1) {
        party[i].delayct++;
        if (party[i].delayct >= party[i].delay) {
            party[i].data1 = 0;
        }
        return;
    }
    if (party[i].data1 > 1) {
        switch (party[i].data2) {
        case 0:
            if (Zone(party[i].cx, party[i].cy - 1) == party[i].data3) {
                MoveUp(i);
            }
            break;
        case 1:
            if (Zone(party[i].cx, party[i].cy + 1) == party[i].data3) {
                MoveDown(i);
            }
            break;
        case 2:
            if (Zone(party[i].cx - 1, party[i].cy) == party[i].data3) {
                MoveLeft(i);
            }
            break;
        case 3:
            if (Zone(party[i].cx + 1, party[i].cy) == party[i].data3) {
                MoveRight(i);
            }
            break;
        }
        party[i].data1--;
        if (party[i].data1 == 1) {
            party[i].delayct = 0;
        }
    }
}

void Whitespace(int i) {
    while (*party[i].scriptofs == ' ') {
        party[i].scriptofs++;
    }
}

void GetArg(int i) {
    int j;
    char token[10];

    j = 0;
    Whitespace(i);
    while (*party[i].scriptofs >= 48 && *party[i].scriptofs <= 57) {
        token[j] = *party[i].scriptofs;
        party[i].scriptofs++;
        j++;
    }
    token[j] = 0;
    party[i].cmdarg = atoi(&token[0]);
}

void GetNextCommand(int i) {
    unsigned char s;

    Whitespace(i);
    s = *party[i].scriptofs;
    party[i].scriptofs++;
    switch (s) {
    case 'U':
        party[i].curcmd = 1;
        GetArg(i);
        break;
    case 'D':
        party[i].curcmd = 2;
        GetArg(i);
        break;
    case 'L':
        party[i].curcmd = 3;
        GetArg(i);
        break;
    case 'R':
        party[i].curcmd = 4;
        GetArg(i);
        break;
    case 'S':
        party[i].curcmd = 5;
        GetArg(i);
        break;
    case 'W':
        party[i].curcmd = 6;
        GetArg(i);
        break;
    case 0:
        party[i].movecode = 0;
        party[i].curcmd = 7;
        party[i].cmdarg = 0;
        party[i].scriptofs = 0;
        break;
    case 'C':
        party[i].curcmd = 8;
        GetArg(i);
        break;
    case 'B':
        party[i].curcmd = 9;
        break;
    case 'X':
        party[i].curcmd = 10;
        GetArg(i);
        break;
    case 'Y':
        party[i].curcmd = 11;
        GetArg(i);
        break;
    case 'F':
        party[i].curcmd = 12;
        GetArg(i);
        break;
    case 'Z':
        party[i].curcmd = 13;
        GetArg(i);
        break;
    default:
        err("Invalid entity movement script.");
    }
}

void MoveScript(int i) {
    if (!party[i].scriptofs) {
        party[i].scriptofs = (msbuf + msofstbl[party[i].movescript]);
    }
    if (!party[i].curcmd) {
        GetNextCommand(i);
    }

    switch (party[i].curcmd) {
    case 1:
        MoveUp(i);
        if (movesuccess) {
            party[i].cmdarg--;
        }
        break;
    case 2:
        MoveDown(i);
        if (movesuccess) {
            party[i].cmdarg--;
        }
        break;
    case 3:
        MoveLeft(i);
        if (movesuccess) {
            party[i].cmdarg--;
        }
        break;
    case 4:
        MoveRight(i);
        if (movesuccess) {
            party[i].cmdarg--;
        }
        break;
    case 5:
        party[i].speed = party[i].cmdarg;
        party[i].cmdarg = 0;
        break;
    case 6:
        party[i].cmdarg--;
        break;
    case 7:
        return;
    case 8:
        ExecuteScript(party[i].cmdarg);
        party[i].cmdarg = 0;
        break;
    case 9:
        party[i].scriptofs = msbuf + msofstbl[party[i].movescript];
        party[i].cmdarg = 0;
        break;
    case 10:
        if (party[i].cx < party[i].cmdarg) {
            MoveRight(i);
        }
        if (party[i].cx > party[i].cmdarg) {
            MoveLeft(i);
        }
        if (party[i].cx == party[i].cmdarg) {
            party[i].cmdarg = 0;
        }
        break;
        break;
    case 11:
        if (party[i].cy < party[i].cmdarg) {
            MoveDown(i);
        }
        if (party[i].cy > party[i].cmdarg) {
            MoveUp(i);
        }
        if (party[i].cy == party[i].cmdarg) {
            party[i].cmdarg = 0;
        }
        break;
        break;
    case 12:
        party[i].facing = party[i].cmdarg;
        party[i].cmdarg = 0;
        break;
    case 13:
        party[i].specframe = party[i].cmdarg;
        party[i].cmdarg = 0;
        break;
    }
    if (!party[i].cmdarg) {
        party[i].curcmd = 0;
    }
}

void TestActive(int i) {
    int dx, dy;

    dx = abs(party[i].x - party[0].x);
    dy = abs(party[i].y - party[0].y);
    if ((dx <= 16 && dy <= 3) ||
            (dx <= 3 && dy <= 16)) {
        if (!party[i].adjactv) {
            ExecuteHookedScript(party[i].actscript);
        }
        party[i].adjactv = 1;
    } else {
        party[i].adjactv = 0;
    }
}

void Chase(int i) {
    int dx, dy, d;

    dx = party[0].cx - party[i].cx;
    dy = party[0].cy - party[i].cy;

    if (abs(dx) < abs(dy)) {
        d = 0;
    } else {
        d = 1;
    }

    if (d && dx < 0) {
        MoveLeft(i);
    }
    if (d && dx > 0) {
        MoveRight(i);
    }
    if (!d && dy < 0) {
        MoveUp(i);
    }
    if (!d && dy > 0) {
        MoveDown(i);
    }
}

void CheckChasing(int i) {
    if (abs(party[0].cx - party[i].cx) <= party[i].chasedist &&
            abs(party[0].cy - party[i].cy) <= party[i].chasedist) {
        party[i].chasing = 2;
        party[i].movecode = 5;
        party[i].speed = party[i].chasespeed;
    }
}

void ProcessEntity(int i) {
    party[i].speedct = 0;
    if (party[i].activmode) {
        TestActive(i);
    }
    if (party[i].chasing == 1) {
        CheckChasing(i);
    }

    if (!party[i].moving) {
        switch (party[i].movecode) {
        case 0:
            return;
        case 1:
            Wander1(i);
            break;
        case 2:
            Wander2(i);
            break;
        case 3:
            Wander3(i);
            break;
        case 4:
            MoveScript(i);
            break;
        case 5:
            Chase(i);
            break;
        default:
            err("*error* unknown entity movement pattern");
        }
    }

    if (party[i].moving) {
        if (party[i].moving == 1) {
            party[i].y++;
            party[i].movcnt--;
            party[i].framectr++;
        }
        if (party[i].moving == 2) {
            party[i].y--;
            party[i].movcnt--;
            party[i].framectr++;
        }
        if (party[i].moving == 3) {
            party[i].x++;
            party[i].movcnt--;
            party[i].framectr++;
        }
        if (party[i].moving == 4) {
            party[i].x--;
            party[i].movcnt--;
            party[i].framectr++;
        }
        if (party[i].framectr == 80) {
            party[i].framectr = 0;
        }
        if (!party[i].movcnt) {
            party[i].moving = 0;
        }
    }
}

