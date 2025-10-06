#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
using namespace std;

class Node { 
 private: 
 std::string name_;
 std::vector<Node> inputs_;
 int run_mem_;
 int output_mem_;
 int time_cost_;
 
 public:
 // constructor
 Node(string name, int run_mem, int output_mem, int time_cost)
     : name_(name), run_mem_(run_mem), output_mem_(output_mem), time_cost_(time_cost) {}
 
 // helper method to add input nodes
 void addInput(const Node& inputNode) {
     inputs_.push_back(inputNode);
 }
 
 // getters for debugging
 string getName() const { return name_; }
 int getRunMem() const { return run_mem_; }
 int getOutputMem() const { return output_mem_; }
 int getTimeCost() const { return time_cost_; }
};

std::vector<Node> parseFile(const char* filename, int& maxMemoryLimit) {
    // open the file
    ifstream infile;
    infile.open(filename);
    
    // file open fail checker
    if (!infile.is_open()) {
        cerr << "Error opening file!" << endl;
        return {};
    }
    
    // Read first line for max memory limit
    string label, line;
    getline(infile, line);
    istringstream iss(line);

    if (iss >> label >> maxMemoryLimit) {
        cout << "maxMemoryLimit: " << maxMemoryLimit << endl;
    }
    else {
        cerr << "Error reading max memory limit!" << endl;
        return {};
    }

    vector<Node> results;
    
    // Temporary storage for input IDs (we'll use these to link nodes later)
    vector<vector<int>> allInputIDs;
    
    int opID;
    string opName;
    int numInputs;
    
    // First pass: Create all nodes without inputs
    while (infile >> opID >> opName >> numInputs) {
        // Read the input node IDs
        vector<int> inputIDs;
        for (int i = 0; i < numInputs; i++) {
            int inputID;
            infile >> inputID;
            inputIDs.push_back(inputID);
        }
        
        // Read the memory and time cost values
        int run_mem, output_mem, time_cost;
        infile >> run_mem >> output_mem >> time_cost;
        
        // Create Node (without inputs for now)
        Node node(opName, run_mem, output_mem, time_cost);
        results.push_back(node);
        allInputIDs.push_back(inputIDs);
        
        // Debug output
        cout << "Created Node " << opID << ": " << opName 
             << ", RunMem: " << run_mem << ", OutputMem: " << output_mem 
             << ", TimeCost: " << time_cost << endl;
    }
    
    // Second pass: Link nodes using input IDs as indices
    for (size_t i = 0; i < results.size(); i++) {
        // for inputID in allInputIDs[i], link input node to current node
        for (int inputID : allInputIDs[i]) {
            // inputID is the index of the predecessor node
            if (inputID >= 0 && inputID < results.size()) {
                results[i].addInput(results[inputID]);
                cout << "Linked Node " << i << " to input Node " << inputID << endl;
            }
        }
    }
    
    return results;
}

int main() {
    int memoryLimit = 0;
    vector<Node> nodes = parseFile("test_out/example1.txt", memoryLimit);
    
    cout << "\nParsed " << nodes.size() << " nodes" << endl;
    cout << "Memory limit: " << memoryLimit << endl;
    
    return 0;
}