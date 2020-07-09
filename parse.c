#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#define MAX_SYMBOL_TABLE_SIZE 1000
//used in parser and vm
#define CODE_SIZE 1000
#define MAX_LEXI_levelELS 3
//used in vm
#define MAX_STACK_HEIGHT 2000

/**variables**/

int lexoutput;
int codegenoutput;
int vmoutput;
int printErrorCount;
int cx, token, num, kind, lexemeListIndex=0, printErrorCount=0, difference, previousDifference=0;
char id[12];

//Given enum for internal representation
typedef enum {
    nulsym = 1, identsym, numbersym, plussym, minussym,
    multsym,  slashsym, oddsym, eqsym, neqsym, lessym, leqsym,
    gtrsym, geqsym, lparentsym, rparentsym, commasym, semicolonsym,
    periodsym, becomessym, beginsym, endsym, ifsym, thensym,
    whilesym, dosym, callsym, constsym, varsym, procsym, writesym,
    readsym , elsesym
} token_type;

//struct used to contain all tokens details
typedef struct {
    token_type tokenID;
    int numberValue;
    char name[12];
}tokenStruct;

tokenStruct lexList[5000];
int lexListIndex;

typedef struct {
     int op; //Opcode
     int l;  //L
     int m;  //M
 } instruction;

instruction code[CODE_SIZE];

typedef struct {
	int kind; 		// const = 1, var = 2, proc = 3
	char name[12];	// name up to 11 chars
	int val; 		// number (ASCII value)
	int levelel; 		// L levelel
	int addr; 		// M address
} symbol;


/**functions**/

void compile(void);

void lex(void);

void parse(void);

void machine(void);

void block(int level, int tex, symbol* table, FILE* inFile, instruction* code);

void constantDeclaration(int levelel, int *ptex, int *pdx, FILE* inFile, symbol* table);

void variableDeclaration(int level, int *ptex, int *pdx, FILE* inFile, symbol* table);

void discription(int level, int *ptex, FILE* inFile, instruction* code, symbol* table);

void condition(int level, int *ptex, FILE* inFile, instruction* code, symbol* table);

void expression(int level, int *ptex, FILE* inFile, instruction* code, symbol* table);

void term(int level, int *ptex, FILE* inFile, instruction* code, symbol* table);

void factor(int level, int *ptex, symbol* table, FILE* inFile, instruction* code);

void send(int op, int l, int m, instruction* code);
void enter(int k, int *ptex, int *pdx, int level, symbol* table);
void printError(int printErrorCase);

int NextToken(FILE* inFile);
int position(char *id, int *ptex, symbol* table, int level);
void program(FILE* inFile, symbol* table, instruction* code);

void printStackFrame(int* stack, int SP, int BP, FILE* outFile);

void executeCycle(instruction* IRstruct, int* stack, int* sp, int* bp, int* pc);

void OPR (int *sp, int* bp, int *pc, int *stack, instruction* IRstruct);

int base(int* bp, instruction* IRstruct, int* stack);

void usage(void);

const char *opcodes[] = {"ILLEGAL", "lit", "opr", "lod", "sto", "cal", "inc", "jmp", "jpc", "sio", "sio"};


//For Reserved Words
const char* reservedWords[]={"const", "var", "procedure", "call", "begin", "end", "if", "then", "else", "while", "do", "read", "write", "odd"};

//For Reserved Special Symbols
const char specialSymbols[]={'+', '-', '*', '/', '(', ')', '=', ',' , '.', '<', '>',  ';' , ':'};


tokenStruct lexList[5000];
int lexListIndex = 0;

//these are flags that will say whether or not we want to print something or not
int lexoutput;
int codegenoutput;
int vmoutput;

int c;
FILE* inFile;

//Given enum for internal representation
const char *lexSystem[] = { "", "nulsym", "identsym", "numbersym", "plussym", "minussym",
    "multsym",  "slashsym", "oddsym", "eqsym", "neqsym", "lessym", "leqsym",
    "gtrsym", "geqsym", "lparentsym", "rparentsym", "commasym", "semicolonsym",
    "periodsym", "becomessym", "beginsym", "endsym", "ifsym", "thensym",
    "whilesym", "dosym", "callsym", "constsym", "varsym", "procsym", "writesym",
    "readsym" , "elsesym"};




FILE* outFile2;
FILE* outFile3;

