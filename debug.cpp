#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <windows.h>
#include <fstream>

#include "debug.h"
#include "log.h"
#include "patch.h"
#include "symbols.h"
#include <bits/chrono.h>
#include "xvpatcher.h"
#include <chrono>
#include <ctime>
#include <iostream>
#include <fstream>
#include <cstdarg>
#include <cstring>

#define DEBUG_BUFFER_SIZE 256


// Define DebugPrintf with format attributes
int __attribute__ ((format (printf, 1, 2))) DebugPrintf(const char* fmt, ...) 
{
    char buffer[DEBUG_BUFFER_SIZE];  // Adjust the buffer size as needed
    va_list args;

    // Format the output into the buffer
    va_start(args, fmt);
    int len = vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);

    // Output the formatted string to console
    std::cout << "Debug: " << buffer << std::endl;

    // Write the formatted string to the log file if open
    if (logFile.is_open()) {
        logFile << GetCurrentDateTime() << " " << buffer;
    } else {
        std::cerr << "Log file is not open!";
    }

    return len;
}


int __attribute__ ((format (printf, 1, 2))) UserPrintf(const char* fmt, ...) 
{
	char *dbg;
	va_list ap;

	dbg = (char *)malloc(DEBUG_BUFFER_SIZE);
	if (!dbg)
		return 0;
		
	va_start(ap, fmt);
	size_t len = vsnprintf(dbg, DEBUG_BUFFER_SIZE, fmt, ap);
	va_end(ap);

	MessageBox(NULL, dbg, "XVPatcher", 0);
	free(dbg);
	return len;
}

int __attribute__ ((format (printf, 2, 3))) Dprintf1(int unk, const char* fmt, ...) 
{
	char *dbg;
	va_list ap;

	dbg = (char *)malloc(DEBUG_BUFFER_SIZE);
	if (!dbg)
		return 0;
		
	va_start(ap, fmt);
	size_t len = vsnprintf(dbg, DEBUG_BUFFER_SIZE, fmt, ap);
	va_end(ap);

	OutputDebugString(dbg);
	free(dbg);
	return len;
}

/*int __attribute__ ((format (printf, 2, 6))) DprintfC(int unk, const char* fmt, int, int, int, ...) 
{
	char *dbg;
	va_list ap;

	dbg = (char *)malloc(DEBUG_BUFFER_SIZE);
	if (!dbg)
		return 0;
		
	va_start(ap, fmt);
	size_t len = vsnprintf(dbg, DEBUG_BUFFER_SIZE, fmt, ap);
	va_end(ap);

	OutputDebugString(dbg);
	free(dbg);
	return len;
}*/

void debug_patches()
{
#ifdef DEBUG

#ifndef XENOVERSE
	HookFunction(DPRINTF1_SYMBOL, NULL, (void *)Dprintf1);
	HookFunction(DPRINTF2_SYMBOL, NULL, (void *)Dprintf1);
	//HookFunction(0x18AEBBB-MY_DUMP_BASE, NULL, (void *)DprintfC);
	
#endif

#endif
}

