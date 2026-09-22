#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "common.hpp"

enum class Val_t : uint8_t { ID, NUM, STR, PTR, NUL };

struct Value {
    Val_t type = Val_t::NUL;
    const void *data = nullptr;
    uint32_t size = 0;
};
Id getId(const void *ptr);
enum class Inst : uint8_t {
    IMATH,
    ICMP,
    UCALL,
    SCALL,
    DEC,
    MOVE,
    STORE,

};

struct instruction {
    Inst head;
    uint8_t option;
    std::vector<Value> args;
    Value result;
};

struct variable {
    Id name;
    Type type = Type::unknown;
    bool isConst = true;
};

Inst getInstuctionHead(OP op);
/*
 * Function
 * -
 * name, type, params,local and instructions are prettry self explanatory.
 *
 *
 * - head : beginning of the function's IR stream.
 * - it   : current read position.
 * - end  : one-past-the-end marker.
 *
 * Invariant:
 *     head <= it <= end
 */
class function {
  private:
    const uint8_t *head;
    const uint8_t *it;
    const uint8_t *end;

    /*
        this is the main parse method it parses this function only starting from
       the operator "FUN" to the operator "END" "string_pool" si the global
       string pool
    */
    void parse();
    /*
     * - DEF tag: v1 : i32 -> tag 10
     * - VAR tag: v2 : i32
     */
    void declaration(bool isConst);

    /*
    * this function should take care of function calls wethere they are user
    defined or standard
    * functions here example format:
    *
    * - CALL STD PRINT tag: strSize(15) "Hello, world!\n"
    * - CALL USR tag: v2 argCount:1 (tag:5) -> tag: v19
    *
    * here the function UID is the number 2 it has one argument and its the
    number 5 and the result is save at the ID 19 (var or const)
    */
    void functionCall();

    void simpleInstuction(int arg_count);
    void parseInstuction();
    // works for strings Ids and numbers
    Value getValue();
    void resolve_types();

  public:
    Id name;
    Type type = Type::unknown;
    std::vector<variable> params;
    std::unordered_map<Id, variable> local;
    const uint8_t *get_iterator() const { return it; }
    std::vector<instruction> instructions;
    void parse_public() {
        parse();
        resolve_types();
    }
    function(const uint8_t *iterator, const int size) : it(iterator) {
        end = it + size;
        head = it;
    }
};
struct string_info {
    size_t offset;
    size_t size;
};
class global {
  public:
    Id main_function;

    std::vector<uint8_t> ir;
    const uint8_t *it;
    const uint8_t *end;

    std::vector<function> functions;
    void gen_context();
    // void compile();

    std::unordered_map<std::string, string_info> string_pool;
    global(const std::string &path);

    void run();
    // void load(const uint8_t *ir, std::size_t size);
    void print(FILE *out = stdout) const;
};
