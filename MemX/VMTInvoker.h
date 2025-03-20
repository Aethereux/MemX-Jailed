//
//  VMTInvoker.h
//  AsuraUME
//
//  Created by Euclid Jan Guillermo on 3/12/25.
//

#pragma once

#include <cstdint>
#include <mach/mach.h>
#include <sys/mman.h>
#include <unistd.h>
#include <utility>

// Base VMT Invoker
template<typename FuncType>
class VMTInvokerBase
{
protected:
    void* Instance;
    int32_t FunctionIndex;
    FuncType* OriginalFunction;

public:
    VMTInvokerBase(void* instance, int32_t index) : Instance(instance), FunctionIndex(index), OriginalFunction(nullptr) {}

    template<typename... Args>
    auto Invoke(Args&&... args) const -> decltype(auto)
    {
        return (*OriginalFunction)(std::forward<Args>(args)...);
    }
};

template<typename FuncType>
class VMTInvokerNoAPI : public VMTInvokerBase<FuncType>
{
public:
    using VMTInvokerBase<FuncType>::Instance;
    using VMTInvokerBase<FuncType>::FunctionIndex;
    using VMTInvokerBase<FuncType>::OriginalFunction;

    VMTInvokerNoAPI(void* instance, int32_t index) : VMTInvokerBase<FuncType>(instance, index)
    {
        void** VTable = *reinterpret_cast<void***>(instance);
        OriginalFunction = reinterpret_cast<FuncType*>(VTable[FunctionIndex]);
    }
};
