#include <string>
#include <vector>
#include <map>
#include <cctype>
#include <cstdint>
#include <algorithm>
#include <stdexcept>

// FIXME: line buffers for output, allow adding lines in earlier part of output during disassembly (for fixing up labels, etc).
// FIXME: indentation for blocks.
// FIXME: capitalization style? (eg. user-customizable case for variable names, function names, etc, since Verge allows any case)
// FIXME: indentation style? (brace on next line like, tab count)
// FIXME: spacing style? (space betwen operators, after commas, etc)

using std::uint8_t;
using std::int32_t;
using std::uint32_t;

using Bytecode = std::vector<uint8_t>;

enum CodeType {
    Script,
    Event,
    Effect,
};

struct Code {
    CodeType type;
    std::vector<uint32_t> offsets;
    Bytecode code;
};

struct Ctx {
    Bytecode& bytecode;
    uint32_t offset;

    std::map<uint32_t, std::string> code;

    bool isEOF() const {
        return offset >= bytecode.size();
    }

    void skip(uint32_t amount) {
        offset += amount;
    }

    void skipC() {
        skip(1);
    }

    void skipD() {
        skip(4);
    }

    void skipString() {
        while (bytecode[offset]) {
            ++offset;
        }
        ++offset;
    }    

    uint8_t getC() {
        return bytecode[offset++];
    }    

    int32_t getD() {
        const auto a = getC();
        const auto b = getC();
        const auto c = getC();
        const auto d = getC();
        return static_cast<int32_t>(
            (static_cast<uint32_t>(d) << 24U)
            | (static_cast<uint32_t>(c) << 16U)
            | (static_cast<uint32_t>(b) << 8U)
            | static_cast<uint32_t>(a));
    }

    std::string getString() {
        const auto startOffset = offset;
        skipString();
        return std::string(bytecode.begin() + startOffset, bytecode.begin() + offset);
    }

    void emit(uint32_t offset, std::string s) {
        code[offset] = std::move(s);
    }

    void dump() {
        for (const auto& pair: code) {
            printf("%s\n", pair.second.c_str());
        }
    }
};

enum class BlockOp {
    Exec = 1,
    Var0Assign = 2,
    Var1Assign = 3,
    Var2Assign = 4,
    If = 5,
    Goto = 7,
    ForLoop0 = 8,
    ForLoop1 = 9,
    Switch = 10,

    End = 255
};

enum class AssignOp {
    Set = 1,
    Increment = 2,
    Decrement = 3,
    IncSet = 4,
    DecSet = 5,
};

enum class BranchOp {
    Zero = 0,
    NonZero = 1,
    Equal = 2,
    NotEqual = 3,
    GreaterThan = 4,
    GreaterThanOrEqual = 5,
    LessThan = 6,
    LessThanOrEqual = 7,
};

enum class ArithmeticOp {
    Add = 1,
    Subtract = 2,
    Divide = 3,
    Multiply = 4,
    Modulo = 5,

    End = 255,
};

enum class OpTermType {
    Immediate = 1,
    Var0 = 2,
    Var1 = 3,
    Var2 = 4,
    Group = 5
};

enum class FuncId {
    MapSwitch = 1,
    Warp = 2,
    AddCharacter = 3,
    SoundEffect = 4,
    GiveItem = 5,
    Text = 6,
    AlterFTile = 7,
    AlterBTile = 8,
    FakeBattle = 9,
    Return = 10,
    PlayMusic = 11,
    StopMusic = 12,
    HealAll = 13,
    AlterParallax = 14,
    FadeIn = 15,
    FadeOut = 16,
    RemoveCharacter = 17,
    Banner = 18,
    EnforceAnimation = 19,
    WaitKeyUp = 20,
    DestroyItem = 21,
    Prompt = 22,
    ChainEvent = 23,
    CallEvent = 24,
    Heal = 25,
    EarthQuake = 26,
    SaveMenu = 27,
    EnableSave = 28,
    DisableSave = 29,
    ReviveChar = 30,
    RestoreMP = 31,
    Redraw = 32,
    SText = 33,
    DisableMenu = 34,
    EnableMenu = 35,
    Wait = 36,
    SetFace = 37,
    MapPaletteGradient = 38,
    BoxFadeOut = 39,
    BoxFadeIn = 40,
    GiveGP = 41,
    TakeGP = 42,
    ChangeZone = 43,
    GetItem = 44,
    ForceEquip = 45,
    GiveXP = 46,
    Shop = 47,
    PaletteMorph = 48,
    ChangeCHR = 49,
    ReadControls = 50,
    VCPutPCX = 51,
    HookTimer = 52,
    HookRetrace = 53,
    VCLoadPCX = 54,
    VCBlitImage = 55,
    PlayFLI = 56, // appears to be deprecated, as it's an illegal opcode.
    VCClear = 57,
    VCClearRegion = 58,
    VCText = 59,
    VCTBlitImage = 60,
    Exit = 61,
    Quit = 62,
    VCCenterText = 63,
    ResetTimer = 64,
    VCBlitTile = 65,
    Sys_ClearScreen = 66,
    Sys_DisplayPCX = 67,
    OldStartupMenu = 68,
    VGADump = 69,
    NewGame = 70,
    LoadSaveErase = 71,
    Delay = 72,
    PartyMove = 73,
    EntityMove = 74,
    AutoOn = 75,
    AutoOff = 76,
    EntityMoveScript = 77,
    VCTextNum = 78,
    VCLoadRaw = 79,
    VCBox = 80,
    VCCharName = 81,
    VCItemName = 82,
    VCItemDesc = 83,
    VCItemImage = 84,
    VCATextNum = 85,
    VCSpc = 86,
    CallEffect = 87,
    CallScript = 88,
    VCLine = 89,
    GetMagic = 90,
    BindKey = 91,
    TextMenu = 92,
    ItemMenu = 93,
    EquipMenu = 94,
    MagicMenu = 95,
    StatusScreen = 96,
    VCCr2 = 97,
    VCSpellName = 98,
    VCSpellDesc = 99,
    VCSpellImage = 100,
    MagicShop = 101,
    VCTextBox = 102,
    PlayVAS = 103,

