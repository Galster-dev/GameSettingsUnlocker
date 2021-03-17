#pragma once

#include <Windows.h>
#include <Psapi.h>
#include <vector>
#include <string>
#include <iostream>

#define GREEN_TEXT 0xA
#define WHITE_TEXT 0xF

struct BytePatch
{
    uintptr_t offset;
    std::vector<byte> on;
    std::vector<byte> off;
};

static const BytePatch PATCH =
{
    0x1A5A48D, // GameAssembly.dll offset to required instructions
    {
        // bytes to enable patch
        0x90, 0x90, 0x90, 0x90, // NOPing CMP instruction since we don't need it
        0xEB // replace jne (jump not equal) with jmp (force jump)
    },
    {
        // bytes to disable patch
        0x83, 0x78, 0x78, 0x01, // cmp byte ptr [eax+74], 1
                                // cmp current GameState with GameState.OnlineGame (1)
        0x75                    // jne 5A            (5A not included)
                                // jump if not equal (goes to `else` code part)
    }
};

static HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);