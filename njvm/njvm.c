#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define HALT 0
#define PUSHC 1
#define ADD 2
#define SUB 3
#define MUL 4
#define DIV 5
#define MOD 6
#define RDINT 7
#define WRINT 8
#define RDCHR 9
#define WRCHR 10
#define IMMEDIATE(x) ((x) & 0x00FFFFFF)
#define SIGN_EXTEND(i) ((i) & 0x00800000 ? (i) | 0xFF000000 : (i))
#define VERSION "0"

int stackPointer = 0;
int stack[10000];

//Main
void push(int a){
    stack[stackPointer] = a;
    stackPointer++;
}
int pop(void){
    stackPointer--;
    int temp = stack[stackPointer];
    return temp;
}
void print_stack(void){
    for(int i = stackPointer; i >= 0; i--){
        if(i == stackPointer){
            printf("|stackPointer->%3d| <empty>|\n", i);
        } else {
            printf("|%7d|  %5d|\n", i, stack[i]);
        }
        printf("'-------+-------'\n\n");
    }
}


void executeOP(unsigned int opc){
    unsigned int opcode = opc >> 24;
    int input = SIGN_EXTEND(opc & 0x00FFFFFF);
    switch (opcode) {
        case PUSHC:
            push(input);
            break;
        case ADD:
            push(pop()+pop());
            break;
        case SUB: {
            int t = pop();
            push(pop() - t);
            break;
        }
        case MUL:
            push(pop() * pop());
            break;
        case DIV: {
            int t = pop();
            push(pop() / t);
            break;
        }
        case MOD: {
            int t = pop();
            push(pop() % t);
            break;
        }
        case RDINT: {
            int input;
            scanf("%d", &input);
            push(input);
            break;
        }
        case WRINT:
            printf("%u", pop());
            break;
        case RDCHR: {
            char input;
            scanf("%c", &input);
            push(input);
            break;
        }
        case WRCHR:
            printf("%c", pop());
            break;
        default:
            break;
    }
}

unsigned int code1[] = {
        (PUSHC << 24) | IMMEDIATE(3),
        (PUSHC << 24) | IMMEDIATE(4),
        (ADD << 24),
        (PUSHC << 24) | IMMEDIATE(10),
        (PUSHC << 24) | IMMEDIATE(6),
        (SUB << 24),
        (MUL << 24),
        (WRINT << 24),
        (PUSHC << 24) | IMMEDIATE(10),
        (WRINT << 24),
        (HALT << 24)
};

unsigned int code2[] = {
        (PUSHC << 24) | IMMEDIATE(-2),
        (RDINT << 24),
        (MUL << 24),
        (PUSHC << 24) | IMMEDIATE(3),
        (ADD << 24),
        (WRINT << 24),
        (PUSHC << 24) | IMMEDIATE('\n'),
        (WRCHR << 24),
        (HALT << 24)
};

unsigned int code3[] = {
        (RDCHR << 24),
        (WRINT << 24),
        (PUSHC << 24) | IMMEDIATE('\n'),
        (WRCHR << 24),
        (HALT << 24)
};
void programm_exe(/*int codeNum,*/ const unsigned int *prog){
    unsigned int programmCounter = 0;
    unsigned int oc = 166;
    int ins;

    while(oc != HALT) {
        ins = prog[programmCounter];
        programmCounter = programmCounter + 1;
        oc = prog[programmCounter] >> 24;
        executeOP(ins);
    }
}

void execute(int p){
    if(p == 1){
        programm_exe(code1);
    } else if(p == 2){
        programm_exe(code2);
    } else if(p == 3){
        programm_exe(code3);
    }

}

void readExecuteFile(string path){
    FILE *pF = fopen(path, "r");
    char buffer[255];
    unsigned int code[20];

    int i = 0;
    if(pF == NULL){
        printf("Wrong file Path. Couldnt open File.")
    } else {
        while(fgets(buffer, 255, pF) != NULL){
            //read...
            code[i] = malloc(strlen(buffer) + 1);
            strcpy(code[i], buffer);
            size_t fread(buffer, 20, 5, pF);
            ++i;
        }
    }


    fclose(pF);
}

int main(int argc, char* argv[]) {
    if (argc == 2) {
        if (strcmp(argv[1], "--version") == 0) {
            printf("Version: " VERSION"\n");
            exit(0);
        } else if (strcmp(argv[1], "--help") == 0) {
            printf("    --version    shows version and exit\n");
            printf("    --help       shows help and exit\n");
            exit(0);
        } else{
            printf( "unknown command line argument %s, try './njvm --help' \n", argv[1]);
        }
    } else{
        printf("Ninja Virtual Machine started\n");
        execute(3);
        printf("Ninja Virtual Machine stopped\n");
    }
    return 0;
}