int main(int argc, char *argv[]) {
     
    //temporary variable to be used to check -l, -a, and -v
    char** temp=argv;

 
        while (argc > 1) {
            if (strcmp(temp[1], "-l") == 0) {
                lexoutput = 1;
            }
            if (strcmp(temp[1], "-a") == 0) {
                codegenoutput = 1;
            }
            if (strcmp(temp[1], "-v") == 0) {
                vmoutput = 1;
            }
            argc--;
            temp++;
        }

    compile();
}

void compile() {
    

    lex();
    

    parse();
    

    machine();
    

 
    if (lexoutput == 1) {

        printf("\n");
        inFile = fopen("lexoutput.txt", "r");
        while((c=fgetc(inFile)) !=EOF){
            printf("%c", c);
        }
        printf("\n\n");
        fclose(inFile);
        
        int i;
        inFile = fopen("lexoutput.txt", "r");
        for(i=0;i<lexListIndex;i++){
            printf("%s ", lexSystem[lexList[i].tokenID]);
            if(lexList[i].tokenID==2){
                printf("%s ", lexList[i].name);
            }
            else if(lexList[i].tokenID==3){
                printf("%d ",lexList[i].numberValue);
            }
        }
        printf("\n\n");
        fclose(inFile);
    }
    
    if (printErrorCount==0) {
        printf("No printErrors, program is syntactically correct\n\n");
    }

    if (codegenoutput == 1) {
        
        inFile = fopen("codegenoutput.txt", "r");
        while((c=fgetc(inFile)) !=EOF){
            printf("%c", c);
        }
        printf("\n");
        fclose(inFile);
    }

    if (vmoutput == 1) {
        
    
        inFile = fopen("vmout.txt", "r");
        while((c=fgetc(inFile)) !=EOF){
            printf("%c", c);
        }
        printf("\n");
        fclose(inFile);
    }

}


void parse(void) {

    FILE* inFile;
    FILE* outFile;

    inFile = fopen("lexoutput.txt", "r");
    outFile = fopen("parserout.txt", "w");
    
    int i;
    int level = 0; 
    int dx = 0; 
    int tex = 0; 

   
    symbol table[MAX_SYMBOL_TABLE_SIZE] = {0};

    
    instruction code[CODE_SIZE];

  
    program(inFile, table, code);

    for (i=0; i<cx;i++) {
        fprintf(outFile, "%d %d %d\n", code[i].op, code[i].l, code[i].m);
    }
    fclose(inFile);
    fclose(outFile);
}

void program(FILE* inFile, symbol* table, instruction* code) {

    token = NextToken(inFile);
    block(0,0, table, inFile, code);

    if (token!=periodsym) {
        printError(9);
    }
}

void block(int level, int tex, symbol* table, FILE* inFile, instruction* code) {
 
    if(level > MAX_LEXI_levelELS) {
        printError(26);
    }
    
    int dx, tex0, cx0;

    dx=4;
    tex0=tex;
    table[tex].addr=cx;
    send(7,0,0, code);

    do {
        if (token==constsym) {
            token = NextToken(inFile);
            do {
                constantDeclaration(level,&tex,&dx, inFile, table);
                while(token==commasym) {
                    token = NextToken(inFile);
                    constantDeclaration(level,&tex,&dx, inFile, table);
                }
                if(token==semicolonsym) {
                    token = NextToken(inFile);
                }
                else {
                    printError(5);
                }
            } while (token==identsym);
        }
        if (token==varsym) {
            token = NextToken(inFile);
            do {
                variableDeclaration(level,&tex,&dx, inFile, table);
                while (token==commasym) {
                    token = NextToken(inFile);
                    variableDeclaration(level,&tex,&dx, inFile, table);
                }
                if(token==semicolonsym) {
                    token = NextToken(inFile);
                }
                else {
                    printError(5);
                }
            } while(token==identsym);
        }
        while(token==procsym) {
            token = NextToken(inFile);

          
            if(token==identsym) {
                enter(3, &tex, &dx, level, table); 
                token = NextToken(inFile);
            }
            else {
                printError(4);
            }
            if(token==semicolonsym) {
                token = NextToken(inFile);
            }
            else {
                printError(5); 
            }
           
            block(level+1, tex, table, inFile, code); 
            if(token==semicolonsym) {
                token = NextToken(inFile);
            }
            else {
                printError(5); 
            }
        }
    }while((token==constsym) || (token==varsym) || (token==procsym));

    code[table[tex0].addr].m=cx;
    table[tex0].addr=cx;
    cx0=cx;
    send(6, 0, dx, code);
    discription(level, &tex, inFile, code, table);
    send(2, 0, 0, code); 
}

