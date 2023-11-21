#include <stddef.h>
#include <stdint.h>

#include "address.h"
#include "keccak.h"
#include "uint256.h"

typedef uint32_t val_t[3];

typedef struct data {
    size_t size;
    uint8_t *content;
} data_t;

typedef struct storageChanges {
    uint256_t key;
    uint256_t before;
    uint256_t after;
    uint64_t warm;
    struct storageChanges *prev;
} storageChanges_t;

// state changes are reverted on failure and returned on success
typedef struct stateChanges {
    address_t account;
    // TODO balance change
    // TODO code change
    // TODO selfdestruct
    // TODO nonce
    // TODO warm
    storageChanges_t *storageChanges; // LIFO
    struct stateChanges *next;
} stateChanges_t;

typedef struct callResult {
    data_t returnData;
    // status is the result pushed onto the stack
    // for CREATE it is the address or zero on failure
    // for CALL it is 1 on success and 0 on failure
    uint256_t status;
    uint64_t gasRemaining;
    stateChanges_t *stateChanges;
} result_t;


void evmInit();
void evmFinalize();

#define EVM_DEBUG_STACK 1
#define EVM_DEBUG_MEMORY 2
#define EVM_DEBUG_OPS 4
#define EVM_DEBUG_GAS (EVM_DEBUG_OPS + 8)
void evmSetDebug(uint64_t flags);

void evmMockBalance(address_t to, val_t balance);
void evmMockCall(address_t to, val_t value, data_t inputData, result_t result);
void evmMockStorage(address_t to, uint256_t key, uint256_t storedValue);

result_t evmCall(address_t from, uint64_t gas, address_t to, val_t value, data_t input);
result_t evmCreate(address_t from, uint64_t gas, val_t value, data_t input);
// TODO gasPrice
result_t txCall(address_t from, uint64_t gas, address_t to, val_t value, data_t input);
result_t txCreate(address_t from, uint64_t gas, val_t value, data_t input);
