#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <bigint.h>
#include <support.h>
#include "support.h"
#include "bigint.h"

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
#define CALL 26
#define RET 27
#define DROP 28
#define PUSHR 29
#define POPR 30
#define DUP 31
#define IMMEDIATE(x) ((x) & 0x00FFFFFF)
#define SIGN_EXTEND(i) ((i) & 0x00800000 ? (i) | 0xFF000000 : (i))
#define VERSION "0"
#define StackSize 10000
#define number_global_vars 50

typedef int Object;
typedef struct {
    unsigned int size;
    unsigned char data[1];
} *ObjRef;

typedef struct {
    bool isObjRef;
    union {
        ObjRef objref;
        int number;
    } u;
} StackSlot;
StackSlot *stack;
ObjRef *sda;


int programmCounter = 0;
int stackPointer = 0;
int framePointer = 0;
//int stack[StackSize];
//int sda[50];
//int rvr;
ObjRef rvr;
unsigned int* code;
int lines;
//Main
/*
ObjRef createObjRef(unsigned int payloadSize, int input) {
    unsigned int objSize = sizeof(unsigned int) + payloadSize;
    ObjRef obj = (ObjRef)malloc(objSize);

    if (obj == NULL) {
        perror("malloc");
        return NULL;
    }

    obj->size = payloadSize;
    *(int *)obj->data = input;
    return obj;
}*/

void * newPrimObject(int dataSize) { // todo richtige size
    ObjRef objRef;
    int size;
    size = sizeof(unsigned int) + dataSize * sizeof(char);
    if((objRef = malloc(size)) == NULL){
        printf("ERROR: newPrim no Mem \n");
    }

    objRef->size = dataSize;
    return objRef;
}

void * getPrimObjectDataPointer(void * obj){
    ObjRef oo = ((ObjRef) (obj));
    return oo->data;
}

void fatalError(char *msg){
    printf("Fatal Error: %s \n", msg);
    exit(1);
}

ObjRef cmpObj(int datasize){
    ObjRef cmpObj;
    unsigned int objSize;
    objSize = sizeof(*cmpObj) + (datasize * sizeof(void*));
    if((cmpObj = malloc(objSize)) == NULL){
        perror("Error cmpObj malloc");
    }
    cmpObj->size = datasize;
    return cmpObj;
}

