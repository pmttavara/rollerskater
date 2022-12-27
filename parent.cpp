
#include "common.h"
#include "vm_defs.hpp"

#define assert(e) ((e) || (__debugbreak(), 0))

int main() {
    STARTUPINFOA sia = {sizeof(sia)};
    sia.dwFlags = STARTF_USESTDHANDLES;
    sia.hStdInput = GetStdHandle(STD_INPUT_HANDLE); // @Leak: CloseHandle()
    sia.hStdOutput = GetStdHandle(STD_ERROR_HANDLE);
    sia.hStdError = GetStdHandle(STD_OUTPUT_HANDLE);
    PROCESS_INFORMATION pi = {};
    assert(CreateProcessA("./vm.exe", NULL, NULL, NULL, TRUE,
                       CREATE_SUSPENDED | DEBUG_ONLY_THIS_PROCESS, (void *)"\0", NULL, &sia, &pi));
    HMODULE module = LoadLibraryA("./vm.exe");
    DWORD (*run_vm)(void *) = (DWORD (*)(void *))GetProcAddress(module, "run_vm");
    assert(run_vm);

    using namespace Op_;
    u8 ops[] = {
        // clang-format off
        MoveImm32Reg, 2, IMM32(65536),
        Reserve, 2,
        MoveRegReg, 0, 1,
        Commit, 1, 2,
        // MoveRegMem64, 0, 3, IncR3, IncR0,
        // MoveRegMem64, 0, 3, IncR3, IncR0,
        // MoveRegMem64, 0, 3, IncR3, IncR0,
        // MoveRegMem64, 0, 3, IncR3, IncR0,
        // MoveRegMem64, 0, 3, IncR3, IncR0,
        // MoveRegMem64, 0, 3, IncR3, IncR0,
        // MoveRegMem64, 0, 3, IncR3, IncR0,
        // MoveRegMem64, 0, 3, IncR3, IncR0,
        // MoveRegMem64, 0, 3, IncR3, IncR0,
        CmpRegReg, 0, 1,
        Break,
        JzRelS8, (u8)+0,
        Decommit, 1, 2,
        Release, 1,
        Halt,
        // clang-format on
    };
    u64 n = sizeof(ops);
    Vm_Param vm_param = {n};
    void *remote_buffer = VirtualAllocEx(pi.hProcess, NULL, sizeof(vm_param) + n,
                                         MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    assert(remote_buffer);
    SIZE_T num_written = 0;
    assert(
        WriteProcessMemory(pi.hProcess, remote_buffer, &vm_param, sizeof(vm_param), &num_written) &&
        num_written == sizeof(vm_param));
    assert(WriteProcessMemory(pi.hProcess, (u8 *)remote_buffer + sizeof(vm_param), ops, n,
                              &num_written) &&
           num_written == n);
    remote_buffer =
        VirtualAllocEx(pi.hProcess, remote_buffer, sizeof(vm_param) + n, MEM_COMMIT, PAGE_READONLY);
    assert(remote_buffer);

    DWORD tid = 0;
    assert(CreateRemoteThread(pi.hProcess, NULL, 65536, run_vm, remote_buffer, 0, &tid));
    DWORD err = GetLastError();
    // ResumeThread(pi.hThread);
    DEBUG_EVENT de = {};

#define printf(...)
    for (;;) {
        if (!WaitForDebugEvent(&de, 1000 / 64)) {
            DebugBreakProcess(pi.hProcess);
            if (!WaitForDebugEvent(&de, 1)) {
                __debugbreak();
                break;
            }
        }
        printf("\n\n");
        if (de.dwDebugEventCode == CREATE_PROCESS_DEBUG_EVENT) {
            printf("Process created\n");
        } else if (de.dwDebugEventCode == CREATE_THREAD_DEBUG_EVENT) {
            printf("Thread created\n");
        } else if (de.dwDebugEventCode == EXCEPTION_DEBUG_EVENT) {
            printf("EXCEPTION!!! Code: %lx (", de.u.Exception.ExceptionRecord.ExceptionCode);
            if (de.u.Exception.ExceptionRecord.ExceptionCode == EXCEPTION_ACCESS_VIOLATION) { printf("EXCEPTION_ACCESS_VIOLATION"); }
            else if (de.u.Exception.ExceptionRecord.ExceptionCode == EXCEPTION_ARRAY_BOUNDS_EXCEEDED) { printf("EXCEPTION_ARRAY_BOUNDS_EXCEEDED"); }
            else if (de.u.Exception.ExceptionRecord.ExceptionCode == EXCEPTION_BREAKPOINT) { printf("EXCEPTION_BREAKPOINT"); }
            else if (de.u.Exception.ExceptionRecord.ExceptionCode == EXCEPTION_DATATYPE_MISALIGNMENT) { printf("EXCEPTION_DATATYPE_MISALIGNMENT"); }
            else if (de.u.Exception.ExceptionRecord.ExceptionCode == EXCEPTION_FLT_DENORMAL_OPERAND) { printf("EXCEPTION_FLT_DENORMAL_OPERAND"); }
            else if (de.u.Exception.ExceptionRecord.ExceptionCode == EXCEPTION_FLT_DIVIDE_BY_ZERO) { printf("EXCEPTION_FLT_DIVIDE_BY_ZERO"); }
            else if (de.u.Exception.ExceptionRecord.ExceptionCode == EXCEPTION_FLT_INEXACT_RESULT) { printf("EXCEPTION_FLT_INEXACT_RESULT"); }
            else if (de.u.Exception.ExceptionRecord.ExceptionCode == EXCEPTION_FLT_INVALID_OPERATION) { printf("EXCEPTION_FLT_INVALID_OPERATION"); }
            else if (de.u.Exception.ExceptionRecord.ExceptionCode == EXCEPTION_FLT_OVERFLOW) { printf("EXCEPTION_FLT_OVERFLOW"); }
            else if (de.u.Exception.ExceptionRecord.ExceptionCode == EXCEPTION_FLT_STACK_CHECK) { printf("EXCEPTION_FLT_STACK_CHECK"); }
            else if (de.u.Exception.ExceptionRecord.ExceptionCode == EXCEPTION_FLT_UNDERFLOW) { printf("EXCEPTION_FLT_UNDERFLOW"); }
            else if (de.u.Exception.ExceptionRecord.ExceptionCode == EXCEPTION_ILLEGAL_INSTRUCTION) { printf("EXCEPTION_ILLEGAL_INSTRUCTION"); }
            else if (de.u.Exception.ExceptionRecord.ExceptionCode == EXCEPTION_IN_PAGE_ERROR) { printf("EXCEPTION_IN_PAGE_ERROR"); }
            else if (de.u.Exception.ExceptionRecord.ExceptionCode == EXCEPTION_INT_DIVIDE_BY_ZERO) { printf("EXCEPTION_INT_DIVIDE_BY_ZERO"); }
            else if (de.u.Exception.ExceptionRecord.ExceptionCode == EXCEPTION_INT_OVERFLOW) { printf("EXCEPTION_INT_OVERFLOW"); }
            else if (de.u.Exception.ExceptionRecord.ExceptionCode == EXCEPTION_INVALID_DISPOSITION) { printf("EXCEPTION_INVALID_DISPOSITION"); }
            else if (de.u.Exception.ExceptionRecord.ExceptionCode == EXCEPTION_NONCONTINUABLE_EXCEPTION) { printf("EXCEPTION_NONCONTINUABLE_EXCEPTION"); }
            else if (de.u.Exception.ExceptionRecord.ExceptionCode == EXCEPTION_PRIV_INSTRUCTION) { printf("EXCEPTION_PRIV_INSTRUCTION"); }
            else if (de.u.Exception.ExceptionRecord.ExceptionCode == EXCEPTION_SINGLE_STEP) { printf("EXCEPTION_SINGLE_STEP"); }
            else if (de.u.Exception.ExceptionRecord.ExceptionCode == EXCEPTION_STACK_OVERFLOW) { printf("EXCEPTION_STACK_OVERFLOW"); }
            printf(")\n");
            // __debugbreak();
            // WaitForSingleObject(pi.hProcess, 2000); // Wait for child to die, but not forever.
            // break;
            if (de.u.Exception.ExceptionRecord.ExceptionCode != EXCEPTION_BREAKPOINT) {
                DebugActiveProcessStop(GetProcessId(pi.hProcess)); // Detach
                break;
            }
        } else if (de.dwDebugEventCode == EXIT_PROCESS_DEBUG_EVENT) {
            printf("Process exited\n");
        } else if (de.dwDebugEventCode == EXIT_THREAD_DEBUG_EVENT) {
            printf("Thread exited\n");
            if (de.dwThreadId == tid) {
                break;
            }
        } else if (de.dwDebugEventCode == LOAD_DLL_DEBUG_EVENT) {
            printf("DLL loaded\n");
        } else if (de.dwDebugEventCode == OUTPUT_DEBUG_STRING_EVENT) {
            printf("OutputDebugString\n");
        } else if (de.dwDebugEventCode == RIP_EVENT) {
            printf("\"Rip event\" whatever that is\n");
        } else if (de.dwDebugEventCode == UNLOAD_DLL_DEBUG_EVENT) {
            printf("DLL unloaded\n");
        } else {
            __debugbreak();
        }
        ContinueDebugEvent(de.dwProcessId, de.dwThreadId, DBG_CONTINUE);
    }
}