    AndySmallText = 104,
    AndyVCEcclipse = 105,

    WyrdStringMenu = 104,
    WyrdVCStringPCX = 105,
    WyrdVCLoadStrPCX = 106,
    WyrdPlayMusStr = 107,
    WyrdVCString = 108,
    WyrdVCCenterString = 109,
    WyrdAssignString = 110,
    WyrdCloseInFile = 111,
    WyrdCloseOutFile = 112,
    WyrdCloseFiles = 113,
    WyrdOpenInFile = 114,
    WyrdOpenOutFile = 115,
    WyrdOpenOutFAppend = 116,
    WyrdOpenInString = 117,
    WyrdOpenOutString = 118,
    WyrdOpenOutSAppend = 119,
    WyrdIntIn = 120,
    WyrdIntOut = 121,
    WyrdStrIn = 122,
    WyrdStrOut = 123,    
    WyrdFileTextOut = 124,
    WyrdStringBanner = 125,
    WyrdCopyString = 126,
    WyrdCompareString = 127,
    WyrdConcatenate = 128,
    WyrdStrText = 129,
    WyrdStrSText = 130,
    WyrdStrPrompt = 131,
    WyrdBStringMenu = 132,
    WyrdWordIn = 133,
    WyrdWordOut = 134,
    WyrdStrChangeCHR = 135,
};

enum class Var0 {
    A = 0,
    B = 1,
    C = 2,
    D = 3,
    E = 4,
    F = 5,
    G = 6,
    H = 7,
    I = 8,
    J = 9,
    K = 10,
    L = 11,
    M = 12,
    N = 13,
    O = 14,
    P = 15,
    Q = 16,
    R = 17,
    S = 18,
    T = 19,
    U = 20,
    V = 21,
    W = 22,
    X = 23,
    Y = 24,
    Z = 25,
    NumChars = 26,
    GP = 27,
    LocX = 28,
    LocY = 29,
    Timer = 30,
    DrawParty = 31,
    Cameratracking = 32,
    XWin = 33,
    YWin = 34,
    B1 = 35,
    B2 = 36,
    B3 = 37,
    B4 = 38,
    Up = 39,
    Down = 40,
    Left = 41,
    Right = 42,
    TimerAnimate = 43,
    Fade = 44,
    Layer0 = 45,
    Layer1 = 46,
    LayerVC = 47,
    QuakeX = 48,
    QuakeY = 49,
    Quake = 50,
    ScreenGradient = 51,
    PMultX = 52,
    PMultY = 53,
    PDivX = 54,
    PDivY = 55,
    Volume = 56,
    ParallaxC = 57,
    CancelFade = 58,
    DrawEntities = 59,
    CurZone = 60,
    TileB = 61,
    TileF = 62,
    ForegroundLock = 63,
    XWin1 = 64,
    YWin1 = 65,
    Layer1Trans = 66,
    LayerVCTrans = 67,
    FontColor = 68,
    KeepAZ = 69,
    LayerVC2 = 70,
    LayerVC2Trans = 71,
    VCWriteLayer = 72,
    ModPosition = 73,
};

enum class Var1 {
    Flags,
    Facing,
    Char,
    Item,
    Var,
    PartyIndex,
    XP,
    CurHP,
    MaxHP,
    CurMP,
    MaxMP,
    Key,
    VCDataBuf,
    SpecialFrame,
    Face,
    Speed,
    EntityMoving,
    EntityCHRIndex,
    MoveCode,
    ActiveMode,
    ObsMode,
    EntityStep,
    EntityDelay,
    EntityLocX,
    EntityLocY,
    EntityX1,
    EntityX2,
    EntityY1,
    EntityY2,
    EntityFace,
    EntityChasing,
    EntityChaseDist,
    EntityChaseSpeed,
    EntityScriptOfs,
    ATK,
    DEF,
    HIT,
    DOD,
    MAG,
    MGR,
    REA,
    MBL,
    FER,
    ItemUse,
    ItemEffect,
    ItemType,
    ItemEquipType,
    ItemEquipIndex,
    ItemPrice,
    SpellUse,
    SpellEffect,
    SpellType,
    SpellPrice,
    SpellCost,
    Lvl,
    Nxt,
    Charstatus,
    Spell
};

enum class Var2 {
    Random,
    Screen,
    Items,
    CanEquip,
    ChooseChar,
    Obs,
    Spells,
};

bool equalsIgnoreCase(const std::string& left, const std::string& right) {
    return std::equal(left.begin(), left.end(), right.begin(),
        [](const char& a, const char& b) {
            return std::tolower(a) == std::tolower(b);
        });
}

const char* getCodeTypeName(CodeType type) {
    switch (type) {
        case CodeType::Event: return "event";
        case CodeType::Script: return "script";
        case CodeType::Effect: return "effect";
        default: throw std::runtime_error(std::string{"*error* getCodeTypeName: unhandled code type "} + std::to_string(static_cast<int>(type)));
    }
}

Code loadCode(const std::string& fileName) {
    FILE* f = fopen(fileName.c_str(), "rb");

    fseek(f, 0, SEEK_END);
    const auto size = static_cast<unsigned>(ftell(f));

    CodeType codeType = CodeType::Script;

    if (fileName.rfind(".map") == fileName.size() - 4) {
        codeType = CodeType::Event;

        fseek(f, 68, SEEK_SET);
        uint16_t mx, my;
        fread(&mx, 1, 2, f);
        fread(&my, 1, 2, f);

        fseek(f, 100+(mx*my*5)+7956, SEEK_SET);

        uint32_t a;
        uint8_t i;
        fread(&a, 1, 4, f);
        fseek(f, 88*a, 1);
        fread(&i, 1, 1, f);
        fread(&a, 1, 4, f);
        fseek(f, (i*4)+a, SEEK_CUR);
    } else {
        fseek(f, 0, SEEK_SET);

        if (equalsIgnoreCase(fileName, "effects.vcs")
        || equalsIgnoreCase(fileName, "magic.vcs")) {
            codeType = CodeType::Effect;
        }
    }

    uint32_t numscripts;
    fread(&numscripts, 1, 4, f);

    std::vector<uint32_t> scriptofstbl;
    scriptofstbl.resize(numscripts);
    fread(scriptofstbl.data(), 4, numscripts, f);

    std::vector<uint8_t> code;
    code.resize(size - static_cast<unsigned>(ftell(f)));
    fread(code.data(), 1, code.size(), f);

    fclose(f);

    return Code{ codeType, std::move(scriptofstbl), std::move(code) };
}

