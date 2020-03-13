//preproc.c by Reverend Matthew B. Gambrell

#include <io.h>
#include <fcntl.h>
#include <stdio.h>
#include "vcc.h"
#define LETTER 1
#define DIGIT 2
#define SPECIAL 3

char* TempPtr;
char* DataPtr;
char chr_tablebeta[256];
char tokenbeta[512];
char ParamArray[100][4][40];
int NumIdentifiers;

void ParseWhiteSpaceBeta ()
{ char c;

        if(chr_tablebeta[*DataPtr]==LETTER||chr_tablebeta[*DataPtr]==DIGIT||*DataPtr=='@') return;
                while (1)
         {
                         while (*DataPtr <= ' ')
                 { if (!*DataPtr) return;                  // EOF reached
                   DataPtr++;
                 }

           if (DataPtr[0] == '/' && DataPtr[1] == '/')         // Skip // comments
                 { while (*DataPtr && (*DataPtr != '\n'))
                         DataPtr++;                        // Skip to next line
                   continue;
                 }

           if (DataPtr[0] == '/' && DataPtr[1] == '*')         // Skip /* until */
                 { while (!(DataPtr[0] == '*' && DataPtr[1] == '/'))
                         {
                           DataPtr++;
                           if (!*DataPtr) return;
                         }
                   DataPtr += 2;
                   continue;
                 }

         break;                                        // end of whitespace
         }
}

void InitCharTableBeta()
{ int i;

        if (verbose) printf("Building chr_tablebeta[] for preprocessor. \n");
        for (i = 0; i < 256; i++) chr_tablebeta[i] = SPECIAL;
        for (i = '0'; i <= '9'; i++) chr_tablebeta[i] = DIGIT;
        for (i = 'A'; i <= 'Z'; i++) chr_tablebeta[i] = LETTER;
        for (i = 'a'; i <= 'z'; i++) chr_tablebeta[i] = LETTER;

        chr_tablebeta[10] = 0;
        chr_tablebeta[13] = 0;
        chr_tablebeta[' '] = 0;
        chr_tablebeta['_'] = LETTER;
}


void GetIdentifierBeta ()
{ int i;

         memset(tokenbeta,0,41);
                i = 0;
                while ((chr_tablebeta[*DataPtr] == LETTER) || (chr_tablebeta[*DataPtr] == DIGIT))
         {
                        tokenbeta[i] = *DataPtr;
            DataPtr++;
            i++;
         }
         tokenbeta[i] = 0;
}

void GetIdentifierDelta ()
{ int i;
char FoundFirstChar;

         memset(tokenbeta,0,41);
                 FoundFirstChar=0;
                i = 0;

                while (*DataPtr!='@'&&*DataPtr!=','&&*DataPtr!=')')
         {
                        if((chr_tablebeta[*DataPtr]==LETTER || chr_tablebeta[*DataPtr]==DIGIT)||FoundFirstChar==1)
                        {
                                FoundFirstChar=1;
                                tokenbeta[i] = *DataPtr;
                                i++;
                        }
                        DataPtr++;
         }
         tokenbeta[i] = 0;
                 if(*DataPtr==')') DataPtr--;
}


void GetIdentifierGamma ()
{ int i;

                memset(tokenbeta,0,512);
                i = 0;
        DataPtr--;
                while(*DataPtr!='('&&*DataPtr!='@')
                {
                        ParseWhiteSpaceBeta();
                        DataPtr++;
                }
                if(*DataPtr=='@') goto NoParams;
                DataPtr++;
                while(*DataPtr!=')')
                {
                        GetIdentifierDelta();
                        memcpy(&ParamArray[NumIdentifiers-1][i][0],tokenbeta,40);
                        i++;
                        DataPtr++;
                }
                while(*DataPtr!='@')
                {
                        ParseWhiteSpaceBeta();
                        DataPtr++;
                }
NoParams:
                DataPtr++;
                i=0;
                while (*DataPtr!='@')
                {
                        tokenbeta[i] = *DataPtr;
            DataPtr++;
                        i++;
         }
         tokenbeta[i] = 0;
                 DataPtr++;
}


