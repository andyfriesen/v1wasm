// ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
// ³                                                                    ³
// ³                     The Verge-C Compiler v.0.10                    ³
// ³                     Copyright (C)1997 BJ Eirich                    ³
// ³                                                                    ³
// ³ Module: COMPILE.C                                                  ³
// ³                                                                    ³
// ³ Description: The main lexical parser and code generator.           ³
// ³                                                                    ³
// ³ Portability: ANSI C - should compile on any 32 bit compiler.       ³
// ³                                                                    ³
// ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ

#include <stdio.h>
#include "code.h"
#include "libfuncs.h"
#include "vcc.h"

// ============================== Constants ===============================

// Character types

#define LETTER 1
#define DIGIT 2
#define SPECIAL 3

// Token types

#define IDENTIFIER 1
#define DIGIT 2
#define CONTROL 3
#define RESERVED 4
#define FUNCTION 5
#define VAR0 5
#define VAR1 6
#define VAR2 7

// ============================== Variables ==============================

struct label                      // goto labels
{
  char ident[40];
  char *pos;
};

struct label labels[200];         // goto labels
struct label gotos[200];          // goto occurence records
char token[2048];                 // current token buffer
char lasttoken[2048];             // restorebuf for NextIs
unsigned int token_nvalue;        // int value of token if it's type DIGIT
char token_type;                  // type of current token.
char token_subtype;               // This is just crap.
char *src;                        // ptr -> current location in sourcefile
char *code;                       // ptr -> generated output code buffer
char *cpos;                       // ptr -> current code buffer location
char chr_table[256];              // character-types lookup table
char *numargsptr;                 // number of arguements to IF ptr
unsigned int scriptofstbl[1024];  // script offset table

int numscripts=0;                 // number of scripts in the VC file
int lines=1;

// Compilation-state flags
char inevent=0,iex=0;
char *scripttoken;
int funcidx;
char numlabels=0,numgotos=0;

// ================================ Code =================================

err(char *str)
{ FILE *f;

         if (!quiet) printf("%s (%d) \n",str,lines);
         if (quiet)
         {    f=fopen("ERROR.TXT","w");
              fprintf(f,"%s (%d)\n",str,lines);
              fclose(f);
         }
         remove("$$TMEP$$.MA_");
         exit(-1);
}

char TokenIs(char *str)
{
         if (!strcmp(str,token)) return 1;
         else return 0;
}

ParseWhitespace ()
{ char c;

  // ParseWhitespace() does what you'd expect - sifts through any white
  // space and advances the file pointer accordingly. Additionally, it
  // handles // style comments as well as /* and */ comments.

         while (1)
         { while (*src <= ' ')
                 { if (!*src) return;                  // EOF reached
                   if (*src == '\n') lines++;
                   src++;
                 }

           if (src[0] == '/' && src[1] == '/')         // Skip // comments
                 { while (*src && (*src != '\n'))
                         src++;                        // Skip to next line
                   continue;
                 }

           if (src[0] == '/' && src[1] == '*')         // Skip /* until */
                 { while (!(src[0] == '*' && src[1] == '/'))
                         { if (*src == '\n') lines++;
                           src++;
                           if (!*src) return;
                         }
                   src += 2;
                   continue;
                 }

         break;                                        // end of whitespace
         }
}

CheckLibFunc()
{ int i;

  // If the current token is a recognized library function, sets
  // token_type to FUNCTION and funcidx to the appropriate function code.

  // Todo: Maybe replace this with a binary tree instead of a linear
  // search. It hasn't gotten slow enough yet tho. :)

         token_nvalue = 0;

         if (strlen(token) == 1 && token[0]>64 && token[0]<91)
            {     token_type = IDENTIFIER;
                  token_nvalue = token[0]-64;
                  return; }

         if (TokenIs("FLAGS") || TokenIs("IF") || TokenIs("FOR") ||
             TokenIs("WHILE") || TokenIs("SWITCH") || TokenIs("CASE") ||
             TokenIs("GOTO"))
            {     token_type = RESERVED;
                  return;
            }
         if (TokenIs("AND"))
            {     token_type = CONTROL;
                  token[0] = '&'; token[1] = '&'; token[2] = 0;
                  return;
            }
         i = 0;
         while (i < numfuncs)
         {     if (!strcmp(funcs[i], token)) break;
               i++;
         }
         if (i != numfuncs) token_type = FUNCTION;
         funcidx = i;
}