void decode(const Code& code, int scriptIndex);
void decodeBlock(Ctx& ctx);
void decodeExec(Ctx& ctx);
void decodeAssignOp(Ctx& ctx);
void decodeVar0Assign(Ctx& ctx);
void decodeVar1Assign(Ctx& ctx);
void decodeVar2Assign(Ctx& ctx);
void decodeIf(Ctx& ctx);
void decodeForLoop0(Ctx& ctx);
void decodeForLoop1(Ctx& ctx);
void decodeGoto(Ctx& ctx);
void decodeSwitch(Ctx& ctx);
void skipOperand(Ctx& ctx);
void skipOperandTerm(Ctx& ctx);
void decodeOperand(Ctx& ctx);
void decodeOperandTerm(Ctx& ctx);
void decodeVar0(Ctx& ctx);
void decodeVar1(Ctx& ctx);
void decodeVar2(Ctx& ctx);

void decode(const Code& code, int scriptIndex) {
    const auto startOffset = code.offsets[scriptIndex];
    const auto endOffset = scriptIndex == static_cast<int>(code.offsets.size()) - 1
        ? code.offsets.size()
        : code.offsets[scriptIndex + 1];

    Bytecode bc{ code.code.begin() + startOffset, code.code.begin() + endOffset };
    Ctx ctx{ bc, 0 };
    decodeBlock(ctx);

    ctx.dump();
}

void decodeBlock(Ctx& ctx) {
    while (!ctx.isEOF()) {
        const auto op = static_cast<BlockOp>(ctx.getC());
        //printf("decode block %d\n", static_cast<int>(op));

        switch (op) {
            case BlockOp::Exec: decodeExec(ctx); break;
            case BlockOp::Var0Assign: decodeVar0Assign(ctx); break;
            case BlockOp::Var1Assign: decodeVar1Assign(ctx); break;
            case BlockOp::Var2Assign: decodeVar2Assign(ctx); break;
            case BlockOp::If: decodeIf(ctx); break;
            case BlockOp::ForLoop0: decodeForLoop0(ctx); break;
            case BlockOp::ForLoop1: decodeForLoop1(ctx); break;
            case BlockOp::Goto: decodeGoto(ctx); break;
            case BlockOp::Switch: decodeSwitch(ctx); break;
            case BlockOp::End: return;
            default: throw std::runtime_error(std::string{"*error* decodeBlock: Illegal opcode in VC code "} + std::to_string(static_cast<int>(op)));
        }
    }
}

void decodeGenericFunc(Ctx& ctx, const std::string& funcName, int argumentCount) {
    printf("%s(", funcName.c_str());
    for (int argumentIndex = 0; argumentIndex < argumentCount; argumentIndex++) {
        decodeOperand(ctx);

        if (argumentIndex < argumentCount - 1) {
            printf(", ");
        }
    }
    printf(");\n");
}

