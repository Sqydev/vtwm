#define IFNULLPANIC(do, NULLexitCode, NULLmsg) do; if(do == NULL) { Panic(NULLexitCode, NULLmsg); return; }
#define IFFALSEPANIC(do, NULLexitCode, NULLmsg) do; if(do == false) { Panic(NULLexitCode, NULLmsg); return; }

#define IFNULLPANICALONE(do, NULLexitCode, NULLmsg) if(do == NULL) { Panic(NULLexitCode, NULLmsg); return; }
#define IFFALSEPANICALONE(do, NULLexitCode, NULLmsg) if(do == false) { Panic(NULLexitCode, NULLmsg); return; }