void pushObj(ObjRef a){
    stack[stackPointer].isObjRef = true;
    stack[stackPointer].u.objref = a;
    stackPointer++;
}
void push(int b){
    stack[stackPointer].isObjRef = false;
    stack[stackPointer].u.number = b;
    stackPointer++;
}
int pop(void){
    stackPointer--;
    return stack[stackPointer].u.number;
}
int popObj(void){
    stackPointer--;
    return stack[stackPointer].u.objref;
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
    printf("exec");
    unsigned int opcode = opc >> 24;
    int input = SIGN_EXTEND(opc & 0x00FFFFFF);
    switch (opcode) {
        case PUSHC: {
            bigFromInt(input);
            pushObj(bip.res);
            break;
        }
        case ADD: {
            bip.op2 = popObj();
            bip.op1 = popObj();
            bigAdd();
            pushObj(bip.res);
            break;
        }
        case SUB: {
            bip.op2 = popObj();
            bip.op1 = popObj();
            bigSub();
            pushObj(bip.res);
            break;
        }
        case MUL: {
            bip.op2 = popObj();
            bip.op1 = popObj();
            bigMul();
            pushObj(bip.res);
            break;
        }
        case DIV: {
            bip.op2 = popObj();
            bip.op1 = popObj();
            bigDiv();
            pushObj(bip.res);
            break;
        }
        case MOD: {
            bip.op2 = popObj();
            bigFromInt(0);
            bip.op1 = bip.res;
            if(bigCmp() == 0){
                perror("Cant do Mod with 0\n");
                exit(1);
            }
            bip.op1 = popObj();
            bigDiv();
            pushObj(bip.rem);
            break;
        }
        case RDINT: {
            bigRead(stdin);
            pushObj(bip.res);
            break;
        }
        case WRINT: { //todo wird nicht ausgeführt jb
            printf("wrint");
            bip.op1 = popObj();
            bigPrint(stdout);
            break;
        }
        case RDCHR: {
            char in;
            scanf("%c", &in);
            bigFromInt(in);
            pushObj(bip.res);
            break;
        }
        case WRCHR: {
            bip.op1 = popObj();
            printf("%c", (char)bigToInt());
            break;
        }
        case PUSHG: {
            pushObj(sda[input]);
            break;
        }
        case POPG: {
            bip.op1 = popObj();
            sda[input] = bip.op1;
            break;
        }
        case ASF: {
            push(framePointer);
            framePointer = stackPointer;
            stackPointer = stackPointer + input;
            break;
        }
        case RSF: {
            stackPointer = framePointer;
            framePointer = pop();
            break;
        }
        case PUSHL: {
            push(stack[framePointer + input].u.objref);
            break;
        }

        case POPL: {
            stack[framePointer + input].u.objref = popObj();
            break;
        }

        case EQ: {
            int tmp;
            bip.op2 = pop();
            bip.op1 = pop();
            tmp = bigCmp();
            if(tmp == 0){
                bigFromInt(true);
                push(bip.res);
            }else{
                bigFromInt(false);
                push(bip.res);
            }
            break;
        }
        case NE: {
            int temp;
            bip.op2 = pop();
            bip.op1 = pop();
            temp = bigCmp();
            if(temp != 0){
                bigFromInt(true);
                push(bip.res);
            }else {
                bigFromInt(false);
                push(bip.res);
            }
            break;
        }
        case LT: {
            int temp;
            bip.op2 = pop();
            bip.op1 = pop();
            temp = bigCmp();
            if(temp < 0){
                bigFromInt(true);
                push(bip.res);
            }else {
                bigFromInt(false);
                push(bip.res);
            }
            break;
        }
        case LE: {
            int temp;
            bip.op2 = pop();
            bip.op1 = pop();
            temp = bigCmp();
            if(temp <= 0){
                bigFromInt(true);
                push(bip.res);
            }else {
                bigFromInt(false);
                push(bip.res);
            }
            break;
        }
        case GT: {
            int temp;
            bip.op2 = pop();
            bip.op1 = pop();
            temp = bigCmp();
            if(temp > 0){
                bigFromInt(true);
                push(bip.res);
            }else {
                bigFromInt(false);
                push(bip.res);
            }
            break;
        }
        case GE: {
            int temp;
            bip.op2 = pop();
            bip.op1 = pop();
            temp = bigCmp();
            if(temp >= 0){
                bigFromInt(true);
                push(bip.res);
            }else {
                bigFromInt(false);
                push(bip.res);
            }
            break;
        }
        case JMP: {
            programmCounter = input;
            break;
        }
        case BRF: {
            int temp;
            bip.op1 = pop();
            temp = bigToInt();
            if(temp == false){
                programmCounter = input;
            }

            break;
        }

        case BRT: {
            int temp;
            bip.op1 = pop();
            temp = bigToInt();
            if(temp == true){
                programmCounter = input;
            }
            break;
        }
        case CALL: {
            push(programmCounter);
            programmCounter = input;
            break;
        }
        case RET: {
            programmCounter = pop();
            break;
        }
        case DROP: {
            for (int i = 0; i < input; ++i) {
                pop();
            }
            break;
        }
        case PUSHR: {
            pushObj(rvr);
            break;
        }
        case POPR: {
            rvr = popObj();
            break;
        }
        case DUP:{
            ObjRef dup = popObj();
            pushObj(dup);
            pushObj(dup);
            break;
        }
        default: {
            fatalError("Invalid opcode");
            break;
        }
    }
}

// executes the op code given in the *prog parameter
void programm_exe(const unsigned int *prog){
    unsigned int oc = 42;
    int ins;
    // the while loop executes the op code until it reaches 0 aka HALT
    while(oc != HALT) {
        ins = prog[programmCounter];
        oc = prog[programmCounter] >> 24;
        printf("soll ausgeführt werden: %d\n",oc);
        printf("pre progcount: %d\n", programmCounter);
        programmCounter = programmCounter + 1;
        printf("post progcount: %d\n", programmCounter);

        executeOP(ins);
        printf("wurde ausgeführt: %d\n",oc);
    }
    free(code);
}

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

        for(int k = 4; k< elements; ++k){
            // -4 because you don't want the "ninja bin head" in the code
            code[k-4] = buffer[k];
        }
        for (int i = 0; i < elements; i++) {
            //printf("%p --> %x\n", &code[i], code[i]);
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
            printf("Ninja Virtual Machine started\n");
            stack = malloc(StackSize);
            sda = malloc(number_global_vars * sizeof(unsigned int));

            readExecuteFile(argv[1]);
            programm_exe(code);
            printf("Ninja Virtual Machine stopped\n");
        }
    }
    return 0;
}
