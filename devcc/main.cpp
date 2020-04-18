
#include <string>
#include <vector>
#include <stdexcept>

using Bytecode = std::vector<unsigned char>;

struct Code {
    std::vector<unsigned> offsets;
    Bytecode code;
};

Code loadCode(const std::string& fileName) {
    FILE* f = fopen(fileName.c_str(), "rb");

    fseek(f, 0, SEEK_END);
    const unsigned size = ftell(f);

    fseek(f, 68, SEEK_SET);

    short mx, my;
    fread(&mx, 1, 2, f);
    fread(&my, 1, 2, f);

    fseek(f, 100+(mx*my*5)+7956, SEEK_SET);

    unsigned a;
    char i;
    fread(&a, 1, 4, f);
    fseek(f, 88*a, 1);
    fread(&i, 1, 1, f);
    fread(&a, 1, 4, f);
    fseek(f, (i*4)+a, SEEK_CUR);

    unsigned numscripts;
    fread(&numscripts, 1, 4, f);

    std::vector<unsigned> scriptofstbl;
    scriptofstbl.resize(numscripts);
    fread(scriptofstbl.data(), 4, numscripts, f);

    std::vector<char> code;
    code.resize(size - ftell(f));
    fread(code.data(), 1, code.size(), f);
    fclose(f);

    return Code{ std::move(scriptofstbl), std::move(code) };
}

enum Op {
    Exec,
    Var0Assign,
    Var1Assign,
    Var2Assign,
    If,
    ForLoop0,
    ForLoop1,
    Goto,
    Switch,
    EndScript
};

void decodeBlock(const Bytecode& code, int& i);
void decodeExec(const Bytecode& code, int& i);
void decodeOperand(const Bytecode& code, int& i);

void decode(const Code& code, int index) {
    const unsigned startOffset = code.offsets[index];
    const unsigned endOffset = (index == code.offsets.size() - 1) ? code.offsets.size() : code.offsets[index + 1];

    Bytecode bc{ code.code.begin() + startOffset, code.code.begin() + endOffset };
    int i = 0;
    decodeBlock(bc, i);
}

unsigned char getC(const Bytecode& code, int& i) {
    return code[i++];
}

std::string getString(const Bytecode& code, int& i) {
    int startI = i;
    while (code[i]) {
        ++i;
    }
    ++i;

    return std::string(code.begin() + startI, code.begin() + i);
}

void decodeBlock(const Bytecode& code, int& i) {
    auto c = getC();

    switch (c) {
        case Exec: {
            decodeExec(code, i);
            break;
        }

        case Var0Assign: {
            break;
        }

        case Var1Assign: {
            break;
        }

        case Var2Assign: {
            break;
        }

        case If: {
            break;
        }

        case ForLoop0: {
            break;
        }

        case ForLoop1: {
            break;
        }

        case Goto: {
            break;
        }

        case Switch: {
            break;
        }

        case EndScript: {
            break;
        }

    }
}

