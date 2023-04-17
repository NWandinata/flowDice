#include <iostream>
#include <fstream>
#include <vector>
#include <climits>
#include <queue>
#include <string>
#include <algorithm>

using namespace std;

class Edge{
    public:
        //from -> to
        class Node *to; //node edge is pointing to
        class Node *from; //node edge is pointing from
        Edge(class Node *to, class Node *from, bool reverse_edge = false); //constructor for edges
        ~Edge(){}; //default destructor // Dev Note: LEAVE THIS ALONE
        Edge *reverse; //edge going the other way
        int original; //original weight per edge
        int residual; //allows for updated weighting during Edmonds-Karp
};

class Node{
	public:
		typedef enum Node_Type {source, sink, word, dice};
		Node(int id, Node_Type type, string word = ""); //constructor for nodes
		~Node(); //default destructor
		friend bool has_letter(char c, Node *die);
		//friend ostream& operator<<(ostream& os, const Node& node); // Dev Note: Probably unnecessary
		int id; //node id
		Node_Type type; //type of node it is (source, sink, word or dice)
		vector<bool> letters; //length 26 with letters contained in word set to 1
		int visited; //for BFS
		vector<Edge*> adj; //adjacency list
		Edge *backedge; //previous edge for Edmonds-Karp
};

class Graph{
 public:
	 Graph(); //constructor initializes graph with source node
	 ~Graph(); //destructor to deallocate memory of graph
	 Node *source; //not necessary but makes code more readable
	 Node *sink;
	 vector<Node *> nodes; //holds the nodes
	 vector<int> spellingIds; //order of flow to spell word
	 int min_nodes; //min number of dice nodes
	 string word;
	 void add_dice_to_graph(string die, int id); //add dice nodes to graph
	 void add_word_to_graph(string word, int id); //add word (letter) nodes to graph
	 bool BFS(); //breadth first search for Edmonds-Karp
	 bool spell_word(); //runs Edmonds-Karp to see if we can spell the word
	 void delete_word_from_graph(); //deletes the word nodes but leaves the dice nodes
	 void print_node_order(string word); //print spelling Ids and word
	 void dump_nodes(); // Only use for debugging
};

// reverse_edge = true - Means we want to make an edge (auto creates reverse edge by calling itself)
// reverse_edge = false - ONLY USE INSIDE CONSTRCUTOR TO MAKE REVERSE EDGES (we want a reverse edge)
Edge::Edge(class Node *to, class Node *from, bool reverse_edge) {
	this->to = to;
	this->from = from;
	if(reverse_edge) {
		original = 1;
		residual = 0;
		reverse = new Edge(from, to, false);
	}
	else {
		original = 0;
		residual = 0;
	}
}

Node::Node(int id, Node_Type type, string word) {
    this->id = id;
    this->type = type;
    visited = 0;
    for(int i = 0; i < 26; i++)
        letters.push_back(false);

    // Uses ASCII table to find each letter's corresponding index
    for(int i = 0; i < word.length(); i++)
        letters[word[i] - 65] = true;

    // Dev Note: Setup backedge based on node type
}

bool has_letter(char c, Node *die) {
    return die->letters[c - 65];
}

Node::~Node() {
	for(int i = 0; i < adj.size(); i++)
		delete adj[i];
}

Graph::Graph(){
    source = new Node(0, Node::source, "");
    sink = NULL;
    min_nodes = 0;
	nodes.push_back(source);
}

Graph::~Graph(){
    for (auto& node : nodes) {
        delete node;
    }
}


void Graph::add_dice_to_graph(string die, int id){ //add to Node and Edge vectors
    Node* node = new Node(id, Node::dice, die);
	//Edge(node, source, true);//add edges later connect to source
	//source -> adj.push_back(node); //adds to adjacency list; might need to fix
	Edge* edge = new Edge(node, source, true);//add edges later connect to source
    source -> adj.push_back(edge); //adds to adjacency list; might need to fix
	nodes.push_back(node);//add to Nodes vector
}


