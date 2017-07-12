#ifdef MLIBS_STATIC_LIB
#define MLIBS_API
#elif defined(MLIBS_EXPORTS)
#define MLIBS_API __declspec(dllexport)
#else
#define MLIBS_API __declspec(dllimport)
#endif
