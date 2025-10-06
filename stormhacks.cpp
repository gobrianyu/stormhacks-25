#include <vector>
#include <string>
#include <unordered_set>
#include <iostream>
#include <unordered_map>
#include <algorithm>
using namespace std;

class Node { 
private: 
    std::string name_; // The name of the operator node, such as Add, Mul, etc. 
    std::vector<Node> inputs_; // All input nodes of the operator node
    int run_mem_; // Memory required for the operator's computation process 
    int output_mem_; // Memory occupied by the operator's computation result 
    int time_cost_; // Time taken for the operator's computation 
public:
    // constructor
    Node(string n="", long r=0, long o=0, long t=0, vector<Node> inps = {})
        : name_(n), run_mem_(r), output_mem_(o), time_cost_(t), inputs_(inps) {}

    // getters
    const string& getName() const { return name_; }
    const vector<Node>& getInputs() const { return inputs_; }
    long getRunMem() const { return run_mem_; }
    long getOutputMem() const { return output_mem_; }
    long getTimeCost() const { return time_cost_; }
};


struct Scheduler {
    const vector<Node> &all_nodes;
    unordered_map<string, int> name2id;
    long total_memory;
    long memory_used = 0;
    long time = 0;
    
    unordered_set<string> stored_nodes;  // node outputs that are currently stored
    vector<Node> exec_seq;  // execution sequence to return

    Scheduler(const vector<Node> &nodes, long total_memory) : all_nodes(nodes), total_memory(total_memory) {
        for (int i = 0; i < (int)nodes.size(); ++i) {
            name2id[nodes[i].getName()] = i;
        }
    }

    const Node& get(const string &name) {
        return all_nodes[name2id.at(name)];
    }


    // Free enough memory to fit a new computation output of size `mem_req`
    void evict_from_mem(long mem_req) {
        // Case: already enough memory
        if (mem_req <= total_memory - memory_used) return;

        long target_mem_req = mem_req - (total_memory - memory_used);
        
        vector<pair<double, string>> evict_candidates;
        for (auto &s : stored_nodes) {
            double score = eviction_score(s);
            if (score != -1e18) {
                evict_candidates.push_back({score, s});
            }
        }

        sort(evict_candidates.begin(), evict_candidates.end());

        long freed_mem = 0;
        size_t index = 0;
        while (freed_mem < target_mem_req && index < evict_candidates.size()) {
            string evict_node = evict_candidates[index].second;
            const Node &node = get(evict_node);
            stored_nodes.erase(evict_node);
            memory_used -= node.getOutputMem();
            freed_mem += node.getOutputMem();
            index++;
        }

        if (freed_mem < target_mem_req) {
            throw runtime_error("Memory limit too small to run any operator nodes.");
        }
    }
    
    // Greedy heuristics <-- may require changes for optimisation
    double eviction_score(const string &name) {
        const Node &node = get(name);
        if (node.getOutputMem() == 0) return -1e18;
        return double(node.getTimeCost()) / double(node.getOutputMem());
    }

    void processNode(const string &name) {
        // Case: already processed node
        if (stored_nodes.count(name)) return;

        const Node &node = get(name);
        for (auto &inp : node.getInputs()) processNode(inp.getName());
        long mem_req = node.getRunMem() + node.getOutputMem();
        evict_from_mem(mem_req);
        time += node.getTimeCost();
        exec_seq.push_back(node);

        stored_nodes.insert(name);
        memory_used += node.getOutputMem();
        
    }

    vector<Node> run(const string &final_node) {
        processNode(final_node);
        return exec_seq;
    }
};


std::vector<Node> ExecuteOrder(const std::vector<Node> &all_nodes, const std::string &output_name, long total_memory) {
    Scheduler sched(all_nodes, total_memory);
    vector<Node> seq = sched.run(output_name);
    return seq;
}