unsigned char SearchVarList ()
{ unsigned char i;

         i = 0;
         while (i < numvars0)
         {     if (!strcmp(vars0[i], token)) break;
               i++;
         }
         if (i != numvars0)
         {
            token_type = RESERVED;
            token_subtype = VAR0;
            return i;
         }

         i = 0;
         while (i < numvars1)
         {     if (!strcmp(vars1[i], token)) break;
               i++;
         }
         if (i != numvars1)
         {
            token_type = RESERVED;
            token_subtype = VAR1;
            return i;
         }

         i = 0;
         while (i < numvars2)
         {     if (!strcmp(vars2[i], token)) break;
               i++;
         }
         if (i != numvars2)
         {
            token_type = RESERVED;
            token_subtype = VAR2;
            return i;
         }
         return i;
}

GetIdentifier ()
{ int i;

  // Retrieves an identifier from the source buffer. Before calling this,
  // it should be guaranteed that the first character is a letter. A check
  // will need to be made afterward to see if it's a reserved word.

         i = 0;
         while ((chr_table[*src] == LETTER) || (chr_table[*src] == DIGIT))
         {       token[i] = *src;
                 src++;
                 i++;
         }
         token[i] = 0;
         strupr(&token);
         CheckLibFunc();
         SearchVarList();
}

GetNumber()
{ int i;

  // Grabs the next number. String version remains in token[], numerical
  // version is placed in token_nvalue.

         i = 0;
         while (chr_table[*src] == DIGIT)
         {      token[i] = *src;
                src++;
                i++;
         }
         token[i] = 0;
         token_nvalue = atoi (&token);
}

GetPunctuation()
{ char c;

  // Grabs the next recognized punctuation type. If a double-char punctuation
  // type is recognized, it will be returned. Ie, differentiate b/w = and ==.

         c = *src;
         switch (c)
         {      case '(': token[0] = '('; token[1] = 0; src ++; break;
                case ')': token[0] = ')'; token[1] = 0; src ++; break;
                case '{': token[0] = '{'; token[1] = 0; src ++; break;
                case '}': token[0] = '}'; token[1] = 0; src ++; break;
                case '[': token[0] = '('; token[1] = 0; src ++; break;
                case ']': token[0] = ')'; token[1] = 0; src ++; break;
                case ',': token[0] = ','; token[1] = 0; src ++; break;
                case ':': token[0] = ':'; token[1] = 0; src ++; break;
                case ';': token[0] = ';'; token[1] = 0; src ++; break;
                case '/': token[0] = '/'; token[1] = 0; src ++; break;
                case '*': token[0] = '*'; token[1] = 0; src ++; break;
                case '%': token[0] = '%'; token[1] = 0; src ++; break;
                case '\"': token[0] = '\"'; token[1] = 0; src ++; break;
                case '\'': token[0] = '\''; token[1] = 0; src ++; break;
                case '+' : token[0] = '+';           // Is it ++ or += or +?
                           src++;
                           if (*src == '+')
                              { token[1] = '+';
                                src++; }
                           else if (*src == '=')
                                   { token[1] = '=';
                                     src++; }
                           else token[1] = 0;
                           token[2] = 0;
                           break;
                case '-' : token[0] = '-';           // Is it -- or -= or -?
                           src++;
                           if (*src == '-')
                              { token[1] = '-';
                                src++; }
                           else if (*src == '=')
                                   { token[1] = '=';
                                     src++; }
                           else token[1] = 0;
                           token[2] = 0;
                           break;
                case '>': token[0] = '>';            // Is it > or >=?
                          src++;
                          if (*src == '=')           // It's >=
                             { token[1] = '=';
                               token[2] = 0;
                               src++; break;
                             }
                          token[1] = 0;              // It's >
                          break;
                case '<': token[0] = '<';            // Is it < or <=?
                          src++;
                          if (*src == '=')           // It's <=
                             { token[1] = '=';
                               token[2] = 0;
                               src++; break;
                             }
                          token[1] = 0;              // It's <
                          break;
                case '!': token[0] = '!';
                          src++;                 // Is it just ! or is it != ?
                          if (*src == '=')           // It's !=
                             { token[1] = '=';
                               token[2] = 0;
                               src++; break;
                             }
                          token[1] = 0;              // It's just !
                          break;
                case '=': token[0] = '=';
                          src++;                     // is = or == ?
                          if (*src == '=')           // Doesn't really matter,
                             { token[1] = 0;         // just skip the last byte
                               src ++;
                             }
                          else token[1] = 0;
                          break;
                case '&': token[0] = '&'; token[1] = '&'; token[2] = 0;
                          src += 2; break;
                default: src ++;              // This should be an error.
         }
}

