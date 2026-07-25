#pragma once
#include "compiler.hpp"

void smart_resolve_Types(varTable& variables, vector<Match>& constraints);
class Resolver {
   private:
    void resolve_fun(const uint8_t*& token);
    void resolve_math(const uint8_t*& token);
    void resolve_cmp(const uint8_t*& token);
    void saveString(const string& str2);

   public:
    int string_offset = 0;
    string* flatStrArray;
    std::unordered_map<string, int>* store_strings;
    vector<uint8_t>* tokens;
    varTable* variables;
    funTable* functions;

    Resolver(string* flat_str_array,
             std::unordered_map<string, int>* store_strings,
             vector<uint8_t>* tokens, varTable* variables, funTable* functions)
        : flatStrArray(flat_str_array),
          store_strings(store_strings),
          tokens(tokens),
          variables(variables),
          functions(functions) {}

    vector<Match> constraints;
    void build_type_context();
};