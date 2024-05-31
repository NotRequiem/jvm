#include "memory.hh"

JNIEnv* env;
JavaVM* vm;

std::string classNameToMonitor;
bool monitorSpecificClasses = false;

void AnalyzeMemory() {
    jsize vmCount;
    if (JNI_GetCreatedJavaVMs(&vm, 1, &vmCount) != 0 || vmCount == 0) {
        std::cerr << "Failed to get Java VM." << std::endl;
        return;
    }

    jint penv = vm->GetEnv((void**)&env, JNI_VERSION_1_8);
    if (penv == JNI_EDETACHED) {
        if (vm->AttachCurrentThread((void**)&env, nullptr) != 0) {
            std::cerr << "Failed to attach current thread to JVM." << std::endl;
            return;
        }
    }
    else if (penv == JNI_EVERSION) {
        std::cerr << "JNI version not supported." << std::endl;
        return;
    }

    if (env == nullptr) {
        std::cerr << "JNIEnv is null." << std::endl;
        return;
    }

    jvmtiEnv* jvmti = nullptr;
    if (vm->GetEnv((void**)&jvmti, JVMTI_VERSION_1) != JNI_OK) {
        std::cerr << "Failed to get JVMTI environment." << std::endl;
        return;
    }

    jclass classClass = env->FindClass("java/lang/Class");
    if (classClass == nullptr) {
        std::cerr << "Failed to find java/lang/Class." << std::endl;
        return;
    }

    jmethodID getNameMethod = env->GetMethodID(classClass, "getName", "()Ljava/lang/String;");
    if (getNameMethod == nullptr) {
        std::cerr << "Failed to get getName method." << std::endl;
        return;
    }

    jclass* classes;
    jint classCount;

    if (jvmti->GetLoadedClasses(&classCount, &classes) != JVMTI_ERROR_NONE) {
        std::cerr << "Failed to get loaded classes." << std::endl;
        return;
    }

    std::string userInput;
    std::cout << "Do you want to scan for specific classes, methods or field names? (yes/no): ";
    std::cin >> userInput;

    bool scanForSpecificNames = (userInput == "yes");

    std::string nameToScan;
    int scanTarget = 0;
    if (scanForSpecificNames) {
        std::cout << "Enter the name to scan for: ";
        std::cin >> nameToScan;

        std::cout << "What do you want to scan?\n1: Classes\n2: Methods\n3: Fields\nEnter choice (1/2/3): ";
        std::cin >> scanTarget;
    }

    for (int i = 0; i < classCount; i++) {
        jclass clazz = classes[i];
        jstring className = (jstring)env->CallObjectMethod(clazz, getNameMethod);
        if (className == nullptr) {
            handleError("Failed to get class name.", __func__, __FILE__, __LINE__);
            continue;
        }

        const char* classNameStr = env->GetStringUTFChars(className, nullptr);
        if (classNameStr == nullptr) {
            handleError("Failed to convert class name to UTF-8.", __func__, __FILE__, __LINE__);
            cleanup(env, jvmti, nullptr, className);
            continue;
        }

        bool matchFound = false;
        std::string classNameStrCpp(classNameStr);

        if (scanTarget == 1 && containsIgnoreCase(classNameStrCpp, nameToScan)) {
            std::cout << "Class: " << classNameStr << ", Address: " << static_cast<void*>(clazz) << std::endl;
            readmem(static_cast<void*>(clazz), true);
            env->ReleaseStringUTFChars(className, classNameStr);
            matchFound = true;
        }

        if (!matchFound && (scanTarget == 2 || scanTarget == 3)) {
            // Print methods if scanning for methods
            jmethodID* methods;
            jint methodCount;
            if (jvmti->GetClassMethods(clazz, &methodCount, &methods) != JVMTI_ERROR_NONE) {
                continue;
            }

            for (int j = 0; j < methodCount; j++) {
                char* methodName;
                char* methodSignature;
                if (jvmti->GetMethodName(methods[j], &methodName, &methodSignature, nullptr) != JVMTI_ERROR_NONE) {
                    continue;
                }
                std::string methodNameStr(methodName);
                if (scanTarget == 2 && containsIgnoreCase(methodNameStr, nameToScan)) {
                    std::cout << "Class: " << classNameStr << ", Method: " << methodName << ", Signature: " << methodSignature << std::endl;
                    matchFound = true;
                }
                jvmti->Deallocate(reinterpret_cast<unsigned char*>(methodName));
                jvmti->Deallocate(reinterpret_cast<unsigned char*>(methodSignature));
            }
        }

        if (!matchFound && scanTarget == 3) {
            // Print fields if scanning for fields
            jfieldID* fields;
            jint fieldCount;
            if (jvmti->GetClassFields(clazz, &fieldCount, &fields) != JVMTI_ERROR_NONE) {
                continue;
            }

            for (int j = 0; j < fieldCount; j++) {
                char* fieldName;
                char* fieldSignature;
                char* fieldGenericSignature;
                if (jvmti->GetFieldName(clazz, fields[j], &fieldName, &fieldSignature, &fieldGenericSignature) != JVMTI_ERROR_NONE) {
                    continue;
                }
                std::string fieldNameStr(fieldName);
                if (containsIgnoreCase(fieldNameStr, nameToScan)) {
                    std::cout << "Class: " << classNameStr << ", Field: " << fieldName << ", Signature: " << fieldSignature << std::endl;
                }
                jvmti->Deallocate(reinterpret_cast<unsigned char*>(fieldName));
                jvmti->Deallocate(reinterpret_cast<unsigned char*>(fieldSignature));
                jvmti->Deallocate(reinterpret_cast<unsigned char*>(fieldGenericSignature));
            }
        }

        env->ReleaseStringUTFChars(className, classNameStr);
        cleanup(env, jvmti, nullptr, className);
    }

    jvmti->Deallocate(reinterpret_cast<unsigned char*>(classes));
}

