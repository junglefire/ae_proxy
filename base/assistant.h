#ifndef SMART_ASSISTANT_H__
#define SMART_ASSISTANT_H__

#include <sys/time.h>
#include <ctime>

#ifndef NAMESPACE_BEGIN
#define NAMESPACE_BEGIN(x) namespace x {
#endif //NAMESPACE_BEGIN

#ifndef NAMESPACE_END
#define NAMESPACE_END(x) }
#endif //NAMESPACE_END

#define MALLOC(T) static_cast<T*>(malloc(sizeof(T)))
#define FREE(P) free(P)

#ifdef __cplusplus
	#define INLINE inline
#elif
	#define INLINE
#endif

INLINE int64_t now() {
	struct timeval tv; 
	struct timezone tz; 
	gettimeofday(&tv, &tz);
	return tv.tv_sec*1000000+tv.tv_usec;
}

#endif // SMART_ASSISTANT_H__