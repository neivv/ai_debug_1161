#include "warn.h"

#include "offsets.h"
#include "text.h"

#include <stdio.h>
#include <stdarg.h>
#include <windows.h>

void Warning(const char *format, ...)
{
    char buf[512];
    va_list varg;
    va_start(varg, format);
    vsnprintf(buf, sizeof buf, format, varg);
    va_end(varg);
    Print("Warning: %s\n", buf);
    if (IsDebuggerPresent())
        INT3();
}

void FatalError(const char *format, ...)
{
    char buf[512];
    va_list varg;
    va_start(varg, format);
    vsnprintf(buf, sizeof buf, format, varg);
    va_end(varg);
    MessageBoxA(0, buf, "Fatal error", 0);
    if (IsDebuggerPresent())
        INT3();
    ExitProcess(1);
}