void decodeExec(const Bytecode& code, int& i) {
    auto funcId = getC(code, i);
    switch (funcId) {
    case 1:
        printf("MapSwitch(");
        printf("\"%s\", ", getString(code, i));
        decodeOperand(code, i);
        printf(", ");
        decodeOperand(code, i);
        printf(", ");
        decodeOperand(code, i);
        printf(");\n");
        break;
//     case 2:
//         Warp();
//         break;
//     case 3:
//         AddCharacter();
//         break;
//     case 4:
//         SoundEffect();
//         break;
//     case 5:
//         GiveItem();
//         break;
//     case 6:
//         Text();
//         break;
//     case 7:
//         AlterFTile();
//         break;
//     case 8:
//         AlterBTile();
//         break;
//     case 9:
//         FakeBattle();
//         break;
//     case 10:
//         break;
//     case 11:
//         PlayMusic();
//         break;
//     case 12:
//         StopMusic();
//         break;
//     case 13:
//         HealAll();
//         break;
//     case 14:
//         AlterParallax();
//         break;
//     case 15:
//         FadeIn();
//         break;
//     case 16:
//         FadeOut();
//         break;
//     case 17:
//         RemoveCharacter();
//         break;
//     case 18:
//         Banner();
//         break;
//     case 19:
//         EnforceAnimation();
//         break;
//     case 20:
//         WaitKeyUp();
//         break;
//     case 21:
//         DestroyItem();
//         break;
//     case 22:
//         Prompt();
//         break;
//     case 23:
//         ChainEvent();
//         break;
//     case 24:
//         CallEvent();
//         break;
//     case 25:
//         Heal();
//         break;                                  // effect
//     case 26:
//         EarthQuake();
//         break;
//     case 27:
//         SaveMenu();
//         break;
//     case 28:
//         EnableSave();
//         break;
//     case 29:
//         DisableSave();
//         break;
//     case 30:
//         ReviveChar();
//         break;                            // effect
//     case 31:
//         RestoreMP();
//         break;                             // effect
//     case 32:
//         Redraw();
//         break;
//     case 33:
//         SText();
//         break;
//     case 34:
//         DisableMenu();
//         break;
//     case 35:
//         EnableMenu();
//         break;
//     case 36:
//         Wait();
//         break;
//     case 37:
//         SetFace();
//         break;
//     case 38:
//         MapPaletteGradient();
//         break;
//     case 39:
//         BoxFadeOut();
//         break;
//     case 40:
//         BoxFadeIn();
//         break;
//     case 41:
//         GiveGP();
//         break;
//     case 42:
//         TakeGP();
//         break;
//     case 43:
//         ChangeZone();
//         break;
//     case 44:
//         GetItem();
//         break;
//     case 45:
//         ForceEquip();
//         break;
//     case 46:
//         GiveXP();
//         break;
//     case 47:
//         Shop();
//         break;
//     case 48:
//         PaletteMorph();
//         break;
//     case 49:
//         ChangeCHR();
//         break;
//     case 50:
//         readcontrols();
//         break;
//     case 51:
//         VCPutPCX();
//         break;
//     case 52:
//         HookTimer();
//         break;
//     case 53:
//         HookRetrace();
//         break;
//     case 54:
//         VCLoadPCX();
//         break;
//     case 55:
//         VCBlitImage();
//         break;
//     case 57:
//         VCClear();
//         break;
//     case 58:
//         VCClearRegion();
//         break;
//     case 59:
//         VCText();
//         break;
//     case 60:
//         VCTBlitImage();
//         break;
//     case 61:
//         Exit();
//         break;
//     case 62:
//         Quit();
//         break;
//     case 63:
//         VCCenterText();
//         break;
//     case 64:
//         ResetTimer();
//         break;
//     case 65:
//         VCBlitTile();
//         break;
//     case 66:
//         Sys_ClearScreen();
//         break;
//     case 67:
//         Sys_DisplayPCX();
//         break;
//     case 68:
//         OldStartupMenu();
//         break;
//     case 69:
//         vgadump();
//         break;
//     case 70:
//         NewGame();
//         break;
//     case 71:
//         LoadSaveErase(0);
//         break;
//     case 72:
//         Delay();
//         break;
//     case 73:
//         PartyMove();
//         break;
//     case 74:
//         EntityMove();
//         break;
//     case 75:
//         AutoOn();
//         break;
//     case 76:
//         AutoOff();
//         break;
//     case 77:
//         EntityMoveScript();
//         break;
//     case 78:
//         VCTextNum();
//         break;
//     case 79:
//         VCLoadRaw();
//         break;

//     case 80:
//         VCBox();
//         break;       /* -- ric: 21/Apr/98 -- */
//     case 81:
//         VCCharName();
//         break;  /* -- ric: 21/Apr/98 -- */
//     case 82:
//         VCItemName();
//         break;  /* -- ric: 21/Apr/98 -- */
//     case 83:
//         VCItemDesc();
//         break;  /* -- ric: 21/Apr/98 -- */
//     case 84:
//         VCItemImage();
//         break; /* -- ric: 22/Apr/98 -- */
//     case 85:
//         VCATextNum();
//         break;  /* -- ric: 24/Apr/98 -- */
//     case 86:
//         VCSpc();
//         break;       /* -- ric: 24/Apr/98 -- */
//     case 87:
//         CallEffect();
//         break;  /* -- ric: 24/Apr/98 -- */
//     case 88:
//         CallScript();
//         break;  /* -- ric: 25/Apr/98 -- */

//     case 89:
//         VCLine();
//         break;
//     case 90:
//         GetMagic();
//         break;
//     case 91:
//         BindKey();
//         break;     /* -- ric: 03/May/98 -- */
//     case 92:
//         TextMenu();
//         break;    /* -- ric: 03/May/98 -- */
//     case 93:
//         itemMenu();
//         break;    /* -- ric: 03/May/98 -- */
//     case 94:
//         equipMenu();
//         break;   /* -- ric: 03/May/98 -- */
//     case 95:
//         magicMenu();
//         break;   /* -- ric: 03/May/98 -- */
//     case 96:
//         statusScreen();
//         break;/* -- ric: 03/May/98 -- */
//     case 97:
//         VCCr2();
//         break;       /* -- ric: 03/May/98 -- */
//     case 98:
//         VCSpellName();
//         break;
//     case 99:
//         VCSpellDesc();
//         break;
//     case 100:
//         VCSpellImage();
//         break;
//     case 101:
//         MagicShop();
//         break;
//     case 102:
//         VCTextBox();
//         break;   /* -- ric: 04/May/98 -- */
//     case 103:
//         PlayVAS();
//         break;

// #if 0
//     case 104:
//         SmallText();
//         break;       // ANDY 03/Mar/99
//     case 105:
//         VCEllipse();
//         break;       // ANDY 30/Jul/99
// #endif

//     case 104:
//         StringMenu();     /* -- Wyrdwad: 07/26/98 -- */
//         break;
//     case 105:
//         VCStringPCX();    /* -- Wyrdwad: 07/26/98 -- */
//         break;
//     case 106:
//         VCLoadStrPCX();   /* -- Wyrdwad: 07/26/98 -- */
//         break;
//     case 107:
//         PlayMusStr();     /* -- Wyrdwad: 07/26/98 -- */
//         break;
//     case 108:
//         VCString();       /* -- Wyrdwad: 07/26/98 -- */
//         break;
//     case 109:
//         VCCenterString(); /* -- Wyrdwad: 07/26/98 -- */
//         break;
//     case 110:
//         AssignString();   /* -- Wyrdwad: 07/26/98 -- */
//         break;
//     case 111:
//         CloseInfile();    /* -- Wyrdwad: 07/26/98 -- */
//         break;
//     case 112:
//         CloseOutfile();   /* -- Wyrdwad: 07/26/98 -- */
//         break;
//     case 113:
//         CloseFiles();     /* -- Wyrdwad: 07/26/98 -- */
//         break;
//     case 114:
//         OpenInfile();     /* -- Wyrdwad: 07/27/98 -- */
//         break;
//     case 115:
//         OpenOutfile();    /* -- Wyrdwad: 07/27/98 -- */
//         break;
//     case 116:
//         OpenOutfAppend(); /* -- Wyrdwad: 07/27/98 -- */
//         break;
//     case 117:
//         OpenInstring();   /* -- Wyrdwad: 07/27/98 -- */
//         break;
//     case 118:
//         OpenOutstring();  /* -- Wyrdwad: 07/27/98 -- */
//         break;
//     case 119:
//         OpenOutsAppend(); /* -- Wyrdwad: 07/27/98 -- */
//         break;
//     case 120:
//         IntIn();          /* -- Wyrdwad: 07/27/98 -- */
//         break;
//     case 121:
//         IntOut();         /* -- Wyrdwad: 07/27/98 -- */
//         break;
//     case 122:
//         StrIn();          /* -- Wyrdwad: 07/30/98 -- */
//         break;
//     case 123:
//         StrOut();         /* -- Wyrdwad: 07/27/98 -- */
//         break;
//     case 124:
//         FileTextOut();    /* -- Wyrdwad: 07/27/98 -- */
//         break;
//     case 125:
//         StringBanner();   /* -- Wyrdwad: 07/30/98 -- */
//         break;
//     case 126:
//         CopyString();     /* -- Wyrdwad: 07/30/98 -- */
//         break;
//     case 127:
//         CompareString();  /* -- Wyrdwad: 07/30/98 -- */
//         break;
//     case 128:
//         Concatenate();    /* -- Wyrdwad: 07/30/98 -- */
//         break;
//     case 129:
//         StrText();        /* -- Wyrdwad: 07/30/98 -- */
//         break;
//     case 130:
//         StrSText();       /* -- Wyrdwad: 07/30/98 -- */
//         break;
//     case 131:
//         StrPrompt();      /* -- Wyrdwad: 07/30/98 -- */
//         break;
//     case 132:
//         BStringMenu();    /* -- Wyrdwad: 07/30/98 -- */
//         break;
//     case 133:
//         WordIn();         /* -- Wyrdwad: 07/31/98 -- */
//         break;
//     case 134:
//         WordOut();        /* -- Wyrdwad: 07/31/98 -- */
//         break;
//     case 135:
//         StrChangeCHR();   /* -- Wyrdwad: 08/02/98 -- */
//         break;


        //    case 104: VCMagicImage(); break;/* -- ric: 04/May/98 -- */
        //    case 105: vcwritelayer(); break; /* -- xBig_D: 05/May/98 */

    default:
        throw std::runtime_error(std::string{"*error* Unknown library function in VC code "} + std::to_string(funcId));
    }
}

enum class OpDesc {
    Immediate,
    Var0,
    Var1,
    Var2,
    Group
}

void decodeOperand(const Bytecode& code, int& i) {
    auto desc = getC(code, i);
    switch (desc) {
        case OpDesc::Immediate:
        case OpDesc::Var0:
        case OpDesc::Var1:
        case OpDesc::Var2:
        case OpDesc::Group:
    }
}

int main(int argc, char** argv) {
    Code code = loadCode(argv[1]);
}
