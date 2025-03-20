# MemX - Memory Manipulation and VMT Hooking Utility

## Overview
MemX is a lightweight memory manipulation utility designed for reading, writing, and hooking functions in iOS applications. It provides efficient and safe methods to interact with memory while ensuring proper pointer validation and optimized memory operations.

## Features
- Retrieve the base address of a loaded image.
- Validate memory addresses to prevent crashes.
- Read raw memory into buffers.
- Read specific data types from memory.
- Read strings safely with null termination handling.
- Write values directly into memory.
- Virtual Method Table (VMT) hooking for function swapping.
- Safe invocation of original VMT functions.

## Usage
### Get Image Base Address
```cpp
uintptr_t base = MemX::GetImageBase("ShooterGame");
```
Retrieves the base address of the specified image.

### Read Memory
```cpp
int value = MemX::Read<int>(some_address);
std::string str = MemX::ReadString(reinterpret_cast<void*>(string_address), 32);
```
Reads a value of type `T` from the given memory address. Supports reading strings with length constraints.

### Write Memory
```cpp
MemX::Write<int>(some_address, 100);
```
Writes an integer value to the specified memory address.

### VMT Hooking Example
```cpp
#include "MemX.h"
#include "VMTHook.h"

static void MyControllerHook(UObject* object, UFunction* function, void* params);
static VMTHook<decltype(MyControllerHook)> ControllerHookInstance(&MyControllerHook, 69); // Index 69th of the VTable

// Hooked Function
static void MyControllerHook(UObject* object, UFunction* function, void* params) {
    string name = function->GetName();
    if (name == "ClientTravelInternal") {
        delayTrigger = true;
    }
  
    
    ControllerHookInstance.InvokeOriginal(object, function, params);
}

static void HookController() {
    uintptr_t MyController = GetMyController();
    if (!MemX::IsValidPointer(MyController)) return;
    // MyController is a Class Instance of APlayerController.
    // Swapping
    ControllerHookInstance.Swap(reinterpret_cast<void*>(MyController));
}
```
Swaps the function at index `69` in the VMT of `APlayerController` with `HookedFunction`, executes the function, and then restores the original function.

### Reset Hook
```cpp
hook.Reset(some_instance);
```
Restores the original VMT of `some_instance`.

### Invoke Original Function
```cpp
hook.InvokeOriginal();
```
Calls the original function before it was hooked.

### VMT Invoker Usage
```cpp
// Calling Process Event
// Assuming ProcessEvent is defined as: void ProcessEvent(uintptr_t Object, uintptr_t Function, void* Params);
VMTInvoker<void(uintptr_t, uintptr_t, void*)> invoker(some_instance, 69); // Assuming ProcessEvent is at 69th Index

// Call ProcessEvent via VMTInvoker
uintptr_t object = 0x12345678;
uintptr_t function = 0x87654321;
void* params = nullptr;

invoker.Invoke(object, function, params);
```
Creates a VMT invoker to call a function at index `69` of `some_instance`.

## Safety Considerations
- **Pointer Validation:** All memory accesses are validated using `IsValidPointer()` to ensure stability.
- **Buffer Protection:** `ReadString()` ensures null-terminated strings to prevent buffer overflows.
- **Restricted Address Range:** Ensures only safe memory regions are accessed within the valid iOS range.
- **Proper Hook Management:** Hooks should be reset when no longer needed to prevent instability.

## License
This utility is provided as-is without any warranties. Use it responsibly within legal and ethical boundaries.

## Disclaimer
Memory manipulation and function hooking may violate the terms of service of certain applications. The author is not responsible for any misuse of this utility.

