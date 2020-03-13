// ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
// ³                                                                    ³
// ³                     The Verge-C Compiler v.0.10                    ³
// ³                     Copyright (C)1997 BJ Eirich                    ³
// ³                                                                    ³
// ³ Module: FUNCLIB.C                                                  ³
// ³                                                                    ³
// ³ Description: This simply parses and generates the output code      ³
// ³ for the built in library functions.                                ³
// ³                                                                    ³
// ³ Portability: ANSI C. Should compile on any 32-bit compiler.        ³
// ³                                                                    ³
// ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ

#include "code.h"
#include "compile.h"

/*  --  Added by Ric --
 * Added functions:
 *       VCBox(x1,y1,x2,y2); (21/Apr/98)
 *       VCCharName(x, y, party.dat index, center); (21/Apr/98)
 *       VCItemName(x, y, items.dat index, center); (21/Apr/98)
 *       VCItemDesc(x, y, items.dat index, center); (21/Apr/98)
 *       VCItemImage(x, y, items.dat index, greyflag); (22/Apr/98)
 *       VCATextNum(x, y, number, align);              (24/Apr/98)
 *       VCSpc(x, y, speech portrait, greyflag);       (24/Apr/98)
 *       CallVCScript(event number, option var1...);   (25/Apr/98)
 *           (used for CallEffect and CallScript)
 *       VCLine(x1, y1, x2, y2, color);                (??/???/??)
 *       GetMagic (character, spell);                  (??/???/??)
 *       BindKey(key code, script);                    (03/May/98)
 *       TextMenu(x,y,flag,ptr,"opt1","opt2",..);      (04/May/98)
 *       ItemMenu(roster order index);                 (03/May/98)
 *       EquipMenu(roster order index);                (03/May/98)
 *       MagicMenu(roster order index);                (03/May/98)
 *       StatusScreen(roster order index);             (03/May/98)
 *       VCCr2(x, y, speech portrait, greyflag);       (03/May/98)
 *       VCSpellName(x, y, magic.dat index, center);   (??/???/??)
 *       VCSpellDesc(x, y, magic.dat index, center);   (??/???/??)
 *       VCSpellImage(x, y, magic.dat index, greyflag);(??/???/??)
 *       MagicShop(spell1, spell2, spell3, ... spell12); (??/??/??)
 *       VCTextBox(x,y,ptr,"opt1","opt2",..);          (04/May/98)
 *       PlayVAS("filename",speed);                    (04/May/98):NichG
 *       VCMagicImage(x, y, items.dat index, greyflag);(04/May/98)
 */
#include "ricvc.c"
#include "nichgvc.c"
/* -- -- */

GenericFunc (unsigned char idcode, int numargs)
{ char i;

         EmitC (EXEC);
         EmitC (idcode);
         if (!numargs)
         {
             Expect ("(");
             Expect (")");
             Expect (";");
             return;
         }
         if (numargs==1)
         {
             Expect ("(");
             EmitOperand ();
             Expect (")");
             Expect (";");
             return;
         }
         Expect ("(");                   // numargs is greater than 1
         for (i=1; i<numargs; i++)
         {
             EmitOperand ();
             Expect (",");
         }
         EmitOperand ();
         Expect (")");
         Expect (";");
}

MapSwitch ()
{
         EmitC (EXEC);                   // Emit function exec code.
         EmitC (1);                      // Emit which function code to exec.
         Expect ("(");
         GetString ();
         EmitString(&token);             // Emit the map filename.
         Expect (",");
         EmitOperand ();
         Expect (",");
         EmitOperand ();
         Expect (",");
         EmitOperand ();
         Expect (")");
         Expect (";");
}

/*
Text ()
{ unsigned char t;
  char bufr[31], curcnt,linectr;
  int tokenptr;
  int lstoken,lscur;

         EmitC (EXEC);
         EmitW (6);
         Expect ("(");
         t = ExpectNumber ();
         EmitC (t);

         GrabString ();
         tokenptr = 0;
         linectr = 0;

         while (linectr<3)
         {
            curcnt = 0;
            linectr ++;

            while (curcnt < 31)
            {      bufr[curcnt] = token[tokenptr];
                   if (token[tokenptr] == ' ')
                      { lstoken = tokenptr;
                        lscur = curcnt;
                      }
                   tokenptr ++;
                   curcnt ++;
            }
         }
         Expect (";");
}
*/