GetString ()
{ int i;

  // Expects a "quoted" string. Places the contents of the string in
  // token[] but does not include the quotes.

         Expect("\"");
         i = 0;
         while (*src != '\"')
         {     token[i] = *src;
               src ++;
               i ++;
         }
         src ++;
         token[i] = 0;
}

GetToken ()
{ int i;
  char c;

         // simply reads in the next statement and places it in the
         // token buffer.

         ParseWhitespace();
         i = 0;
         switch (chr_table[*src])
         {  case  LETTER: { token_type = IDENTIFIER; GetIdentifier(); break; }
            case   DIGIT: { token_type = DIGIT; GetNumber(); break; }
            case SPECIAL: { token_type = CONTROL; GetPunctuation(); break; }
         }

         if (!*src && inevent) err("Unexpected end of file");
}

char NextIs(char *str)
{ char *ptr,tt,tst,i;
  int olines,nv;

         ptr = src;
         olines = lines;
         tt = token_type;
         tst = token_subtype;
         nv = token_nvalue;
         memcpy(&lasttoken, &token, 2048);
         GetToken();
         src = ptr;
         lines = olines;
         token_nvalue = nv;
         tst = token_subtype;
         tt = token_type;
         if (!strcmp(str,token)) i=1; else i=0;
         memcpy(&token, &lasttoken, 2048);
         return i;
}

Expect(char *str)
{ FILE *f;

         GetToken ();
         if (!strcmp(str,token)) return;
         if (!quiet) printf ("error: %s expected, %s got (%d)", str, &token, lines);
         if (quiet)
         {    f=fopen("ERROR.TXT","w");
              fprintf(f,"error: %s expected, %s got (%d) \n", str, &token, lines);
              fclose(f);
         }
         exit (-1);
}

int ExpectNumber ()
{
         GetToken();
         if (token_type != DIGIT) err ("error: Numerical value expected");
         return token_nvalue;
}

EmitC (char c)
{
         *cpos = c;
         cpos ++;

}

EmitW (short int w)
{ char *ptr;

         ptr = (char *)&w;
         *cpos = *ptr;
         cpos ++;
         ptr ++;
         *cpos = *ptr;
         cpos ++;
}

EmitD (int w)
{ char *ptr;

         ptr = (char *)&w;
         *cpos = *ptr;
         cpos ++;
         ptr ++;
         *cpos = *ptr;
         cpos ++;
         ptr ++;
         *cpos = *ptr;
         cpos ++;
         ptr ++;
         *cpos = *ptr;
         cpos ++;
}

EmitString (char *str)
{ int i;

         i = 0;
         while (str[i])
         {
               *cpos = str[i];
               cpos ++;
               i ++;
         }
         *cpos = 0;
         cpos ++;
}

HandleOperand ()
{ unsigned char varidx;

         GetToken ();
         if (token_type == DIGIT)
         {
             EmitC (OP_IMMEDIATE);
             EmitD (token_nvalue);
         } else
         if (token_subtype == VAR0)
         {
             EmitC (OP_VAR0);
             varidx = SearchVarList ();
             if (varidx == numvars0) err ("error: Unknown identifier.");
             EmitC (varidx);
         } else
         if (token_subtype == VAR1)
         {
             EmitC (OP_VAR1);
             varidx = SearchVarList ();
             if (varidx == numvars1) err ("error: Unknown identifier.");
             EmitC (varidx);
             Expect ("(");
             EmitOperand ();
             Expect (")");
         } else
         if (token_subtype == VAR2)
         {
             EmitC (OP_VAR2);
             varidx = SearchVarList ();
             if (varidx == numvars2) err ("error: Unknown identifier.");
             EmitC (varidx);
             Expect ("(");
             EmitOperand ();
             Expect (",");
             EmitOperand ();
             Expect (")");
         }
}

