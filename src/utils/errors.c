#include "./utils.h"

#include <stdarg.h>

inline void* IFNULLPANIC(void* value, int exitCode, const char* msg, ...) {
	if(value == NULL) {
		va_list args;
		va_start(args, msg);
		vaPanic(exitCode, msg, args);
		va_end(args);
	}
	return value;
}

inline bool IFFALSEPANIC(bool value, int exitCode, const char* msg, ...) {
	if(value == false) {
		va_list args;
		va_start(args, msg);
		vaPanic(exitCode, msg, args);
		va_end(args);
	}
	return value;
}

inline void IFNULLPANICALONE(void* value, int exitCode, const char* msg, ...) {
	if(value == NULL) {
		va_list args;
		va_start(args, msg);
		vaPanic(exitCode, msg, args);
		va_end(args);
	}
}

inline void IFFALSEPANICALONE(bool value, int exitCode, const char* msg, ...) {
	if(value == false) {
		va_list args;
		va_start(args, msg);
		vaPanic(exitCode, msg, args);
		va_end(args);
	}
}

inline const void* IFNULLPANICCONST(const void* value, int exitCode, const char* msg, ...) {
	if(value == NULL) {
		va_list args;
		va_start(args, msg);
		vaPanic(exitCode, msg, args);
		va_end(args);
	}
	return value;
}

inline void IFNULLPANICALONECONST(const void* value, int exitCode, const char* msg, ...) {
	if(value == NULL) {
		va_list args;
		va_start(args, msg);
		vaPanic(exitCode, msg, args);
		va_end(args);
	}
}
