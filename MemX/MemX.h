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
    struct AddrRange {
        uintptr_t start;
        uintptr_t end;
    };
     
    // From Titanox's version
    // https://developer.apple.com/documentation/kernel/mach_header/
    inline const std::vector<AddrRange>& GetFullAddr() {
        static std::vector<AddrRange> ranges;
        // we just need to get ranges once
        // calling over n over is redundant
        if (!ranges.empty()) {
            return ranges;
        }

        for (uint32_t i = 0; i < _dyld_image_count(); ++i) {
            const mach_header* header = _dyld_get_image_header(i);
            intptr_t slide = _dyld_get_image_vmaddr_slide(i);
            if (!header) continue;

            const uint8_t* ptr = reinterpret_cast<const uint8_t*>(header);
            const load_command* cmd = nullptr;
            uint32_t ncmds = 0;

            switch (header->magic) {
                // ncmds -> https://developer.apple.com/documentation/kernel/mach_header/1525650-ncmds
                //64-bit
                case MH_MAGIC_64: {
                    const auto* hdr = reinterpret_cast<const mach_header_64*>(ptr);
                    cmd = reinterpret_cast<const load_command*>(hdr + 1);
                    ncmds = hdr->ncmds;
                    break;
                }
                //32-bit
                case MH_MAGIC: {
                    const auto* hdr = reinterpret_cast<const mach_header*>(ptr);
                    cmd = reinterpret_cast<const load_command*>(hdr + 1);
                    ncmds = hdr->ncmds;
                    break;
                }
                default:
                    continue;
            }

            for (uint32_t j = 0; j < ncmds; ++j) {
                switch (cmd->cmd) {
                    // https://developer.apple.com/documentation/kernel/segment_command_64
                    // goes through the load commands
                    case LC_SEGMENT_64: {
                        const auto* seg = reinterpret_cast<const segment_command_64*>(cmd);
                        uintptr_t start = static_cast<uintptr_t>(seg->vmaddr + slide); // ASLR start
                        uintptr_t end = start + static_cast<uintptr_t>(seg->vmsize); // vmsize is end
                        ranges.push_back({start, end});
                        break;
                    }
                    default:
                        break;
                }
                cmd = reinterpret_cast<const load_command*>(
                    reinterpret_cast<const uint8_t*>(cmd) + cmd->cmdsize);
            }
        }
        return ranges;
    }
    
    // better 'IsValidPointer'
    inline bool IsValidPointer(uintptr_t addr) {
        const auto& ranges = GetFullAddr();
        for (const auto& r : ranges) {
            if (addr >= r.start && addr < r.end) {
                return true;
            }
        }
        return false;
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
