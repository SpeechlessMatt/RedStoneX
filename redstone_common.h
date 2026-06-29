#ifndef REDSTONE_COMMON_H
#define REDSTONE_COMMON_H

#include <stdio.h>
#include <stdlib.h>

// Gemini评审发现我的realloc没做空指针检查 于是整个宏定义
static void* xrealloc_impl(void* ptr, size_t size) {
    void* tmp = realloc(ptr, size);
    if (tmp == NULL && size > 0) {
        fprintf(stderr, "[FATAL ERROR] 内存不足...\n");
        exit(EXIT_FAILURE); 
    }
    return tmp;
}

#define SAFE_REALLOC(ptr, count, type) \
    ((ptr) = (type*)xrealloc_impl((ptr), (count) * sizeof(type)))

#endif
