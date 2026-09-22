#pragma once
#include "context.hpp"
#include <iostream>
uint64_t readNum(const Value &v);
class compile {
  public:
    global G;

    std::string flat_str_array;
    size_t flat_str_size = 0;
    std::vector<std::string> functions;
    std::ostream &out;
    int counter = 0;

    // Id load_ptr(const variable &var);
    Id load_ptr(const variable *var);
    // Id check_var(const variable &var);
    Id check_var(const variable *var);
    // void write_var(const variable &var, Id id = 0);
    void write_var(const variable *var, Id id = 0);
    //
    //
    class operand {
      public:
        const bool isId;
        const Value &val;
        const variable *var;
        const Id newId;
        operand(compile &c, const function &f, const Value &val)
            : isId(val.type == Val_t::ID), val(val),
              var(isId ? &f.local.at(getId(val.data)) : nullptr),
              newId(isId ? c.check_var(var) : 0) {}
        void write(compile &c) {
            if (isId) {
                c.write_var(var, newId);
            } else {
                c.out << readNum(val);
            }
        }
    };
    //
    //
    void fill_str_array();
    void cmp_def(const function &f, const instruction &i);
    void cmp_var(const function &f, const instruction &i);
    void cmp_call(const function &f, const instruction &i);
    void cmp_store(const function &f, const instruction &i);
    void cmp_ret(const function &f, const instruction &i);
    void cmp_cmp(const function &f, const instruction &i); // compile comparison
    void cmp_math(const function &f, const instruction &i);
    void cmp_function(const function &f);
    void cmp_instruction(const function &f, const instruction &i,
                         size_t index = 0);

    void run();

    compile(const std::string &path, std::ostream &output_stream = std::cout)
        : G(path), out(output_stream) {}
};