void decodeExec(Ctx& ctx) {
    const auto funcId = static_cast<FuncId>(ctx.getC());

    switch (funcId) {
        case FuncId::MapSwitch: {
            printf("MapSwitch(");
            printf("\"%s\", ", ctx.getString().c_str());
            decodeOperand(ctx);
            printf(", ");
            decodeOperand(ctx);
            printf(", ");
            decodeOperand(ctx);
            printf(");\n");
            break;
        }
        case FuncId::Warp: decodeGenericFunc(ctx, "Warp", 3); break;
        case FuncId::AddCharacter: decodeGenericFunc(ctx, "AddCharacter", 1); break;
        case FuncId::SoundEffect: decodeGenericFunc(ctx, "SoundEffect", 1); break;
        case FuncId::GiveItem: decodeGenericFunc(ctx, "GiveItem", 1); break;
        case FuncId::Text: {
            printf("Text(");
            decodeOperand(ctx);
            printf(", ");
            printf("\"%s\", ", ctx.getString().c_str());
            printf("\"%s\", ", ctx.getString().c_str());
            printf("\"%s\"", ctx.getString().c_str());
            printf(");\n");
            break;
        }
        case FuncId::AlterFTile: decodeGenericFunc(ctx, "AlterFTile", 4); break;
        case FuncId::AlterBTile: decodeGenericFunc(ctx, "AlterBTile", 4); break;
        case FuncId::FakeBattle: decodeGenericFunc(ctx, "FakeBattle", 0); break;
        case FuncId::Return: printf("return;\n"); break;
        case FuncId::PlayMusic: {
            printf("PlayMusic(");
            printf("\"%s\", ", ctx.getString().c_str());
            printf(");\n");
            break;
        }
        case FuncId::StopMusic: printf("StopMusic();\n"); break;
        case FuncId::HealAll: printf("HealAll();\n"); break;
        case FuncId::AlterParallax: decodeGenericFunc(ctx, "AlterParallax", 3); break;
        case FuncId::FadeIn: decodeGenericFunc(ctx, "FadeIn", 1); break;
        case FuncId::FadeOut: decodeGenericFunc(ctx, "FadeOut", 2); break;
        case FuncId::RemoveCharacter: decodeGenericFunc(ctx, "RemoveCharacter", 1); break;
        case FuncId::Banner: {
            printf("Banner(\n");
            printf("\"%s\", ", ctx.getString().c_str());
            printf(", ");
            decodeOperand(ctx);
            printf(");\n");     
            break;
        }
        case FuncId::EnforceAnimation: decodeGenericFunc(ctx, "EnforceAnimation", 0); break;
        case FuncId::WaitKeyUp: decodeGenericFunc(ctx, "WaitKeyUp", 0); break;
        case FuncId::DestroyItem: decodeGenericFunc(ctx, "DestroyItem", 2); break;
        case FuncId::Prompt: {
            printf("Text(");
            decodeOperand(ctx);
            printf(", ");
            printf("\"%s\", ", ctx.getString().c_str());
            printf("\"%s\", ", ctx.getString().c_str());
            printf("\"%s\", ", ctx.getString().c_str());
            decodeOperand(ctx);
            printf(", ");
            printf("\"%s\", ", ctx.getString().c_str());
            printf("\"%s\"", ctx.getString().c_str());
            printf(");\n");
            break;
        }        
        case FuncId::ChainEvent: {
            const auto argumentCount = ctx.getC() + 1;
            decodeGenericFunc(ctx, "ChainEvent", argumentCount);
            break;
        }
        case FuncId::CallEvent: {
            const auto argumentCount = ctx.getC() + 1;
            decodeGenericFunc(ctx, "CallEvent", argumentCount);
            break;
        }
        case FuncId::Heal: decodeGenericFunc(ctx, "Heal", 2); break;
        case FuncId::EarthQuake: decodeGenericFunc(ctx, "EarthQuake", 3); break;
        case FuncId::SaveMenu: decodeGenericFunc(ctx, "SaveMenu", 0); break;
        case FuncId::EnableSave: decodeGenericFunc(ctx, "EnableSave", 0); break;
        case FuncId::DisableSave: decodeGenericFunc(ctx, "DisableSave", 0); break;
        case FuncId::ReviveChar: decodeGenericFunc(ctx, "ReviveChar", 1); break;
        case FuncId::RestoreMP: decodeGenericFunc(ctx, "RestoreMP", 2); break;
        case FuncId::Redraw: decodeGenericFunc(ctx, "Redraw", 0); break;
        case FuncId::SText: {
            printf("SText(");
            decodeOperand(ctx);
            printf(", ");
            printf("\"%s\", ", ctx.getString().c_str());
            printf("\"%s\", ", ctx.getString().c_str());
            printf("\"%s\"", ctx.getString().c_str());
            printf(");\n");
            break;
        }
        case FuncId::DisableMenu: decodeGenericFunc(ctx, "DisableMenu", 0); break;
        case FuncId::EnableMenu: decodeGenericFunc(ctx, "EnableMenu", 0); break;
        case FuncId::Wait: decodeGenericFunc(ctx, "Wait", 1); break;
        case FuncId::SetFace: decodeGenericFunc(ctx, "SetFace", 2); break;
        case FuncId::MapPaletteGradient: decodeGenericFunc(ctx, "MapPaletteGradient", 4); break;
        case FuncId::BoxFadeOut: decodeGenericFunc(ctx, "BoxFadeOut", 1); break;
        case FuncId::BoxFadeIn: decodeGenericFunc(ctx, "BoxFadeIn", 1); break;
        case FuncId::GiveGP: decodeGenericFunc(ctx, "GiveGP", 1); break;
        case FuncId::TakeGP: decodeGenericFunc(ctx, "TakeGP", 1); break;
        case FuncId::ChangeZone: decodeGenericFunc(ctx, "ChangeZone", 3); break;
        case FuncId::GetItem: decodeGenericFunc(ctx, "ChangeZone", 2); break;
        case FuncId::ForceEquip: decodeGenericFunc(ctx, "ForceEquip", 2); break;
        case FuncId::GiveXP: decodeGenericFunc(ctx, "GiveXP", 2); break;
        case FuncId::Shop: {
            const auto argumentCount = ctx.getC();
            decodeGenericFunc(ctx, "Shop", argumentCount);
            break;
        }
        case FuncId::PaletteMorph: decodeGenericFunc(ctx, "PaletteMorph", 5); break;
        case FuncId::ChangeCHR: {
            printf("ChangeCHR(");
            decodeOperand(ctx);
            printf(", ");
            printf("\"%s\"", ctx.getString().c_str());
            printf(");\n");
            break;
        }
        case FuncId::ReadControls: decodeGenericFunc(ctx, "ReadControls", 0); break;
        case FuncId::VCPutPCX: {
            printf("VCPutPCX(");
            printf("\"%s\", ", ctx.getString().c_str());
            decodeOperand(ctx);
            printf(", ");
            decodeOperand(ctx);            
            printf(");\n");
            break;
        }
        case FuncId::HookTimer: decodeGenericFunc(ctx, "HookTimer", 1); break;
        case FuncId::HookRetrace: decodeGenericFunc(ctx, "HookRetrace", 1); break;
        case FuncId::VCLoadPCX: {
            printf("VCLoadPCX(");
            printf("\"%s\", ", ctx.getString().c_str());
            decodeOperand(ctx);
            printf(");\n");
            break;
        }
        case FuncId::VCBlitImage: decodeGenericFunc(ctx, "VCBlitImage", 5); break;
        case FuncId::PlayFLI: throw std::runtime_error(std::string{"*error* TODO: PlayFLI"});
        case FuncId::VCClear: decodeGenericFunc(ctx, "VCClear", 0); break;
        case FuncId::VCClearRegion: decodeGenericFunc(ctx, "VCClear", 4); break;
        case FuncId::VCText: {
            printf("VCText(");
            decodeOperand(ctx);
            printf(", ");
            decodeOperand(ctx);
            printf(", ");            
            printf("\"%s\"", ctx.getString().c_str());
            printf(");\n");
            break;
        }
        case FuncId::VCTBlitImage: decodeGenericFunc(ctx, "VCTBlitImage", 5); break;
        case FuncId::Exit: decodeGenericFunc(ctx, "Exit", 0); break;
        case FuncId::Quit: {
            printf("Quit(");
            printf("\"%s\"", ctx.getString().c_str());
            printf(");\n");
            break;
        }
        case FuncId::VCCenterText: {
            printf("VCCenterText(");
            decodeOperand(ctx);
            printf(", ");
            decodeOperand(ctx);
            printf(", ");            
            printf("\"%s\"", ctx.getString().c_str());
            printf(");\n");
            break;
        }
        case FuncId::ResetTimer: decodeGenericFunc(ctx, "ResetTimer", 0); break;
        case FuncId::VCBlitTile: decodeGenericFunc(ctx, "VCBlitTile", 3); break;
        case FuncId::Sys_ClearScreen: decodeGenericFunc(ctx, "Sys_ClearScreen", 0); break;
        case FuncId::Sys_DisplayPCX: {
            printf("Sys_DisplayPCX(");
            printf("\"%s\"", ctx.getString().c_str());
            printf(");\n");
            break;
        }        
        case FuncId::OldStartupMenu: decodeGenericFunc(ctx, "OldStartupMenu", 0); break;
        case FuncId::VGADump: decodeGenericFunc(ctx, "VGADump", 0); break;
        case FuncId::NewGame: {
            printf("NewGame(");
            printf("\"%s\"", ctx.getString().c_str());
            printf(");\n");
            break;
        }
        case FuncId::LoadSaveErase: decodeGenericFunc(ctx, "LoadSaveErase", 0); break;
        case FuncId::Delay: decodeGenericFunc(ctx, "Delay", 1); break;
        case FuncId::PartyMove: {
            printf("PartyMove(");
            printf("\"%s\"", ctx.getString().c_str());
            printf(");\n");
            break;
        }
        case FuncId::EntityMove: {
            printf("EntityMove(");
            decodeOperand(ctx);
            printf(", ");            
            printf("\"%s\"", ctx.getString().c_str());
            printf(");\n");
            break;
        } 
        case FuncId::AutoOn: decodeGenericFunc(ctx, "AutoOn", 0); break;
        case FuncId::AutoOff: decodeGenericFunc(ctx, "AutoOff", 0); break;
        case FuncId::EntityMoveScript: decodeGenericFunc(ctx, "EntityMoveScript", 2); break;
        case FuncId::VCTextNum: decodeGenericFunc(ctx, "VCTextNum", 3); break;
        case FuncId::VCLoadRaw: {
            printf("VCLoadRaw(");
            printf("\"%s\", ", ctx.getString().c_str());
            decodeOperand(ctx);
            printf(", ");
            decodeOperand(ctx);
            printf(", ");
            decodeOperand(ctx);
            printf(");\n");
            break;
        }
        case FuncId::VCBox: decodeGenericFunc(ctx, "VCBox", 4); break;
        case FuncId::VCCharName: decodeGenericFunc(ctx, "VCCharName", 4); break;
        case FuncId::VCItemName: decodeGenericFunc(ctx, "VCItemName", 4); break;
        case FuncId::VCItemDesc: decodeGenericFunc(ctx, "VCItemDesc", 4); break;
        case FuncId::VCItemImage: decodeGenericFunc(ctx, "VCItemImage", 4); break;
        case FuncId::VCATextNum: decodeGenericFunc(ctx, "VCATextNum", 4); break;
        case FuncId::VCSpc: decodeGenericFunc(ctx, "VCSpc", 4); break;
        case FuncId::CallEffect: {
            const auto argumentCount = ctx.getC() + 1;
            decodeGenericFunc(ctx, "CallEffect", argumentCount);
            break;
        }
        case FuncId::CallScript: {
            const auto argumentCount = ctx.getC() + 1;
            decodeGenericFunc(ctx, "CallScript", argumentCount);
            break;
        }
        case FuncId::VCLine: decodeGenericFunc(ctx, "VCLine", 5); break;
        case FuncId::GetMagic: decodeGenericFunc(ctx, "GetMagic", 2); break;
        case FuncId::BindKey: decodeGenericFunc(ctx, "BindKey", 2); break;
        case FuncId::TextMenu: {
            printf("TextMenu(");
            decodeOperand(ctx);
            printf(", ");
            decodeOperand(ctx);
            printf(", ");
            decodeOperand(ctx);
            printf(", ");
            decodeOperand(ctx);
            printf(", ");

            const auto argumentCount = ctx.getC() + 1;
            for (int argumentIndex = 0; argumentIndex < argumentCount; argumentIndex++) {
                printf("\"%s\"", ctx.getString().c_str());

                if (argumentIndex < argumentCount - 1) {
                    printf(", ");
                }
            }

            printf(");\n");
            break;
        }
        case FuncId::ItemMenu: decodeGenericFunc(ctx, "ItemMenu", 1); break;
        case FuncId::EquipMenu: decodeGenericFunc(ctx, "EquipMenu", 1); break;
        case FuncId::MagicMenu: decodeGenericFunc(ctx, "MagicMenu", 1); break;
        case FuncId::StatusScreen: decodeGenericFunc(ctx, "StatusScreen", 1); break;
        case FuncId::VCCr2: decodeGenericFunc(ctx, "VCCr2", 4); break;
        case FuncId::VCSpellName: decodeGenericFunc(ctx, "VCSpellName", 4); break;
        case FuncId::VCSpellDesc: decodeGenericFunc(ctx, "VCSpellDesc", 4); break;
        case FuncId::VCSpellImage: decodeGenericFunc(ctx, "VCSpellImage", 4); break;
        case FuncId::MagicShop: {
            const auto argumentCount = ctx.getC() + 1;
            decodeGenericFunc(ctx, "MagicShop", argumentCount);
            break;
        }
        case FuncId::VCTextBox: {
            printf("VCTextBox(");
            decodeOperand(ctx);
            printf(", ");
            decodeOperand(ctx);
            printf(", ");
            decodeOperand(ctx);
            printf(", ");

            const auto argumentCount = ctx.getC() + 1;
            for (int argumentIndex = 0; argumentIndex < argumentCount; argumentIndex++) {
                printf("\"%s\"", ctx.getString().c_str());

                if (argumentIndex < argumentCount - 1) {
                    printf(", ");
                }
            }

            printf(");\n");
            break;
        }
        case FuncId::PlayVAS: {
            printf("PlayVAS(");
            printf("\"%s\", ", ctx.getString().c_str());
            decodeOperand(ctx);
            printf(", ");
            decodeOperand(ctx);
            printf(", ");
            decodeOperand(ctx);
            printf(", ");
            decodeOperand(ctx);
            printf(", ");
            decodeOperand(ctx);
            printf(");\n");
            break;
        }

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
            throw std::runtime_error(std::string{"*error* decodeExec: Unknown library function in VC code "} + std::to_string(static_cast<int>(funcId)));
    }
}