void MonitorMemory(std::map<std::string, int>& classValues) {
    jsize vmCount;
    if (JNI_GetCreatedJavaVMs(&vm, 1, &vmCount) != 0 || vmCount == 0) {
        std::cerr << "Failed to get Java VM." << std::endl;
        return;
    }

    jint penv = vm->GetEnv((void**)&env, JNI_VERSION_1_8);
    if (penv == JNI_EDETACHED) {
        if (vm->AttachCurrentThread((void**)&env, nullptr) != 0) {
            std::cerr << "Failed to attach current thread to JVM." << std::endl;
            return;
        }
    }
    else if (penv == JNI_EVERSION) {
        std::cerr << "JNI version not supported." << std::endl;
        return;
    }

    if (env == nullptr) {
        std::cerr << "JNIEnv is null." << std::endl;
        return;
    }

    jvmtiEnv* jvmti = nullptr;
    if (vm->GetEnv((void**)&jvmti, JVMTI_VERSION_1) != JNI_OK) {
        std::cerr << "Failed to get JVMTI environment." << std::endl;
        return;
    }

    jclass classClass = env->FindClass("java/lang/Class");
    if (classClass == nullptr) {
        std::cerr << "Failed to find java/lang/Class." << std::endl;
        return;
    }

    jmethodID getNameMethod = env->GetMethodID(classClass, "getName", "()Ljava/lang/String;");
    if (getNameMethod == nullptr) {
        std::cerr << "Failed to get getName method." << std::endl;
        return;
    }

    jclass* classes;
    jint classCount;

    if (jvmti->GetLoadedClasses(&classCount, &classes) != JVMTI_ERROR_NONE) {
        std::cerr << "Failed to get loaded classes." << std::endl;
        return;
    }

    static bool run_once = true;
    if (run_once) {
        std::string userInput;
        std::cout << "Do you want to monitor for specific class names? (yes/no): ";
        std::cin >> userInput;

        monitorSpecificClasses = (userInput == "yes");

        if (monitorSpecificClasses) {
            std::cout << "Enter the name of the class to monitor for: ";
            std::cin >> classNameToMonitor;
        }

        run_once = false;
    }

    for (int i = 0; i < classCount; i++) {
        jclass clazz = classes[i];
        jstring className = (jstring)env->CallObjectMethod(clazz, getNameMethod);
        if (className == nullptr) {
            handleError("Failed to get class name.", __func__, __FILE__, __LINE__);
            continue;
        }

        const char* classNameStr = env->GetStringUTFChars(className, nullptr);
        if (classNameStr == nullptr) {
            handleError("Failed to convert class name to UTF-8.", __func__, __FILE__, __LINE__);
            cleanup(env, jvmti, nullptr, className);
            continue;
        }

        std::string classNameStrCpp(classNameStr);

        if (!monitorSpecificClasses || containsIgnoreCase(classNameStrCpp, classNameToMonitor)) {
            int value = readmem(static_cast<void*>(clazz), false);
            classValues[classNameStr] = value;
        }

        env->ReleaseStringUTFChars(className, classNameStr);
        cleanup(env, jvmti, nullptr, className);
    }

    jvmti->Deallocate(reinterpret_cast<unsigned char*>(classes));
}