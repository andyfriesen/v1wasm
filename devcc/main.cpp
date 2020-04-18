#include <string>
#include <vector>
#include <cstdint>
#include <stdexcept>

// FIXME: line buffers for output, allow adding lines in earlier part of output during disassembly (for fixing up labels, etc).
// FIXME: indentation for blocks.
// FIXME: capitalization style? (eg. user-customizable case for variable names, function names, etc, since Verge allows any case)
// FIXME: indentation style? (brace on next line like, tab count)
// FIXME: spacing style? (space betwen operators, after commas, etc)

using Bytecode = std::vector<std::uint8_t>;

struct Code {
    std::vector<std::uint32_t> offsets;
    Bytecode code;
};

Code loadCode(const std::string& fileName) {
    FILE* f = fopen(fileName.c_str(), "rb");

    fseek(f, 0, SEEK_END);
    const auto size = static_cast<unsigned>(ftell(f));

    // Map files have more stuff.
    if (fileName.rfind(".map") == fileName.size() - 4) {
        fseek(f, 68, SEEK_SET);
        std::uint16_t mx, my;
        fread(&mx, 1, 2, f);
        fread(&my, 1, 2, f);

        fseek(f, 100+(mx*my*5)+7956, SEEK_SET);

        std::uint32_t a;
        std::uint8_t i;
        fread(&a, 1, 4, f);
        fseek(f, 88*a, 1);
        fread(&i, 1, 1, f);
        fread(&a, 1, 4, f);
        fseek(f, (i*4)+a, SEEK_CUR);
    } else {
        fseek(f, 0, SEEK_SET);
    }

    std::uint32_t numscripts;
    fread(&numscripts, 1, 4, f);

    std::vector<std::uint32_t> scriptofstbl;
    scriptofstbl.resize(numscripts);
    fread(scriptofstbl.data(), 4, numscripts, f);

    std::vector<std::uint8_t> code;
    code.resize(size - static_cast<unsigned>(ftell(f)));
    fread(code.data(), 1, code.size(), f);

    fclose(f);

    return Code{ std::move(scriptofstbl), std::move(code) };
}


std::uint8_t getC(const Bytecode& code, int& i) {
    return code[i++];
}

std::int32_t getD(const Bytecode& code, int& i) {
    const auto a = getC(code, i);
    const auto b = getC(code, i);
    const auto c = getC(code, i);
    const auto d = getC(code, i);
    return static_cast<std::int32_t>(
        (static_cast<std::uint32_t>(d) << 24U)
        | (static_cast<std::uint32_t>(c) << 16U)
        | (static_cast<std::uint32_t>(b) << 8U)
        | static_cast<std::uint32_t>(a));
}

std::string getString(const Bytecode& code, int& i) {
    int startI = i;
    while (code[i]) {
        ++i;
    }
    ++i;

    return std::string(code.begin() + startI, code.begin() + i);
}

void decode(const Code& code, int index);
void decodeBlock(const Bytecode& code, int& i);
void decodeExec(const Bytecode& code, int& i);
void decodeAssignOp(const Bytecode& code, int& i);
void decodeVar0Assign(const Bytecode& code, int& i);
void decodeVar1Assign(const Bytecode& code, int& i);
void decodeVar2Assign(const Bytecode& code, int& i);
void decodeIf(const Bytecode& code, int& i);
void decodeForLoop0(const Bytecode& code, int& i);
void decodeForLoop1(const Bytecode& code, int& i);
void decodeGoto(const Bytecode& code, int& i);
void decodeSwitch(const Bytecode& code, int& i);
void skipOperand(const Bytecode& code, int& i);
void skipOperandTerm(const Bytecode& code, int& i);
void decodeOperand(const Bytecode& code, int& i);
void decodeOperandTerm(const Bytecode& code, int& i);
void decodeVar0(const Bytecode& code, int& i);
void decodeVar1(const Bytecode& code, int& i);
void decodeVar2(const Bytecode& code, int& i);

void decode(const Code& code, int index) {
    const auto startOffset = code.offsets[index];
    const auto endOffset = (index == static_cast<int>(code.offsets.size()) - 1) ? code.offsets.size() : code.offsets[index + 1];

    Bytecode bc{ code.code.begin() + startOffset, code.code.begin() + endOffset };
    int i = 0;
    decodeBlock(bc, i);
}

