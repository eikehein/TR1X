#ifndef TR1MAIN_UTIL_H
#define TR1MAIN_UTIL_H

#define FEATURE_NOCD_DATA

#include <stdint.h>
#include <stdio.h>

#pragma pack(push, 1)
typedef struct {
    uint8_t opcode; // must be 0xE9
    uint32_t offset;
} JMP;
#pragma pack(pop)

#define TRACE(...)                                                             \
    {                                                                          \
        printf("%s:%d %s ", __FILE__, __LINE__, __func__);                     \
        printf(__VA_ARGS__);                                                   \
        printf("\n");                                                          \
        fflush(stdout);                                                        \
    }

#define VAR_U_(address, type) (*(type*)(address))
#define VAR_I_(address, type, value) (*(type*)(address))
#define ARRAY_(address, type, length) (*(type(*) length)(address))

void tr1m_inject_func(void* from, void* to);
void tr1m_print_stack_trace();

#define INJECT(from, to)                                                       \
    {                                                                          \
        tr1m_inject_func((void*)from, (void*)to);                              \
    }

#endif