EmitOperand ()
{
         while (1)                // Modifier-process loop.
         {
              if (NextIs("("))
              {
                 EmitC (OP_GROUP);
                 GetToken();
                 EmitOperand();
                 Expect(")");
              }
              else HandleOperand ();

              if (NextIs("+"))
              {
                  EmitC (ADD);
                  GetToken ();
                  continue;
              }
              else if (NextIs("-"))
              {
                  EmitC (SUB);
                  GetToken ();
                  continue;
              }
              else if (NextIs("/"))
              {
                  EmitC (DIV);
                  GetToken ();
                  continue;
              }
              else if (NextIs("*"))
              {
                  EmitC (MULT);
                  GetToken ();
                  continue;
              }
              else if (NextIs("%"))
              {
                  EmitC (MOD);
                  GetToken ();
                  continue;
              }
              else
              {
                  EmitC (OP_END);
                  break;
              }
         }
}

char HandleExpression()
{
  // Parses one single "expression". Can be anything short of a new script.

         GetToken ();
         if (token_type == FUNCTION)
            {     OutputCode (funcidx);
                  return 1;
            }
         if (token_type == RESERVED && TokenIs("IF"))
            {     ProcessIf ();
                  return 1;
            }
         if (token_type == RESERVED && TokenIs("FOR"))
            {     ProcessFor ();
                  return 1;
            }
         if (token_type == RESERVED && TokenIs("WHILE"))
            {     ProcessWhile();
                  return 1;
            }
         if (token_type == RESERVED && TokenIs("SWITCH"))
            {     ProcessSwitch();
                  return 1;
            }
         if (token_type == RESERVED && TokenIs("GOTO"))
            {     ProcessGoto();
                  return 1;
            }
         if (token_type == RESERVED && token_subtype == VAR0)
            {     ProcessVar0Assign ();
                  return 1;
            }
         if (token_type == RESERVED && token_subtype == VAR1)
            {     ProcessVar1Assign ();
                  return 1;
            }
         if (token_type == RESERVED && token_subtype == VAR2)
            {     ProcessVar2Assign ();
                  return 1;
            }
         if (NextIs(":"))
            {
              memcpy(labels[numlabels].ident,lasttoken,40);
              (int) labels[numlabels].pos=(int) cpos-(int) code;
              if (verbose) printf("label %s found on line %d, cpos: %d. \n",&lasttoken, lines, cpos-code);
              numlabels++;
              Expect (":");
              return 1;
            }
         return 0;                      // Not a valid command;
}

ProcessVar0Assign ()
{ int a;

         EmitC (VAR0_ASSIGN);
         a=SearchVarList();
         EmitC (a);
         if (!write0[a]) err("error: Variable is read-only.");

         GetToken ();                        // Find relational operator
         if (TokenIs("="))
             EmitC (SET);
         else if (TokenIs("+="))
                  EmitC (INCSET);
         else if (TokenIs("-="))
                  EmitC (DECSET);
         else if (TokenIs("++"))
                  { EmitC (INCREMENT);
                    Expect (";");
                    return; }
         else if (TokenIs("--"))
                  { EmitC (DECREMENT);
                    Expect (";");
                    return; }

         EmitOperand ();
         Expect (";");
}

ProcessVar1Assign ()
{ int a;

         EmitC (VAR1_ASSIGN);
         a=SearchVarList();
         EmitC (a);
         if (!write1[a]) err("error: Variable is read-only.");

         Expect ("(");
         EmitOperand ();
         Expect (")");

         GetToken ();                        // Find relational operator
         if (TokenIs("="))
             EmitC (SET);
         else if (TokenIs("+="))
                  EmitC (INCSET);
         else if (TokenIs("-="))
                  EmitC (DECSET);
         else if (TokenIs("++"))
                  { EmitC (INCREMENT);
                    Expect (";");
                    return; }
         else if (TokenIs("--"))
                  { EmitC (DECREMENT);
                    Expect (";");
                    return; }

         EmitOperand ();
         Expect (";");
}

