#pragma once

#include "util.hh"
#include "memory.hh"

FILE* file = nullptr;

/*
std::string getFieldValue(JNIEnv* env, jobject fieldValue) {
    if (fieldValue == nullptr) return "null";

    jclass fieldClass = env->GetObjectClass(fieldValue);
    jmethodID toStringMethod = env->GetMethodID(fieldClass, "toString", "()Ljava/lang/String;");
    jstring valueStr = (jstring)env->CallObjectMethod(fieldValue, toStringMethod);

    return jstringToString(env, valueStr);
}

void getStaticFields(JNIEnv* env, jclass clazz) {
    jclass classClass = env->GetObjectClass(clazz);
    jmethodID getFieldsMethod = env->GetMethodID(classClass, "getDeclaredFields", "()[Ljava/lang/reflect/Field;");
    jobjectArray fields = (jobjectArray)env->CallObjectMethod(clazz, getFieldsMethod);

    jsize numFields = env->GetArrayLength(fields);
    for (jsize i = 0; i < numFields; ++i) {
        jobject field = env->GetObjectArrayElement(fields, i);

        jclass fieldClass = env->GetObjectClass(field);
        jmethodID isStaticMethod = env->GetMethodID(fieldClass, "isStatic", "()Z");
        jboolean isStatic = env->CallBooleanMethod(field, isStaticMethod);

        if (isStatic) {
            jmethodID getNameMethod = env->GetMethodID(fieldClass, "getName", "()Ljava/lang/String;");
            jstring fieldName = (jstring)env->CallObjectMethod(field, getNameMethod);
            std::string fieldNameStr = jstringToString(env, fieldName);

            jmethodID getFieldValueMethod = env->GetMethodID(fieldClass, "get", "(Ljava/lang/Object;)Ljava/lang/Object;");
            jobject fieldValue = env->CallObjectMethod(field, getFieldValueMethod, clazz);
            std::string fieldValueStr = getFieldValue(env, fieldValue);

            std::cout << "    Field: " << fieldNameStr << ", Value: " << fieldValueStr << std::endl;
        }

        env->DeleteLocalRef(field);
    }

    env->DeleteLocalRef(fields);
}
*/
