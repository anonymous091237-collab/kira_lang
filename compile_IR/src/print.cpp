#include <format>

#include "../../include/debug.h"
#include "compiler.hpp"
#include "navigateIR.hpp"

void generate_llvm_str(const string& src, string& dst, int& size) {
    size = 0;
    dst.clear();

    const char* c = src.data();
    const char* end = c + src.size();

    while (c < end) {
        if (*c == '\\' && (c + 1) < end) {
            // Special chars
            char next = *(c + 1);

            if (next == 'n') {
                dst += "\\0A";
                c += 2;
            } else if (next == 't') {
                dst += "\\09";
                c += 2;
            } else if (next == 'r') {
                dst += "\\0D";
                c += 2;
            } else if (next == '\\') {
                dst += "\\5C";
                c += 2;
            } else if (next == '\"') {
                dst += "\\22";
                c += 2;
            } else {
                dst += "\\5C";
                c += 1;
            }
        } else {  // Normal printable character
            dst += *c;
            c++;
        }

        // Every char =  1 byte in the final  array
        size++;
    }
    size++;  // for the null terminator
    dst += "\\00";
}
VarId extend_variable(string& result, Type type, const VarId& name)

{
    VarId dst = create_temp();
    result += std::format("%v{}  = zext {} %v{} to i32\n", dst,
                          Type_to_str.at(type), name);
    return dst;
}

void LlvmBuilder::print_function() {
    DEBUG_PRINT("entering print_function()\n");
    const Tag printTag = nextTag(current_token);

    if (Tag::STR8 <= printTag && printTag <= Tag::STR64) {
        DEBUG_PRINT("print arg is a string\n");
        VarInfo v;

        v.id = create_temp();
        v.is_pointer = true;
        v.type = Type::i8;

        variables[v.id] = v;

        const string printedString = nextStr(current_token, printTag);
        DEBUG_PRINT("printedString %s\n", printedString.c_str());
        const int offset = store_strings.at(printedString);

        result += std::format(
            "%v{} = getelementptr inbounds [{} x i8], ptr @flat_block, i32 0, "
            "i32 {}\n"
            "call i32 (ptr, ...) @printf(ptr %v{})\n",
            v.id, string_offset, offset, v.id);

    } else if (printTag == Tag::ID) {
        const VarInfo v = variables.at(nextId(current_token));

        const VarId var_name = (v.is_pointer) ? load_ptr(result, v) : v.id;

        switch (v.type) {
            case Type::i64:

                result += std::format(
                    "call i32 (ptr, ...) @printf(ptr @long_print, i64 %v{})\n",
                    var_name);
                break;
            case Type::u64:
                result += std::format(
                    "call i32 (ptr, ...) @printf(ptr @ulong_print, i64 %v{})\n",
                    var_name);
                break;
            case Type::i32:
                result += std::format(
                    "call i32 (ptr, ...) @printf(ptr @int_print, i32 %v{})\n",
                    var_name);
                break;
            case Type::u32:
                result += std::format(
                    "call i32 (ptr, ...) @printf(ptr @uint_print, i32 %v{})\n",
                    var_name);
                break;
            case Type::i16:
            case Type::u16:
            case Type::i8:
            case Type::u8: {
                const VarId newVar = extend_variable(result, v.type, var_name);
                result += std::format(
                    "call i32 (ptr, ...) @printf(ptr @int_print, i32 %v{})\n",
                    newVar);
                break;
            }

            default:
                throw std::runtime_error("unsupported print type !");
                break;
        }
    }

    DEBUG_PRINT("token : %d", (int)(*current_token));
    DEBUG_PRINT("exiting print function\n");
}

void LlvmBuilder::define_globals() {
    result += std::format(
        "@long_print = private unnamed_addr constant [5 x i8] "
        "c\"%lld\\00\", align 1\n");
    result += std::format(
        "@int_print = private unnamed_addr constant [3 x i8] "
        "c\"%d\\00\", align 1\n");
    result += std::format(
        "@ulong_print = private unnamed_addr constant [5 x i8] "
        "c\"%llu\\00\", align 1\n");
    result += std::format(
        "@uint_print = private unnamed_addr constant [3 x i8] "
        "c\"%u\\00\", align 1\n");

    result += std::format(
        "@flat_block = private unnamed_addr constant [{} x i8] c\"{}\", align "
        "1\n",
        string_offset, flat_str_array);

    result += "declare i32 @printf(ptr noundef, ...)\n";
}