void constantDeclaration(int level, int *ptex, int *pdx, FILE* inFile, symbol* table) {
    
    if (token==identsym) {
        token = NextToken(inFile);
        if ((token==eqsym) || (token==becomessym)) {
            if (token==becomessym) {
                printError(1); 
            }
            token = NextToken(inFile);
            if (token==numbersym) {
                enter(1,ptex,pdx,level, table);
                token = NextToken(inFile);
            }
        }
    }
}

void variableDeclaration(int level, int *ptex, int *pdx, FILE* inFile, symbol* table) {
    
    if (token==identsym) {
        enter(2,ptex,pdx,level, table);
        token = NextToken(inFile);
    }
    else printError(4); 
}

void discription(int level, int *ptex, FILE* inFile, instruction* code, symbol* table) {
    
    int i, cx1, cx2;

    if (token==identsym){
        i=position(id,ptex, table, level);
        if(i==0) {
            printError(11); 
        }
        else if (table[i].kind!=2) { 
                printError(12); 
                i=0;
        }
        token = NextToken(inFile);
        if (token==becomessym) {
            token = NextToken(inFile);
        }
        else {
            printError(13); 
        }
        expression(level, ptex, inFile, code, table);
        if (i!=0) {
            send(4, level-table[i].levelel, table[i].addr, code); 
        }
    }
    else if (token==callsym) {
        token = NextToken(inFile);
        if (token!=identsym) {
            printError(14); 
        }
        else {
            i=position(id, ptex, table, level);
            if(i==0) {
                printError(11); 
            }
            else if (table[i].kind==3) {
                send(5,level-table[i].levelel, table[i].addr, code); 
            }
            else {
                    printError(15);
            }
            token = NextToken(inFile);
        }
    }


    else if (token == ifsym) {
        token = NextToken(inFile);
        condition(level, ptex, inFile, code, table);
        if(token == thensym) {
            token = NextToken(inFile);
        }
        else {
            printError(16);  
        }
        
        cx1 = cx;
        send(8, 0, 0, code); 
        discription(level, ptex, inFile, code, table);
    
        if(token == elsesym) {
            token = NextToken(inFile);
            
            code[cx1].m = cx+1; 
            cx1 = cx;
            send(7, 0, 0, code); 
            discription(level, ptex, inFile, code, table);
        }
        code[cx1].m = cx; 
    }

    else if (token == beginsym) {
        token = NextToken(inFile);
        discription(level, ptex, inFile, code, table);
        
         while (token == semicolonsym) {
            token = NextToken(inFile);
            discription(level, ptex, inFile, code, table);
         }

        if (token == endsym) {
            token = NextToken(inFile);
        }
        else {
            printError(17);  
        }
    }


    else if (token == whilesym) {
        cx1 =cx;
        token = NextToken(inFile);
        condition(level,ptex, inFile, code, table);
        cx2 = cx;
        send(8, 0, 0, code); 

        if(token == dosym) {
            token = NextToken(inFile);
        }
        else {
            printError(18);  
        }
        discription(level, ptex, inFile, code, table);
        send(7, 0, cx1, code);
        code[cx2].m = cx;
    }

   
    else if (token == writesym) {
        token = NextToken(inFile);
        expression(level, ptex, inFile, code, table);
        send(9,0,1, code);
    }
    
    else if (token == readsym) {
        token = NextToken(inFile);
        send(10,0,2, code); 
        i=position(id,ptex, table, level);
        if(i==0) {
            printError(11); 
        }
        else if (table[i].kind!=2) { 
            printError(12); 
            i=0;
        }
        if (i!=0) {
            send(4, level-table[i].levelel, table[i].addr, code); 
        }
         token = NextToken(inFile);
    }

}

