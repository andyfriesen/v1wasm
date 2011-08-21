
#pragma once

#ifdef DEBUG

#include <stdio.h>
#include <cassert>
#include <vector>

struct StackKnower {
    StackKnower(const char* filename, int line)
        : filename(filename)
        , line(line)
    {
        printf("STACK %s:%i\n", filename, line);
        stack.push_back(this);
    }

    ~StackKnower() {
        assert(this == stack.back());
        stack.pop_back();
    }

    static std::vector<StackKnower*> stack;
    const char* filename;
    const int line;
};

#define CONCAT_IMPL( x, y ) x##y
#define MACRO_CONCAT( x, y ) CONCAT_IMPL( x, y )
#define STACK StackKnower MACRO_CONCAT( sn, __COUNTER__ ) (__FILE__, __LINE__)

#else

#define STACK

#endif
