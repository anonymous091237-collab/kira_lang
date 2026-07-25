#pragma once

#include <string>
#include <vector>

#include "../../include/constants.hpp"

//===========================================================
struct VarInfo {
    VarId id;
    Type type = Type::unknown;
    bool is_pointer = false;
};

struct FunInfo {
    VarId id;
    Type type = Type::unknown;
    int param_count = 0;
};

struct Match {  // v1 and v2 should match type
    VarId v1;
    VarId v2;
};
//===========================================================
using std::string;
using std::string_view;
using std::vector;
using varTable = std::unordered_map<VarId, VarInfo>;
using funTable = std::unordered_map<VarId, FunInfo>;
using LookupTable = std::unordered_map<string_view, Type>;
//===========================================================
char get_alignment(const Type& type);
string_view llvm_cmp(const OP& op, bool isSigned);
bool is_signed(const Type& type);
void gen_math_expression(string& result, const OP& oprtr, uint64_t& A,
                         uint64_t& B, const Tag tagA, const Tag tagB,
                         const VarInfo& dst, varTable& variables);

VarId load_ptr(string& output, const VarInfo& v);
void extend_variable(string& result, Type Type, const string_view& name,
                     string& dst);

bool is_numeric(string_view sv);
void fill_both_lists(const vector<string_view>& tokens, varTable& variables,
                     vector<Match>& constraints, funTable& functions);
void smart_resolve_Types(varTable& vars, vector<Match>& constraints);
void generate_llvm_str(const string& src, string& dst, int& size);
VarId create_temp();
//===========================================================
inline int var_count = 1;
inline int tmpnum = 1;
class LlvmBuilder {
   private:
    string result;

    const uint8_t* current_token;
    const uint8_t* last_token;

    void define_globals();
    void standard_functions();
    void print_function();
    void gen_var();
    void gen_math(const OP& oprtr);
    void gen_negative();
    void gen_const();
    void gen_store();
    void generate_instuctions(VarId function_name, OP& op);
    void generate_function();

   public:
    VarId mainId;
    vector<uint8_t> tokens;
    string flat_str_array;
    std::unordered_map<string, int> store_strings;  // stores string literals
    int string_offset = 0;  // stores accumulated string sizes (matches the

    varTable variables;
    funTable functions;
    void load_IR(const string& IR_path);
    void generate_ll(const string& ll_path);
    void run();

    LlvmBuilder() = default;
};