void decodeAssignOp(Ctx& ctx) {
    const auto assignOp = static_cast<AssignOp>(ctx.getC());

    switch (assignOp) {
        case AssignOp::Set: printf(" = "); break;
        case AssignOp::Increment: printf("++"); break;
        case AssignOp::Decrement: printf("--"); break;
        case AssignOp::IncSet: printf(" += "); break;
        case AssignOp::DecSet: printf(" -= "); break;
        default:
            throw std::runtime_error(std::string{"*error* decodeAssignOp: Unknown assignment opcode in VC code "} + std::to_string(static_cast<int>(assignOp)));
    } 
}

void decodeVar0Assign(Ctx& ctx) {
    decodeVar0(ctx);
    decodeAssignOp(ctx);
    decodeOperand(ctx);
    printf(";\n");
}

void decodeVar1Assign(Ctx& ctx) {
    decodeVar1(ctx);
    decodeAssignOp(ctx);
    decodeOperand(ctx);
    printf(";\n");
}

void decodeVar2Assign(Ctx& ctx) {
    decodeVar2(ctx);
    decodeAssignOp(ctx);
    decodeOperand(ctx);
    printf(";\n");
}

void decodeIf(Ctx& ctx) {
    const auto argumentCount = static_cast<int>(ctx.getC());
    ctx.skipD();

    printf("if ("); 
    for (int argumentIndex = 0; argumentIndex < argumentCount; argumentIndex++) {
        const auto previousOffset = ctx.offset;
        skipOperand(ctx);

        // FIXME: remove skipOperand if we can actually modify existing output, we only need to seek back to prepend unary !

        const auto branchOp = static_cast<BranchOp>(ctx.getC());
        ctx.offset = previousOffset;

        switch (branchOp) {
            case BranchOp::Zero: {
                decodeOperand(ctx);
                ctx.skipC();
                break;
            }
            case BranchOp::NonZero: {
                printf("!");
                decodeOperand(ctx);
                ctx.skipC();
                break;
            }
            case BranchOp::Equal: {
                decodeOperand(ctx);
                ctx.skipC();
                printf(" == ");
                decodeOperand(ctx);
                break;
            }
            case BranchOp::NotEqual: {
                decodeOperand(ctx);
                ctx.skipC();
                printf(" != ");
                decodeOperand(ctx);
                break;
            }
            case BranchOp::GreaterThan: {
                decodeOperand(ctx);
                ctx.skipC();
                printf(" >" );
                decodeOperand(ctx);
                break;
            }
            case BranchOp::GreaterThanOrEqual: {
                decodeOperand(ctx);
                ctx.skipC();
                printf(" >= ");
                decodeOperand(ctx);
                break;
            }
            case BranchOp::LessThan: {
                decodeOperand(ctx);
                ctx.skipC();
                printf(" < ");
                decodeOperand(ctx);
                break;
            }
            case BranchOp::LessThanOrEqual: {
                decodeOperand(ctx);
                ctx.skipC();
                printf(" <= ");
                decodeOperand(ctx);
                break;
            }
            default:
                throw std::runtime_error(std::string{"*error* Unknown branch opcode in VC code "} + std::to_string(static_cast<int>(branchOp)));;
        }

        if (argumentIndex < argumentCount - 1) {
            printf(" && ");
        }
    }
    printf(")\n"); 
    printf("{\n"); 
    decodeBlock(ctx);
    printf("}\n"); 
}