void condition(int level, int *ptex, FILE* inFile, instruction* code, symbol* table) {
    
    int relationSwitch;

 if (token == oddsym) {
     token = NextToken(inFile);
        expression(level,ptex, inFile, code, table);
     send(2, 0, 6, code); 

    }
    else {
        expression(level,ptex, inFile, code, table);
        if ((token!=eqsym) && (token!=neqsym) && (token!=lessym) && (token!=leqsym) && (token!=gtrsym) && (token!=geqsym)) {
            printError(20); 
        }
        else { 
            relationSwitch = token;
            token = NextToken(inFile);
            expression(level, ptex, inFile, code, table);
            switch(relationSwitch) {
                case 9:
                    send(2,0,8, code); 
                    break;
                case 10:
                    send(2,0,9, code);
                    break;
                case 11:
                    send(2,0,10, code);
                    break;
                case 12:
                    send(2,0,11, code); 
                    break;
                case 13:
                    send(2,0,12, code); 
                    break;
                case 14:
                    send(2,0,13, code); 
                    break;
            }
        }
    }
}

void expression(int level, int *ptex, FILE* inFile, instruction* code, symbol* table) {

    int add;
    if (token == plussym || token == minussym) {
        add = token;
        token = NextToken(inFile);
        term(level, ptex, inFile, code, table);
        if(add == minussym) {
            send(2, 0, 1, code);
        }
    }
    else {
        term (level, ptex, inFile, code, table);
    }
    while (token == plussym || token == minussym) {
        add = token;
        token = NextToken(inFile);
        term(level, ptex, inFile, code, table);
        if (add == plussym) {
            send(2, 0, 2, code); 
        }
        else {
            send(2, 0, 3, code); 
        }
    }
}

void term(int level, int *ptex, FILE* inFile, instruction* code, symbol* table) {
    
    int mulop;
    factor(level, ptex, table, inFile, code);
    while(token == multsym || token == slashsym) {
        mulop = token;
        token = NextToken(inFile);
        factor(level, ptex, table, inFile, code);
        if(mulop == multsym) {
            send(2, 0, 4, code); 
        }
        else {
            send(2, 0, 5, code); 
        }
    }
}

void factor(int level, int *ptex, symbol* table, FILE* inFile, instruction* code) {
    
    int i, levelel, adr, val;

    while ((token==identsym)||(token==numbersym)||(token==lparentsym)){
        if (token==identsym) {
            i=position(id,ptex, table, level);
            if (i==0) {
                printError(11); 
            }
            else {
                kind=table[i].kind;
                levelel=table[i].levelel;
                adr=table[i].addr;
                val=table[i].val;
                if (kind==1) {
                    send(1,0,val, code);
                }
                else if (kind==2) {
                    send(3,level-levelel,adr, code);
                }
                else {
                    printError(21);
                }
            }
            token = NextToken(inFile);
        }
        
        else if(token==numbersym) {
            if (num>2047) { 
                printError(25);
                num=0;
            }
            send(1,0,num, code); 
            token = NextToken(inFile);
        }
        else if(token==lparentsym) {
            token = NextToken(inFile);
            expression(level,ptex, inFile, code, table);
            if (token==rparentsym) {
                token = NextToken(inFile);
            }
            else {
                printError(22);
            }
        }
    }
}

void send(int op, int l, int m, instruction* code) {

    if (cx > CODE_SIZE)
        printf("Program too long! cx > CODE_SIZE\n");
    else {
        code[cx].op = op; 	
        code[cx].l = l;	
        code[cx].m = m;	
        cx++;
    }
}

void enter(int k, int *ptex, int *pdx, int level, symbol* table) {
    
    char *id1;
    int ii, len;
    (*ptex)++; 
    id1=id; 
    len=strlen(id);
    for (ii=0;ii<=len;ii++) {
        table[*ptex].name[ii]=*id1; 
        id1++;
    }
    table[*ptex].kind=k;
     if (k==1) {//const
        table[*ptex].val=num;
    }
 
    else if (k==2) {//var
        table[*ptex].levelel=level;
        table[*ptex].addr=*pdx;
        (*pdx)++;
    }
    else {//procedure
        table[*ptex].levelel=level;
    }
}

