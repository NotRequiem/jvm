/*
#include <Psapi.h>
#include <iostream>
const void* getjvm(HANDLE process) {
    HMODULE hMods[1024];
    DWORD cbNeeded;

    if (EnumProcessModulesEx(process, hMods, sizeof(hMods), &cbNeeded, LIST_MODULES_ALL)) {
        for (unsigned int i = 0; i < (cbNeeded / sizeof(HMODULE)); i++) {
            TCHAR szModName[MAX_PATH];

            if (GetModuleFileNameEx(process, hMods[i], szModName, sizeof(szModName) / sizeof(TCHAR))) {
                std::string moduleName(szModName);

                if (moduleName.find("jvm.dll") != std::wstring::npos) {
                    printf("JVM Module found at address: %p\n", (const void*)hMods[i]);
                    return (const void*)hMods[i];
                }
            }
        }
    }

    printf("JVM Module not found\n");
    return nullptr;
}
*/

#include "jimmy.hpp"

const void* jvm;

static HANDLE handle;
void readmem(void* buf, const void* addr, size_t size) {
    if (ReadProcessMemory(handle, (void*)addr, buf, size, 0) == 0) {
        memset(buf, 0, size);
        printf("Failed to read memory at address %p\n", addr);
    }
}

void jmy_init(HANDLE process, const void* jvmbase) {
    handle = process;
    jvm = jvmbase;
}

/* from java/lang/String */
static unsigned int hashcode(const char* s) {
    unsigned int h = 0;
    size_t n = strlen(s);
    while (n-- > 0) {
        h = 31 * h + *s++;
    }
    return h;
}

static Jobj system_loader() {
    Jobj systemCl = 0;
    readmem(&systemCl, (void*)((uintptr_t)jvm + 0x007E1AB8), sizeof(systemCl));
    if (systemCl == 0) {
        printf("Failed to get system class loader\n");
    }
    return systemCl;
}

static void* findsym(const char* sym) {
    unsigned int seed;
    int index;
    unsigned int hash;
    void* symtab;
    unsigned int symtablen;
    void* hashmap;
    void* listelem;

    readmem(&seed, (void*)((uintptr_t)jvm + 0x007FD7F0), sizeof(seed));
    if (seed != 0) {
        printf("Invalid seed: %u\n", seed);
        return 0;
    }

    hash = hashcode(sym);
    readmem(&symtab, (void*)((uintptr_t)jvm + 0x007E17B8), sizeof(symtab));
    readmem(&symtablen, symtab, sizeof(symtablen));
    readmem(&hashmap, (void*)((uintptr_t)symtab + 8), sizeof(hashmap));
    if (symtablen == 0) {
        printf("Symbol table length is zero\n");
        return 0;
    }
    index = hash % symtablen;

    readmem(&listelem, (void*)((uintptr_t)hashmap + static_cast<unsigned long long>(index) * 8), sizeof(listelem));
    while (listelem != 0) {
        unsigned int hashelem;
        readmem(&hashelem, listelem, sizeof(hashelem));
        if (hashelem == hash) {
            void* symbol;
            readmem(&symbol, (void*)((uintptr_t)listelem + 0x10), sizeof(symbol));
            unsigned short symlen;
            readmem(&symlen, symbol, sizeof(symlen));
            if (symlen == strlen(sym)) {
                void* buf = malloc(symlen);
                readmem(buf, (void*)((uintptr_t)symbol + 8), symlen);
                if (buf != 0) {
                    if (memcmp(buf, sym, symlen) == 0) {
                        free(buf);
                        return symbol;
                    }
                }
                free(buf);
            }
        }
        readmem(&listelem, (void*)((uintptr_t)listelem + 8), sizeof(listelem));
    }

    printf("Symbol '%s' not found\n", sym);
    return 0;
}

Jclass jmy_findclassfromcl(const char* clsname, Jobj classloader) {
    void* sym, * loaderdata, * dict, * dictht, * entry;
    Jclass klass = 0;
    unsigned int hash, index, dictlen, entryhash;

    sym = findsym(clsname);
    if (sym == 0) {
        printf("Class '%s' not found in symbol table\n", clsname);
        return 0;
    }

    readmem(&hash, (void*)((uintptr_t)sym + 4), sizeof(hash));
    if (classloader != 0) {
        readmem(&loaderdata, classloader, sizeof(loaderdata));
        hash ^= ((size_t)loaderdata >> 8) & 0x7fffffff;
    }

    readmem(&dict, (void*)((uintptr_t)jvm + 0x7E17F0), sizeof(dict));
    readmem(&dictlen, dict, sizeof(dictlen));
    if (dictlen == 0) {
        printf("Dictionary length is zero\n");
        return 0;
    }

    readmem(&dictht, (void*)((uintptr_t)dict + 8), sizeof(dictht));
    index = hash % dictlen;

    readmem(&entry, (void*)((uintptr_t)dictht + static_cast<unsigned long long>(index) * 8), sizeof(entry));
    while (entry != 0) {
        readmem(&entryhash, entry, sizeof(entryhash));
        if (entryhash == hash) {
            readmem(&klass, (void*)((uintptr_t)entry + 0x10), sizeof(klass));
            if (klass != 0) {
                return klass;
            }
        }
        readmem(&entry, (void*)((uintptr_t)entry + 8), sizeof(entry));
    }

    printf("Class '%s' not found in dictionary\n", clsname);
    return 0;
}

Jclass jmy_findclass(const char* clsname) {
    return jmy_findclassfromcl(clsname, system_loader());
}
