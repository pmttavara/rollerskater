#pragma once

#include "common.h"

// RDTSC instruction with full precision -- safe due to subprocess isolation
// 

// 32 bit version of most instructions for ARM32 support

// leverage powerful ops (CISC)
// this is how gpus work - everything is fma (except div)
// fma with bit in the opcode - "negate the addend/factor" (one bit flip)

// mem0   instr
// memcpy instr
// memset instr

// function call (call my_fn())
// syscall
// dynamic call (call rax)

// load fence
// store fence

// atomic clear/load/xchg/compxchg/test-and-set (single-bit compxchg)
// atomic add/and/xor/or (sub == add negated)

#define Op_values(X)                                                                               \
    X(Halt, )                                                                                      \
    X(Nop, )                                                                                       \
    X(Break, )                                                                                     \
                                                                                                   \
    X(Reserve, )                                                                                   \
    X(Commit, )                                                                                    \
    X(Decommit, )                                                                                  \
    X(Release, )                                                                                   \
                                                                                                   \
    X(JzRelU8, )                                                                                   \
    X(JzRelS8, )                                                                                   \
    X(JeRelU8, = JzRelU8)                                                                          \
    X(JeRelS8, = JzRelS8)                                                                          \
    X(JnzRelU8, )                                                                                  \
    X(JnzRelS8, )                                                                                  \
    X(JneRelU8, = JnzRelU8)                                                                        \
    X(JneRelS8, = JnzRelS8)                                                                        \
                                                                                                   \
    X(IncR0, )                                                                                     \
    X(IncR1, )                                                                                     \
    X(IncR2, )                                                                                     \
    X(IncR3, )                                                                                     \
    X(IncR4, )                                                                                     \
    X(IncR5, )                                                                                     \
    X(IncR6, )                                                                                     \
    X(IncR7, )                                                                                     \
                                                                                                   \
    X(MoveRegReg, )                                                                                \
    X(MoveMem8Reg, )                                                                               \
    X(MoveMem16Reg, )                                                                              \
    X(MoveMem32Reg, )                                                                              \
    X(MoveMem64Reg, )                                                                              \
    X(MoveRegMem8, )                                                                               \
    X(MoveRegMem16, )                                                                              \
    X(MoveRegMem32, )                                                                              \
    X(MoveRegMem64, )                                                                              \
    X(MoveImm8Reg, )                                                                               \
    X(MoveImm16Reg, )                                                                              \
    X(MoveImm32Reg, )                                                                              \
    X(MoveImm64Reg, )                                                                              \
                                                                                                   \
    X(AddRegReg, )                                                                                 \
    X(SubRegReg, )                                                                                 \
    X(CmpRegReg, )

EnumDef(Op, : u8);

#define IMM16(x) (((x) >> 0) & 0xff), (((x) >> 8) & 0xff)
#define IMM32(x)                                                                                   \
    (((x) >> 0) & 0xff), (((x) >> 8) & 0xff), (((x) >> 16) & 0xff), (((x) >> 24) & 0xff)
#define IMM64(x)                                                                                   \
    (((x) >> 0) & 0xff), (((x) >> 8) & 0xff), (((x) >> 16) & 0xff), (((x) >> 24) & 0xff),          \
        (((x) >> 32) & 0xff), (((x) >> 40) & 0xff), (((x) >> 48) & 0xff), (((x) >> 56) & 0xff)

#define ExceptionCode_values(X)                                                                    \
    X(None, )                                                                                      \
    X(IllegalInstruction, )                                                                        \
    X(Halt, )                                                                                      \
    X(Break, )                                                                                     \
    X(PageFault, )

EnumDef(ExceptionCode, );

struct Vm {
    bool single_step : 1;
    bool zero : 1;
    bool carry : 1;
    bool sign : 1;
    bool overflow : 1;
    u64 r0 = 0;
    u64 r1 = 0;
    u64 r2 = 0;
    u64 r3 = 0;
    u64 r4 = 0;
    u64 r5 = 0;
    u64 r6 = 0;
    u64 r7 = 0;
    u64 ri = 0;
    u64 cycle_count = 0;
};

struct Vm_Param {
    u64 n = 0;
};
