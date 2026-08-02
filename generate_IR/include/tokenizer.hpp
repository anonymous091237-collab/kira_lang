#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>


enum class TokenType : uint8_t {
    None,
    Number,
    Float,
    Keyword,
    String,
    Char,
    Identifier,
    Operator,
    Delimiter
};

struct TokenPos {
    uint32_t line;
    uint32_t chr;
    uint32_t index;
};

// and offset to where the data : <tokentype><string>+\0  are stored in memory
using TokenData = uintptr_t;

struct Token {
    TokenPos pos;
    TokenData id;
};
using CheckRepeat = std::unordered_map<std::string_view, int>;

class Tokenizer {
   private:
    TokenPos pos;

    bool tokenizeComment(char a, char b, CheckRepeat& rep);
    bool tokenizePairOp(char a, char b, CheckRepeat& rep);
    bool tokenizeString(char a, CheckRepeat& rep);
    bool tokenizeUinOp(char a, CheckRepeat& rep);
    bool tokenizeNum(char a, CheckRepeat& rep);
    bool tokenizeSpecial(char a, CheckRepeat& rep);

    bool isNumber(std::string_view string);
    bool isString(std::string_view string);
    bool isChar(std::string_view string);
    bool isDelimiter(std::string_view string);
    bool isKeyword(std::string_view string);
    bool isIdentifier(std::string_view string);

    // the only reason i did this is because i was bored
    Token createToken(std::string_view string, CheckRepeat& avoidRepetition,
                      TokenType type = TokenType::None);
    void flush(CheckRepeat& avoidRepetition);
    const char* p;
    const char* tokenHead;
    const char* end;

   public:
    std::array<TokenData, 7> KeywordTable;
    std::string fileContent;
    std::vector<Token> tokens;
    uint32_t offset;
    uint32_t max;
    void* tokensAdr;

    Tokenizer(const std::string& filepath);

    void run();
};