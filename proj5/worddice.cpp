#include <iostream>
#include <fstream>
#include <vector>

using namespace std;

class Node{
	public:
		typedef enum Node_Type {source, sink, word, dice};
		Node(int id, Node_Type type, string word = ""); //constructor for nodes
		~Node(); //default destructor
		friend bool has_letter(char c, Node *die);
		friend ostream& operator<<(ostream& os, const Node& node);
		int id; //node id
		Node_Type type; //type of node it is (source, sink, word or dice)
		vector<bool> letters; //length 26 with letters contained in word set to 1
		int visited; //for BFS
		vector<Edge*> adj; //adjacency list
		Edge *backedge; //previous edge for Edmonds-Karp
};

class Edge{
	public:
		//from -> to
		class Node *to; //node edge is pointing to
		class Node *from; //node edge is pointing from
		Edge(class Node *to, class Node *from, bool reverse_edge = false); //constructor for edges
		~Edge(){}; //default destructor
		Edge *reverse; //edge going the other way
		int original; //original weight per edge
		int residual; //allows for updated weighting during Edmonds-Karp
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
	 void add_word_to_graph(string word, int& id); //add word (letter) nodes to graph
	 bool BFS(); //breadth first search for Edmonds-Karp
	 bool spell_word(); //runs Edmonds-Karp to see if we can spell the word
	 void delete_word_from_graph(); //deletes the word nodes but leaves the dice nodes
	 void print_node_order(string word); //print spelling Ids and word
	 void dump_node(class Node *node); // Only use for debugging
};

Node::Node(int id, Node_Type type, string word = "") {
	this->id = id;
	this->type = type;
	visited = 0;
	for(int i = 0; i < 26; i++)
		letters.push_back(false);

	// Uses ASCII table to find each letter's corresponding index
	for(int i = 0; i < word.length(); i++)
		letters[word[i] - 65] = true;
}

Node::friend bool has_letter(char c, Node *die) {
	return die->letters[c - 65];
}

// reverse_edge = true - Means we want to make an edge (auto creates reverse edge by calling itself)
// reverse_edge = false - ONLY USE INSIDE CONSTRCUTOR TO MAKE REVERSE EDGES (we want a reverse edge)
Edge::Edge(class Node *to, class Node *from, bool reverse_edge = false) {
	this->to = to;
	this->from = from;
	if(reverse_edge) {
		original = 1;
		residual = 0;
		reverse = Edge(from, to, false);
	}
	else {
		original = 0;
		residual = 0;
	}
}


Graph::Graph(){
    source = new Node(0, Node::source, "");
    sink = NULL;
    min_nodes = 0;

}

Graph::~Graph(){
    for (auto& node : nodes) {
        delete node;
    }
}


void Graph::add_dice_to_graph(string die, int id){ //add to Node and Edge vectors
    Node* node = new Node(id, Node::dice, die);
	Edge(node, source, true);//add edges later connect to source
	source -> adj.push_back(node); //adds to adjacency list; might need to fix
	nodes.push_back(node);//add to Nodes vector
}


void Graph::add_word_to_graph(string word, int id){ //add &id back in if things don't work
	for (int i = 0; i < word.size(); i++) {  // iterate over each index in the string 'word' and allocate each letter in the word
        string let(1, word[i]);
        Node* node = new Node(id, Node::word, let);
        nodes.push_back(node);
		Edge(sink, node, true); //make edge from letter to sink
		node -> adj.push_back(sink); //add to adj list

		for(int j = 0; j < 4; j++){
			if(has_letter(word[i], nodes[i])){ //if a letter in the die matches the asking letter
				Edge(node, nodes[i], true); //make edge from dice with matching letter to letter
				nodes[i] -> adj.push_back(node); //add to adj
			}
		}
	}
}

Graph::void dump_node(class Node *node) {
	cout << "Node " << node->id << ": " << node->type << " Edges to "
	for(int i = 0; i < node->adj.size(); i++) {
		cout << node->adj[i]->to << " "
	}
	cout << endl;
}

int main(int argc, char *argv[]) {
	string word, die;
	vector<string> words;

	// Use argv and file streams to read input
	// Dev Note: Put dice into graph while reading input but add word when done
	ifstream finD, finW;
	finD.open(argv[1]);
	finW.open(argv[2]);

	finD.close();
	finW.close();

	return 0;
}
