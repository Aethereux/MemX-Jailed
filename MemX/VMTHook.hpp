#pragma once

#include <cstdlib>
#include <cstring>
#include <cstdint>
#include <utility>
#include <mach/mach.h>
#include <sys/mman.h>
#include <unistd.h>

template<typename FuncType>
class VMTHook
{
private:
    void** OriginalVTable;
    void** DetourVTable;
    FuncType* OriginalFunction;
    FuncType* NewFunction;
    int32_t FunctionIndex;
    void* HookedInstance; 

    int GetNumMethods(void** VTable) const
    {
        int Count = 0;
        while (VTable[Count]) { ++Count; }
        return Count;
    }

    bool IsValidPointer(uintptr_t address) {
        return address >= 0x100000000 && address < 0x3000000000;
    }

public:
    VMTHook(FuncType* NewFunc, int32_t Index) : NewFunction(NewFunc), FunctionIndex(Index), HookedInstance(nullptr) {}

    void Swap(void* Class)
    {
        if (!Class || !NewFunction)
            return;

        void** VTable = *reinterpret_cast<void***>(Class);
        if (!VTable || VTable[FunctionIndex] == NewFunction || !IsValidPointer(reinterpret_cast<uintptr_t>(VTable)))
            return;

        // If the instance changed, reset first
        if (HookedInstance && HookedInstance != Class)
        {
            Reset(HookedInstance);
        }

        int MethodCount = GetNumMethods(VTable);
        DetourVTable = static_cast<void**>(malloc(MethodCount * sizeof(void*)));
        if (!DetourVTable)
            return;

        memcpy(DetourVTable, VTable, MethodCount * sizeof(void*));
        OriginalVTable = VTable;
        OriginalFunction = reinterpret_cast<FuncType*>(DetourVTable[FunctionIndex]);
        DetourVTable[FunctionIndex] = reinterpret_cast<void*>(NewFunction);
        *(void***)Class = DetourVTable;
        HookedInstance = Class;
    }

    void Reset(void* Class)
    {
        if (Class && OriginalVTable)
        {
            *(void***)Class = OriginalVTable;
        }
        if (DetourVTable)
        {
            free(DetourVTable);
            DetourVTable = nullptr;
        }
        OriginalVTable = nullptr;
        HookedInstance = nullptr;
    }

    template<typename... Args>
    auto InvokeOriginal(Args&&... args) const -> decltype(auto)
    {
        return (*OriginalFunction)(std::forward<Args>(args)...);
    }
};
