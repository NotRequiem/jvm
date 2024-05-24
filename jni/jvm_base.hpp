#pragma once

#include <Windows.h>
#include <TlHelp32.h>
#include <cstdint>
#include <string>

const void* CalculateJVMBASE(HANDLE process);