ProcessVar2Assign ()
{ int a;

         EmitC (VAR2_ASSIGN);
         a=SearchVarList();
         EmitC (a);
         if (!write2[a]) err("error: Variable is read-only.");

         Expect ("(");
         EmitOperand ();
         Expect (",");
         EmitOperand ();
         Expect (")");

         GetToken ();                        // Find relational operator
         if (TokenIs("="))
             EmitC (SET);
         else if (TokenIs("+="))
                  EmitC (INCSET);
         else if (TokenIs("-="))
                  EmitC (DECSET);
         else if (TokenIs("++"))
                  { EmitC (INCREMENT);
                    Expect (";");
                    return; }
         else if (TokenIs("--"))
                  { EmitC (DECREMENT);
                    Expect (";");
                    return; }

         EmitOperand ();
         Expect (";");
}

ProcessIf ()
{ unsigned char numargs=0, excl=0, varidx;
  char *returnptr, *buf;

  // The general opcode form of an IF is:
  // <BYTE: GENERAL_IF>
  // <BYTE: number of arguements to IF>
  // <DWORD: ptr -> execution branch of IF evaluates false>
  // <1 .. number of arguements: Argument descriptions>

         EmitC (GENERAL_IF);
         Expect ("(");
         numargsptr = cpos;
         EmitC (0);               // We'll come back and fix this. <numargs>
         returnptr = cpos;
         EmitD (0);               // This too.                     <elseofs>

  // Here we begin the loop to write out IF arguements.

         while (1)
         {
              numargs ++;
              excl = 0;

              if (NextIs("!"))
              {
                  excl = 1;
                  GetToken ();
              }

              EmitOperand ();                     // First Operand

 // Now we need to output the conditional operator, which is a bit more
 // complicated by the possibility of zero/nonzero styles.

              if (excl)
                  EmitC (ZERO);

              GetToken ();
              if (TokenIs("&&") || TokenIs(")"))
              {
                  if (!excl) EmitC (NONZERO);
                  if (TokenIs("&&")) continue;
                  break;
              }
              if (TokenIs("="))
                 EmitC (EQUALTO);
              else if (TokenIs("!="))
                      EmitC (NOTEQUAL);
              else if (TokenIs(">"))
                      EmitC (GREATERTHAN);
              else if (TokenIs(">="))
                      EmitC (GREATERTHANOREQUAL);
              else if (TokenIs("<"))
                      EmitC (LESSTHAN);
              else if (TokenIs("<="))
                      EmitC (LESSTHANOREQUAL);
              else err ("error: Unknown IF relational operator");

              EmitOperand ();        // Emit the second operand if applicable

              GetToken ();           // See if there are more arguements
              if (TokenIs("&&")) continue;
              if (TokenIs(")")) break;

              err ("error: &&, AND, or ) expected");
         }

  // Now that we've parsed the conditional arguements of the IF, go back to
  // the IF header and set the correct number of arguements.

         buf = cpos;
         cpos = numargsptr;
         EmitC (numargs);
         cpos = buf;

         if (NextIs("{"))                     // It's a compound statement.
         {
             Expect ("{");
             while (HandleExpression());
             if (!TokenIs("}")) err ("error: } expected, or unknown identifier");
             buf = cpos;
             cpos = returnptr;
             EmitD (buf - code);
             cpos = buf;
             return;
         }
         else                               // Just a single statement.
         {
             HandleExpression();
             buf = cpos;
             cpos = returnptr;
             EmitD (buf - code);
             cpos = buf;
         }
}

ProcessFor0 ()
{
         EmitC (FOR_LOOP0);
         EmitC (SearchVarList());

         Expect (",");
         EmitOperand ();
         Expect (",");
         EmitOperand ();
         Expect (",");
         if (NextIs("-"))
         {
             EmitC (0);
             GetToken ();
         }
         else EmitC (1);
         EmitOperand ();
         Expect (")");

         Expect ("{");
         while (HandleExpression());
         if (!TokenIs("}")) err ("error: } expected, or unknown identifier");
         EmitC (ENDSCRIPT);
}

ProcessFor1 ()
{
         EmitC (FOR_LOOP1);
         EmitC (SearchVarList());
         Expect("(");
         EmitOperand ();
         Expect(")");

         Expect (",");
         EmitOperand ();
         Expect (",");
         EmitOperand ();
         Expect (",");
         if (NextIs("-"))
         {
             EmitC (0);
             GetToken ();
         }
         else EmitC (1);
         EmitOperand ();
         Expect (")");

         Expect ("{");
         while (HandleExpression());
         if (!TokenIs("}")) err ("error: } expected, or unknown identifier");
         EmitC (ENDSCRIPT);
}