void printError(int printErrorCase) {

    printErrorCount++;
    
    switch (printErrorCase) {
        case 1:
            printf("printError 1: ");
            printf("Use = instead of :=.\n");
            break;
        case 2:
            printf("printError 2: ");
            printf("= must be followed by a number.\n");
            break;
        case 3:
            printf("printError 3: ");
            printf("Identifier must be followed by =.\n");
            break;
        case 4:
            printf("printError 4: ");
            printf("const, var, procedure must be followed by identifier.\n");
            break;
        case 5:
            printf("printError 5: ");
            printf("Semicolon or comma missing.\n");
            break;
        case 6:
            printf("printError 6: ");
            printf("Incorrect symbol after procedure declaration.\n");
            break;
        case 7:
            printf("printError 7: ");
            printf("discription expected\n");
            break;
        case 8:
            printf("printError 8: ");
            printf("Incorrect symbol after discription part in block.\n");
            break;
        case 9:
            printf("printError 9: ");
            printf("Period expected.\n");
            break;
        case 10:
            printf("printError 10: ");
            printf("Semicolon between discriptions missing.\n");
            break;
        case 11:
            printf("printError 11: ");
            printf("Undeclared identifier.\n");
            break;
        case 12:
            printf("printError 12: ");
            printf("Assignment to constant or procedure is not allowed.\n");
            break;
        case 13:
            printf("printError 13: ");
            printf("Assignment operator expected.\n");
            break;
        case 14:
            printf("printError 14: ");
            printf("call must be followed by an identifier\n");
            break;
        case 15:
            printf("printError 15: ");
            printf("Call of a constant or variable is meaningless.\n");
            break;
        case 16:
            printf("printError 16: ");
            printf("then expected\n");
            break;
        case 17:
            printf("printError 17: ");
            printf("Semicolon or } expected\n");
            break;
        case 18:
            printf("printError 18: ");
            printf("do expected\n");
            break;
        case 19:
            printf("printError 19: ");
            printf("Incorrect symbol following discription.\n");
            break;
        case 20:
            printf("printError 20: ");
            printf("Relational operator expected.\n");
            break;
        case 21:
            printf("printError 21: ");
            printf("Expression must not contain a procedure identifier.\n");
            break;
        case 22:
            printf("printError 22: ");
            printf("Right parenthesis missing.\n");
            break;
        case 23:
            printf("printError 23: ");
            printf("The preceding factor cannot begin with this symbol.\n");
            break;
        case 24:
            printf("printError 24: ");
            printf("An expression cannot begin with this symbol.\n");
            break;
        case 25:
            printf("printError 25: ");
            printf("This number is too large.\n");
            break;
        case 26:
            printf("printError: 26 ");
            printf("levelel is larger than the maximum allowed lexicographical levelels!\n");
            break;
        default:
            break;
    }
    //stops program when printError occurs
    exit(1);
}

int NextToken(FILE* inFile) {
    
    token = lexList[lexemeListIndex].tokenID;
    
    //Takes care of variables, always represented by "2 | variable"
    if(token==2){
        strcpy(id, lexList[lexemeListIndex].name);
    }
    //Takes care of numbers, always represented by "3 | number"
    else if(token==3){
        num = lexList[lexemeListIndex].numberValue;
    }
    
    lexemeListIndex++;
    return token;
}

int position(char *id, int *ptex, symbol* table, int level) {
    
    int s;
    s=*ptex;
    
    int currentS;
    int differenceCount = 0;

    while(s!=0) {
        if (strcmp(table[s].name,id) == 0) {
           
            if(table[s].levelel<=level) {
               
                if (differenceCount!=0) {
                    previousDifference = difference;
                }
                
                difference = level-table[s].levelel;
                
                if(differenceCount==0) {
                    currentS=s;
                }
                
                if (difference<previousDifference) {
                    currentS=s;
                }
                differenceCount++;
            }
        }
        s--;
    }
    
    return currentS;
}



