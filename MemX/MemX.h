//
//  MemX.h
//
//  Created by Aethereux on 3/9/25.
//
#pragma once

#include <cstdint>
#include <cstring> // For memcpy
#include <vector>
#include <string>
#include <unistd.h>

#include <Foundation/Foundation.h>
#include <mach-o/dyld.h>
#include <mach/mach.h>
#include <sys/mman.h>
#import <mach-o/getsect.h>


namespace MemX {
    /*
     Example Usage:
     uintptr_t base = MemX::GetImageBase("ShooterGame");
    */
    static uintptr_t GetImageBase(const std::string& imageName) {
        static uintptr_t imageBase = 0;
    
        if (imageBase) return imageBase;

        for (uint32_t i = 0; i < _dyld_image_count(); ++i) {
            const char* dyldImageName = _dyld_get_image_name(i);
            if (strstr(dyldImageName, imageName.c_str())) {
                imageBase = reinterpret_cast<uintptr_t>(_dyld_get_image_header(i));
                break;
            }
        }
        return imageBase;
    }

    inline bool IsValidPointer(uintptr_t address) {
        return address >= 0x100000000 && address < 0x3000000000;
    }

    inline bool _read(uintptr_t addr, void* buffer, size_t len) {
        if (!IsValidPointer(addr)) {
            return false;
        }

        std::memcpy(buffer, reinterpret_cast<void*>(addr), len);
        return true;
    }

    template <typename T>
    inline T Read(uintptr_t address) {
        T data{};
        if (!_read(address, &data, sizeof(T))) {
            return T{};
        }
        return data;
    }

    inline std::string ReadString(void* address, size_t max_len) {
        if (!IsValidPointer(reinterpret_cast<uintptr_t>(address))) {
            return "Invalid Pointer!!";
        }

        std::vector<char> chars(max_len + 1, '\0'); // Ensure null termination
        if (!_read(reinterpret_cast<uintptr_t>(address), chars.data(), max_len)) {
            return "";
        }

        return std::string(chars.data()); // Handles null termination properly
    }

    template <typename T>
    inline void Write(uintptr_t address, const T& value) {
        if (!IsValidPointer(address)) {
            return;
        }
        *reinterpret_cast<T*>(address) = value;
    }
} // namespace MemX
