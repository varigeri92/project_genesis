#ifdef BUILD_DLL
#define GEN_API __declspec(dllexport)
#else
#define GEN_API __declspec(dllimport)
#endif // BUILD_DLL