void machine(void) {

    FILE* inFile;
    FILE* outFile;
    
    inFile = fopen("parserout.txt", "r");
    outFile = fopen("VMout.txt", "w");
    outFile2 = fopen("codegenoutput.txt", "w");
    outFile3 = fopen("vmout.txt", "w");

    int i=0;
    int stack[MAX_STACK_HEIGHT] = {0};

    int SP = 0;
    int BP = 1;
    int PC = 0;
    int IR = 0;

  
    instruction arrayStruct[CODE_SIZE];
    instruction *IRstruct;



        fprintf(outFile, "Line\tOP\tL\tM\n");
        fprintf(outFile2, "Line\tOP\tL\tM\n");
         while (fscanf(inFile, "%d %d %d", &arrayStruct[i].op, &arrayStruct[i].l, &arrayStruct[i].m) != EOF) {
             fprintf(outFile, "%d\t%s\t%d\t%d\n", i, opcodes[arrayStruct[i].op], arrayStruct[i].l, arrayStruct[i].m);
             fprintf(outFile2, "%d\t%s\t%d\t%d\n", i, opcodes[arrayStruct[i].op], arrayStruct[i].l, arrayStruct[i].m);
             i++;
         }



        fprintf(outFile, "\n\n");
        fprintf(outFile, "\t\t\t\tpc \tbp \tsp \tstack\n");
        fprintf(outFile, "Initial values\t\t\t%d  \t%d \t%d \n", PC, BP, SP);
    
        fprintf(outFile3, "\t\t\t\tpc \tbp \tsp \tstack\n");
        fprintf(outFile3, "Initial values\t\t\t%d  \t%d \t%d \n", PC, BP, SP);


    while (BP!=0) {

  
        IRstruct=&arrayStruct[PC];
            fprintf(outFile, "%d\t%s \t%d \t%d",PC, opcodes[IRstruct->op], IRstruct->l, IRstruct->m);
            fprintf(outFile3, "%d\t%s \t%d \t%d",PC, opcodes[IRstruct->op], IRstruct->l, IRstruct->m);
        PC++;

        
        executeCycle(IRstruct, stack, &SP, &BP, &PC);
            fprintf(outFile, "\t%d  \t%d \t%d \t", PC, BP, SP);

            fprintf(outFile3, "\t%d  \t%d \t%d \t", PC, BP, SP);
    
        printStackFrame(stack, SP, BP, outFile);
            fprintf(outFile, "\n");

            fprintf(outFile3, "\n");
    }

    fclose(inFile);
    fclose(outFile);
    fclose(outFile2);
    fclose(outFile3);
}


void printStackFrame(int* stack, int SP, int BP, FILE* outFile) {

    int i=0;


    if (BP==0) {
        return;
    }
 
    else if (BP==1) {

        for(i=1;i<=SP;i++){
                fprintf(outFile, "%d ",stack[i]);
            
                fprintf(outFile3, "%d ",stack[i]);
        }
        return;
    }
   
    else {
        printStackFrame(stack, BP-1, stack[BP+2], outFile);

     
        if (SP<BP) {
                fprintf(outFile, "| ");
    
                fprintf(outFile3, "| ");
            
            for (i=0;i<4;i++) {
                    fprintf(outFile, "%d ", stack[BP+i]);

                    fprintf(outFile3, "%d ", stack[BP+i]);
            }
        }

        else {
            fprintf(outFile, "| ");
            
            fprintf(outFile3, "| ");
            for (i=BP;i<=SP;i++) {
                    fprintf(outFile, "%d ", stack[i]);
                
                    fprintf(outFile3, "%d ", stack[i]);
            }
        }
        return;
    }
}


void executeCycle(instruction* IRstruct, int* stack, int* sp, int* bp, int* pc) {

    switch(IRstruct->op) {
        case 1: 
            *sp=*sp+1;
            stack[*sp]=IRstruct->m;
            break;
        case 2: 
            OPR(sp, bp, pc, stack, IRstruct);
            break;
        case 3: 
            *sp=*sp+1;
            stack[*sp]=stack[base(bp, IRstruct, stack)+IRstruct->m];
            break;
        case 4: 
            stack[base(bp, IRstruct, stack)+IRstruct->m]=stack[*sp];
            *sp=*sp-1;
            break;
        case 5: 
            stack[*sp+1]=0; 
            stack[*sp+2]=base(bp, IRstruct, stack); 
            stack[*sp+3]=*bp; 
            stack[*sp+4]=*pc; 
            *bp=*sp+1;
            *pc=IRstruct->m;
            break;
        case 6: 
            *sp=*sp+IRstruct->m;
            break;
        case 7: 
            *pc=IRstruct->m;
            break;
        case 8: 
            if (stack[*sp]==0) {
                *pc=IRstruct->m;
            }
            *sp=*sp-1;
            break;
        case 9: 
            printf("%d\n", stack[*sp]);
            *sp=*sp-1;
            break;
        case 10: 
            *sp=*sp+1;
            scanf("%d", &stack[*sp]);
            break;
        default:
            printf("Illegal OPR!\n");
    }
}