Text ()
{
         EmitC (EXEC);
         EmitC (6);
         Expect ("(");
         EmitOperand ();
         Expect (",");
         GetString ();
         EmitString (&token);
         Expect (",");
         GetString ();
         EmitString (&token);
         Expect (",");
         GetString ();
         EmitString (&token);
         Expect (")");
         Expect (";");
}

DoReturn ()
{
         EmitC (ENDSCRIPT);
         Expect (";");
}

PlayMusic ()
{
         EmitC (EXEC);
         EmitC (11);
         Expect ("(");
         GetString ();
         EmitString (&token);
         Expect (")");
         Expect (";");
}

Banner ()
{
         EmitC (EXEC);
         EmitC (18);
         Expect ("(");
         GetString ();
         EmitString (&token);
         Expect (",");
         EmitOperand ();
         Expect (")");
         Expect (";");
}

Prompt ()
{
         EmitC (EXEC);
         EmitC (22);
         Expect ("(");
         EmitOperand ();
         Expect (",");
         GetString ();
         EmitString (&token);
         Expect (",");
         GetString ();
         EmitString (&token);
         Expect (",");
         GetString ();
         EmitString (&token);
         Expect (",");
         EmitOperand ();
         Expect (",");
         GetString ();
         EmitString (&token);
         Expect (",");
         GetString ();
         EmitString (&token);
         Expect (")");
         Expect (";");
}

ChainEvent ()
{ char *ptr, *optr, varctr=0;

         EmitC (EXEC);
         EmitC (23);
         Expect ("(");
         ptr = cpos;
         EmitC (varctr);
         EmitOperand ();
         while (!NextIs(")"))
         {
                Expect (",");
                EmitOperand();
                varctr++;
         }
         optr = cpos;
         cpos = ptr;
         EmitC (varctr);
         cpos = optr;
         Expect(")");
         Expect(";");
}

CallEvent ()
{ char *ptr, *optr, varctr=0;

         EmitC (EXEC);
         EmitC (24);
         Expect ("(");
         ptr = cpos;
         EmitC (varctr);
         EmitOperand ();
         while (!NextIs(")"))
         {
                Expect (",");
                EmitOperand();
                varctr++;
         }
         optr = cpos;
         cpos = ptr;
         EmitC (varctr);
         cpos = optr;
         Expect(")");
         Expect(";");
}

SText ()
{
         EmitC (EXEC);
         EmitC (33);
         Expect ("(");
         EmitOperand ();
         Expect (",");
         GetString ();
         EmitString (&token);
         Expect (",");
         GetString ();
         EmitString (&token);
         Expect (",");
         GetString ();
         EmitString (&token);
         Expect (")");
         Expect (";");
}

Shop ()
{ char *ptr, *optr, varctr=1;

         EmitC (EXEC);
         EmitC (47);
         Expect ("(");
         ptr = cpos;
         EmitC (varctr);
         EmitOperand ();
         while (!NextIs(")"))
         {
                Expect (",");
                EmitOperand();
                varctr++;
         }
         optr = cpos;
         cpos = ptr;
         EmitC (varctr);
         cpos = optr;
         Expect(")");
         Expect(";");
}

ChangeCHR ()
{
         EmitC (EXEC);
         EmitC (49);
         Expect ("(");
         EmitOperand ();
         Expect (",");
         GetString ();
         EmitString (&token);
         Expect (")");
         Expect (";");
}

VCPutPCX ()
{
         EmitC (EXEC);
         EmitC (51);
         Expect ("(");
         GetString ();
         EmitString (&token);
         Expect (",");
         EmitOperand ();
         Expect (",");
         EmitOperand ();
         Expect (")");
         Expect (";");
}

VCLoadPCX ()
{
         EmitC (EXEC);
         EmitC (54);
         Expect ("(");
         GetString ();
         EmitString (&token);
         Expect (",");
         EmitOperand ();
         Expect (")");
         Expect (";");
}

PlayFLI ()
{
         EmitC (EXEC);
         EmitC (56);
         Expect ("(");
         GetString ();
         EmitString(&token);
         Expect (")");
         Expect (";");
}

