#include "util.hh"

void AttachConsole() {
    if (!AllocConsole()) {
        std::cerr << "Failed to allocate console: " << GetLastError() << std::endl;
        return;
    }

    FILE* file;
    if (freopen_s(&file, "CONOUT$", "w", stdout) != 0 ||
        freopen_s(&file, "CONOUT$", "w", stderr) != 0 ||
        freopen_s(&file, "CONIN$", "r", stdin) != 0) {
        std::cerr << "Failed to redirect standard streams: " << GetLastError() << std::endl;
        return;
    }

    std::cout.clear();
    std::cerr.clear();
    std::cin.clear();

    HANDLE hConout = CreateFile("CONOUT$", GENERIC_WRITE, FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hConout == INVALID_HANDLE_VALUE) {
        std::cerr << "Failed to open CONOUT$ handle: " << GetLastError() << std::endl;
        return;
    }

    if (!SetStdHandle(STD_OUTPUT_HANDLE, hConout) || !SetStdHandle(STD_ERROR_HANDLE, hConout)) {
        std::cerr << "Failed to set standard handles: " << GetLastError() << std::endl;
        CloseHandle(hConout);
        return;
    }
}

int readmem(void* address, bool debug) {
    int value = 0;
    if (!ReadProcessMemory(GetCurrentProcess(), address, &value, sizeof(value), nullptr)) {
        std::cerr << "Failed to read memory at address " << address << std::endl;
        return -1;
    }

    if (debug) {
        std::cout << "Value at address " << address << ": " << value << std::endl;
    }
    
    return value;
}

void handleError(const std::string& errorMessage, const std::string& functionName, const std::string& fileName, int lineNumber) {
    std::ostringstream oss;
    oss << "Error occurred in function: " << functionName << ", File: " << fileName << ", Line: " << lineNumber << "\n";
    oss << "Error message: " << errorMessage << "\n";
    std::cerr << oss.str();
}

void cleanup(JNIEnv* env, jvmtiEnv* jvmti, jclass* classes, jstring className) {
    if (env && className)
        env->DeleteLocalRef(className);
    if (jvmti && classes)
        jvmti->Deallocate(reinterpret_cast<unsigned char*>(classes));
}

void printUnchangedClasses(const std::map<std::string, int>& currentValues) {
    std::cout << "Unchanged classes since the last monitoring session:" << std::endl;
    for (const auto& [className, previousValue] : previousValues) {
        if (currentValues.find(className) == currentValues.end()) {
            std::cout << className << std::endl;
        }
    }
}
