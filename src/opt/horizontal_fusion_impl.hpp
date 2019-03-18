#ifndef MIGRAPHX_GUARD_RTGLIB_HORIZONTAL_FUSION_IMPL_HPP
#define MIGRAPHX_GUARD_RTGLIB_HORIZONTAL_FUSION_IMPL_HPP
#include "common_header.hpp"
#include <migraphx/config.hpp>

#include <set>

#define MIGRAPHX_DEBUG_H_FUSION

namespace migraphx {

inline namespace MIGRAPHX_INLINE_NS {

// Nodes representing hashed instructions.
struct hash_value
{
    unsigned id = 0;
    unsigned cur_point = 0;
};

using hash_value_ptr = hash_value*;
using key_type = unsigned long long;

//  Instruction encoding information, used to hash instructions.
struct encode_info
{
    key_type key;
    bool valid;
    encode_info(key_type k, bool v) : key(k), valid(v)
    {
    }

    void add_input(hash_value_ptr p) { inputs.push_back(p); }
    key_type get_key() const { return key; }
    void set_key(key_type k) { key = k; }
    const std::vector<hash_value_ptr>& get_inputs() const { return inputs; }
    bool is_valid() const { return valid; }

    private:
    std::vector<hash_value_ptr> inputs;
};


using Ins2Val = std::unordered_map<instruction_ref, hash_value_ptr>;
using String2Val = std::unordered_map<std::string, unsigned>;
           
using Encoder = std::function<encode_info(instruction_ref, Ins2Val&, unsigned)>;

struct horizontal_fusion_impl
{
    horizontal_fusion_impl(program* p)
        : p_program(p)
    {
        instr2_hash.clear();
        instr2_value.clear();
        point2_instr.clear();
        encode2_value.clear();
        cur_point = 0;
        opcode_id = 0;
        hash_inputs.clear();
        hash_outputs.clear();
        hash_instrs.clear();
        values.reserve(p_program->size());
        opcode_table.clear();
        register_all();
    }
    void run();
    void process(instruction_ref ins);
    hash_value_ptr hash(instruction_ref ins);
    void add_root(hash_value_ptr ptr)
    {
        root_values.push_back(ptr);
    }

    hash_value& get_value(unsigned id) { return values[id]; }

    hash_value& create_value(instruction_ref ins);

    void add_instr(unsigned id)
    {
        if (hash_instrs.find(id) == hash_instrs.end())
        {
            std::set<unsigned> vals;
            vals.insert(cur_point);
            hash_instrs[id] = vals;
        }
        else {
            hash_instrs[id].insert(cur_point);
        }
    }

    void add_input(unsigned id, hash_value_ptr ptr)
    {
        if (hash_inputs.find(id) == hash_inputs.end())
        {
            std::set<hash_value_ptr> vals;
            vals.insert(ptr);
            hash_inputs[id] = vals;
        } else if (hash_inputs[id].find(ptr) == hash_inputs[id].end())
            hash_inputs[id].insert(ptr);
    }

    void add_output(unsigned id, hash_value_ptr ptr)
    {
        if (hash_outputs.find(id) == hash_outputs.end())
        {
            std::set<hash_value_ptr> vals;
            vals.insert(ptr);
            hash_outputs[id] = vals;
        } else if (hash_outputs[id].find(ptr) == hash_outputs[id].end())
            hash_outputs[id].insert(ptr);
    }
    unsigned hash_opcode(instruction_ref ins)
    {
        std::ostringstream stream;
        stream << ins->get_operator();
        std::string str = stream.str();
        if (opcode_table.find(str) == opcode_table.end())
            opcode_table[str] = opcode_id++;
        return opcode_table[str];
    }
    
    void register_op(std::string, Encoder);
    void register_all();
    void transform();
    
#ifdef MIGRAPHX_DEBUG_H_FUSION
    void dump_program();
    void dump_hash_value(hash_value&);
    void dump_hash_tree();
#endif
   private:
    program* p_program;
    // Flag an instruction to hash.
    std::unordered_map<instruction_ref, bool> instr2_hash;
    // Map an instruction to a hash value pointer.
    Ins2Val instr2_value;
    std::unordered_map<unsigned, instruction_ref> point2_instr;
    // Map an encoding to a hash value pointer.
    std::unordered_map<key_type, hash_value_ptr> encode2_value;
    // Map an operation name to its encoder function.
    std::unordered_map<std::string, Encoder> op_registry;
    // Map an opcode string to a value.
    String2Val opcode_table;
    // Universe of hash values.
    std::vector<hash_value> values;
    // A collection of root nodes in the hash_value tree.
    std::vector<hash_value_ptr> root_values;
    // Map of hash value id to hash-value inputs.
    std::unordered_map<unsigned, std::set<hash_value_ptr>> hash_inputs;
    // Map of hash value id to hash-value outputs.
    std::unordered_map<unsigned, std::set<hash_value_ptr>> hash_outputs;
    // Map of hash value id to instructions having the same hash value.
    std::unordered_map<unsigned, std::set<unsigned>> hash_instrs;
    // Current program point.
    unsigned cur_point;
    // Opcode id.
    unsigned opcode_id;
};

           
// Encoding functions.
encode_info EncodeCommon(instruction_ref in, Ins2Val&instr2_value, unsigned);
encode_info EncodeConvCommon(instruction_ref in, Ins2Val&instr2_value, unsigned);
           
} // namespace MIGRAPHX_INLINE_NS
} // namespace migraphx
#endif