VCText ()
{
         EmitC (EXEC);
         EmitC (59);
         Expect ("(");
         EmitOperand ();
         Expect (",");
         EmitOperand ();
         Expect (",");
         GetString ();
         EmitString(&token);
         Expect (")");
         Expect (";");
}

Quit ()
{
         EmitC (EXEC);
         EmitC (62);
         Expect ("(");
         GetString ();
         EmitString(&token);
         Expect (")");
         Expect (";");
}

VCCenterText ()
{
         EmitC (EXEC);
         EmitC (63);
         Expect ("(");
         EmitOperand ();
         Expect (",");
         GetString ();
         EmitString(&token);
         Expect (")");
         Expect (";");
}

Sys_DisplayPCX ()
{
         EmitC (EXEC);
         EmitC (67);
         Expect ("(");
         GetString ();
         EmitString(&token);
         Expect (")");
         Expect (";");
}

NewGame ()
{
         EmitC (EXEC);
         EmitC (70);
         Expect ("(");
         GetString ();
         EmitString(&token);
         Expect (")");
         Expect (";");
}

PartyMove ()
{
         EmitC (EXEC);
         EmitC (73);
         Expect ("(");
         GetString ();
         EmitString(&token);
         Expect (")");
         Expect (";");
}

EntityMove ()
{
         EmitC (EXEC);
         EmitC (74);
         Expect ("(");
         EmitOperand ();
         Expect (",");
         GetString ();
         EmitString(&token);
         Expect (")");
         Expect (";");
}

VCLoadRaw ()
{
         EmitC (EXEC);
         EmitC (79);
         Expect ("(");
         GetString ();
         EmitString(&token);
         Expect(",");
         EmitOperand();
         Expect(",");
         EmitOperand();
         Expect(",");
         EmitOperand();
         Expect (")");
         Expect (";");
}

