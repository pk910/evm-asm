#include "disassemble.h"
#include "hex.h"

#include <stdint.h>
#include <stdio.h>

static statement_stack_t stack;
static uint32_t pc;

typedef uint32_t pc_t;
VECTOR(pc, pcs);
static pcs_t jumpdests;

void disassembleInit() {
    pc = 0;
    pcs_init(&jumpdests, 8);
    statement_stack_init(&stack, 8);
}
static void disassembleWaste(const char **iter) {
    while (**iter && !isHex(**iter)) {
        (*iter)++;
    }
    if (**iter == '0' && *(*iter + 1) == 'x') {
        (*iter) += 2;
        disassembleWaste(iter);
    }
}
static void disassemblePushDecimal(op_t op, uint8_t pushlen, const char **iter) {
    uint64_t value = 0;
    while (pushlen) {
        disassembleWaste(iter);
        value *= 16;
        value += hexString8ToUint8(*(*iter)++);
        disassembleWaste(iter);
        value *= 16;
        value += hexString8ToUint8(*(*iter)++);
        pushlen--;
        pc++;
    }
    size_t bufLength = 20;
    char *str = (char *)calloc(bufLength, 1);
    int strLength = snprintf(str, bufLength, "%llu", value);
    statement_t pushDec = {
        strLength,
        bufLength,
        str
    };
    statement_stack_append(&stack, pushDec);
}
static void disassemblePushHex(op_t op, uint8_t pushlen, const char **iter) {
    size_t strLength = 2 * pushlen + 2;
    char *str = (char *)calloc(strLength, 1);
    str[0] = '0';
    str[1] = 'x';
    uint8_t i = 2;
    while (pushlen) {
        disassembleWaste(iter);
        str[i++] = *(*iter)++;
        disassembleWaste(iter);
        str[i++] = *(*iter)++;
        pushlen--;
        pc++;
    }
    statement_t pushHex = {
        strLength,
        strLength,
        str
    };
    statement_stack_append(&stack, pushHex);
}


statement_t labelForPc(pc_t pc) {
    const uint32_t bufLen = 8;
    char *str = calloc(bufLen, 1);
    uint32_t strLen = (uint32_t) snprintf(str, bufLen, "%u:", pc);
    /*  TODO label
    for (uint8_t i = 0; i < strLen - 1; i++) {
        str[i] += 'a' - '0';
    }
    */
    return (statement_t) {
        strLen,
        bufLen,
        str,
    };
}

static void disassemblePush(op_t op, const char **iter) {
    uint8_t pushlen = op - PUSH1 + 1;
    if (pushlen <= 8) {
        disassemblePushDecimal(op, pushlen, iter);
    } else {
        disassemblePushHex(op, pushlen, iter);
    }
}

void disassembleNextOp(const char **iter) {
    disassembleWaste(iter);
    op_t op = hexString16ToUint8(*iter);
    *iter += 2;
    if (op >= PUSH1 && op <= PUSH32) {
        pc++;
        disassemblePush(op, iter);
        return;
    }
    statement_t op_statement;
    if (op == JUMPDEST) {
        pcs_append(&jumpdests, pc);
        op_statement = labelForPc(pc);
    } else {
        uint32_t bufLen = 15;
        char *str = calloc(bufLen, 1);
        char *end = stpncpy(str, opString[op], bufLen);
        uint32_t strLen = end - str;
        op_statement = (statement_t){
            strLen,
            bufLen,
            str
        };
    }
    int argc = argCount[op];
    if (argc && argc <= stack.num_statements) {
        statement_append(&op_statement, '(');
        while (argc--) {
            statement_t arg = statement_stack_pop(&stack);
            for (uint32_t i = 0; i < arg.num_schars; i++) {
                statement_append(&op_statement,arg.schars[i]);
            }
            free(arg.schars);
            if (argc) {
                statement_append(&op_statement,',');
            }
        }
        statement_append(&op_statement, ')');
    }
    statement_stack_append(&stack, op_statement);
    pc++;
}

int disassembleValid(const char **iter) {
    disassembleWaste(iter);
    return **iter;
}

void disassembleFinalize() {
    for (size_t j = 0; j < jumpdests.num_pcs; j++) {
        uint32_t jumpdest = jumpdests.pcs[j]; 
        for (size_t i = 0; i < stack.num_statements; i++) {
            // TODO scan for label
        }
    }
    for (size_t i = 0; i < stack.num_statements; i++) {
        puts(stack.statements[i].schars);
    }
}
