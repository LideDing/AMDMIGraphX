#include "pre_scheduling_impl.hpp"
#include <migraphx/iterator_for.hpp>
#include <stack>
namespace migraphx {


void pre_scheduling_impl::compute_weights()
{
    int ndx = 0;
    std::unordered_map<dag_node*, bool> visited;
    for(auto ins : iterator_for(*p_program))
    {
        dag_node& node = nodes[ndx];
        std::string name = ins->name();
        std::pair<int,int> weight = weight_func(name);
        node.weight = weight.first;
        node.run_on_cpu = weight.second;
        node.weight_sum += node.weight;
        visited.clear();

        for(auto&& arg : ins->inputs()) {
            assert(instr2_node.find(arg) != instr2_node.end());
            dag_node* def_node = instr2_node[arg];
            if (visited.find(def_node) == visited.end()) {
                node.weight_sum += def_node->weight_sum;
                visited[def_node] = true;
            }
        }
        if (ins->outputs().empty()) {
            exit_nodes.push_back(&node);
        } 
        node.ins = ins;
        node.ins_ndx = ndx++;
        instr2_node[ins] = &node;
    }
    int size = exit_nodes.size();
    if (size > 1) {
        std::sort(exit_nodes.begin(), exit_nodes.end(), compare_exit_nodes);
    }
}

void pre_scheduling_impl::reorder()
{
    std::list<dag_node*> sorted_nodes;
    std::stack<dag_node*> stack;
    std::priority_queue<dag_node*, std::vector<dag_node*>, weighted_topology_ordering> child_queue;
    std::unordered_map<dag_node*, bool> visited;
    std::unordered_map<dag_node*, bool> dequeued;
    
    for (auto&& node : exit_nodes)
    {
        stack.push(node);
        node->partition = partition_info.create_partition();
        partition_info.add_weight(node);
        while (!stack.empty()) {
            auto cur = stack.top();
            if (dequeued.find(cur) != dequeued.end()) {
                stack.pop();
                continue;
            } else if ((visited.find(cur) != visited.end()) || cur->ins->inputs().empty()) {
                stack.pop();
                sorted_nodes.push_back(cur);
                dequeued[cur] = true;
                continue;
            }
            // sort child nodes.
            for(auto&& arg : cur->ins->inputs()) {
                dag_node* child_node = instr2_node[arg];
                if (dequeued.find(child_node) == dequeued.end()) {
                    child_queue.push(child_node);
                }
            }

            // Last item in queue is on critical path.
            while (!child_queue.empty()) {
                dag_node * child = child_queue.top();
                stack.push(child);
                child_queue.pop();
                if (child->weight_sum < min_partition_threshold)
                    child->partition = cur->partition;
                else if (!child_queue.empty())
                    child->partition = partition_info.create_partition();
                else {
                    cur->first_child = child;
                    child->partition = cur->partition;
                }
                partition_info.add_weight(child);
            }
            visited[cur] = true;
        }
    }

#ifdef MIGRAPH_DEBUG_OPT
    MIGRAPH_DEBUG(dump("---After weighted topology sort---"));
    MIGRAPH_DEBUG(dump(sorted_nodes));
#endif
#if 1
    schedule(sorted_nodes);
#endif    
    splice(sorted_nodes);

#ifdef MIGRAPH_DEBUG_OPT
    verify();
#endif
}

int pre_scheduling_impl::get_stream(stream_info& info, dag_node* node)
{
    int max_cycle = info.max_cycle;
    if (max_cycle == 0)
        return 0;
    int partition_load = partition_info.weight_sum[node->partition];
    int earliest_cycle = node->earliest_cycle;
    int min_cycle = -1;
    int min_cycle_stream = -1;
    for (auto stream = 0; stream < num_of_streams; ++stream)
    {
        int cycle = std::max(info.next_cycles[stream], earliest_cycle);
        if ((cycle < max_cycle) && ((max_cycle - cycle) > partition_load))
            return stream;
        if ((min_cycle_stream == -1) || (cycle < min_cycle))
        {
            min_cycle = cycle;
            min_cycle_stream = stream;
        }
    }
    return min_cycle_stream;
}

void pre_scheduling_impl::record(stream_info& info, dag_node* node)
{
    int stream = node->stream;
    int next_cycle = info.next_cycles[stream];
    node->sched_cycle = std::max(node->earliest_cycle, next_cycle);
    next_cycle = node->sched_cycle + node->weight;
#if 0    
    if (node->run_on_cpu) {
        for (auto iter = 0; iter < num_of_streams; ++iter)
        {
            info.next_cycles[iter] = std::max(info.next_cycles[iter], next_cycle);
        }
    }
#endif    
    
    info.next_cycles[stream] = next_cycle;
    info.max_cycle = std::max(info.max_cycle, next_cycle);
    for (auto&& arg : node->ins->outputs())
    {
        assert(instr2_node.find(arg) != instr2_node.end());
        dag_node* use_node = instr2_node[arg];
        use_node->earliest_cycle = std::max(use_node->earliest_cycle, next_cycle);
    }
    if (node->can_use_stream()) {
        node->ins->set_stream(stream);
        for(auto&& arg : node->ins->inputs()) {
            int arg_s = arg->get_stream();
            if ((arg_s < 0) || (arg_s == stream))
                continue;
            arg->add_mask(RECORD_EVENT);
            node->ins->add_mask(WAIT_EVENT);
        }
    }
}
    
void pre_scheduling_impl::schedule(std::list<dag_node*>& sorted_nodes)
{
    if (num_of_streams == 0)
        return;
    stream_info info(num_of_streams);
    std::unordered_map<int, int> partition2_stream;
    partition2_stream.clear();
    std::priority_queue<dag_node*, std::vector<dag_node*>, post_schedule_ordering> queue;
    bool stream_used = false;
    for (auto&& node : sorted_nodes) {
        int cur_partition = node->partition;
        assert(cur_partition >= 0);
        if (partition2_stream.find(cur_partition) != partition2_stream.end())
        {
            node->stream = partition2_stream[cur_partition];
        }
        else {
            node->stream = get_stream(info, node);
        }
        assert(node->stream >= 0);
        stream_used = true;
        record(info, node);
        partition2_stream[cur_partition] = node->stream;
        queue.push(node);
    }

#ifdef MIGRAPH_DEBUG_OPT    
    MIGRAPH_DEBUG(dump("---After assigning stream---"));
    MIGRAPH_DEBUG(dump(sorted_nodes));
#endif    
    if (stream_used) {
        sorted_nodes.clear();
        while (!queue.empty())
            {
                dag_node* node = queue.top();
                queue.pop();
                sorted_nodes.push_back(node);
            }
    }

#ifdef MIGRAPH_DEBUG_OPT
    MIGRAPH_DEBUG(dump("---After sorting schedule---"));
    MIGRAPH_DEBUG(dump(sorted_nodes));
#endif
}
    
void pre_scheduling_impl::splice(std::list<dag_node*>& sorted_nodes)
{
    auto begin = sorted_nodes.begin();
    auto iter = sorted_nodes.end();
    instruction_ref insert_before = (*(--iter))->ins;
    do {
        iter--;
        insert_before = p_program->move_instruction((*iter)->ins, insert_before);
    } while (iter != begin);

#ifdef MIGRAPH_DEBUG_OPT
    MIGRAPH_DEBUG(dump("---After pre-scheduling---"));
    MIGRAPH_DEBUG(dump_program());
#endif    
}
    
void pre_scheduling_impl::run()
{
    std::size_t num_of_instrs = p_program->size();
    if(num_of_instrs == 0)
        return;
    MIGRAPH_DEBUG(dump("---Before pre-scheduling---"));
    MIGRAPH_DEBUG(dump_program());
    nodes.resize(num_of_instrs);
    compute_weights();
    reorder();
}

#ifdef MIGRAPH_DEBUG_OPT
void pre_scheduling_impl::dump(const std::string& str)
{
    std::cout << str << std::endl;
}

void pre_scheduling_impl::dump_program()
{
    std::cout << *p_program << std::endl;
}

void pre_scheduling_impl::dump(std::list<dag_node*>& sorted_nodes)
{
    for (auto&& node : sorted_nodes)
    {
        node->dump();
        if (!node->ins->inputs().empty()) {
            std::cout << " inputs: ";
            for(auto&& arg : node->ins->inputs()) {
                dag_node* def_node = instr2_node[arg];
                std::cout << " @" << def_node->ins_ndx;
            }
            std::cout << std::endl;
        }
    }
}

void pre_scheduling_impl::verify()
{
    std::unordered_map<instruction_ref, bool> visited;
    for(auto ins : iterator_for(*p_program))
    {
        for(auto&& arg : ins->inputs()) {
            assert(visited.find(arg) != visited.end());
        }
        visited[ins] = true;
    }
}

void dag_node::dump()
{
    std::cout << " @" << ins_ndx;
    std::cout << " name: " << ins->name();
    std::cout << " weight: " << weight;
    std::cout << " weight_sum: " << weight_sum;
    if (can_use_stream())
        std::cout << " stream: " << stream;
    std::cout << " partition: " << partition;
    std::cout << " sched_cycle: " << sched_cycle;
    std::cout << std::endl;
}
#endif    
    
} // namespace migraph