enum class Op {
    Exec = 1,
    Var0Assign = 2,
    Var1Assign = 3,
    Var2Assign = 4,
    If = 5,
    Goto = 7,
    ForLoop0 = 8,
    ForLoop1 = 9,
    Switch = 10,

    EndScript = 255
};

void decodeBlock(const Bytecode& code, int& i) {
    while (i < static_cast<int>(code.size())) {
        const auto op = static_cast<Op>(getC(code, i));

        switch (op) {
            case Op::Exec: decodeExec(code, i); break;
            case Op::Var0Assign: decodeVar0Assign(code, i); break;
            case Op::Var1Assign: decodeVar1Assign(code, i); break;
            case Op::Var2Assign: decodeVar2Assign(code, i); break;
            case Op::If: decodeIf(code, i); break;
            case Op::ForLoop0: decodeForLoop0(code, i); break;
            case Op::ForLoop1: decodeForLoop1(code, i); break;
            case Op::Goto: decodeGoto(code, i); break;
            case Op::Switch: decodeSwitch(code, i); break;
            case Op::EndScript: return;
            default: throw std::runtime_error(std::string{"*error* decodeBlock: Illegal opcode in VC code "} + std::to_string(static_cast<int>(op)));
        }
    }
}

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

void decodeGenericFunc(const Bytecode& code, int& i, const std::string& funcName, int argumentCount) {
    printf("%s(", funcName.c_str());
    for (int argumentIndex = 0; argumentIndex < argumentCount; argumentIndex++) {
        decodeOperand(code, i);

        if (argumentIndex < argumentCount - 1) {
            printf(", ");
        }
    }
    printf(");\n");
}