void decodeForLoop0(Ctx& ctx) {
    printf("for(");
    decodeVar0(ctx);
    printf(",");
    decodeOperand(ctx);
    printf(",");
    decodeOperand(ctx);
    printf(",");
    printf(ctx.getC() == 0 ? "-" : "");
    decodeOperand(ctx);
    printf(")\n");
    printf("{\n"); 
    decodeBlock(ctx);
    printf("}\n"); 
}

void decodeForLoop1(Ctx& ctx) {
    printf("for(");
    decodeVar1(ctx);
    printf(",");
    decodeOperand(ctx);
    printf(",");
    decodeOperand(ctx);
    printf(",");
    printf(ctx.getC() == 0 ? "-" : "");
    decodeOperand(ctx);
    printf(")\n");
    printf("{\n"); 
    decodeBlock(ctx);
    printf("}\n");
}

void decodeGoto(Ctx& ctx) {
    // FIXME: could be a goto, or a while loop! we can't know!
    // both are allowed, and any reverse jump will require a way to insert labels into earlier code, or replace an if with while {}
    //
    // while might be somewhat predictable, if it comes as the last statement of an if statement.
    // note: labels don't exist in bytecode, so we need to generate ones (and possibly validate ones that can't map to a line).
    //
    // hmm. should we keep track of each line generated for each statement?
    // We generate each 'block' opcode into a 1 single-line statement, so we could keep a buffer for each line of code,
    // and a table that maps opcode -> generated line index (update as necessary),
    // so that we can add things in the middle (statement-level granularity)
    // 
    // instead of printf, have a writeable file buffer we build in memory, and then flush it at the end.    
    const auto dest = ctx.getD();
    printf("// FIXME: goto or while to offset %d\n", dest);
}

void decodeSwitch(Ctx& ctx) {
    printf("switch (");
    decodeOperand(ctx);
    printf(")");
    printf("{\n");

    while (!ctx.isEOF()) {
        const auto op = static_cast<BlockOp>(ctx.getC());

        switch (op) {
            // Anything besides ScriptEnd is supposed to be treated as a case.
            // (according to vc interpreter, even if vcc uses a CASE opcode there)
            default: {
                printf("case ");
                decodeOperand(ctx);
                printf(":\n");
                static_cast<void>(ctx.getD());
                decodeBlock(ctx);
            }
            case BlockOp::End: {
                printf("}\n");
                return;
            }            
        }

    }
}

void skipOperand(Ctx& ctx) {
    skipOperandTerm(ctx);

    while (!ctx.isEOF()) {
        const auto arithmeticOp = static_cast<ArithmeticOp>(ctx.getC());
        //printf("skip arith op %d\n", static_cast<int>(arithmeticOp));

        switch (arithmeticOp) {
            case ArithmeticOp::Add:
            case ArithmeticOp::Subtract:
            case ArithmeticOp::Divide:
            case ArithmeticOp::Multiply: {
            case ArithmeticOp::Modulo:
                skipOperandTerm(ctx);
                break;
            }
            case ArithmeticOp::End: return;
            default:
                throw std::runtime_error(std::string{"*error* skipOperand: Unknown arithmetic opcode in VC code "} + std::to_string(static_cast<int>(arithmeticOp)));
        }
    }
}

