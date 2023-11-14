#ifdef __cplusplus
extern "C" {
#endif

#undef assert
#ifdef NDEBUG
#define assert(ignore) ((void) 0)
#else
// TODO
#define assert(ignore) ((void) 0)
#endif

#ifdef __cplusplus
}
#endif