#pragma once

#include <iostream>
#include <sstream>
#include <string>
#include <windows.h>
#include <jvmti.hpp>
#include <jni.hpp>
#include <map>
#pragma warning (disable: 4244)
#include <algorithm>
#pragma warning (default: 4244)

static std::map<std::string, int> previousValues;

void AllocateConsole();
int readmem(void* address, bool debug);
void handleError(const std::string& errorMessage, const std::string& functionName, const std::string& fileName, int lineNumber);
void cleanup(JNIEnv* env, jvmtiEnv* jvmti, jclass* classes, jstring className);
void printUnchangedClasses(const std::map<std::string, int>& currentValues);
bool containsIgnoreCase(const std::string& str, const std::string& substr);