void FirstPass()
{
        FILE* inf;  //all purpose input file
        FILE* outf;  //all purpose output file
        char IdentifierArray[100][40]; //list of macro names
        char IncludeString[512]; //filename in #include directives
        char errstring[512]; //used to construct an error string
        char ExpansionArray[100][512]; //macro expansion code
/*The 512 element is the max number of bytes of expansion code.
  a bit too constrictive?  Maybe increase it.*/
        char ch;  //byte variable used for file copying
        char TempParam[4][40]; //param list used for macro expansion
        char* IncludeSpot; //keeps track of the latest byte before an
                                           //#include directive
        char* WorkingSpot; //misc ptr used in #include code
        char* FinIncSpot;  //ptr to the end of the current actual
                                           //#include text i nthe src file
        char tc[41];
         char tc2[512];
         char gross[50];
         char* MagicNumber;
         int x,z,i,bob;
         int x2,z2;
         char* IRemember;
         char SehrGut;

     //PostStartupFilesBeta();
         memset(ParamArray,0,sizeof(ParamArray));
     DataPtr=source;
         NumIdentifiers=0;
     InitCharTableBeta();
         MagicNumber=DataPtr;
         IncludeSpot=source;
         outf=fopen("$$tmep$$.ma_","wb");
         while(*DataPtr!=0)
         {
                 fwrite(DataPtr,1,1,outf);
                 DataPtr++;
         }
         fclose(outf);
         DataPtr=source;
         while(*DataPtr!=0)
     {
                 if(*DataPtr=='#')
          {
                                DataPtr++;
                                if(*DataPtr=='D'||*DataPtr=='d')
                                {
                                DataPtr++;
                                if(*DataPtr=='E'||*DataPtr=='e')
                                {
                                DataPtr++;
                                if(*DataPtr=='F'||*DataPtr=='f')
                                {
                                DataPtr++;
                                if(*DataPtr=='I'||*DataPtr=='i')
                                {
                                DataPtr++;
                                if(*DataPtr=='N'||*DataPtr=='n')
                                {
                                DataPtr++;
                                if(*DataPtr=='E'||*DataPtr=='e')
                                {
                                        DataPtr++;
                    NumIdentifiers++;
                                        ParseWhiteSpaceBeta();
                                        GetIdentifierBeta();
                                        memcpy(&IdentifierArray[NumIdentifiers-1][0],tokenbeta,40);
                                        if(verbose) printf("Found macro identifier %s\n",IdentifierArray[NumIdentifiers-1]);
                                        GetIdentifierGamma();
                                        memcpy(&ExpansionArray[NumIdentifiers-1][0],tokenbeta,512);
                                        if(verbose) printf("Expands to: %s\n",ExpansionArray[NumIdentifiers-1]);
                                        if(verbose) printf("params: %s%s%s%s\n",ParamArray[NumIdentifiers-1][0],ParamArray[NumIdentifiers-1][1],ParamArray[NumIdentifiers-1][2],ParamArray[NumIdentifiers-1][3]);
                                        MagicNumber=DataPtr;
                                }
                                }
                                }
                                }
                                }
                                }
                                else
                                if(*DataPtr=='I'||*DataPtr=='i')
                                {
                                DataPtr++;
                                if(*DataPtr=='N'||*DataPtr=='n')
                                {
                                DataPtr++;
                                if(*DataPtr=='C'||*DataPtr=='c')
                                {
                                DataPtr++;
                                if(*DataPtr=='L'||*DataPtr=='l')
                                {
                                DataPtr++;
                                if(*DataPtr=='U'||*DataPtr=='u')
                                {
                                DataPtr++;
                                if(*DataPtr=='D'||*DataPtr=='d')
                                {
                                DataPtr++;
                                if(*DataPtr=='E'||*DataPtr=='e')
                                {
                                        DataPtr++;
                                        ParseWhiteSpaceBeta();
                                        x=0;
                                        while(chr_tablebeta[*DataPtr]==LETTER||chr_tablebeta[*DataPtr]==DIGIT||*DataPtr=='.')
                                        {
                                                IncludeString[x]=*DataPtr;
                                                x++;
                                                DataPtr++;
                                        }
                                        IncludeString[x]=0;
                                        if (!quiet) printf("Including file %s\n",IncludeString);
                                        if(!(inf=fopen(IncludeString,"rb")))
                                        {
                                                sprintf(errstring,"Include File %s not found",IncludeString);
                                                err(errstring);
                                        }
                                        outf=fopen("$$tmep$$.ma_","wb");
                                        FinIncSpot=DataPtr;
                                        WorkingSpot=source;
                                        while(WorkingSpot!=IncludeSpot)
                                        {
                                                fwrite(WorkingSpot,1,1,outf);
                                                WorkingSpot++;
                                        }
                                        while(fread(&ch,1,1,inf)!=0)
                                        {
                                                fwrite(&ch,1,1,outf);
                                        //      WorkingSpot++;
                                        }
                                WorkingSpot=FinIncSpot;
                                        while(*WorkingSpot!=0)
                                        {
                                                fwrite(WorkingSpot,1,1,outf);
                                                WorkingSpot++;
                                        }
                                        fclose(inf);
                                        fclose(outf);
                                        PostStartupFilesBeta();
                                        DataPtr=source-1;
                                        memset(IdentifierArray,0,sizeof(IdentifierArray));
                                        memset(ExpansionArray,0,sizeof(ExpansionArray));
                                        NumIdentifiers=0;
                                        memset(ParamArray,0,sizeof(ParamArray));


                                }
                                }
                                }
                                }
                                }
                                }
                                }
                 }
         DataPtr++;
         IncludeSpot=DataPtr;
         }
         //now the tables of identifiers/expansions are set up.
         //next we tear through the file finding/replacing
    PostStartupFilesBeta();
         if (verbose) printf("Expanding macros...\n");

         outf=fopen("$$tmep$$.ma_","wb");

         DataPtr=MagicNumber;
         while(*DataPtr!=0)
         {
                IRemember=DataPtr;
                z=0;
                while(chr_tablebeta[*DataPtr]==LETTER || chr_tablebeta[*DataPtr]==DIGIT)
                {
                        tc[z]=*DataPtr;
                        z++;
                        DataPtr++;
                }
                tc[z]=0;
                SehrGut=0;
                for(x=0; x<NumIdentifiers; x++)
                        if(strcmp(tc,IdentifierArray[x])==0)
                        {
                                TempPtr=ExpansionArray[x];
                                i=0;
                                DataPtr++;
                                while(*DataPtr!=')')
                                {
                                        GetIdentifierDelta();
                                        memcpy(&TempParam[i][0],tokenbeta,40);
                                        i++;
                                        DataPtr++;
                                }
                                tc2[0]=0;
                                i=0;
                                while(*TempPtr!=0)
                                {
                                        if(chr_tablebeta[*TempPtr]==LETTER || chr_tablebeta[*TempPtr]==DIGIT)
                                        {
                                                tc2[i]=*TempPtr;
                                                i++;
                                                tc2[i]=0;
                                        }
                                        else
                                        {
                                                fwrite(tc2,strlen(tc2),1,outf);
                                                fwrite(TempPtr,1,1,outf);
                                                i=0;
                                                tc2[0]=0;
                                        }
                                        for(x2=0; x2<4; x2++)
                                        {
                                                if(strcmp(tc2,ParamArray[x][x2])==0&&tc2[0]!=0)
                                                {
                                                        fwrite(TempParam[x2],strlen(TempParam[x2]),1,outf);
                                                        i=0;
                                                        tc2[i]=0;
                                                }
                                        }
                                        TempPtr++;
                                }
                                SehrGut=1;
                                DataPtr;
                        }
                if(SehrGut==0)
                {
                        fwrite(IRemember,1,1,outf);
                        DataPtr=IRemember;
                }
                DataPtr++;
         }
         fclose(outf);
         if (verbose) printf("Preprocessing completed successfully\n");
         if (verbose) printf("Compiling scripts... \n");
}

PostStartupFilesBeta()
{
FILE* inf;
        if (!( inf = fopen ("$$tmep$$.ma_", "rb")))
            { if (!quiet) printf ("*error* Could not open input file. \n");
              exit (-1); }
        memset (source, 0, 100000);
         fread (source, 1, 100000, inf);
                  fclose (inf);
}
