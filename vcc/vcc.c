
// ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
// ³                                                                    ³
// ³                     The Verge-C Compiler v.0.10                    ³
// ³                     Copyright (C)1997 BJ Eirich                    ³
// ³                                                                    ³
// ³ Module: VCC.C                                                      ³
// ³                                                                    ³
// ³ Description: Handles setup, command line parsing, determines       ³
// ³ compilation target / mode, other misc things.                      ³
// ³                                                                    ³
// ³ Portability: ANSI C - should compile on any compiler.              ³
// ³                                                                    ³
// ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ

#include <malloc.h>
#include <string.h>

#include <stdio.h>
#include "compile.h"
#define LETTER 1
#define DIGIT 2
#define SPECIAL 3

// ============================== Variables ==============================

char quiet, verbose;                  // output modes for VCC
char *strbuf, fname[100];             // string buffers (mostly temporary)
FILE *inf, *outf;                     // input/output file handles.
char effect=0,scrpt=0;                // compiling events/effects/misc scripts

// NEW CODE
char magic=0;
// END NEW CODE

char *source;                         // ptr -> source file in memory.
char chr_tablebeta[256];
char tokenbeta[512];

extern void FirstPass();
extern void InitCompileSystem();
extern void Compile();

// ================================ Code =================================

PostStartupFiles()
{
         if (!( inf = fopen ("$$tmep$$.ma_", "rb")))
            { if (!quiet) printf ("*error* Could not open input file. \n");
              exit (-1); }

         if (verbose) printf ("Reading source file into memory... \n");
         memset (source, 0, 100000);
         memset (code, 0, 100000);
         fread (source, 1, 100000, inf);
         fclose (inf);
}

PreStartupFiles ()
{ char i;

         // Open input and output files; make sure filenames are correct.

         i = strlen (strbuf);
         strbuf[i] = '.';
         strbuf[i+1] = 'v';
         strbuf[i+2] = 'c';
         strbuf[i+3] = 0;

         if (!( inf = fopen (strbuf, "rb")))
            { if (!quiet) printf ("*error* Could not open input file. \n");
              exit (-1); }

         if (verbose) printf ("Reading source file into memory... \n");
         source = (char *) malloc (100000);
         code = (char *) malloc (100000);
         memset (source, 0, 100000);
         memset (code, 0, 100000);
         fread (source, 1, 100000, inf);
         fclose (inf);
}

// NEW CODE
WriteMagicOutput ()
{ FILE *f;

         f=fopen("MAGIC.VCS","wb");
         fwrite (&numscripts, 1, 4, f);
         fwrite (&scriptofstbl, 4, numscripts, f);
         fwrite (code, 1, (cpos-code), f);
         fclose (f);

         remove("ERROR.TXT");
         remove("$$TMEP$$.MA_");
}

// END NEW CODE

WriteEffectOutput ()
{ FILE *f;

         f=fopen("EFFECTS.VCS","wb");
         fwrite (&numscripts, 1, 4, f);
         fwrite (&scriptofstbl, 4, numscripts, f);
         fwrite (code, 1, (cpos-code), f);
         fclose (f);

         remove("ERROR.TXT");
         remove("$$TMEP$$.MA_");
}

WriteScriptOutput ()
{ FILE *f;

         f=fopen("STARTUP.VCS","wb");
         fwrite (&numscripts, 1, 4, f);
         fwrite (&scriptofstbl, 4, numscripts, f);
         fwrite (code, 1, (cpos-code), f);
         fclose (f);

         remove("ERROR.TXT");
         remove("$$TMEP$$.MA_");
}

WriteOutput ()
{ FILE *f;
  char i;
  short int mx, my;
  int a;

         i = strlen (fname);
         memcpy (strbuf, &fname, i);
         strbuf [i] = '.';
         strbuf [i+1] = 'M';
         strbuf [i+2] = 'A';
         strbuf [i+3] = 'P';
         strbuf [i+4] = 0;

         f = fopen (strbuf, "rb+");
         fseek (f, 68, 0);
         fread (&mx, 1, 2, f);
         fread (&my, 1, 2, f);
         fseek (f, 100+(mx*my*5)+7956, 0);
         fread (&a, 1, 4, f);
         fseek (f, 88*a, 1);
         fread (&i, 1, 1, f);
         fread (&a, 1, 4, f);
         fseek (f, (i*4)+a, 1);

         fwrite (&numscripts, 1, 4, f);
         fwrite (&scriptofstbl, 4, numscripts, f);
         fwrite (code, 1, (cpos-code), f);
         fclose (f);

         remove("ERROR.TXT");
         remove("$$TMEP$$.MA_");
}

main (int argc, char *argv[])
{ char i;

         // Test the command line and set any necessary flags.

         strbuf = (char *) malloc (100);
         switch (argc)
         { case 1: { printf ("vcc v.04.Jun.98 Copyright (C)1997 BJ Eirich \n");
                     printf ("Usage: vcc <vc file> [flag] \n");
                     printf ("Where [flag] is one of the following: \n");
                     printf ("        q   Quiet mode - no output. \n");
                     printf ("        v   Super-Verbose mode - more output than you want. \n \n");
                     exit (-1);
                   }
           case 3: { strbuf = argv[2];
                     if (strbuf[0] == 'q')
                        quiet = 1;
                     if (strbuf[0] == 'v')
                        verbose = 1;
                   }
           case 2: { strbuf = argv[1];
                     for ( i=0; i<100; i++ )
                         fname[i] = strbuf[i];
                     strupr (fname);
                     if (!strcmp(fname,"EFFECTS")) effect=1;
// NEW CODE
                     if (!strcmp(fname,"MAGIC")) magic=1;
// END NEW CODE
                     if (!strcmp(fname,"STARTUP")) scrpt=1;
                     break;
                   }
           default: { printf ("vcc: Too many parameters. \n");
                      exit (-1);
                    }
         }

         if (!quiet) printf ("vcc v.04.Jun.98 Copyright (C)1997 BJ Eirich \n");
         PreStartupFiles();      // startup for preprocessing
         FirstPass();            // preprocessing
         PostStartupFiles();     // startup for postprocessing? no,
                                 // that doesn't make sense...
         InitCompileSystem ();
         Compile ();
         if (effect) WriteEffectOutput();
         else if (magic) WriteMagicOutput();
         else if (scrpt) WriteScriptOutput();
            else WriteOutput();
}
