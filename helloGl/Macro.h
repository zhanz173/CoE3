#pragma once
#ifdef DEBUG
#define ASSERT(x) if (!x) __debugbreak();
#define GlCall(x)  GlClearError();\
    x;\
    ASSERT(GlLogCall(#x, __FILE__, __LINE__))

void GlClearError();
bool GlLogCall(const char* function, const char* file, int line);

#else
#define ASSERT(x) if (!x) __debugbreak();
#define GlCall(x)  x

#endif // DEBUG
