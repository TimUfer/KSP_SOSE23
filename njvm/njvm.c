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
#define NEW 32
#define GETF 33
#define PUTF 34
#define NEWA 35
#define GETFA 36
#define PUTFA 37
#define GETSZ 38
#define PUSHN 39
#define REFEQ 40
#define REFNE 41

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

#define MSB (1 << (8 * sizeof(unsigned int) - 1))
#define IS_PRIMITIVE(objRef) (((objRef)->size & MSB) == 0)
#define GET_ELEMENT_COUNT(objRef) ((objRef)->size & ~MSB)
#define GET_REFS_PTR(objRef) ((ObjRef *) (objRef)->data)

int programmCounter = 0;
int stackPointer = 0;
int framePointer = 0;
ObjRef rvr = NULL;
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

void * newPrimObject(int dataSize) {
    ObjRef bigObjRef;

    bigObjRef = malloc(sizeof(unsigned int) +
                       dataSize * sizeof(unsigned char));
    if (bigObjRef == NULL) {
        fatalError("newPrimObject() got no memory");
    }
    bigObjRef->size = dataSize;
    return bigObjRef;
}

void * getPrimObjectDataPointer(void * obj){
    ObjRef oo = ((ObjRef) (obj));
    return oo->data;
}

void fatalError(char *msg){
    printf("Fatal Error: %s \n", msg);
    exit(1);
}

ObjRef newCompoundObject(int numObjRefs){
    ObjRef cmpObj;
    unsigned int objSize;
    objSize = sizeof(unsigned int) +
              numObjRefs * sizeof(unsigned char);
    if((cmpObj = malloc(objSize)) == NULL){
        perror("Error cmpObj malloc");
    }
    cmpObj->size = (numObjRefs|MSB); // MSB
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
ObjRef popObj(void){
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

    unsigned int opcode = opc >> 24;
    int input = SIGN_EXTEND(opc & 0x00FFFFFF);
    switch (opcode) {
        case 0: {
            printf("Ninja Virtual Machine stopped\n");
            exit(0);
            break;
        }
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
        case WRINT: {
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
            pushObj(stack[framePointer + input].u.objref);
            break;
        }

        case POPL: {
            stack[framePointer + input].u.objref = popObj();
            break;
        }

        case EQ: {
            int tmp;
            bip.op2 = popObj();
            bip.op1 = popObj();
            tmp = bigCmp();
            if(tmp == 0){
                bigFromInt(true);
                pushObj(bip.res);
            }else{
                bigFromInt(false);
                pushObj(bip.res);
            }
            break;
        }
        case NE: {
            int temp;
            bip.op2 = popObj();
            bip.op1 = popObj();
            temp = bigCmp();
            if(temp != 0){
                bigFromInt(true);
                pushObj(bip.res);
            }else {
                bigFromInt(false);
                pushObj(bip.res);
            }
            break;
        }
        case LT: {
            int temp;
            bip.op2 = popObj();
            bip.op1 = popObj();
            temp = bigCmp();
            if(temp < 0){
                bigFromInt(true);
                pushObj(bip.res);
            }else {
                bigFromInt(false);
                pushObj(bip.res);
            }
            break;
        }
        case LE: {
            int temp;
            bip.op2 = popObj();
            bip.op1 = popObj();
            temp = bigCmp();
            if(temp <= 0){
                bigFromInt(true);
                pushObj(bip.res);
            }else {
                bigFromInt(false);
                pushObj(bip.res);
            }
            break;
        }
        case GT: {
            int temp;
            bip.op2 = popObj();
            bip.op1 = popObj();
            temp = bigCmp();
            if(temp > 0){
                bigFromInt(true);
                pushObj(bip.res);
            }else {
                bigFromInt(false);
                pushObj(bip.res);
            }
            break;
        }
        case GE: {
            int temp;
            bip.op2 = popObj();
            bip.op1 = popObj();
            temp = bigCmp();
            if(temp >= 0){
                bigFromInt(true);
                pushObj(bip.res);
            }else {
                bigFromInt(false);
                pushObj(bip.res);
            }
            break;
        }
        case JMP: {
            programmCounter = input;
            break;
        }
        case BRF: {
            int temp;
            bip.op1 = popObj();
            temp = bigToInt();
            if(temp == false){
                programmCounter = input;
            }

            break;
        }

        case BRT: {
            int temp;
            bip.op1 = popObj();
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
        case NEW: {
            ObjRef o = newCompoundObject(input);
            pushObj(o);
            break;

        }
        case GETF: {
            ObjRef a = popObj();
            if(IS_PRIMITIVE(a) == 0 && GET_ELEMENT_COUNT(a) > input){
                pushObj(GET_REFS_PTR(a)[input]);
            } else {
                fatalError("ERROR: GETF no cmpobj ontop of stack\n");
            }
            break;
        }
        case PUTF: {
            ObjRef a = popObj();
            ObjRef b = popObj();
            GET_REFS_PTR(b)[input] = a;
            break;
        }
        case NEWA: {
            ObjRef obj = popObj();
            if(IS_PRIMITIVE(obj)){
                bip.op1 = obj;
                ObjRef cmpObj = newCompoundObject(bigToInt());
                pushObj(cmpObj);
            } else {
                fatalError("Object ontop of stack is ot primitive\n");
            }
            break;
        }
        case GETFA: {
            bip.op1 = popObj();
            ObjRef  a = popObj();
            pushObj(GET_REFS_PTR(a)[bigToInt()]);
            break;
        }
        case PUTFA: {
            ObjRef value = popObj();
            bip.op1 = popObj();
            ObjRef array = popObj();
            GET_REFS_PTR(array)[bigToInt()] = value;
            break;
        }
        case GETSZ: {
            ObjRef tmp = popObj();
            if(IS_PRIMITIVE(tmp)){
                bigFromInt(-1);
            } else{
                bigFromInt(GET_ELEMENT_COUNT(tmp));
            }
            pushObj(bip.res);
            break;
        }
        case PUSHN: {
            pushObj(NULL);
            break;
        }
        case REFEQ: {
            ObjRef a = popObj();
            ObjRef b = popObj();
            if(a == b){
                bigFromInt(1);
            } else{
                bigFromInt(0);
            }
            pushObj(bip.res);
            break;
        }
        case REFNE: {
            ObjRef a = popObj();
            ObjRef b = popObj();
            if(a != b){
                bigFromInt(1);
            } else{
                bigFromInt(0);
            }
            pushObj(bip.res);
            break;
        }
        default: {
            printf("default");
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
        programmCounter = programmCounter + 1;
        executeOP(ins);
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

        }
    }
    return 0;
}