void skipOperandTerm(Ctx& ctx) {
    const auto termType = static_cast<OpTermType>(ctx.getC());
    //printf("skip op term %d\n", static_cast<int>(termType));

    switch (termType) {
        case OpTermType::Immediate: {
            ctx.skipD();
            break;
        }
        case OpTermType::Var0: {
            ctx.skipC();
            break;
        }
        case OpTermType::Var1: {
            ctx.skipC();
            skipOperand(ctx);
            break;
        }
        case OpTermType::Var2: {
            ctx.skipC();
            skipOperand(ctx);
            skipOperand(ctx);
            break;
        }
        case OpTermType::Group: {
            skipOperand(ctx);
            break;
        }
        default:
            throw std::runtime_error(std::string{"*error* skipOperandTerm: Unknown operand term type in VC code "} + std::to_string(static_cast<int>(termType)));        
    }
}

void decodeOperand(Ctx& ctx) {
    decodeOperandTerm(ctx);

    while (!ctx.isEOF()) {
        const auto arithmeticOp = static_cast<ArithmeticOp>(ctx.getC());
        //printf("decode arith op %d\n", static_cast<int>(arithmeticOp));

        switch (arithmeticOp) {            
            case ArithmeticOp::Add: printf(" + "); decodeOperandTerm(ctx); break;
            case ArithmeticOp::Subtract: printf(" - "); decodeOperandTerm(ctx); break;
            case ArithmeticOp::Divide: printf(" / "); decodeOperandTerm(ctx); break;
            case ArithmeticOp::Multiply: printf(" * "); decodeOperandTerm(ctx); break;
            case ArithmeticOp::Modulo: printf(" %% "); decodeOperandTerm(ctx); break;
            case ArithmeticOp::End: return;
            default:
                throw std::runtime_error(std::string{"*error* decodeOperand: Unknown arithmetic opcode in VC code "} + std::to_string(static_cast<int>(arithmeticOp)));            
        }
    }
}

void decodeOperandTerm(Ctx& ctx) {
    const auto termType = static_cast<OpTermType>(ctx.getC());
    //printf("decode op term %d\n", static_cast<int>(termType));

    switch (termType) {
        case OpTermType::Immediate: printf("%d", ctx.getD()); break;
        case OpTermType::Var0: decodeVar0(ctx); break;
        case OpTermType::Var1: decodeVar1(ctx); break;
        case OpTermType::Var2: decodeVar2(ctx); break;
        case OpTermType::Group: decodeOperand(ctx); break;
        default:
            throw std::runtime_error(std::string{"*error* decodeOperandTerm: Unknown operand term type in VC code "} + std::to_string(static_cast<int>(termType)));
    }
}

void decodeVar0(Ctx& ctx) {
    const auto var0 = static_cast<Var0>(ctx.getC());

    switch (var0) {
        case Var0::A: printf("a"); break;
        case Var0::B: printf("b"); break;
        case Var0::C: printf("c"); break;
        case Var0::D: printf("d"); break;
        case Var0::E: printf("e"); break;
        case Var0::F: printf("f"); break;
        case Var0::G: printf("g"); break;
        case Var0::H: printf("h"); break;
        case Var0::I: printf("i"); break;
        case Var0::J: printf("j"); break;
        case Var0::K: printf("k"); break;
        case Var0::L: printf("l"); break;
        case Var0::M: printf("m"); break;
        case Var0::N: printf("n"); break;
        case Var0::O: printf("o"); break;
        case Var0::P: printf("p"); break;
        case Var0::Q: printf("q"); break;
        case Var0::R: printf("r"); break;
        case Var0::S: printf("s"); break;
        case Var0::T: printf("t"); break;
        case Var0::U: printf("u"); break;
        case Var0::V: printf("v"); break;
        case Var0::W: printf("w"); break;
        case Var0::X: printf("x"); break;
        case Var0::Y: printf("y"); break;
        case Var0::Z: printf("z"); break;
        case Var0::NumChars: printf("numchars"); break;
        case Var0::GP: printf("gp"); break;
        case Var0::LocX: printf("locx"); break;
        case Var0::LocY: printf("locy"); break;
        case Var0::Timer: printf("timer"); break;
        case Var0::DrawParty: printf("drawparty"); break;
        case Var0::Cameratracking: printf("cameratracking"); break;
        case Var0::XWin: printf("xwin"); break;
        case Var0::YWin: printf("ywin"); break;
        case Var0::B1: printf("b1"); break;
        case Var0::B2: printf("b2"); break;
        case Var0::B3: printf("b3"); break;
        case Var0::B4: printf("b4"); break;
        case Var0::Up: printf("up"); break;
        case Var0::Down: printf("down"); break;
        case Var0::Left: printf("left"); break;
        case Var0::Right: printf("right"); break;
        case Var0::TimerAnimate: printf("timeranimate"); break;
        case Var0::Fade: printf("fade"); break;
        case Var0::Layer0: printf("layer0"); break;
        case Var0::Layer1: printf("layer1"); break;
        case Var0::LayerVC: printf("layervc"); break;
        case Var0::QuakeX: printf("quakex"); break;
        case Var0::QuakeY: printf("quakey"); break;
        case Var0::Quake: printf("quake"); break;
        case Var0::ScreenGradient: printf("screengradient"); break;
        case Var0::PMultX: printf("pmultx"); break;
        case Var0::PMultY: printf("pmulty"); break;
        case Var0::PDivX: printf("pdivx"); break;
        case Var0::PDivY: printf("pdivy"); break;
        case Var0::Volume: printf("volume"); break;
        case Var0::ParallaxC: printf("parallaxc"); break;
        case Var0::CancelFade: printf("cancelfade"); break;
        case Var0::DrawEntities: printf("drawentities"); break;
        case Var0::CurZone: printf("curzone"); break;
        case Var0::TileB: printf("tileb"); break;
        case Var0::TileF: printf("tilef"); break;
        case Var0::ForegroundLock: printf("foregroundlock"); break;
        case Var0::XWin1: printf("xwin1"); break;
        case Var0::YWin1: printf("ywin1"); break;
        case Var0::Layer1Trans: printf("layer1trans"); break;
        case Var0::LayerVCTrans: printf("layervctrans"); break;
        case Var0::FontColor: printf("fontcolor"); break;
        case Var0::KeepAZ: printf("keepaz"); break;
        case Var0::LayerVC2: printf("layervc2"); break;
        case Var0::LayerVC2Trans: printf("layervc2trans"); break;
        case Var0::VCWriteLayer: printf("vcwritelayer"); break;
        case Var0::ModPosition: printf("modposition"); break;
        default:
            throw std::runtime_error(std::string{"*error* decodeVar0: Unknown var0 index in VC code "} + std::to_string(static_cast<int>(var0)));
    }
}