void decodeExec(const Bytecode& code, int& i) {
    const auto funcId = static_cast<FuncId>(getC(code, i));

    switch (funcId) {
        case FuncId::MapSwitch: {
            printf("MapSwitch(");
            printf("\"%s\", ", getString(code, i).c_str());
            decodeOperand(code, i);
            printf(", ");
            decodeOperand(code, i);
            printf(", ");
            decodeOperand(code, i);
            printf(");\n");
            break;
        }
        case FuncId::Warp: decodeGenericFunc(code, i, "Warp", 3); break;
        case FuncId::AddCharacter: decodeGenericFunc(code, i, "AddCharacter", 1); break;
        case FuncId::SoundEffect: decodeGenericFunc(code, i, "SoundEffect", 1); break;
        case FuncId::GiveItem: decodeGenericFunc(code, i, "GiveItem", 1); break;
        case FuncId::Text: {
            printf("Text(");
            decodeOperand(code, i);
            printf(", ");
            printf("\"%s\", ", getString(code, i).c_str());
            printf("\"%s\", ", getString(code, i).c_str());
            printf("\"%s\"", getString(code, i).c_str());
            printf(");\n");
            break;
        }
        case FuncId::AlterFTile: decodeGenericFunc(code, i, "AlterFTile", 4); break;
        case FuncId::AlterBTile: decodeGenericFunc(code, i, "AlterBTile", 4); break;
        case FuncId::FakeBattle: decodeGenericFunc(code, i, "FakeBattle", 0); break;
        case FuncId::Return: printf("return;\n"); break;
        case FuncId::PlayMusic: {
            printf("PlayMusic(");
            printf("\"%s\", ", getString(code, i).c_str());
            printf(");\n");
            break;
        }
        case FuncId::StopMusic: printf("StopMusic();\n"); break;
        case FuncId::HealAll: printf("HealAll();\n"); break;
        case FuncId::AlterParallax: decodeGenericFunc(code, i, "AlterParallax", 3); break;
        case FuncId::FadeIn: decodeGenericFunc(code, i, "FadeIn", 1); break;
        case FuncId::FadeOut: decodeGenericFunc(code, i, "FadeOut", 2); break;
        case FuncId::RemoveCharacter: decodeGenericFunc(code, i, "RemoveCharacter", 1); break;
        case FuncId::Banner: {
            printf("Banner(\n");
            printf("\"%s\", ", getString(code, i).c_str());
            printf(", ");
            decodeOperand(code, i);
            printf(");\n");     
            break;
        }
        case FuncId::EnforceAnimation: decodeGenericFunc(code, i, "EnforceAnimation", 0); break;
        case FuncId::WaitKeyUp: decodeGenericFunc(code, i, "WaitKeyUp", 0); break;
        case FuncId::DestroyItem: decodeGenericFunc(code, i, "DestroyItem", 2); break;
        case FuncId::Prompt: {
            printf("Text(");
            decodeOperand(code, i);
            printf(", ");
            printf("\"%s\", ", getString(code, i).c_str());
            printf("\"%s\", ", getString(code, i).c_str());
            printf("\"%s\", ", getString(code, i).c_str());
            decodeOperand(code, i);
            printf(", ");
            printf("\"%s\", ", getString(code, i).c_str());
            printf("\"%s\"", getString(code, i).c_str());
            printf(");\n");
            break;
        }        
        case FuncId::ChainEvent: {
            const auto argumentCount = getC(code, i) + 1;
            decodeGenericFunc(code, i, "ChainEvent", argumentCount);
            break;
        }
        case FuncId::CallEvent: {
            const auto argumentCount = getC(code, i) + 1;
            decodeGenericFunc(code, i, "CallEvent", argumentCount);
            break;
        }
        case FuncId::Heal: decodeGenericFunc(code, i, "Heal", 2); break;
        case FuncId::EarthQuake: decodeGenericFunc(code, i, "EarthQuake", 3); break;
        case FuncId::SaveMenu: decodeGenericFunc(code, i, "SaveMenu", 0); break;
        case FuncId::EnableSave: decodeGenericFunc(code, i, "EnableSave", 0); break;
        case FuncId::DisableSave: decodeGenericFunc(code, i, "DisableSave", 0); break;
        case FuncId::ReviveChar: decodeGenericFunc(code, i, "ReviveChar", 1); break;
        case FuncId::RestoreMP: decodeGenericFunc(code, i, "RestoreMP", 2); break;
        case FuncId::Redraw: decodeGenericFunc(code, i, "Redraw", 0); break;
        case FuncId::SText: {
            printf("SText(");
            decodeOperand(code, i);
            printf(", ");
            printf("\"%s\", ", getString(code, i).c_str());
            printf("\"%s\", ", getString(code, i).c_str());
            printf("\"%s\"", getString(code, i).c_str());
            printf(");\n");
            break;
        }
        case FuncId::DisableMenu: decodeGenericFunc(code, i, "DisableMenu", 0); break;
        case FuncId::EnableMenu: decodeGenericFunc(code, i, "EnableMenu", 0); break;
        case FuncId::Wait: decodeGenericFunc(code, i, "Wait", 1); break;
        case FuncId::SetFace: decodeGenericFunc(code, i, "SetFace", 2); break;
        case FuncId::MapPaletteGradient: decodeGenericFunc(code, i, "MapPaletteGradient", 4); break;
        case FuncId::BoxFadeOut: decodeGenericFunc(code, i, "BoxFadeOut", 1); break;
        case FuncId::BoxFadeIn: decodeGenericFunc(code, i, "BoxFadeIn", 1); break;
        case FuncId::GiveGP: decodeGenericFunc(code, i, "GiveGP", 1); break;
        case FuncId::TakeGP: decodeGenericFunc(code, i, "TakeGP", 1); break;
        case FuncId::ChangeZone: decodeGenericFunc(code, i, "ChangeZone", 3); break;
        case FuncId::GetItem: decodeGenericFunc(code, i, "ChangeZone", 2); break;
        case FuncId::ForceEquip: decodeGenericFunc(code, i, "ForceEquip", 2); break;
        case FuncId::GiveXP: decodeGenericFunc(code, i, "GiveXP", 2); break;
        case FuncId::Shop: {
            const auto argumentCount = getC(code, i);
            decodeGenericFunc(code, i, "Shop", argumentCount);
            break;
        }
        case FuncId::PaletteMorph: decodeGenericFunc(code, i, "PaletteMorph", 5); break;
        case FuncId::ChangeCHR: {
            printf("ChangeCHR(");
            decodeOperand(code, i);
            printf(", ");
            printf("\"%s\"", getString(code, i).c_str());
            printf(");\n");
            break;
        }
        case FuncId::ReadControls: decodeGenericFunc(code, i, "ReadControls", 0); break;
        case FuncId::VCPutPCX: {
            printf("VCPutPCX(");
            printf("\"%s\", ", getString(code, i).c_str());
            decodeOperand(code, i);
            printf(", ");
            decodeOperand(code, i);            
            printf(");\n");
            break;
        }
        case FuncId::HookTimer: decodeGenericFunc(code, i, "HookTimer", 1); break;
        case FuncId::HookRetrace: decodeGenericFunc(code, i, "HookRetrace", 1); break;
        case FuncId::VCLoadPCX: {
            printf("VCLoadPCX(");
            printf("\"%s\", ", getString(code, i).c_str());
            decodeOperand(code, i);
            printf(");\n");
            break;
        }
        case FuncId::VCBlitImage: decodeGenericFunc(code, i, "VCBlitImage", 5); break;
        case FuncId::PlayFLI: throw std::runtime_error(std::string{"*error* TODO: PlayFLI"});
        case FuncId::VCClear: decodeGenericFunc(code, i, "VCClear", 0); break;
        case FuncId::VCClearRegion: decodeGenericFunc(code, i, "VCClear", 4); break;
        case FuncId::VCText: {
            printf("VCText(");
            decodeOperand(code, i);
            printf(", ");
            decodeOperand(code, i);
            printf(", ");            
            printf("\"%s\"", getString(code, i).c_str());
            printf(");\n");
            break;
        }
        case FuncId::VCTBlitImage: decodeGenericFunc(code, i, "VCTBlitImage", 5); break;
        case FuncId::Exit: decodeGenericFunc(code, i, "Exit", 0); break;
        case FuncId::Quit: {
            printf("Quit(");
            printf("\"%s\"", getString(code, i).c_str());
            printf(");\n");
            break;
        }
        case FuncId::VCCenterText: {
            printf("VCCenterText(");
            decodeOperand(code, i);
            printf(", ");
            decodeOperand(code, i);
            printf(", ");            
            printf("\"%s\"", getString(code, i).c_str());
            printf(");\n");
            break;
        }
        case FuncId::ResetTimer: decodeGenericFunc(code, i, "ResetTimer", 0); break;
        case FuncId::VCBlitTile: decodeGenericFunc(code, i, "VCBlitTile", 3); break;
        case FuncId::Sys_ClearScreen: decodeGenericFunc(code, i, "Sys_ClearScreen", 0); break;
        case FuncId::Sys_DisplayPCX: {
            printf("Sys_DisplayPCX(");
            printf("\"%s\"", getString(code, i).c_str());
            printf(");\n");
            break;
        }        
        case FuncId::OldStartupMenu: decodeGenericFunc(code, i, "OldStartupMenu", 0); break;
        case FuncId::VGADump: decodeGenericFunc(code, i, "VGADump", 0); break;
        case FuncId::NewGame: {
            printf("NewGame(");
            printf("\"%s\"", getString(code, i).c_str());
            printf(");\n");
            break;
        }
        case FuncId::LoadSaveErase: decodeGenericFunc(code, i, "LoadSaveErase", 0); break;
        case FuncId::Delay: decodeGenericFunc(code, i, "Delay", 1); break;
        case FuncId::PartyMove: {
            printf("PartyMove(");
            printf("\"%s\"", getString(code, i).c_str());
            printf(");\n");
            break;
        }
        case FuncId::EntityMove: {
            printf("EntityMove(");
            decodeOperand(code, i);
            printf(", ");            
            printf("\"%s\"", getString(code, i).c_str());
            printf(");\n");
            break;
        } 
        case FuncId::AutoOn: decodeGenericFunc(code, i, "AutoOn", 0); break;
        case FuncId::AutoOff: decodeGenericFunc(code, i, "AutoOff", 0); break;
        case FuncId::EntityMoveScript: decodeGenericFunc(code, i, "EntityMoveScript", 2); break;
        case FuncId::VCTextNum: decodeGenericFunc(code, i, "VCTextNum", 3); break;
        case FuncId::VCLoadRaw: {
            printf("VCLoadRaw(");
            printf("\"%s\", ", getString(code, i).c_str());
            decodeOperand(code, i);
            printf(", ");
            decodeOperand(code, i);
            printf(", ");
            decodeOperand(code, i);
            printf(");\n");
            break;
        }
        case FuncId::VCBox: decodeGenericFunc(code, i, "VCBox", 4); break;
        case FuncId::VCCharName: decodeGenericFunc(code, i, "VCCharName", 4); break;
        case FuncId::VCItemName: decodeGenericFunc(code, i, "VCItemName", 4); break;
        case FuncId::VCItemDesc: decodeGenericFunc(code, i, "VCItemDesc", 4); break;
        case FuncId::VCItemImage: decodeGenericFunc(code, i, "VCItemImage", 4); break;
        case FuncId::VCATextNum: decodeGenericFunc(code, i, "VCATextNum", 4); break;
        case FuncId::VCSpc: decodeGenericFunc(code, i, "VCSpc", 4); break;
        case FuncId::CallEffect: {
            const auto argumentCount = getC(code, i) + 1;
            decodeGenericFunc(code, i, "CallEffect", argumentCount);
            break;
        }
        case FuncId::CallScript: {
            const auto argumentCount = getC(code, i) + 1;
            decodeGenericFunc(code, i, "CallScript", argumentCount);
            break;
        }
        case FuncId::VCLine: decodeGenericFunc(code, i, "VCLine", 5); break;
        case FuncId::GetMagic: decodeGenericFunc(code, i, "GetMagic", 2); break;
        case FuncId::BindKey: decodeGenericFunc(code, i, "BindKey", 2); break;
        case FuncId::TextMenu: {
            printf("TextMenu(");
            decodeOperand(code, i);
            printf(", ");
            decodeOperand(code, i);
            printf(", ");
            decodeOperand(code, i);
            printf(", ");
            decodeOperand(code, i);
            printf(", ");

            const auto argumentCount = getC(code, i) + 1;
            for (int argumentIndex = 0; argumentIndex < argumentCount; argumentIndex++) {
                printf("\"%s\"", getString(code, i).c_str());

                if (argumentIndex < argumentCount - 1) {
                    printf(", ");
                }
            }

            printf(");\n");
            break;
        }
        case FuncId::ItemMenu: decodeGenericFunc(code, i, "ItemMenu", 1); break;
        case FuncId::EquipMenu: decodeGenericFunc(code, i, "EquipMenu", 1); break;
        case FuncId::MagicMenu: decodeGenericFunc(code, i, "MagicMenu", 1); break;
        case FuncId::StatusScreen: decodeGenericFunc(code, i, "StatusScreen", 1); break;
        case FuncId::VCCr2: decodeGenericFunc(code, i, "VCCr2", 4); break;
        case FuncId::VCSpellName: decodeGenericFunc(code, i, "VCSpellName", 4); break;
        case FuncId::VCSpellDesc: decodeGenericFunc(code, i, "VCSpellDesc", 4); break;
        case FuncId::VCSpellImage: decodeGenericFunc(code, i, "VCSpellImage", 4); break;
        case FuncId::MagicShop: {
            const auto argumentCount = getC(code, i) + 1;
            decodeGenericFunc(code, i, "MagicShop", argumentCount);
            break;
        }
        case FuncId::VCTextBox: {
            printf("VCTextBox(");
            decodeOperand(code, i);
            printf(", ");
            decodeOperand(code, i);
            printf(", ");
            decodeOperand(code, i);
            printf(", ");

            const auto argumentCount = getC(code, i) + 1;
            for (int argumentIndex = 0; argumentIndex < argumentCount; argumentIndex++) {
                printf("\"%s\"", getString(code, i).c_str());

                if (argumentIndex < argumentCount - 1) {
                    printf(", ");
                }
            }

            printf(");\n");
            break;
        }
        case FuncId::PlayVAS: {
            printf("PlayVAS(");
            printf("\"%s\", ", getString(code, i).c_str());
            decodeOperand(code, i);
            printf(", ");
            decodeOperand(code, i);
            printf(", ");
            decodeOperand(code, i);
            printf(", ");
            decodeOperand(code, i);
            printf(", ");
            decodeOperand(code, i);
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

enum class AssignOp {
    Set = 1,
    Increment = 2,
    Decrement = 3,
    IncSet = 4,
    DecSet = 5,
};

void decodeAssignOp(const Bytecode& code, int& i) {
    const auto assignOp = static_cast<AssignOp>(getC(code, i));

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

void decodeVar0Assign(const Bytecode& code, int& i) {
    decodeVar0(code, i);
    decodeAssignOp(code, i);
    decodeOperand(code, i);
    printf(";\n");
}

void decodeVar1Assign(const Bytecode& code, int& i) {
    decodeVar1(code, i);
    decodeAssignOp(code, i);
    decodeOperand(code, i);
    printf(";\n");
}

void decodeVar2Assign(const Bytecode& code, int& i) {
    decodeVar2(code, i);
    decodeAssignOp(code, i);
    decodeOperand(code, i);
    printf(";\n");
}

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

void decodeIf(const Bytecode& code, int& i) {
    const auto argumentCount = static_cast<int>(getC(code, i));
    static_cast<void>(getD(code, i));

    printf("if ("); 
    for (int argumentIndex = 0; argumentIndex < argumentCount; argumentIndex++) {
        const auto previousOffset = i;
        skipOperand(code, i);

        // FIXME: remove skipOperand if we can actually modify existing output, we only need to seek back to prepend unary !

        const auto branchOp = static_cast<BranchOp>(getC(code, i));
        i = previousOffset;

        switch (branchOp) {
            case BranchOp::Zero: {
                decodeOperand(code, i);
                i++;
                break;
            }
            case BranchOp::NonZero: {
                printf("!");
                decodeOperand(code, i);
                i++;
                break;
            }
            case BranchOp::Equal: {
                decodeOperand(code, i);
                i++;
                printf(" == ");
                decodeOperand(code, i);
                break;
            }
            case BranchOp::NotEqual: {
                decodeOperand(code, i);
                i++;
                printf(" != ");
                decodeOperand(code, i);
                break;
            }
            case BranchOp::GreaterThan: {
                decodeOperand(code, i);
                i++;
                printf(" >" );
                decodeOperand(code, i);
                break;
            }
            case BranchOp::GreaterThanOrEqual: {
                decodeOperand(code, i);
                i++;
                printf(" >= ");
                decodeOperand(code, i);
                break;
            }
            case BranchOp::LessThan: {
                decodeOperand(code, i);
                i++;
                printf(" < ");
                decodeOperand(code, i);
                break;
            }
            case BranchOp::LessThanOrEqual: {
                decodeOperand(code, i);
                i++;
                printf(" <= ");
                decodeOperand(code, i);
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
    decodeBlock(code, i);
    printf("}\n"); 
}

void decodeForLoop0(const Bytecode& code, int& i) {
    printf("for(");
    decodeVar0(code, i);
    printf(",");
    decodeOperand(code, i);
    printf(",");
    decodeOperand(code, i);
    printf(",");
    printf(getC(code, i) == 0 ? "-" : "");
    decodeOperand(code, i);
    printf(")\n");
    printf("{\n"); 
    decodeBlock(code, i);
    printf("}\n"); 
}

void decodeForLoop1(const Bytecode& code, int& i) {
    printf("for(");
    decodeVar1(code, i);
    printf(",");
    decodeOperand(code, i);
    printf(",");
    decodeOperand(code, i);
    printf(",");
    printf(getC(code, i) == 0 ? "-" : "");
    decodeOperand(code, i);
    printf(")\n");
    printf("{\n"); 
    decodeBlock(code, i);
    printf("}\n");
}

void decodeGoto(const Bytecode& code, int& i) {
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
    const auto dest = getD(code, i);
    printf("// FIXME: goto or while to offset %d\n", dest);
}

void decodeSwitch(const Bytecode& code, int& i) {
    // FIXME: argh, how do we know where a switch ends.

    printf("switch (");
    decodeOperand(code, i);
    printf(")");
    printf("{\n");

    while (i < static_cast<int>(code.size())) {
        const auto op = static_cast<Op>(getC(code, i));

        switch (op) {
            // Anything besides ScriptEnd is supposed to be treated as a case.
            // (according to vc interpreter, even if vcc uses a CASE opcode there)
            default: {
                printf("case ");
                decodeOperand(code, i);
                printf(":\n");
                static_cast<void>(getD(code, i));
                decodeBlock(code, i);
            }
            case Op::EndScript: {
                printf("}\n");
                return;
            }            
        }

    }
}

enum class ArithmeticOp {
    Add = 1,
    Sub = 2,
    Div = 3,
    Mul = 4,
    Mod = 5,
    End = 255,
};

enum class OpDesc {
    Immediate = 1,
    Var0 = 2,
    Var1 = 3,
    Var2 = 4,
    Group = 5
};

void skipOperand(const Bytecode& code, int& i) {
    skipOperandTerm(code, i);

    while (i < static_cast<int>(code.size())) {
        const auto arithmeticOp = static_cast<ArithmeticOp>(getC(code, i));
        //printf("skip arith op %d\n", static_cast<int>(arithmeticOp));

        switch (arithmeticOp) {
            case ArithmeticOp::Add:
            case ArithmeticOp::Sub:
            case ArithmeticOp::Div:
            case ArithmeticOp::Mul:
            case ArithmeticOp::Mod:
                skipOperandTerm(code, i);
                break;
            case ArithmeticOp::End:
                return;
            default:
                throw std::runtime_error(std::string{"*error* skipOperand: Unknown arithmetic opcode in VC code "} + std::to_string(static_cast<int>(arithmeticOp)));
        }
    }
}

void skipOperandTerm(const Bytecode& code, int& i) {
    const auto desc = static_cast<OpDesc>(getC(code, i));
    //printf("skip op desc %d\n", static_cast<int>(desc));

    switch (desc) {
        case OpDesc::Immediate: {
            static_cast<void>(getD(code, i));
            break;
        }
        case OpDesc::Var0: {
            static_cast<void>(getC(code, i));
            break;
        }
        case OpDesc::Var1: {
            static_cast<void>(getC(code, i));
            skipOperand(code, i);
            break;
        }
        case OpDesc::Var2: {
            static_cast<void>(getC(code, i));
            skipOperand(code, i);
            skipOperand(code, i);
            break;
        }
        case OpDesc::Group: {
            skipOperand(code, i);
            break;
        }
        default:
            throw std::runtime_error(std::string{"*error* skipOperandTerm: Unknown operand descriptor in VC code "} + std::to_string(static_cast<int>(desc)));        
    }
}

void decodeOperand(const Bytecode& code, int& i) {
    decodeOperandTerm(code, i);

    while (i < static_cast<int>(code.size())) {
        const auto arithmeticOp = static_cast<ArithmeticOp>(getC(code, i));
        //printf("decode arith op %d\n", static_cast<int>(arithmeticOp));

        switch (arithmeticOp) {            
            case ArithmeticOp::Add: printf(" + "); decodeOperandTerm(code, i); break;
            case ArithmeticOp::Sub: printf(" - "); decodeOperandTerm(code, i); break;
            case ArithmeticOp::Div: printf(" / "); decodeOperandTerm(code, i); break;
            case ArithmeticOp::Mul: printf(" * "); decodeOperandTerm(code, i); break;
            case ArithmeticOp::Mod: printf(" %% "); decodeOperandTerm(code, i); break;
            case ArithmeticOp::End: return;
            default:
                throw std::runtime_error(std::string{"*error* decodeOperand: Unknown arithmetic opcode in VC code "} + std::to_string(static_cast<int>(arithmeticOp)));            
        }
    }
}

void decodeOperandTerm(const Bytecode& code, int& i) {
    const auto desc = static_cast<OpDesc>(getC(code, i));
    //printf("decode op desc %d\n", static_cast<int>(desc));

    switch (desc) {
        case OpDesc::Immediate: printf("%d", getD(code, i)); break;
        case OpDesc::Var0: decodeVar0(code, i); break;
        case OpDesc::Var1: decodeVar1(code, i); break;
        case OpDesc::Var2: decodeVar2(code, i); break;
        case OpDesc::Group: decodeOperand(code, i); break;
        default:
            throw std::runtime_error(std::string{"*error* decodeOperandTerm: Unknown operand descriptor in VC code "} + std::to_string(static_cast<int>(desc)));
    }
}

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

void decodeVar0(const Bytecode& code, int& i) {
    const auto var0 = static_cast<Var0>(getC(code, i));

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

void decodeVar1(const Bytecode& code, int& i) {
    const auto var1 = static_cast<Var1>(getC(code, i));

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
    decodeOperand(code, i);
    printf("]");
}

enum class Var2 {
    Random,
    Screen,
    Items,
    CanEquip,
    ChooseChar,
    Obs,
    Spells,
};

void decodeVar2(const Bytecode& code, int& i) {
    const auto var2 = static_cast<Var2>(getC(code, i));

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
    decodeOperand(code, i);
    decodeOperand(code, i);
    printf(")");    
}

int main(int argc, char** argv) {
    if (argc < 2) {
        fprintf(stderr, "usage: %s filename\n", argv[0]);
        return 1;
    }

    Code code = loadCode(argv[1]);
    decode(code, 0);
    return 0;
}
