#include "memory.hh"

JNIEnv* env;
JavaVM* vm;

void AnalyzeMemory() {
    jsize vmCount;
    if (JNI_GetCreatedJavaVMs(&vm, 1, &vmCount) != 0 || vmCount == 0) {
        std::cerr << "Failed to get Java VM." << std::endl;
        return;
    }

    jint penv = vm->GetEnv((void**)&env, 0x00010008);
    if (penv == (-2)) {
        if (vm->AttachCurrentThread((void**)&env, nullptr) != 0) {
            std::cerr << "Failed to attach current thread to JVM." << std::endl;
            return;
        }
    }
    else if (penv == (-3)) {
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

        std::cout << "Class: " << classNameStr << ", Address: " << static_cast<void*>(clazz) << std::endl;
        readmem(static_cast<void*>(clazz), true);
        env->ReleaseStringUTFChars(className, classNameStr);
        cleanup(env, jvmti, nullptr, className);

#ifdef _DEBUG
        // Print methods
        jmethodID* methods;
        jint methodCount;
        if (jvmti->GetClassMethods(clazz, &methodCount, &methods) != JVMTI_ERROR_NONE) {
            std::cerr << "Failed to get methods for class." << std::endl;
            continue;
        }

        for (int j = 0; j < methodCount; j++) {
            char* methodName;
            char* methodSignature;
            if (jvmti->GetMethodName(methods[j], &methodName, &methodSignature, nullptr) != JVMTI_ERROR_NONE) {
                std::cerr << "Failed to get method name or signature." << std::endl;
                continue;
            }
            std::cout << "  Method: " << methodName << ", Signature: " << methodSignature << std::endl;
            jvmti->Deallocate(reinterpret_cast<unsigned char*>(methodName));
            jvmti->Deallocate(reinterpret_cast<unsigned char*>(methodSignature));
        }

        // Print fields (objects)
        jfieldID* fields;
        jint fieldCount;
        if (jvmti->GetClassFields(clazz, &fieldCount, &fields) != JVMTI_ERROR_NONE) {
            std::cerr << "Failed to get fields for class." << std::endl;
            continue;
        }

        for (int j = 0; j < fieldCount; j++) {
            char* fieldName;
            char* fieldSignature;
            char* fieldGenericSignature;
            if (jvmti->GetFieldName(clazz, fields[j], &fieldName, &fieldSignature, &fieldGenericSignature) != JVMTI_ERROR_NONE) {
                std::cerr << "Failed to get field name or signature." << std::endl;
                continue;
            }
            std::cout << "  Field: " << fieldName << ", Signature: " << fieldSignature << std::endl;
            jvmti->Deallocate(reinterpret_cast<unsigned char*>(fieldName));
            jvmti->Deallocate(reinterpret_cast<unsigned char*>(fieldSignature));
            jvmti->Deallocate(reinterpret_cast<unsigned char*>(fieldGenericSignature));
        }
#endif
    }

    jvmti->Deallocate(reinterpret_cast<unsigned char*>(classes));
}

void MonitorMemory(std::map<std::string, int>& classValues) {
    jsize vmCount;
    if (JNI_GetCreatedJavaVMs(&vm, 1, &vmCount) != 0 || vmCount == 0) {
        std::cerr << "Failed to get Java VM." << std::endl;
        return;
    }

    jint penv = vm->GetEnv((void**)&env, 0x00010008);
    if (penv == (-2)) {
        if (vm->AttachCurrentThread((void**)&env, nullptr) != 0) {
            std::cerr << "Failed to attach current thread to JVM." << std::endl;
            return;
        }
    }
    else if (penv == (-3)) {
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

        int value = readmem(static_cast<void*>(clazz), false);
        classValues[classNameStr] = value;

        env->ReleaseStringUTFChars(className, classNameStr);
        cleanup(env, jvmti, nullptr, className);
    }

    jvmti->Deallocate(reinterpret_cast<unsigned char*>(classes));
}