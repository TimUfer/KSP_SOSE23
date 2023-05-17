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
#define PUSHG 11
#define POPG 12
#define ASF 13
#define RSF 14
#define PUSHL 15
#define POPL 16
#define EQ 17
#define NE 18
#define LT 19
#define LE 20
#define GT 21
#define GE 22
#define JMP 23
#define BRF 24
#define BRT 25
#define IMMEDIATE(x) ((x) & 0x00FFFFFF)
#define SIGN_EXTEND(i) ((i) & 0x00800000 ? (i) | 0xFF000000 : (i))
#define VERSION "0"

int programmCounter = 0;
int stackPointer = 0;
int framePointer = 0;
int stack[10000];
int sda[50];
unsigned int* code;
int lines;
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
            printf("%d", pop());
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
        case PUSHG:
            push(sda[input]);
            break;
        case POPG:
            sda[input] = pop();
            break;
        case ASF:
            push(framePointer);
            framePointer = stackPointer;
            stackPointer = stackPointer + input;
            break;
        case RSF:
            stackPointer = framePointer;
            framePointer = pop();
            break;
        case PUSHL:
            push(stack[framePointer + input]);
            break;
        case POPL:
            stack[input] = pop();
            break;
        case EQ: {
            int temp1 = pop();
            if(temp1 == pop()){
                push(1);
            } else {
                push(0);
            }
            break;
        }
        case NE: {
            int temp2 = pop();
            if(temp2 != pop()){
                push(1);
            } else {
                push(0);
            }
            break;
        }
        case LT: {
            int temp3 = pop();
            if(temp3 < pop()){
                push(1);
            } else {
                push(0);
            }
            break;
        }
        case LE: {
            int temp4 = pop();
            if(temp4 <= pop()){
                push(1);
            } else {
                push(0);
            }
            break;
        }
        case GT: {
            int temp1 = pop();
            if(temp1 > pop()){
                push(1);
            } else {
                push(0);
            }
            break;
        }
        case GE: {
            int temp1 = pop();
            if(temp1 >= pop()){
                push(1);
            } else {
                push(0);
            }
            break;
        }
        case JMP:
            programmCounter = input;
            break;
        case BRF:
            break;
        case BFT:
            break;

        default:
            break;
    }
}

/*unsigned int code1[] = {
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
};*/

// executes the op code given in the *prog parameter
void programm_exe(const unsigned int *prog){
    unsigned int oc = 166;
    int ins;

    // the while loop executes the op code until it reaches 0 aka HALT
    while(oc != HALT) {
        ins = prog[programmCounter];
        programmCounter = programmCounter + 1;
        oc = prog[programmCounter] >> 24;
        executeOP(ins);
    }
    free(code);
}

/*void execute(int p){
    if(p == 1){
        programm_exe(code1);
    } else if(p == 2){
        programm_exe(code2);
    } else if(p == 3){
        programm_exe(code3);
    }

}*/
void readElements(char path[]) {
    FILE *p = fopen(path, "rb");
    int elements[4];
    int read_len;

    if (p == NULL) {
        printf("Wrong file Path. Couldnt open File.");
    } else {
        // read the file line by line
        read_len = fread(elements, sizeof(unsigned int), 4, p);
    }
    if (read_len != 4) {
        printf("Error reading file.\n");
        exit(1);
    }

    if(fclose(p) != 0){
        perror("ERROR while closing");
    }
    printf("actual asm lines: %u\n",elements[2]);
    lines = elements[2];
}

//reads the path and saves the values in the "code" array. elements gives the value of lines which you want to read.
void readExecuteFile(char path[]){
    readElements(path);
    code = (unsigned int*) malloc(lines * sizeof(unsigned int));
    FILE *pF = fopen(path, "rb");
    // +4 Because of the "bin head from ninja". the actual values start at the 5th position

    int elements = lines+4;
    size_t read_len;
    unsigned int buffer[250];

    if(pF == NULL){
        printf("Wrong file Path. Couldnt open File.");
    } else {
        // reading the file to the buffer array
        read_len = fread(buffer, sizeof(unsigned int), elements, pF);

        if (read_len != elements) {
            printf("Error reading file.\n");
            exit(1);
        }

        if(fclose(pF) != 0){
            perror("ERROR while closing");
        }

        printf("Elements read: %zu\n", read_len);

        for(int k = 4; k< elements; ++k){
            // -4 because you don't want the "ninja bin head" in the code
            code[k-4] = buffer[k];
        }
    }

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
        readExecuteFile("prog/prog_1.bin");
        programm_exe(code);
        printf("Ninja Virtual Machine stopped\n");
    }
    return 0;
}