OutputCode (int idx)
{
         switch (idx)
         { case 0: MapSwitch(); break;
           case 1: GenericFunc(2,3); break;
           case 2: GenericFunc(3,1); break;
           case 3: GenericFunc(4,1); break;
           case 4: GenericFunc(5,1); break;
           case 5: Text(); break;
           case 6: GenericFunc(7,4); break;
           case 7: GenericFunc(8,4); break;
           case 8: GenericFunc(9,0); break;
           case 9: DoReturn(); break;
           case 10: PlayMusic(); break;
           case 11: GenericFunc(12,0); break;
           case 12: GenericFunc(13,0); break;
           case 13: GenericFunc(14,3); break;
           case 14: GenericFunc(15,1); break;
           case 15: GenericFunc(16,1); break;
           case 16: GenericFunc(17,1); break;
           case 17: Banner(); break;
           case 18: GenericFunc(19,0); break;
           case 19: GenericFunc(20,0); break;
           case 20: GenericFunc(21,2); break;
           case 21: Prompt(); break;
           case 22: ChainEvent(); break;
           case 23: CallEvent(); break;
           case 24: GenericFunc(25,2); break;
           case 25: GenericFunc(26,3); break;
           case 26: GenericFunc(27,0); break;
           case 27: GenericFunc(28,0); break;
           case 28: GenericFunc(29,0); break;
           case 29: GenericFunc(30,1); break;
           case 30: GenericFunc(31,2); break;
           case 31: GenericFunc(32,0); break;
           case 32: SText(); break;
           case 33: GenericFunc(34,0); break;
           case 34: GenericFunc(35,0); break;
           case 35: GenericFunc(36,1); break;
           case 36: GenericFunc(37,2); break;
           case 37: GenericFunc(38,4); break;
           case 38: GenericFunc(39,1); break;
           case 39: GenericFunc(40,1); break;
           case 40: GenericFunc(41,1); break;
           case 41: GenericFunc(42,1); break;
           case 42: GenericFunc(43,3); break;
           case 43: GenericFunc(44,2); break;
           case 44: GenericFunc(45,2); break;
           case 45: GenericFunc(46,2); break;
           case 46: Shop(); break;
           case 47: GenericFunc(48,5); break;
           case 48: ChangeCHR(); break;
           case 49: GenericFunc(50,0); break;
           case 50: VCPutPCX(); break;
           case 51: GenericFunc(52,1); break;
           case 52: GenericFunc(53,1); break;
           case 53: VCLoadPCX(); break;
           case 54: GenericFunc(55,5); break;
           case 55: PlayFLI(); break;
           case 56: GenericFunc(57,0); break;
           case 57: GenericFunc(58,4); break;
           case 58: VCText(); break;
           case 59: GenericFunc(60,5); break;
           case 60: GenericFunc(61,0); break;
           case 61: Quit(); break;
           case 62: VCCenterText(); break;
           case 63: GenericFunc(64,0); break;
           case 64: GenericFunc(65,3); break;
           case 65: GenericFunc(66,0); break;
           case 66: Sys_DisplayPCX(); break;
           case 67: GenericFunc(68,0); break;
           case 68: GenericFunc(69,0); break;
           case 69: NewGame(); break;
           case 70: GenericFunc(71,0); break;
           case 71: GenericFunc(72,1); break;
           case 72: PartyMove(); break;
           case 73: EntityMove(); break;
           case 74: GenericFunc(75,0); break;
           case 75: GenericFunc(76,0); break;
           case 76: GenericFunc(77,2); break;
           case 77: GenericFunc(78,3); break;
           case 78: VCLoadRaw(); break;

           case 79: GenericFunc(80,4); break; /* -- ric:21/Apr/98 - VCBox       -- */
           case 80: GenericFunc(81,4); break; /* -- ric:21/Apr/98 - VCCharName  -- */
           case 81: GenericFunc(82,4); break; /* -- ric:21/Apr/98 - VCItemName  -- */
           case 82: GenericFunc(83,4); break; /* -- ric:21/Apr/98 - VCItemDesc  -- */
           case 83: GenericFunc(84,4); break; /* -- ric:22/Apr/98 - VCItemImage -- */
           case 84: GenericFunc(85,4); break; /* -- ric:24/Apr/98 - VCATextNum  -- */
           case 85: GenericFunc(86,4); break; /* -- ric:24/Apr/98 - VCSpc       -- */
           case 86: CallVCScript(87); break;  /* -- ric:25/Apr/98 - CallEffect  -- */
           case 87: CallVCScript(88); break;  /* -- ric:25/Apr/98 - CallScript  -- */
           case 88: GenericFunc(89,5); break; /* -- NichG:??/??/?? - VCLine     -- */
           case 89: GenericFunc(90,2); break; /* -- NichG:??/??/?? - GetMagic   -- */
           case 90: GenericFunc(91,2); break; /* -- ric:03/May/98 - BindKey     -- */
           case 91: TextMenu(92); break;      /* -- ric:04/May/98 - TextMenu    -- */
           case 92: GenericFunc(93,1); break; /* -- ric:03/May/98 - ItemMenu    -- */
           case 93: GenericFunc(94,1); break; /* -- ric:03/May/98 - EquipMenu   -- */
           case 94: GenericFunc(95,1); break; /* -- ric:03/May/98 - MagicMenu   -- */
           case 95: GenericFunc(96,1); break; /* -- ric:03/May/98 - StatusScreen -- */
           case 96: GenericFunc(97,4); break; /* -- ric:24/Apr/98 - VCCr2       -- */
           case 97: GenericFunc(98,4); break; /* -- NichG\Ric: ??/??/?? - VCSpellName -- */
           case 98: GenericFunc(99,4); break; /* -- NichG\Ric: ??/??/?? - VCSpellDesc -- */
           case 99: GenericFunc(100,4); break;/* -- NichG\Ric: ??/??/?? - VCSpellImage -- */
           case 100: MagicShop(); break;      /* -- NichG: ??/??/?? - MagicShop -- */
           case 101: VCTextBox(102); break;   /* -- ric:04/May/98 - VCTextBox   -- */
           case 102: PlayVAS(); break;        /* -- NichG: ??/??/?? - PlayVAS   -- */     
//           case 103: GenericFunc(104,4); break;/* -- ric:04/May/98 - VCMagicImage -- */
//           case 104: GenericFunc(105,1); break;/* -- xBig_D:05/May/98 - VCLayerWrite -- */

           default: err("*error* Internal error: Unknown std function.");
         }
}
