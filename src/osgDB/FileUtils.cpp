// moved each platforms file access definitions into their own file
// to make it easier to manage them.
#if defined(WIN32) && !defined(__CYGWIN__)
    #include "FileUtils_Windows.cpp"
#elif defined(TARGET_API_MAC_CARBON)
    #include "FileUtils_Mac.cpp"
#else
    #include "FileUtils_Unix.cpp"
#endif