void Graph::add_word_to_graph(string word, int id){ //add &id back in if things don't work
	sink = new Node(0, Node::sink, "");
    for (int i = 0; i < word.size(); i++) {  // iterate over each index in the string 'word' and allocate each letter in the word
        string let(1, word[i]);
        Node* node = new Node(id, Node::word, let);
        nodes.push_back(node);
        Edge* edge = new Edge(sink, node, true);//add edges later connect to source
        node -> adj.push_back(edge); //adds to adjacency list; might need to fix


        for(int j = 1; j < 5; j++){
            if(has_letter(word[i], nodes[j])){ //if a letter in the die matches the asking letter
                Edge* edge = new Edge(node, nodes[j], true);//add edges later connect to source
                nodes[j] -> adj.push_back(edge); //adds to adjacency list; might need to fix
            }
        }
    }
    nodes.push_back(sink);
}

bool Graph::BFS(){ //spell_word() function, which uses the Edmonds-Karp algorithm to find the maximum flow in the graph.
	for(auto it = nodes.begin(); it != nodes.end(); it++){
		(*it) -> visited = 0;
		(*it) -> backedge = NULL;
	}

	queue<Node*> bfs;
	bfs.push(source);
	source -> visited = 1;

	while(!bfs.empty()){
		Node *current_node = bfs.front();
		bfs.pop();

		for (auto it : current_node->adj) {
            Node* next = it -> to;
            if (next -> visited == 0 && it -> residual > 0) {
                next -> visited = 1;
                next -> backedge = it;
                bfs.push(next);

                if (next == sink) { // found a path to the sink node
                    return true;
                }
            }
        }
	}

	//if no path to sink was found
	return false;
}

bool Graph::spell_word() {
    int total_flow = 0;
    while (BFS()) {
		//unvisited nodes are 0
        source->visited = 1;
        int bottle = INT_MAX;

        if (bottle == 0) { //might not be needed
            break;
        }
        total_flow += bottle;
    }

    for (auto node : nodes) {
        node -> visited = 1;
    }

    for (auto id : spellingIds) {
        Node* node = nodes[id];
        if (!node -> visited) {
            return false;
        }
        spellingIds.push_back(id);
    }

	if(total_flow == spellingIds.size()) return true;
	else return false;
}

void Graph::dump_nodes() {
	string nodeType = "Missing Type";
    for (int i = 0; i < nodes.size(); i++) {
        if(nodes[i]->type == Node::Node_Type::source) nodeType = "SOURCE";
        else if(nodes[i]->type == Node::Node_Type::sink) nodeType = "SINK";
        else if(nodes[i]->type == Node::Node_Type::word) nodeType = "WORD";
        else if(nodes[i]->type == Node::Node_Type::dice) nodeType = "DICE";
        cout << "Node " << i << ": " << nodeType << " Edges to ";
        for(int j = 0; j < nodes[i]->adj.size(); j++) {
			int nodeNum = -1;
			for(int k = 0; k < nodes.size(); k++)
				if(nodes[i]->adj[j]->to == nodes[k]) nodeNum = k;
            cout << nodeNum << " ";
		}
        cout << endl;
    }
}

int main(int argc, char *argv[]) {
	string word, die;
	int id = 0;
	Graph *graph;
	graph = new Graph();

	ifstream finD, finW;
	finD.open(argv[1]);
	finW.open(argv[2]);

	while(true) {
		finD >> die;
		if(finD.eof())
			break;
		graph->add_dice_to_graph(die, id);
		id += 1;
	}
	finD.close();
	id = 0;

	//while(true) {
		finW >> word;
		graph->add_word_to_graph(word, id);
		graph->dump_nodes();
		// Delete word
		id += 1;
	//}
	finW.close();

	return 0;
}