ProcessFor ()
{
         Expect ("(");
         GetToken ();
         if (token_subtype==VAR0) ProcessFor0();
         else if (token_subtype==VAR1) ProcessFor1();
         else err ("Parse error in FOR loop.");
}

ProcessWhile ()
{ unsigned char numargs=0, excl=0, varidx;
  char *buf, *start, *returnptr;

 // The WHILE statement is actually pretty easy to do. It basically has an IF
 // header, and then at the bottom of the IF processing, is a GOTO back to the
 // top. So essentially it will continuously loop until the IF is false.
 // It in fact uses an IF opcode.

         start=cpos;
         EmitC (GENERAL_IF);
         Expect ("(");
         numargsptr = cpos;
         EmitC (0);               // We'll come back and fix this. <numargs>
         returnptr = cpos;
         EmitD (0);               // This too.                     <elseofs>

  // Here we begin the loop to write out WHILE arguements.

         while (1)
         {
              numargs ++;
              excl = 0;

              if (NextIs("!"))
              {
                  excl = 1;
                  GetToken ();
              }

              EmitOperand ();                     // First Operand

 // Now we need to output the conditional operator, which is a bit more
 // complicated by the possibility of zero/nonzero styles.

              if (excl)
                  EmitC (ZERO);

              GetToken ();
              if (TokenIs("&&") || TokenIs(")"))
              {
                  if (!excl) EmitC (NONZERO);
                  if (TokenIs("&&")) continue;
                  break;
              }
              if (TokenIs("="))
                 EmitC (EQUALTO);
              else if (TokenIs("!="))
                      EmitC (NOTEQUAL);
              else if (TokenIs(">"))
                      EmitC (GREATERTHAN);
              else if (TokenIs(">="))
                      EmitC (GREATERTHANOREQUAL);
              else if (TokenIs("<"))
                      EmitC (LESSTHAN);
              else if (TokenIs("<="))
                      EmitC (LESSTHANOREQUAL);
              else err ("error: Unknown IF relational operator");

              EmitOperand ();        // Emit the second operand if applicable

              GetToken ();           // See if there are more arguements
              if (TokenIs("&&")) continue;
              if (TokenIs(")")) break;

              err ("error: &&, AND, or ) expected");
         }

  // Now that we've parsed the conditional arguements of the WHILE, go back to
  // the WHILE header and set the correct number of arguements.

         buf = cpos;
         cpos = numargsptr;
         EmitC (numargs);
         cpos = buf;

         if (NextIs("{"))                     // It's a compound statement.
         {
             Expect ("{");
             while (HandleExpression());
             if (!TokenIs("}")) err ("error: } expected, or unknown identifier");
             EmitC (GOTO);
             EmitD (start - code);
             buf = cpos;
             cpos = returnptr;
             EmitD (buf - code);
             cpos = buf;
             return;
         }
         else                               // Just a single statement.
         {
             HandleExpression();
             EmitC (GOTO);
             EmitD (start - code);
             buf = cpos;
             cpos = returnptr;
             EmitD (buf - code);
             cpos = buf;
         }
}

ProcessSwitch ()
{ char *buf,*retrptr;

  // Special thanks for Zeromus for giving me a good idea on how to implement
  // this... Even tho I changed his idea around a bit :)

         EmitC (SWITCH);
         Expect ("(");
         EmitOperand ();
         Expect (")");
         Expect ("{");

  // case .. option loop

         while (!NextIs("}"))
         {
              Expect ("CASE");
              EmitC (CASE);
              EmitOperand ();
              Expect (":");
              retrptr=cpos;
              EmitD (0);
              while (!NextIs("CASE") && !NextIs("}")) HandleExpression ();
              EmitC (ENDSCRIPT);
              buf=cpos;
              cpos=retrptr;
              EmitD ((int) buf - (int) code);
              cpos=buf;
         }
         Expect ("}");
         EmitC (ENDSCRIPT);
}

ProcessGoto ()
{
         EmitC (GOTO);
         GetToken ();
         memcpy (&gotos[numgotos].ident, &token, 40);
         if (verbose) printf("GOTO tagged on line %d at cpos %d, label %s. \n", lines, cpos-code, &token);
         gotos[numgotos].pos=cpos;
         EmitD (0);
         numgotos++;
         Expect (";");
}