void OPR (int *sp, int* bp, int *pc, int *stack, instruction* IRstruct) {

    switch (IRstruct->m) {

        case 0: 
            *sp=*bp-1;
            *pc=stack[*sp+4];
            *bp=stack[*sp+3];
            break;
        case 1: 
            stack[*sp]=-stack[*sp];
            break;
        case 2: 
            *sp=*sp-1;
            stack[*sp]=stack[*sp]+stack[*sp+1];
            break;
        case 3: 
            *sp=*sp-1;
            stack[*sp]=stack[*sp]-stack[*sp+1];
            break;
        case 4:
            *sp=*sp-1;
            stack[*sp]=stack[*sp]*stack[*sp+1];
            break;
        case 5: 
            *sp=*sp-1;
            stack[*sp]=stack[*sp]/stack[*sp+1];
            break;
        case 6: 
            stack[*sp]=stack[*sp]%2;
            break;
        case 7: 
            *sp=*sp-1;
            stack[*sp]=stack[*sp]%stack[*sp+1];
            break;
        case 8: 
            *sp=*sp-1;
            stack[*sp]=stack[*sp]==stack[*sp+1];
            break;
        case 9: 
            *sp=*sp-1;
            stack[*sp]=stack[*sp]!=stack[*sp+1];
            break;
        case 10: 
            *sp=*sp-1;
            stack[*sp]=stack[*sp]<stack[*sp+1];
            break;
        case 11: 
            *sp=*sp-1;
            stack[*sp]=stack[*sp]<=stack[*sp+1];
            break;
        case 12: 
            *sp=*sp-1;
            stack[*sp]=stack[*sp]>stack[*sp+1];
            break;
        case 13: 
            *sp=*sp-1;
            stack[*sp]=stack[*sp]>=stack[*sp+1];
            break;
    }
}


int base(int* bp, instruction* IRstruct, int* stack) {
    int l = IRstruct->l;
    int b1; 
    b1 = *bp;
    while (l>0) {
        b1=stack[b1+1];
        l--;
    }
    return b1;
}


