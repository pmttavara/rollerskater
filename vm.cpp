
#include "common.h"
#include "vm_defs.hpp"

static void update_flags(Vm *v, u64 val) {
    v->zero = val == 0;
    v->sign = val & (1ull << 63);
}

static u64 reg_from_i(Vm *v, u8 i) {
    switch (i) {
    case 0:
        return v->r0;
    case 1:
        return v->r1;
    case 2:
        return v->r2;
    case 3:
        return v->r3;
    case 4:
        return v->r4;
    case 5:
        return v->r5;
    case 6:
        return v->r6;
    case 7:
        return v->r7;
    }
    return 0;
}

static u64 set_reg_from_i(Vm *v, u8 i, u64 val) {
    switch (i) {
    case 0:
        return v->r0 = val;
    case 1:
        return v->r1 = val;
    case 2:
        return v->r2 = val;
    case 3:
        return v->r3 = val;
    case 4:
        return v->r4 = val;
    case 5:
        return v->r5 = val;
    case 6:
        return v->r6 = val;
    case 7:
        return v->r7 = val;
    }
    return 0;
}

static ExceptionCode run(Vm *v, void *ops_, u64 n) {
    using namespace Op_;
    ExceptionCode result = ExceptionCode::None;
    Op *ops = (Op *)ops_;
    u64 i = v->ri;
    // u64 cycles_done
    for (; i < n; i++) {
        ++v->cycle_count;
        Op op = (Op)ops[i];
        v->ri = i;

#define ImplInc(op_name, reg)                                                                      \
    if (op == (op_name))                                                                           \
        do {                                                                                       \
            update_flags(v, ++(v->reg));                                                           \
            v->overflow = v->zero;                                                                 \
    } while (0)

        if (op == Halt) {
            return ExceptionCode::Halt;
        } else if (op == Nop) {
        } else if (op == Break) {
            v->ri = ++i;
            return ExceptionCode::Break;
        } else if (op == Reserve) {
            if (i + 1 >= n) {
                return ExceptionCode::PageFault;
            }
            u8 reg = ops[++i];
            if (reg > 7) {
                return ExceptionCode::IllegalInstruction;
            }
            u64 size = reg_from_i(v, reg);
            update_flags(v, v->r0 = (u64)VirtualAlloc(NULL, size, MEM_RESERVE, PAGE_READWRITE));
        } else if (op == Commit) {
            if (i + 2 >= n) {
                return ExceptionCode::PageFault;
            }
            u8 reg_addr = ops[++i];
            if (reg_addr > 7) {
                return ExceptionCode::IllegalInstruction;
            }
            u8 reg_size = ops[++i];
            if (reg_size > 7) {
                return ExceptionCode::IllegalInstruction;
            }
            u64 addr = reg_from_i(v, reg_addr);
            u64 size = reg_from_i(v, reg_size);
            update_flags(v,
                         v->r0 = (u64)VirtualAlloc((void *)addr, size, MEM_COMMIT, PAGE_READWRITE));
        } else if (op == Decommit) {
            if (i + 2 >= n) {
                return ExceptionCode::PageFault;
            }
            u8 reg_addr = ops[++i];
            if (reg_addr > 7) {
                return ExceptionCode::IllegalInstruction;
            }
            u8 reg_size = ops[++i];
            if (reg_size > 7) {
                return ExceptionCode::IllegalInstruction;
            }
            u64 addr = reg_from_i(v, reg_addr);
            u64 size = reg_from_i(v, reg_size);
            update_flags(v, v->r0 = (u64)VirtualFree((void *)addr, size, MEM_DECOMMIT));
        } else if (op == Release) {
            if (i + 1 >= n) {
                return ExceptionCode::PageFault;
            }
            u8 reg_addr = ops[++i];
            if (reg_addr > 7) {
                return ExceptionCode::IllegalInstruction;
            }
            u64 addr = reg_from_i(v, reg_addr);
            update_flags(v, v->r0 = (u64)VirtualFree((void *)addr, 0, MEM_RELEASE));
        }

        else if (op == JzRelU8) {
            if (i + 1 + 8 / 8 >= n) {
                return ExceptionCode::PageFault;
            }
            u8 rel = *(u8 *)&ops[++i];
            u64 dst = i + 1 + rel;
            if (dst > n) {
                return ExceptionCode::PageFault;
            }
            // TODO: statically analyze jump targets
            if (v->zero) {
                i = dst - 1;
            }
        } else if (op == JzRelS8) {
            if (i + 1 + 8 / 8 >= n) {
                return ExceptionCode::PageFault;
            }
            s8 rel = *(s8 *)&ops[++i];
            u64 dst = i + 1 + rel;
            if (dst > n) {
                return ExceptionCode::PageFault;
            }
            // TODO: statically analyze jump targets
            if (v->zero) {
                i = dst - 1;
            }
        } else
            ImplInc(IncR0, r0);
        else ImplInc(IncR1, r1);
        else ImplInc(IncR2, r2);
        else ImplInc(IncR3, r3);
        else ImplInc(IncR4, r4);
        else ImplInc(IncR5, r5);
        else ImplInc(IncR6, r6);
        else ImplInc(IncR7, r7);
        else if (op == MoveRegReg) {
            if (i + 1 + 1 >= n) {
                return ExceptionCode::PageFault;
            }
            u8 reg_src = ops[++i];
            if (reg_src > 7) {
                return ExceptionCode::IllegalInstruction;
            }
            u8 reg_dst = ops[++i];
            if (reg_dst > 7) {
                return ExceptionCode::IllegalInstruction;
            }
            update_flags(v, set_reg_from_i(v, reg_dst, reg_from_i(v, reg_src)));
        }

#define ImplArithRegReg(opcode, eval)                                                              \
    if (op == opcode)                                                                              \
        do {                                                                                       \
            if (i + 1 + 1 >= n) {                                                                  \
                return ExceptionCode::PageFault;                                                   \
            }                                                                                      \
            u8 reg_lhs = ops[++i];                                                                 \
            if (reg_lhs > 7) {                                                                     \
                return ExceptionCode::IllegalInstruction;                                          \
            }                                                                                      \
            u64 lhs = reg_from_i(v, reg_lhs);                                                      \
            u8 reg_rhs = ops[++i];                                                                 \
            if (reg_rhs > 7) {                                                                     \
                return ExceptionCode::IllegalInstruction;                                          \
            }                                                                                      \
            u64 rhs = reg_from_i(v, reg_rhs);                                                      \
            update_flags(v, (eval));                                                               \
    } while (0)

        else ImplArithRegReg(AddRegReg, set_reg_from_i(v, reg_lhs, lhs + rhs));
        else ImplArithRegReg(SubRegReg, set_reg_from_i(v, reg_lhs, lhs - rhs));
        else ImplArithRegReg(CmpRegReg, lhs - rhs);

#define ImplMoveMemReg(opcode, N, eval)                                                            \
    if (op == opcode)                                                                              \
        do {                                                                                       \
            if (i + 1 + 1 >= n) {                                                                  \
                return ExceptionCode::PageFault;                                                   \
            }                                                                                      \
            u8 reg_mem = ops[++i];                                                                 \
            if (reg_mem > 7) {                                                                     \
                return ExceptionCode::IllegalInstruction;                                          \
            }                                                                                      \
            u64 mem = reg_from_i(v, reg_mem);                                                      \
            u8 reg = ops[++i];                                                                     \
            if (reg > 7) {                                                                         \
                return ExceptionCode::IllegalInstruction;                                          \
            }                                                                                      \
            u##N *val = (u##N *)mem;                                                               \
            update_flags(v, (eval));                                                               \
    } while (0)

        else ImplMoveMemReg(MoveMem8Reg, 8, set_reg_from_i(v, reg, *val));
        else ImplMoveMemReg(MoveMem16Reg, 16, set_reg_from_i(v, reg, *val));
        else ImplMoveMemReg(MoveMem32Reg, 32, set_reg_from_i(v, reg, *val));
        else ImplMoveMemReg(MoveMem64Reg, 64, set_reg_from_i(v, reg, *val));
        else ImplMoveMemReg(MoveRegMem8, 8, *val = (u8)reg_from_i(v, reg));
        else ImplMoveMemReg(MoveRegMem16, 16, *val = (u16)reg_from_i(v, reg));
        else ImplMoveMemReg(MoveRegMem32, 32, *val = (u32)reg_from_i(v, reg));
        else ImplMoveMemReg(MoveRegMem64, 64, *val = (u64)reg_from_i(v, reg));

#define ImplMoveImmReg(N)                                                                          \
    if (op == MoveImm##N##Reg)                                                                     \
        do {                                                                                       \
            if (i + 1 + N / 8 >= n) {                                                              \
                return ExceptionCode::PageFault;                                                   \
            }                                                                                      \
            u8 reg_dst = ops[++i];                                                                 \
            if (reg_dst > 7) {                                                                     \
                return ExceptionCode::IllegalInstruction;                                          \
            }                                                                                      \
            u##N imm = *(u##N *)&ops[i + 1];                                                       \
            i += N / 8;                                                                            \
            update_flags(v, set_reg_from_i(v, reg_dst, imm));                                      \
    } while (0)

        else ImplMoveImmReg(8);
        else ImplMoveImmReg(16);
        else ImplMoveImmReg(32);
        else ImplMoveImmReg(64);

#define ImplAddImmReg(N)                                                                           \
    if (op == MoveImm##N##Reg)                                                                     \
        do {                                                                                       \
            __debugbreak();                                                                        \
    } while (0)

        else if (op == AddRegReg) {
            __debugbreak();
        }

        else ImplAddImmReg(8);
        else ImplAddImmReg(16);
        else ImplAddImmReg(32);
        else ImplAddImmReg(64);
        else __debugbreak();

        if (v->single_step) {
            v->ri = ++i;
            return ExceptionCode::Break;
        }
    }
    v->ri = i;
    return ExceptionCode::None;
}

static u32 format_reg(char (&b)[1024], u64 x) {
    b[0] = 0;
    wsprintfA(b, "%016I64x", x);
    u32 result = 0;
    while (b[result]) {
        ++result;
    }
    return result;
}
static u32 format_byte(char (&b)[1024], u8 x) {
    b[0] = 0;
    wsprintfA(b, "%02x", x);
    u32 result = 0;
    while (b[result]) {
        ++result;
    }
    return result;
}
static u32 format_bool(char (&b)[1024], bool x) {
    b[0] = '0' + !!x;
    b[1] = 0;
    return 1;
}

static void format_mem(char (&b)[1024], u64 reg) {
    log(" [");
    for (s64 i = -4; i <= +4; ++i) {
        u64 addr = (reg + i);
        MEMORY_BASIC_INFORMATION mbi = {};
        VirtualQuery((void *)addr, &mbi, sizeof(mbi));
        if (mbi.State == MEM_COMMIT &&
            (mbi.Protect == PAGE_READWRITE || mbi.Protect == PAGE_EXECUTE_READ ||
             mbi.Protect == PAGE_EXECUTE_READWRITE)) {
            logn(b, format_byte(b, *(u8 *)addr));
        } else {
            log("??");
        }
        if (i == -1) {
            log("[");
        } else if (i == 0) {
            log("]");
        } else {
            log(" ");
        }
    }
    log("]");
}

static void dump(Vm *v, Op *ops) {
    char b[1024];
    // log("&dump:  ");
    // format_mem(b, (u64)&dump);
    // log("\n");
    log("r0:  ");
    logn(b, format_reg(b, v->r0));
    format_mem(b, v->r0);
    log("\nr1:  ");
    logn(b, format_reg(b, v->r1));
    format_mem(b, v->r1);
    log("\nr2:  ");
    logn(b, format_reg(b, v->r2));
    format_mem(b, v->r2);
    log("\nr3:  ");
    logn(b, format_reg(b, v->r3));
    format_mem(b, v->r3);
    log("\nr4:  ");
    logn(b, format_reg(b, v->r4));
    format_mem(b, v->r4);
    log("\nr5:  ");
    logn(b, format_reg(b, v->r5));
    format_mem(b, v->r5);
    log("\nr6:  ");
    logn(b, format_reg(b, v->r6));
    format_mem(b, v->r6);
    log("\nr7:  ");
    logn(b, format_reg(b, v->r7));
    format_mem(b, v->r7);
    log("\n-----------------------\n");
    log("ri:  ");
    logn(b, format_reg(b, v->ri));
    format_mem(b, (u64)ops + v->ri);
    log("\nflags:\n");
    log("  single_step:");
    logn(b, format_bool(b, v->single_step));
    log("\n  zero:");
    logn(b, format_bool(b, v->zero));
    log("\n  carry:");
    logn(b, format_bool(b, v->carry));
    log("\n  sign:");
    logn(b, format_bool(b, v->sign));
    log("\n  overflow:");
    logn(b, format_bool(b, v->overflow));
    log("\n-----------------------\n");
}

EXCEPTION_DISPOSITION
__C_specific_handler(struct _EXCEPTION_RECORD *ExceptionRecord, void *EstablisherFrame,
                     struct _CONTEXT *ContextRecord,
                     struct _DISPATCHER_CONTEXT *DispatcherContext) {
    typedef EXCEPTION_DISPOSITION Function(struct _EXCEPTION_RECORD *, void *, struct _CONTEXT *,
                                           _DISPATCHER_CONTEXT *);
    static Function *FunctionPtr;

    if (!FunctionPtr) {
        HMODULE Library = LoadLibraryA("msvcrt.dll");
        FunctionPtr = (Function *)GetProcAddress(Library, "__C_specific_handler");
    }

    return FunctionPtr(ExceptionRecord, EstablisherFrame, ContextRecord, DispatcherContext);
}

extern "C" {
extern __declspec(dllexport) DWORD run_vm(void *userdata) {
    Vm_Param param = *(Vm_Param *)userdata;
    Op *ops = (Op *)((Vm_Param *)userdata + 1);
    u64 n = param.n;
    Vm v_ = {};
    Vm *v = &v_;
    // v->single_step = true;
    while (v->ri < n) {
        ExceptionCode exception = ExceptionCode::Halt;
        // __try {
        exception = run(v, ops, n);
        // } __except (EXCEPTION_EXECUTE_HANDLER) {
        //     exception = ExceptionCode::PageFault;
        // }
        dump(v, (Op *)&ops[0]);
        if (exception == ExceptionCode::IllegalInstruction) {
            log("Illegal instruction!\n");
            break;
        }
        if (exception == ExceptionCode::Halt) {
            log("Halt!\n");
            break;
        }
        if (exception == ExceptionCode::Break) {
            log("Debug break.\n");
        }
        if (exception == ExceptionCode::PageFault) {
            log("Page fault!\n");
            break;
        }
    }
    log("Done.\n");
    return 0;
}
}

void mainCRTStartup() { ExitProcess(-1); }
void WinMainCRTStartup() { return mainCRTStartup(); }
