#include "tokenizer.hpp"

#include <fstream>
#include <iostream>
#include <sstream>

Tokenizer::Tokenizer(const std::string& filepath) {
    std::ifstream file(filepath);

    if (!file.is_open()) {
        throw std::runtime_error("Failed to open source file.\n");
    }

    std::stringstream buffer;
    buffer << file.rdbuf();

    fileContent = buffer.str();
}

void Tokenizer::flush(CheckRepeat& rep) {
    if (tokenHead != p) {
        std::string_view word(tokenHead, p - tokenHead);
        TokenType type = TokenType::None;

        if (isString(word))
            type = TokenType::String;

        else if (isChar(word))
            type = TokenType::Char;

        else if (isKeyword(word))
            type = TokenType::Keyword;

        else if (isIdentifier(word))
            type = TokenType::Identifier;

        tokens.push_back(createToken(word, rep, type));

        pos.chr += p - tokenHead;
    }

    tokenHead = p;
}

bool alpha_(const char& c) {
    return ((c == '_') || ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z'));
}

bool alphanum_(const char& c) {
    return ((c == '_') || ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') ||
            ('0' <= c && c <= '9'));
}

bool Tokenizer::isIdentifier(std::string_view string) {
    if (!alpha_((string[0]))) return false;
    for (auto& c : string) {
        if (!alphanum_(c)) return false;
    }
    return true;
}

bool Tokenizer::isKeyword(std::string_view string) {
    return (string == "fn" || string == "if" || string == "let" ||
            string == "var" || string == "else" || string == "while" ||
            string == "for");
}

bool Tokenizer::isDelimiter(std::string_view string) {
    return (string == "{" || string == "}" || string == "(" || string == ")" ||
            string == "[" || string == "]");
}

bool Tokenizer::isChar(std::string_view string) {
    if (string.size() == 4) {
        return (string == "'\\n'" || string == "\\t'" || string == "'\\\\'" ||
                string == "'\\r'" || string == "'\0'");
    } else if (string.size() == 3) {
        return string[0] == string.back() && string.back() == '\'';
    }
    return false;
}

bool Tokenizer::isNumber(std::string_view string) {
    for (auto& c : string) {
        if (c < '0' || c > '9') return false;
    }
    return true;
}

bool Tokenizer::isString(std::string_view string) {
    if (string.size() < 2) return false;
    if (string[0] == '"' && string.back() == '"') {
        return true;
    }
    return false;
}

bool isPairOperator(char a, char b) {
    if (b == '=') {
        return (a == '<' || a == '=' || a == '>' || a == '+' || a == '-' ||
                a == '*' || a == '/' || a == '!');
    } else if (a == b) {
        return (/*a == '-' ||*/ a == ':' /*|| a == '+'*/ || a == '<' ||
                a == '>' || a == '|' || a == '&');
    } else {
        return ((a == '-' && b == '>') || (a == '<' && b == '-'));
    }
}

bool isSingleOperator(char a) {
    return a == '+' || a == '-' || a == '*' || a == '/' || a == '=' ||
           a == '<' || a == '>' || a == '!' || a == '.' || a == ':' ||
           a == '#' || a == '&' || a == '|' || a == ';' || a == ',' ||
           a == '(' || a == ')' || a == '{' || a == '}' || a == '[' || a == ']';
}
bool Tokenizer::tokenizeComment(char a, char b, CheckRepeat& rep) {
    if (a == '/' && b == '/') {
        flush(rep);

        while (p < end && *p != '\n') p++;

        if (p < end) {
            p++;
            pos.line++;
            pos.chr = 1;
        }

        tokenHead = p;
        return true;
    }

    if (a == '/' && b == '*') {
        flush(rep);

        p += 2;  // consume /*

        while (p < end - 1) {
            if (*p == '*' && *(p + 1) == '/') {
                p += 2;  // consume */
                tokenHead = p;
                return true;
            }

            if (*p == '\n') {
                pos.line++;
                pos.chr = 1;
            }

            p++;
        }

        throw std::runtime_error("Unclosed multiline comment");
    }

    return false;
}

bool Tokenizer::tokenizePairOp(char a, char b, CheckRepeat& rep) {
    if (isPairOperator(a, b)) {
        flush(rep);
        tokens.push_back(createToken(std::string_view(tokenHead, 2), rep,
                                     TokenType::Operator));

        pos.chr += 2;
        p += 2;
        tokenHead = p;
        return true;
    }
    return false;
}
bool Tokenizer::tokenizeString(char a, CheckRepeat& rep) {
    if (a == '"' || a == '\'') {
        char expectedEnd = a;

        flush(rep);

        tokenHead = p;

        p++;

        while (p < end && *p != '\n' && *p != expectedEnd) {
            // treat special chars \t etc... as one char
            if (*p == '\\') {
                p++;
            }
            p++;
        }

        if (p >= end) {
            throw std::runtime_error(
                "Unexpected end of file inside string literal\n");
        }

        if (*p == '\n') {
            std::string_view bad(tokenHead, p - tokenHead);

            throw std::runtime_error(
                "A string has to be declared in one line: " + std::string(bad));
        }

        p++;
        flush(rep);
        return true;
    }
    return false;
}
bool Tokenizer::tokenizeUinOp(char a, CheckRepeat& rep) {
    std::string_view sv(p, 1);

    if (isSingleOperator(a)) {
        flush(rep);

        if (isDelimiter(sv)) {
            tokens.push_back(createToken(sv, rep, TokenType::Delimiter));
        } else {
            tokens.push_back(createToken(sv, rep, TokenType::Operator));
        }

        pos.chr++;

        p++;
        tokenHead = p;
        return true;
    }
    return false;
}
bool Tokenizer::tokenizeNum(char a, CheckRepeat& rep) {
    if ('0' <= a && a <= '9' && tokenHead == p) {
        flush(rep);
        p++;
        while (p < end && '0' <= a && a <= '9') {
            a = *p;
            p++;
        }

        if (p + 1 < end && a == '.' && '0' <= *(p + 1) && *(p + 1) <= '9') {
            p += 2;
            a = *p;
            while (p < end && '0' <= a && a <= '9') {
                p++;
                a = *p;
            }

            std::string_view word(tokenHead, p - tokenHead);
            tokens.push_back(createToken(word, rep, TokenType::Float));
        } else {
            p--;
            std::string_view word(tokenHead, p - tokenHead);
            tokens.push_back(createToken(word, rep, TokenType::Number));
        }

        pos.chr += p - tokenHead;
        tokenHead = p;
        return true;
        ;
    }
    return false;
}

bool Tokenizer::tokenizeSpecial(char a, CheckRepeat& rep) {
    if (a == '\n') {
        flush(rep);
        pos.line++;

        pos.chr = 1;
        p++;
        tokenHead = p;
        return true;
    } else if (a == ' ' || a == '\t') {
        flush(rep);

        pos.chr++;
        p++;
        tokenHead = p;
        return true;
    }
    return false;
}
void Tokenizer::run() {
    offset = 0;
    max = 4096;
    tokensAdr = malloc(sizeof(char) * 4096);
    std::cout << "allocated 4 kb of ram!\n";

    CheckRepeat rep;

    pos.index = 0;
    pos.chr = 1;
    pos.line = 1;

    // point to the first and last char of the file
    tokenHead = fileContent.data();
    p = tokenHead;
    end = tokenHead + fileContent.size();

    char a;
    char b;

    while (p < end) {
        a = *p;
        if (p + 1 < end) {
            b = *(p + 1);

            // handle comments
            if (tokenizeComment(a, b, rep)) continue;
            // handle pair operators
            if (tokenizePairOp(a, b, rep)) continue;
        }

        // handle strings and chars
        if (tokenizeString(a, rep)) continue;

        // handle single operators or chars that are not alphanum
        if (tokenizeUinOp(a, rep)) continue;
        // handle numbers and floats
        if (tokenizeNum(a, rep)) continue;

        // handle special characters \n \t " "
        if (tokenizeSpecial(a, rep)) continue;
        p++;
    }

    flush(rep);
    std::cout << "Tokenization finished successfully!\n";
    std::ofstream outputFile(".\\k_build\\tokens.txt");

    for (const auto& t : tokens) {
        outputFile << std::string_view((char*)tokensAdr + t.id + 1)
                   << std::endl;
    }

    outputFile.close();
}

/*void Tokenizer::FillKeywordTable(std::unordered_map<std::string_view, int>
&avoidRepetition)
{
    static constexpr std::string_view keywordnames[] = {"if", "else", "while",
"fn", "var", "let", "return"};

    avoidRepetition.reserve(7);

    uint8_t *base = static_cast<uint8_t *>(tokensAdr);

    int i = 0;

    for (const auto &keyword : keywordnames) {
        size_t needed = offset + keyword.size() + 2;

        if (needed >= max) {
            while (max < needed)
                max *= 2;

            tokensAdr = realloc(tokensAdr, max);
            base = static_cast<uint8_t *>(tokensAdr);
        }

        avoidRepetition[keyword] = offset;

        uint8_t *cursor = base + offset;

        *cursor++ = static_cast<uint8_t>(TokenType::Keyword);

        memcpy(cursor, keyword.data(), keyword.size());
        cursor += keyword.size();

        *cursor = '\0';

        KeywordTable[i++] = offset;
        offset += keyword.size() + 2;
    }
}
*/
Token Tokenizer::createToken(std::string_view string,
                             CheckRepeat& avoidRepetition, TokenType type) {
    Token n;
    n.pos.chr = pos.chr;
    n.pos.line = pos.line;
    // pos.index = fileContent.data() - p;
    n.pos.index = static_cast<uint32_t>(p - fileContent.data());
    ;

    if (avoidRepetition.contains(string)) {
        n.id = avoidRepetition.at(string);
    } else {
        size_t needed = offset + string.size() + 2;
        if (needed >= max) {
            while (max < needed) max *= 2;
            tokensAdr = realloc(tokensAdr, sizeof(char) * max);
        }
        uint8_t* base = static_cast<uint8_t*>(tokensAdr);
        avoidRepetition[string] = offset;
        uint8_t* cursor = base + offset;
        n.id = offset;

        *cursor++ = static_cast<uint8_t>(type);
        memcpy(cursor, string.data(), string.size());
        cursor += string.size();

        *cursor = '\0';

        offset += string.size() + 2;  // 1 for \0 and one byte for the TokenType
    }

    return n;
};