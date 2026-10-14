#include "./utils.h"

inline void* IFNULLPANIC(void* value, int exitCode, const char* msg) {
	if(value == NULL) { Panic(exitCode, "%s", msg); }
	return value;
}

inline bool IFFALSEPANIC(bool value, int exitCode, const char* msg) {
	if(value == false) { Panic(exitCode, "%s", msg); }
	return value;
}

inline void IFNULLPANICALONE(void* value, int exitCode, const char* msg) {
	if(value == NULL) { Panic(exitCode, "%s", msg); }
}

inline void IFFALSEPANICALONE(bool value, int exitCode, const char* msg) {
	if(value == false) { Panic(exitCode, "%s", msg); }
}
