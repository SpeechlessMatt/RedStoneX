#ifndef REDSTONEX_COMMON_H
#define REDSTONEX_COMMON_H

#include <stdlib.h>
#include <assert.h>

// Gemini评审发现我的realloc没做空指针检查 于是整个宏定义
static inline void* xrealloc_impl(void* ptr, size_t size) {
    void* tmp = realloc(ptr, size);
    if (tmp == NULL && size > 0) {
        assert((tmp != NULL || size == 0) && "FATAL ERROR: Out of memory in SAFE_REALLOC!");
    }
    return tmp;
}

#define SAFE_REALLOC(ptr, count, type) \
    ((ptr) = (type*)xrealloc_impl((ptr), (count) * sizeof(type)))

#define UNUSED(x) (void)(x)

#endif
