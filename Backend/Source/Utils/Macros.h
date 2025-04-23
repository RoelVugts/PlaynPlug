#pragma once

#if JUCE_WINDOWS
    #include <windows.h>

    #define DL_OPEN(path) LoadLibrary(path)
    #define DL_SYM(handle, funcName) GetProcAddress(handle, funcName)
    #define DL_CLOSE(handle) FreeLibrary(handle)
    #define DL_ERROR                                                    \
        {                                                               \
            DWORD error = GetLastError();                               \
            printf("LoadLibrary failed with error code %lu\n", error);  \
            fflush(stdout);                                             \
        }
#elif JUCE_MAC
    #include <dlfcn.h>

    #define DL_OPEN(path) dlopen(path, RTLD_NOW )
    #define DL_SYM(handle, funcName) dlsym(handle, funcName)
    #define DL_CLOSE(handle) (dlclose(handle) == 0)
    #define DL_ERROR                                       \
      printf("[%s] Error: %s\n", __FILE__, dlerror());     \
      fflush(stdout)
#endif
