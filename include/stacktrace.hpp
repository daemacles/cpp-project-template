// stacktrace.h (c) 2008, Timo Bingmann from http://idlebox.net/
// published under the WTFPL v2.0

#pragma once

#include <string>
#include <vector>

// Prints the current stack to the console.
//
void PrintStack();

// Returns a vector of pointers to the functions on the stack when it was
// called.
std::vector<void *> GetStackFunctionPointers();

// Returns a formated string with the current stack trace.
//
std::string GetStackTrace();