void lex(void){

    int count;
    for(count=0;count<5000;count++){
        lexList[count].tokenID=0;
    }
    
    FILE* inFile;
    FILE* outFile;
    
    

    inFile = fopen("input.txt", "r");
    outFile = fopen("lexoutput.txt", "w");

    int i,j=0,k=0;
    int printErrorHolder;
    int c;
    int comments=0;
    int lookAhead=0;

 

    inFile=fopen("input.txt","r");
    c=fgetc(inFile);
    while(c!=EOF){
        if(c==' '||c=='\t'||c=='\r' ||c=='\n'){
            c=fgetc(inFile);
            lookAhead=0;
            continue;
        }
       
        if(isalpha(c)){
            char characterString[12];
            memset(characterString, 0, sizeof characterString);
            
            int index=0;
            characterString[index]=c;

            index++;
            lookAhead=1;
          
            while(isalpha(c=fgetc(inFile))||isdigit(c)){
                if(index>10){
                
                    while (isalpha(c=fgetc(inFile))||isdigit(c)) {

                    }
                    printErrorHolder=1;

                    
                    break;
                }
                characterString[index]=c;
                index++;
            }

         
            if(printErrorHolder==1) {
                printErrorHolder=0;
                continue;
            }

         
            int reservedSwitch=-1;

            for(i=0; i<14;i++){
                if(strcmp(characterString, reservedWords[i])==0){
                    reservedSwitch=i;
                }
            }
            
            switch(reservedSwitch){

                case 0:
                    lexList[lexListIndex].tokenID = constsym;
                    break;
                case 1:
                    lexList[lexListIndex].tokenID = varsym;
                    break;
                case 2:
                    lexList[lexListIndex].tokenID = procsym;
                    break;
                case 3:
                    lexList[lexListIndex].tokenID = callsym;
                    break;
                case 4:
                    lexList[lexListIndex].tokenID = beginsym;
                    break;
                case 5:
                    lexList[lexListIndex].tokenID = endsym;
                    break;
                case 6:
                    lexList[lexListIndex].tokenID = ifsym;
                    break;
                case 7:
                    lexList[lexListIndex].tokenID = thensym;
                    break;
                case 8:
                    lexList[lexListIndex].tokenID = elsesym;
                    break;
                case 9:
                    lexList[lexListIndex].tokenID = whilesym;
                    break;
                case 10:
                    lexList[lexListIndex].tokenID = dosym;
                    break;
                case 11:
                    lexList[lexListIndex].tokenID = readsym;
                    break;
                case 12:
                    lexList[lexListIndex].tokenID = writesym;
                    break;
                case 13:
                    lexList[lexListIndex].tokenID = oddsym;
                    break;

                default:
                    lexList[lexListIndex].tokenID = identsym;
                    strcpy(lexList[lexListIndex].name,characterString);
                    break;
            }
            lexListIndex++;
        }
       
        else if(isdigit(c)){
            int number=c-'0';
            int d;
            int place=1;

            lookAhead=1;

          
            while(isdigit(c=fgetc(inFile))){
                if(place>4){
                   
                    while (isdigit(c=fgetc(inFile))) {

                    }
                    printErrorHolder=1;
                    break;
                }
                d=c-'0';
                number=10*number+d;
                place++;
            }

          
            if(isalpha(c)){
           
                while(isalpha(c=fgetc(inFile))||isdigit(c)){

                }
                continue;
             }

            if(printErrorHolder==1) {
                printErrorHolder=0;
                continue;
            }

            lexList[lexListIndex].tokenID=numbersym;
            lexList[lexListIndex].numberValue=number;
            lexListIndex++;
        }

        
        else {
            lookAhead=0;
            int spec=-1;
            for(i=0;i<13;i++){
                if(c==specialSymbols[i]){
                    spec=i;
                }
            }
          
            switch(spec){
                case 0:
                    lexList[lexListIndex].tokenID = plussym;
                    lexListIndex++;
                    break;
                case 1:
                    lexList[lexListIndex].tokenID = minussym;
                    lexListIndex++;
                    break;
                case 2:
                    lexList[lexListIndex].tokenID = multsym;
                    lexListIndex++;
                    break;

                case 3:
                    c=fgetc(inFile);
                    lookAhead=1;
                    if(c=='*'){
                        comments=1;
                        lookAhead=0;
                        c=fgetc(inFile);
                        while(comments==1){
                            if(c=='*'){
                                c=fgetc(inFile);
                                if(c=='/'){
                                    comments=0;
                                }
                            }
                            else{
                                c=fgetc(inFile);
                            }
                        }
                    }
                    else{
                        lexList[lexListIndex].tokenID = slashsym;
                        lexListIndex++;
                    }
                    break;
                case 4:
                    lexList[lexListIndex].tokenID = lparentsym;
                    lexListIndex++;
                    break;
                case 5:
                    lexList[lexListIndex].tokenID = rparentsym;
                    lexListIndex++;
                    break;
                case 6:
                    lexList[lexListIndex].tokenID = eqsym;
                    lexListIndex++;
                    break;
                case 7:
                    lexList[lexListIndex].tokenID = commasym;
                    lexListIndex++;
                    break;
                case 8:
                    lexList[lexListIndex].tokenID = periodsym;
                    lexListIndex++;
                    break;
                case 9:
                    c=fgetc(inFile);
                    lookAhead=1;
                    if(c=='>'){
                        lexList[lexListIndex].tokenID = neqsym;
                        lookAhead=0;
                    }
                    else if(c=='='){
                        lexList[lexListIndex].tokenID = leqsym;
                        lookAhead=0;
                    }
                    else{
                        lexList[lexListIndex].tokenID = lessym;
                    }
                    lexListIndex++;
                    break;
                case 10:
                    c=fgetc(inFile);
                    lookAhead=1;
                    if(c=='='){
                        lexList[lexListIndex].tokenID = geqsym;
                        lookAhead=0;
                    }
                    else{
                        lexList[lexListIndex].tokenID = gtrsym;
                    }
                    lexListIndex++;
                    break;
                case 11:
                    lexList[lexListIndex].tokenID = semicolonsym;
                    lexListIndex++;
                    break;
                case 12:
                    c=fgetc(inFile);
                    if(c == '='){
                        lexList[lexListIndex].tokenID = becomessym;
                        lexListIndex++;
                    }
                    
                  
                    break;
                
                default:
                  
                    break;
            }
        }

        if(lookAhead==0){
            c=fgetc(inFile);
        }

    }
   
    fprintf(outFile,"%d", lexList[0].tokenID);
    if(lexList[0].tokenID==2){
        fprintf(outFile," %s", lexList[0].name);
    }
    
    else if(lexList[0].tokenID==3){
        fprintf(outFile," %d",lexList[0].numberValue);
    }

    for(i=1;i<lexListIndex;i++){
        fprintf(outFile," %d", lexList[i].tokenID);
        if(lexList[i].tokenID==2){
            fprintf(outFile," %s", lexList[i].name);
        }
        else if(lexList[i].tokenID==3){
            fprintf(outFile," %d",lexList[i].numberValue);
        }
    }
    fclose(inFile);
    fclose(outFile);
}