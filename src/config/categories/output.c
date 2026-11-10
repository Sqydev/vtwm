#include "./categories.h"

#include "../../coredata.h"

void ParseOutputBlock(ConfigParser* parser) {
	while(parser->token.data) {
		if(ParserAccept(parser, "}")) { return; }
		if(ParserAccept(parser, ";")) { continue; }

		Token key = parser->token;
		size_t entryLine = key.line;
		ParserAdvance(parser);

		if(TokenEquals(key, "wdirOnOutput")) {
			if(ParserBeginValue(parser, entryLine)) {
				ParseBooleanValue(parser, &CONFIG.output.wdirOnOutput, entryLine);
			}
		}
		else {
			ConfigWarning(entryLine, "Unknown Output field, ignoring");
			SkipUnknownEntry(parser, entryLine);
		}
	}

	ConfigWarning(parser->line, "Unexpected end of file inside Output");
}
