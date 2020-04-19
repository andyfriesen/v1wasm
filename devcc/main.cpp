#include <string>
#include <cassert>
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

enum class CodeType {
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

    std::vector<std::pair<uint32_t, std::string>> code;

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
        return "\"" + std::string(bytecode.begin() + startOffset, bytecode.begin() + offset - 1) + "\"";
    }

    void emit(uint32_t offset, std::string s) {
        code.push_back(std::make_pair(offset, std::move(s)));
    }

    void dump() {
        std::sort(code.begin(), code.end(), [](auto a, auto b) { return a.first < b.first; });
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
std::string decodeOperand(Ctx& ctx);
std::string decodeOperandTerm(Ctx& ctx);
std::string decodeVar0(Ctx& ctx);
std::string decodeVar1(Ctx& ctx);
std::string decodeVar2(Ctx& ctx);

void decode(const Code& code, int scriptIndex) {
    const auto startOffset = code.offsets[scriptIndex];
    const auto endOffset = scriptIndex == static_cast<int>(code.offsets.size()) - 1
        ? code.code.size()
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
    auto o = ctx.offset - 1; // ctx.offset has already gobbled up the function ID once this function is called.
    std::string res;
    res = funcName + "(";
    for (int argumentIndex = 0; argumentIndex < argumentCount; argumentIndex++) {
        res += decodeOperand(ctx);

        if (argumentIndex < argumentCount - 1) {
            res += ", ";
        }
    }
    res += ");";
    ctx.emit(o, std::move(res));
}

void emitFunctionCall(Ctx& ctx, int offset, const std::string& name, const std::string& desc) {
    std::string res = name + "(";

    bool first = true;
    for (char c: desc) {
        if (first) {
            first = false;
        } else {
            res += ", ";
        }

        if (c == 'i') {
            res += decodeOperand(ctx);
        } else if (c == 's') {
            res += ctx.getString();
        } else {
            throw std::runtime_error("what");
        }
    }

    res += ");";
    ctx.emit(offset, res);
}

void emitFunctionCall(Ctx& ctx, int offset, const std::string& name, std::initializer_list<std::string> args) {
    std::string res = name + "(";
    bool first = true;

    for (const auto& s: args) {
        if (first) {
            first = false;
        } else {
            res += ", ";
        }

        res += s;
    }

    res += ");";
    ctx.emit(offset, res);
}

void decodeExec(Ctx& ctx) {
    const auto funcId = static_cast<FuncId>(ctx.getC());
    auto offset = ctx.offset;

    switch (funcId) {
        case FuncId::MapSwitch: emitFunctionCall(ctx, offset, "MapSwitch", "siii"); break;
        case FuncId::Warp: decodeGenericFunc(ctx, "Warp", 3); break;
        case FuncId::AddCharacter: decodeGenericFunc(ctx, "AddCharacter", 1); break;
        case FuncId::SoundEffect: decodeGenericFunc(ctx, "SoundEffect", 1); break;
        case FuncId::GiveItem: decodeGenericFunc(ctx, "GiveItem", 1); break;
        case FuncId::Text: emitFunctionCall(ctx, offset, "Text", "siii"); break;
        case FuncId::AlterFTile: decodeGenericFunc(ctx, "AlterFTile", 4); break;
        case FuncId::AlterBTile: decodeGenericFunc(ctx, "AlterBTile", 4); break;
        case FuncId::FakeBattle: decodeGenericFunc(ctx, "FakeBattle", 0); break;
        case FuncId::Return: ctx.emit(offset, "return;"); break;
        case FuncId::PlayMusic: {
            emitFunctionCall(ctx, offset, "PlayMusic", "s");
            break;
        }
        case FuncId::StopMusic: ctx.emit(offset, "StopMusic();"); break;
        case FuncId::HealAll: ctx.emit(offset, "HealAll();"); break;
        case FuncId::AlterParallax: decodeGenericFunc(ctx, "AlterParallax", 3); break;
        case FuncId::FadeIn: decodeGenericFunc(ctx, "FadeIn", 1); break;
        case FuncId::FadeOut: decodeGenericFunc(ctx, "FadeOut", 1); break;
        case FuncId::RemoveCharacter: decodeGenericFunc(ctx, "RemoveCharacter", 1); break;
        case FuncId::Banner: {
            emitFunctionCall(ctx, offset, "Banner", "si");
            break;
        }
        case FuncId::EnforceAnimation: decodeGenericFunc(ctx, "EnforceAnimation", 0); break;
        case FuncId::WaitKeyUp: decodeGenericFunc(ctx, "WaitKeyUp", 0); break;
        case FuncId::DestroyItem: decodeGenericFunc(ctx, "DestroyItem", 2); break;
        case FuncId::Prompt: {
            emitFunctionCall(ctx, offset, "Prompt", "isssiss");
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
            emitFunctionCall(ctx, offset, "SText", "isss");
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
            emitFunctionCall(ctx, offset, "ChangeCHR", "is");
            break;
        }
        case FuncId::ReadControls: decodeGenericFunc(ctx, "ReadControls", 0); break;
        case FuncId::VCPutPCX: {
            emitFunctionCall(ctx, offset, "VCPutPCX", "sii");
            break;
        }
        case FuncId::HookTimer: decodeGenericFunc(ctx, "HookTimer", 1); break;
        case FuncId::HookRetrace: decodeGenericFunc(ctx, "HookRetrace", 1); break;
        case FuncId::VCLoadPCX: {
            emitFunctionCall(ctx, offset, "VCLoadPCX", "si");
            break;
        }
        case FuncId::VCBlitImage: decodeGenericFunc(ctx, "VCBlitImage", 5); break;
        case FuncId::PlayFLI: throw std::runtime_error(std::string{"*error* TODO: PlayFLI"});
        case FuncId::VCClear: decodeGenericFunc(ctx, "VCClear", 0); break;
        case FuncId::VCClearRegion: decodeGenericFunc(ctx, "VCClear", 4); break;
        case FuncId::VCText: {
            emitFunctionCall(ctx, offset, "VCText", "iis");
            break;
        }
        case FuncId::VCTBlitImage: decodeGenericFunc(ctx, "VCTBlitImage", 5); break;
        case FuncId::Exit: decodeGenericFunc(ctx, "Exit", 0); break;
        case FuncId::Quit: {
            emitFunctionCall(ctx, offset, "Quit", "s");
            break;
        }
        case FuncId::VCCenterText: {
            emitFunctionCall(ctx, offset, "VCCenterText", "is");
            break;
        }
        case FuncId::ResetTimer: decodeGenericFunc(ctx, "ResetTimer", 0); break;
        case FuncId::VCBlitTile: decodeGenericFunc(ctx, "VCBlitTile", 3); break;
        case FuncId::Sys_ClearScreen: decodeGenericFunc(ctx, "Sys_ClearScreen", 0); break;
        case FuncId::Sys_DisplayPCX: {
            emitFunctionCall(ctx, offset, "Sys_DisplayPCX", "s");
            break;
        }        
        case FuncId::OldStartupMenu: decodeGenericFunc(ctx, "OldStartupMenu", 0); break;
        case FuncId::VGADump: decodeGenericFunc(ctx, "VGADump", 0); break;
        case FuncId::NewGame: {
            emitFunctionCall(ctx, offset, "NewGame", "s");
            break;
        }
        case FuncId::LoadSaveErase: decodeGenericFunc(ctx, "LoadSaveErase", 0); break;
        case FuncId::Delay: decodeGenericFunc(ctx, "Delay", 1); break;
        case FuncId::PartyMove: {
            emitFunctionCall(ctx, offset, "PartyMove", "s");
            break;
        }
        case FuncId::EntityMove: {
            emitFunctionCall(ctx, offset, "EntityMove", "is");
            break;
        } 
        case FuncId::AutoOn: decodeGenericFunc(ctx, "AutoOn", 0); break;
        case FuncId::AutoOff: decodeGenericFunc(ctx, "AutoOff", 0); break;
        case FuncId::EntityMoveScript: decodeGenericFunc(ctx, "EntityMoveScript", 2); break;
        case FuncId::VCTextNum: decodeGenericFunc(ctx, "VCTextNum", 3); break;
        case FuncId::VCLoadRaw: {
            emitFunctionCall(ctx, offset, "VCLoadRaw", "siii");
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
            std::string res = "TextMenu(" +
                decodeOperand(ctx) + ", " + 
                decodeOperand(ctx) + ", " + 
                decodeOperand(ctx) + ", " + 
                decodeOperand(ctx) +
                ", ";

            const auto argumentCount = ctx.getC() + 1;
            for (int argumentIndex = 0; argumentIndex < argumentCount; argumentIndex++) {
                res += ctx.getString();

                if (argumentIndex < argumentCount - 1) {
                    res += ", ";
                }
            }

            res += ");";
            ctx.emit(offset, std::move(res));
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
            std::string res = "VCTextBox(" +
                decodeOperand(ctx) + ", " +
                decodeOperand(ctx) + ", " +
                decodeOperand(ctx) + ", "
            ;
            const auto argumentCount = ctx.getC() + 1;
            for (int argumentIndex = 0; argumentIndex < argumentCount; argumentIndex++) {
                res += ctx.getString();

                if (argumentIndex < argumentCount - 1) {
                    res += ", ";
                }
            }

            res += ");";
            ctx.emit(offset, res);
            break;
        }
        case FuncId::PlayVAS: {
            emitFunctionCall(ctx, offset, "PlayVAS", "siiiii");
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
    auto offset = ctx.offset;

    const auto argumentCount = static_cast<int>(ctx.getC());
    ctx.skipD();

    std::string res = "if (";

    for (int argumentIndex = 0; argumentIndex < argumentCount; argumentIndex++) {
        const auto previousOffset = ctx.offset;
        skipOperand(ctx);

        // FIXME: remove skipOperand if we can actually modify existing output, we only need to seek back to prepend unary !

        const auto branchOp = static_cast<BranchOp>(ctx.getC());
        ctx.offset = previousOffset;

        switch (branchOp) {
            case BranchOp::Zero: {
                res += decodeOperand(ctx);
                ctx.skipC();
                break;
            }
            case BranchOp::NonZero: {
                res += "!";
                res += decodeOperand(ctx);
                ctx.skipC();
                break;
            }
            case BranchOp::Equal: {
                res += decodeOperand(ctx);
                ctx.skipC();
                res += " == ";
                res += decodeOperand(ctx);
                break;
            }
            case BranchOp::NotEqual: {
                res += decodeOperand(ctx);
                ctx.skipC();
                res += " != ";
                res += decodeOperand(ctx);
                break;
            }
            case BranchOp::GreaterThan: {
                res += decodeOperand(ctx);
                ctx.skipC();
                res += " >" ;
                res += decodeOperand(ctx);
                break;
            }
            case BranchOp::GreaterThanOrEqual: {
                res += decodeOperand(ctx);
                ctx.skipC();
                res += " >= ";
                res += decodeOperand(ctx);
                break;
            }
            case BranchOp::LessThan: {
                res += decodeOperand(ctx);
                ctx.skipC();
                res += " < ";
                res += decodeOperand(ctx);
                break;
            }
            case BranchOp::LessThanOrEqual: {
                res += decodeOperand(ctx);
                ctx.skipC();
                res += " <= ";
                res += decodeOperand(ctx);
                break;
            }
            default:
                throw std::runtime_error(std::string{"*error* Unknown branch opcode in VC code "} + std::to_string(static_cast<int>(branchOp)));;
        }

        if (argumentIndex < argumentCount - 1) {
            res += " && ";
        }
    }
    res += ") {";
    ctx.emit(offset, res);
    decodeBlock(ctx);
    ctx.emit(ctx.offset, "}");
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
    ctx.emit(ctx.offset, "switch (" + decodeOperand(ctx) + ") {");

    while (!ctx.isEOF()) {
        const auto op = static_cast<BlockOp>(ctx.getC());

        switch (op) {
            // Anything besides ScriptEnd is supposed to be treated as a case.
            // (according to vc interpreter, even if vcc uses a CASE opcode there)
            default: {
                ctx.emit(ctx.offset, "case " + decodeOperand(ctx) + ":");
                // printf("case ");
                // decodeOperand(ctx);
                // printf(":\n");
                static_cast<void>(ctx.getD());
                decodeBlock(ctx);
            }
            case BlockOp::End: {
                ctx.emit(ctx.offset, "}");
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

std::string decodeOperand(Ctx& ctx) {
    std::string res = decodeOperandTerm(ctx);

    while (!ctx.isEOF()) {
        const auto arithmeticOp = static_cast<ArithmeticOp>(ctx.getC());
        //printf("decode arith op %d\n", static_cast<int>(arithmeticOp));

        switch (arithmeticOp) {            
            case ArithmeticOp::Add:      res += " + " + decodeOperandTerm(ctx); break;
            case ArithmeticOp::Subtract: res += " - " + decodeOperandTerm(ctx); break;
            case ArithmeticOp::Divide:   res += " / " + decodeOperandTerm(ctx); break;
            case ArithmeticOp::Multiply: res += " * " + decodeOperandTerm(ctx); break;
            case ArithmeticOp::Modulo:   res += " % " + decodeOperandTerm(ctx); break;
            case ArithmeticOp::End:      return res;
            default:
                throw std::runtime_error(std::string{"*error* decodeOperand: Unknown arithmetic opcode in VC code "} + std::to_string(static_cast<int>(arithmeticOp)));            
        }
    }

    return res;
}

std::string decodeOperandTerm(Ctx& ctx) {
    const auto termType = static_cast<OpTermType>(ctx.getC());
    //printf("decode op term %d\n", static_cast<int>(termType));

    switch (termType) {
        case OpTermType::Immediate: return std::to_string(ctx.getD());
        case OpTermType::Var0:      return decodeVar0(ctx);
        case OpTermType::Var1:      return decodeVar1(ctx);
        case OpTermType::Var2:      return decodeVar2(ctx);
        case OpTermType::Group:     return decodeOperand(ctx);
        default:
            throw std::runtime_error(std::string{"*error* decodeOperandTerm: Unknown operand term type in VC code "} + std::to_string(static_cast<int>(termType)));
    }

    assert(0);
}

std::string decodeVar0(Ctx& ctx) {
    const auto var0 = static_cast<Var0>(ctx.getC());

    switch (var0) {
        case Var0::A:               return "a";
        case Var0::B:               return "b";
        case Var0::C:               return "c";
        case Var0::D:               return "d";
        case Var0::E:               return "e";
        case Var0::F:               return "f";
        case Var0::G:               return "g";
        case Var0::H:               return "h";
        case Var0::I:               return "i";
        case Var0::J:               return "j";
        case Var0::K:               return "k";
        case Var0::L:               return "l";
        case Var0::M:               return "m";
        case Var0::N:               return "n";
        case Var0::O:               return "o";
        case Var0::P:               return "p";
        case Var0::Q:               return "q";
        case Var0::R:               return "r";
        case Var0::S:               return "s";
        case Var0::T:               return "t";
        case Var0::U:               return "u";
        case Var0::V:               return "v";
        case Var0::W:               return "w";
        case Var0::X:               return "x";
        case Var0::Y:               return "y";
        case Var0::Z:               return "z";
        case Var0::NumChars:        return "numchars";
        case Var0::GP:              return "gp";
        case Var0::LocX:            return "locx";
        case Var0::LocY:            return "locy";
        case Var0::Timer:           return "timer";
        case Var0::DrawParty:       return "drawparty";
        case Var0::Cameratracking:  return "cameratracking";
        case Var0::XWin:            return "xwin";
        case Var0::YWin:            return "ywin";
        case Var0::B1:              return "b1";
        case Var0::B2:              return "b2";
        case Var0::B3:              return "b3";
        case Var0::B4:              return "b4";
        case Var0::Up:              return "up";
        case Var0::Down:            return "down";
        case Var0::Left:            return "left";
        case Var0::Right:           return "right";
        case Var0::TimerAnimate:    return "timeranimate";
        case Var0::Fade:            return "fade";
        case Var0::Layer0:          return "layer0";
        case Var0::Layer1:          return "layer1";
        case Var0::LayerVC:         return "layervc";
        case Var0::QuakeX:          return "quakex";
        case Var0::QuakeY:          return "quakey";
        case Var0::Quake:           return "quake";
        case Var0::ScreenGradient:  return "screengradient";
        case Var0::PMultX:          return "pmultx";
        case Var0::PMultY:          return "pmulty";
        case Var0::PDivX:           return "pdivx";
        case Var0::PDivY:           return "pdivy";
        case Var0::Volume:          return "volume";
        case Var0::ParallaxC:       return "parallaxc";
        case Var0::CancelFade:      return "cancelfade";
        case Var0::DrawEntities:    return "drawentities";
        case Var0::CurZone:         return "curzone";
        case Var0::TileB:           return "tileb";
        case Var0::TileF:           return "tilef";
        case Var0::ForegroundLock:  return "foregroundlock";
        case Var0::XWin1:           return "xwin1";
        case Var0::YWin1:           return "ywin1";
        case Var0::Layer1Trans:     return "layer1trans";
        case Var0::LayerVCTrans:    return "layervctrans";
        case Var0::FontColor:       return "fontcolor";
        case Var0::KeepAZ:          return "keepaz";
        case Var0::LayerVC2:        return "layervc2";
        case Var0::LayerVC2Trans:   return "layervc2trans";
        case Var0::VCWriteLayer:    return "vcwritelayer";
        case Var0::ModPosition:     return "modposition";
        default:
            throw std::runtime_error(std::string{"*error* decodeVar0: Unknown var0 index in VC code "} + std::to_string(static_cast<int>(var0)));
    }

    assert(0);
}

std::string decodeVar1(Ctx& ctx) {
    const auto var1 = static_cast<Var1>(ctx.getC());

    std::string result;

    switch (var1) {
        case Var1::Flags:               result = "flags"; break;
        case Var1::Facing:              result = "facing"; break;
        case Var1::Char:                result = "char"; break;
        case Var1::Item:                result = "item"; break;
        case Var1::Var:                 result = "var"; break;
        case Var1::PartyIndex:          result = "partyindex"; break;
        case Var1::XP:                  result = "xp"; break;
        case Var1::CurHP:               result = "curhp"; break;
        case Var1::MaxHP:               result = "maxhp"; break;
        case Var1::CurMP:               result = "curmp"; break;
        case Var1::MaxMP:               result = "maxmp"; break;
        case Var1::Key:                 result = "key"; break;
        case Var1::VCDataBuf:           result = "vcdatabuf"; break;
        case Var1::SpecialFrame:        result = "specialframe"; break;
        case Var1::Face:                result = "face"; break;
        case Var1::Speed:               result = "speed"; break;
        case Var1::EntityMoving:        result = "entity.moving"; break;
        case Var1::EntityCHRIndex:      result = "entity.chrindex"; break;
        case Var1::MoveCode:            result = "movecode"; break;
        case Var1::ActiveMode:          result = "activemode"; break;
        case Var1::ObsMode:             result = "obsmode"; break;
        case Var1::EntityStep:          result = "entity.step"; break;
        case Var1::EntityDelay:         result = "entity.delay"; break;
        case Var1::EntityLocX:          result = "entity.locx"; break;
        case Var1::EntityLocY:          result = "entity.locy"; break;
        case Var1::EntityX1:            result = "entity.x1"; break;
        case Var1::EntityX2:            result = "entity.x2"; break;
        case Var1::EntityY1:            result = "entity.y1"; break;
        case Var1::EntityY2:            result = "entity.y2"; break;
        case Var1::EntityFace:          result = "entity.face"; break;
        case Var1::EntityChasing:       result = "entity.chasing"; break;
        case Var1::EntityChaseDist:     result = "entity.chasedist"; break;
        case Var1::EntityChaseSpeed:    result = "entity.chasespeed"; break;
        case Var1::EntityScriptOfs:     result = "entity.scriptofs"; break;
        case Var1::ATK:                 result = "atk"; break;
        case Var1::DEF:                 result = "def"; break;
        case Var1::HIT:                 result = "hit"; break;
        case Var1::DOD:                 result = "dod"; break;
        case Var1::MAG:                 result = "mag"; break;
        case Var1::MGR:                 result = "mgr"; break;
        case Var1::REA:                 result = "rea"; break;
        case Var1::MBL:                 result = "mbl"; break;
        case Var1::FER:                 result = "fer"; break;
        case Var1::ItemUse:             result = "item.use"; break;
        case Var1::ItemEffect:          result = "item.effect"; break;
        case Var1::ItemType:            result = "item.type"; break;
        case Var1::ItemEquipType:       result = "item.equiptype"; break;
        case Var1::ItemEquipIndex:      result = "item.equipindex"; break;
        case Var1::ItemPrice:           result = "item.price"; break;
        case Var1::SpellUse:            result = "spell.use"; break;
        case Var1::SpellEffect:         result = "spell.effect"; break;
        case Var1::SpellType:           result = "spell.type"; break;
        case Var1::SpellPrice:          result = "spell.price"; break;
        case Var1::SpellCost:           result = "spell.cost"; break;
        case Var1::Lvl:                 result = "lvl"; break;
        case Var1::Nxt:                 result = "nxt"; break;
        case Var1::Charstatus:          result = "charstatus"; break;
        case Var1::Spell:               result = "spell"; break;
        default:
            throw std::runtime_error(std::string{"*error* decodeVar1: Unknown var1 index in VC code "} + std::to_string(static_cast<int>(var1)));
    }
    // '(' and '[' are equivalent in V1, so we use '[' for 1-argument array-like things, eg. flags.
    return result + "[" + decodeOperand(ctx) + "]";
}

std::string decodeVar2(Ctx& ctx) {
    const auto var2 = static_cast<Var2>(ctx.getC());

    std::string result;

    switch (var2) {
        case Var2::Random:      result = "random"; break;
        case Var2::Screen:      result = "screen"; break;
        case Var2::Items:       result = "items"; break;
        case Var2::CanEquip:    result = "canequip"; break;
        case Var2::ChooseChar:  result = "choosechar"; break;
        case Var2::Obs:         result = "obs"; break;
        case Var2::Spells:      result = "spells"; break;
        default:
            throw std::runtime_error(std::string{"*error* decodeVar2: Unknown var2 index in VC code "} + std::to_string(static_cast<int>(var2)));
    }
    auto op1 = decodeOperand(ctx);
    auto op2 = decodeOperand(ctx);
    return result + "(" + op1 + ", " + op2 + ")";
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