void decodeVar1(Ctx& ctx) {
    const auto var1 = static_cast<Var1>(ctx.getC());

    switch (var1) {
        case Var1::Flags: printf("flags"); break;
        case Var1::Facing: printf("facing"); break;
        case Var1::Char: printf("char"); break;
        case Var1::Item: printf("item"); break;
        case Var1::Var: printf("var"); break;
        case Var1::PartyIndex: printf("partyindex"); break;
        case Var1::XP: printf("xp"); break;
        case Var1::CurHP: printf("curhp"); break;
        case Var1::MaxHP: printf("maxhp"); break;
        case Var1::CurMP: printf("curmp"); break;
        case Var1::MaxMP: printf("maxmp"); break;
        case Var1::Key: printf("key"); break;
        case Var1::VCDataBuf: printf("vcdatabuf"); break;
        case Var1::SpecialFrame: printf("specialframe"); break;
        case Var1::Face: printf("face"); break;
        case Var1::Speed: printf("speed"); break;
        case Var1::EntityMoving: printf("entity.moving"); break;
        case Var1::EntityCHRIndex: printf("entity.chrindex"); break;
        case Var1::MoveCode: printf("movecode"); break;
        case Var1::ActiveMode: printf("activemode"); break;
        case Var1::ObsMode: printf("obsmode"); break;
        case Var1::EntityStep: printf("entity.step"); break;
        case Var1::EntityDelay: printf("entity.delay"); break;
        case Var1::EntityLocX: printf("entity.locx"); break;
        case Var1::EntityLocY: printf("entity.locy"); break;
        case Var1::EntityX1: printf("entity.x1"); break;
        case Var1::EntityX2: printf("entity.x2"); break;
        case Var1::EntityY1: printf("entity.y1"); break;
        case Var1::EntityY2: printf("entity.y2"); break;
        case Var1::EntityFace: printf("entity.face"); break;
        case Var1::EntityChasing: printf("entity.chasing"); break;
        case Var1::EntityChaseDist: printf("entity.chasedist"); break;
        case Var1::EntityChaseSpeed: printf("entity.chasespeed"); break;
        case Var1::EntityScriptOfs: printf("entity.scriptofs"); break;
        case Var1::ATK: printf("atk"); break;
        case Var1::DEF: printf("def"); break;
        case Var1::HIT: printf("hit"); break;
        case Var1::DOD: printf("dod"); break;
        case Var1::MAG: printf("mag"); break;
        case Var1::MGR: printf("mgr"); break;
        case Var1::REA: printf("rea"); break;
        case Var1::MBL: printf("mbl"); break;
        case Var1::FER: printf("fer"); break;
        case Var1::ItemUse: printf("item.use"); break;
        case Var1::ItemEffect: printf("item.effect"); break;
        case Var1::ItemType: printf("item.type"); break;
        case Var1::ItemEquipType: printf("item.equiptype"); break;
        case Var1::ItemEquipIndex: printf("item.equipindex"); break;
        case Var1::ItemPrice: printf("item.price"); break;
        case Var1::SpellUse: printf("spell.use"); break;
        case Var1::SpellEffect: printf("spell.effect"); break;
        case Var1::SpellType: printf("spell.type"); break;
        case Var1::SpellPrice: printf("spell.price"); break;
        case Var1::SpellCost: printf("spell.cost"); break;
        case Var1::Lvl: printf("lvl"); break;
        case Var1::Nxt: printf("nxt"); break;
        case Var1::Charstatus: printf("charstatus"); break;
        case Var1::Spell: printf("spell"); break;
        default:
            throw std::runtime_error(std::string{"*error* decodeVar1: Unknown var1 index in VC code "} + std::to_string(static_cast<int>(var1)));
    }
    // '(' and '[' are equivalent in V1, so we use '[' for 1-argument array-like things, eg. flags.
    printf("[");
    decodeOperand(ctx);
    printf("]");
}

void decodeVar2(Ctx& ctx) {
    const auto var2 = static_cast<Var2>(ctx.getC());

    switch (var2) {
        case Var2::Random: printf("random"); break;
        case Var2::Screen: printf("screen"); break;
        case Var2::Items: printf("items"); break;
        case Var2::CanEquip: printf("canequip"); break;
        case Var2::ChooseChar: printf("choosechar"); break;
        case Var2::Obs: printf("obs"); break;
        case Var2::Spells: printf("spells"); break;
        default:
            throw std::runtime_error(std::string{"*error* decodeVar2: Unknown var2 index in VC code "} + std::to_string(static_cast<int>(var2)));
    }
    printf("(");
    decodeOperand(ctx);
    decodeOperand(ctx);
    printf(")");    
}

int main(int argc, char** argv) {
    if (argc < 2) {
        fprintf(stderr, "usage: %s filename\n", argv[0]);
        return 1;
    }

    Code code = loadCode(argv[1]);
    int scriptIndex = 0;
    for (uint32_t offset: code.offsets) {
        printf("%s /* %d offset=%d */\n{\n", getCodeTypeName(code.type), scriptIndex, offset);
        decode(code, scriptIndex);
        printf("}\n");
        ++scriptIndex;
    }
    return 0;
}