ProcessEvent ()
{
         Expect ("{");
         inevent = 1;
         scriptofstbl[numscripts] = (cpos-code);
         numscripts++;

         while (1)
         {
            GetToken ();
            if (token_type == CONTROL)
               { if (!TokenIs("}"))
                    err("error: Identifier expected");
                 else
                    { EmitC(ENDSCRIPT);            // End of script
                      inevent = 0;
                      return;
                    }
               }

            if (token_type == RESERVED)
               {    if (TokenIs("IF"))
                       {     ProcessIf ();
                             continue;
                       }
                    if (TokenIs("FOR"))
                       {     ProcessFor ();
                             continue;
                       }
                    if (TokenIs("WHILE"))
                       {     ProcessWhile ();
                             continue;
                       }
                    if (TokenIs("SWITCH"))
                       {     ProcessSwitch();
                             continue;
                       }
                    if (TokenIs("GOTO"))
                       {     ProcessGoto();
                             continue;
                       }
               }
            if (token_type == RESERVED && token_subtype == VAR0)
                {     ProcessVar0Assign ();
                      continue;
                }
            if (token_type == RESERVED && token_subtype == VAR1)
                {     ProcessVar1Assign ();
                      continue;
                }
            if (token_type == RESERVED && token_subtype == VAR2)
                {     ProcessVar2Assign ();
                      continue;
                }
            if (token_type != FUNCTION && NextIs(":"))
            {
              memcpy(labels[numlabels].ident,lasttoken,40);
              (int) labels[numlabels].pos=(int) cpos-(int) code;
              if (verbose) printf("label %s found on line %d, cpos: %d. \n",&lasttoken, lines, cpos-code);
              numlabels++;
              Expect (":");
              continue;
            }
            if (token_type != FUNCTION) err("error: Function-identifier expected.");
            OutputCode (funcidx);
         }
}

char *GetLabelAddr (char *str)
{ int i;

         for (i=0; i<numlabels; i++)
            if (!strcmp(str,labels[i].ident)) return labels[i].pos;

         sprintf (token,"Undefined label %s.",str);
         err (token);
}

ResolveGotos ()
{ char *a,*ocp;
  int i;

         ocp=cpos;
         for (i=0; i<numgotos; i++)
         {
            a=GetLabelAddr(gotos[i].ident);
            if (verbose) printf("resolving goto %d (%s) as %d at cpos %d. \n",i,gotos[i].ident,a,gotos[i].pos-code);
            cpos=gotos[i].pos;
            EmitD ((int) a);
         }
         cpos=ocp;
}

char Parse ()
{ char c;

  // Eh.. Parse doesn't really do hardly anything. It's just the script loop.

        token[0] = 0;             // clear token-buffer
        ParseWhitespace();        // Sift through any whitespace.
        if (!*src) return 1;      // EOF
        GetToken ();              // Grab next token

        if (TokenIs(scripttoken)) ProcessEvent ();
        ParseWhitespace();
        if (!*src) return 1;
        if (!NextIs(scripttoken) && !iex)
        {
           if (!quiet) printf ("warning: Unknown token outside scripts (%d) \n",lines);
           iex=1;
        }
        return 0;
}

Compile ()
{
        src = source;
        cpos = code;

        if (effect) scripttoken="EFFECT";
        else if (scrpt) scripttoken="SCRIPT";
        else if (magic) scripttoken="EFFECT";
               else scripttoken="EVENT";

        while (!Parse());

        ResolveGotos ();

        if (!quiet)
           printf ("%d scripts successfully compiled. (%d lines) \n", numscripts,lines);
}

InitCompileSystem ()
{ int i;

        if (verbose) printf("Building chr_table[]. \n");
        for (i = 0; i < 256; i++) chr_table[i] = SPECIAL;
        for (i = '0'; i <= '9'; i++) chr_table[i] = DIGIT;
        for (i = 'A'; i <= 'Z'; i++) chr_table[i] = LETTER;
        for (i = 'a'; i <= 'z'; i++) chr_table[i] = LETTER;

        chr_table[10] = 0;
        chr_table[13] = 0;
        chr_table[' '] = 0;
        chr_table['_'] = LETTER;
        chr_table['.'] = LETTER;
}
