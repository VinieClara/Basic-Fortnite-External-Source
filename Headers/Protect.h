#include "Headers/include.h"
#include "Data/basic_data.h"
#include "font.h"
#include "icons.h"


typedef NTSTATUS(WINAPI* lpQueryInfo)(HANDLE, LONG, PVOID, ULONG, PULONG);

//all over the internet, i didnt make this
PVOID DetourFunc(BYTE* src, const BYTE* dst, const int len)
{
    BYTE* jmp = (BYTE*)malloc(len + 5); DWORD dwback;
    VirtualProtect(src, len, PAGE_EXECUTE_READWRITE, &dwback);
    memcpy(jmp, src, len); jmp += len; jmp[0] = 0xE9;

    *(DWORD*)(jmp + 1) = (DWORD)(src + len - jmp) - 5; src[0] = 0xE9;
    *(DWORD*)(src + 1) = (DWORD)(dst - src) - 5;

    VirtualProtect(src, len, dwback, &dwback);
    return (jmp - len);
}

//not proper way to detour, but since we arent continuing thread context we dont return context.
//to continue thread execution after detour do something like this I think
//void CaptureThread(PCONTEXT context, PVOID arg1, PVOID arg2)
//return (new ldrThunk) -> Thunk name(PCONTEXT context, PVOID arg1, PVOID arg2) <- current thread context.

void CaptureThread()
{
    //getting thread start address isnt needed, it just gives extra information on the thread stack which allows you to see some potential injection methods used
    auto ThreadStartAddr = [](HANDLE hThread) -> DWORD {

        //Hook NtQueryInformationThread
        lpQueryInfo ThreadInformation = (lpQueryInfo)GetProcAddress(GetModuleHandle("ntdll.dll"), "NtQueryInformationThread");

        DWORD StartAddress;
        //Get information from current thread handle
        ThreadInformation(hThread, 9, &StartAddress, sizeof(DWORD), NULL);

        return StartAddress;
    };

    //Gets handle of current thread. (HANDLE)(LONG_PTR)-1 is handle of CurrentProcess if you need it
    HANDLE CurrentThread = (HANDLE)(LONG_PTR)-2;
    //Gets thread information from thread handle.
    DWORD  StartAddress = ThreadStartAddr(CurrentThread);

    //address 0x7626B0E0 is a static address which is assigned to exit thread of the application
    //we need to whitelist it otherwise you cant close the application from usermode
    if (StartAddress != 0x7626B0E0) {
        printf("\n[+] Block [TID: %d][Start Address: %p]", (DWORD)GetThreadId(CurrentThread), (CHAR*)StartAddress);
        //Exits thread and stops potential code execution
        //if you dont term thread it will crash if you dont handle context properly
        if (!TerminateThread(CurrentThread, 0xC0C)) exit(0);
    }
    else exit(0);
}

BOOL HookLdrInitializeThunk()
{
    //Gets handle of ntdll.dll in the current process, which allows us to detour LdrInitializeThunk calls in given context
    HMODULE hModule = LoadLibraryA("ntdll.dll");
    if (hModule && (PBYTE)GetProcAddress(hModule, reinterpret_cast<LPCSTR>("LdrInitializeThunk")))
    {
        DetourFunc((PBYTE)GetProcAddress(hModule, "LdrInitializeThunk"), (PBYTE)CaptureThread, 5);
        return TRUE;
    }
    else return FALSE;
}