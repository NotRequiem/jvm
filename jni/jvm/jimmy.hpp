#pragma once

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>

#undef interface

	void jmy_init(HANDLE process, const void* jvmbase);

	typedef const void* Jobj;
	typedef const void* Jclass;

void readmem(void* buf, const void* addr, size_t size);

Jclass jmy_findclass(const char *clsname);