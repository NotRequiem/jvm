#include "dllmain.hpp"

BOOL __stdcall DllMain(HMODULE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved
)
{
    UNREFERENCED_PARAMETER(lpReserved);
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    {
        DisableThreadLibraryCalls(hModule);

        int monitoringChoice = MessageBoxA(nullptr, "Do you want to monitor the JVM memory for changes?", "Monitoring Mode", MB_ICONQUESTION | MB_YESNO);
        int analysisChoice = MessageBoxA(nullptr, "Do you want to analyze memory for loaded Java objects?", "Analysis Mode", MB_ICONQUESTION | MB_YESNO);

        if (analysisChoice == IDYES) {
            AllocateConsole();
            AnalyzeMemory();
        }
        else if (monitoringChoice == IDYES) {
            AllocateConsole();

            std::cout << "Monitoring memory. Waiting for memory changes...\n";
            while (true) {
                std::map<std::string, int> currentValues;
                MonitorMemory(currentValues);

                for (const auto& [className, currentValue] : currentValues) {
                    auto it = previousValues.find(className);
                    if (it != previousValues.end() && it->second != currentValue) {
                        std::cout << "Value change detected for class " << className << ": "
                            << it->second << " -> " << currentValue << std::endl;
                    }
                }

                Sleep(5000);
                previousValues = std::move(currentValues);
            }
        }
        else {}

        break;
    }
    case DLL_PROCESS_DETACH:
        if (GetAsyncKeyState(VK_DELETE) & 0x8000 && previousValues.size() > 0) {
            std::map<std::string, int> currentValues;
            MonitorMemory(currentValues);
            printUnchangedClasses(currentValues);
            FreeConsole();
        }
        break;

    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
        break;
    default: {
        if (GetAsyncKeyState(VK_DELETE) & 0x8000 && previousValues.size() > 0) {
            std::map<std::string, int> currentValues;
            MonitorMemory(currentValues);
            printUnchangedClasses(currentValues);
        }
        break;
    }
    }

    return TRUE;
}