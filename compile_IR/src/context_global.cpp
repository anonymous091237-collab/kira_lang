#include "context.hpp"
#include "debug.hpp"
#include "navigateIR.hpp"

#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
// Id getId(const void *ptr) { return *(static_cast<const Id *>(ptr)); }
global::global(const std::string &path) {

    if (!std::filesystem::exists(path)) {
        std::cerr << "Error: file not found: " << path << '\n';
    }

    std::ifstream file(path, std::ios::binary | std::ios::ate);

    if (!file) {
        throw std::runtime_error("Error: could not open: " + path + '\n');
    }

    const std::size_t size = static_cast<std::size_t>(file.tellg());

    file.seekg(0);
    ir.resize(size);

    if (!file.read(reinterpret_cast<char *>(ir.data()), size)) {
        throw std::runtime_error("Error: failed to read: " + path + '\n');
    }

    it = ir.data();
    end = it + ir.size() - 5;
}
int get_str_size(const std::string &src) {
    const char *c = src.data();
    const char *e = c + src.size();
    int res = 0;
    while (c < e) {
        if (*c != '\\') {
            c++;
        }
        c++;
        res++;
    }
    return res;
}
void global::gen_context() {

    while (it < end) {
        if (static_cast<OP>(*it) != OP::FUN) {
            throw std::runtime_error("global::load - expected FUN, got byte " +
                                     std::to_string(*it) + " at offset " +
                                     std::to_string(it - ir.data()));
        }

        const uint8_t *fn_start = it;
        const int remaining_size = static_cast<int>(end - fn_start);

        functions.emplace_back(fn_start, remaining_size);
        function &fn = functions.back();

        fn.parse_public();

        // Advance iterator past this function's OP::END
        it = fn.get_iterator();
    }

    // parse footer (5 bytes) = 1bt (tag) + 4bt (uint32_t) Id
    if (it < end + 5) {

        skipTag(it);
        main_function = nextId(it);
    }

    int accumulated_size = 0;

    // Final pass through the whole code

    for (auto &f : functions) {
        for (auto &i : f.instructions) {
            if (i.head == Inst::UCALL) {
                std::cout << "function call resolve type\n";
                const auto funId = getId(i.args[0].data);
                const auto var = getId(i.result.data);

                // find the function
                bool found = false;
                for (auto &function : functions) {
                    if (function.name == funId) {
                        found = true;
                        f.local.at(var).type = function.type;
                        if (f.local.contains(funId)) {
                            f.local.erase(funId);
                        }
                        break;
                    }
                }
                if (!found) {
                    throw std::runtime_error("expected a valid function call, "
                                             "function does not exist!\n");
                }
            } else {
                for (auto &val : i.args) {

                    if (val.type == Val_t::STR) {
                        const std::string str = std::string{
                            static_cast<const char *>(val.data), val.size};

                        const auto it = string_pool.find(str);
                        if (it == string_pool.end()) {
                            string_pool[str] = {
                                .offset = (size_t)accumulated_size, .size = 0};
                            accumulated_size += get_str_size(str);
                        }
                    }
                }
            }
        }
    }
}

void global::run() {
    if (ir.size() < 5) {
        throw std::runtime_error(
            "global::load - file size too small for header/footer\n");
    }
    std::cout << "beginning context generation...\n";
    gen_context();
}

void global::print(FILE *out) const {
    fprintf(out, "=== IR dump  (%zu functions) ===\n\n", functions.size());

    if (!string_pool.empty()) {
        fprintf(out, "--- string pool (%zu entries) ---\n", string_pool.size());
        for (const auto &[str, id] : string_pool)
            fprintf(out, "  [%zu] \"%s\"\n", id, str.c_str());
        fprintf(out, "\n");
    }

    // fprintf(out, "=== IR dump  (%zu functions) ===\n\n", functions.size());

    for (const auto &fn : functions) {
        print_function(fn, out);
    }

    fprintf(out, "=== end of dump ===\n");
}
