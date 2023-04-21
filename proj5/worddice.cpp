/*Nicholas Wandinata and Makenzie Johnson
 * project 5
 * cs 302
 * 4-21-23*/

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
        ~Edge(){}; //default destructor
        Edge *reverse; //edge going the other way
        int original; //original weight per edge
        int residual; //allows for updated weighting during Edmonds-Karp
};

class Node{
	public:
		enum Node_Type {source, sink, word, dice};
		Node(int id, Node_Type type, string word = ""); //constructor for nodes
		~Node(); //default destructor
		friend bool has_letter(char c, Node *node);
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
	 string word;
	 void add_dice_to_graph(string die, int id); //add dice nodes to graph
	 void add_word_to_graph(string word, int id, int numDice); //add word (letter) nodes to graph
	 bool BFS(); //breadth first search for Edmonds-Karp
	 bool spell_word(string word); //runs Edmonds-Karp to see if we can spell the word
	 void delete_word_from_graph(); //deletes the word nodes but leaves the dice nodes
	 void print_node_order(string word, int numDice); //print spelling Ids and word
	 void dump_nodes(); // Only use for debugging
	 void reset_edges();
};

// Sets up the flow for forward and reverse edges
Edge::Edge(class Node *to, class Node *from, bool reverse_edge) {
	this->to = to;
	this->from = from;
	if(reverse_edge) {
		original = 0; 
		residual = 1; 
	}
	else {
		original = 1; 
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
    for(int i = 0; i < (int)word.length(); i++)
        letters[word[i] - 65] = true;
}

// Checks if a node contains a specific letter 
bool has_letter(char c, Node *node) {
    return node->letters[c - 65];
}

Node::~Node() {
	for(int i = 0; i < (int)adj.size(); i++)
		delete adj[i];
}

Graph::Graph(){
    source = new Node(-1, Node::source, "");
    sink = NULL;
	nodes.push_back(source);
}

Graph::~Graph(){
    for (auto& node : nodes) {
        delete node;
    }
}

//add to Node vector and adjacency list
void Graph::add_dice_to_graph(string die, int id){
    Node* node = new Node(id, Node::dice, die);
	Edge* edge = new Edge(node, source, false);
    Edge *rev = new Edge(source, node, true);
	edge -> reverse = rev;
	rev -> reverse = edge;
	node -> adj.push_back(rev);
	source -> adj.push_back(edge); //adds to adjacency list;
	nodes.push_back(node);
}

 // iterate over each index in the string 'word' and allocate each letter in the word
void Graph::add_word_to_graph(string word, int id, int numDice){
	sink = new Node(-2, Node::sink, "");
    for (int i = 0; i < (int)word.size(); i++) {
        string let(1, word[i]);
        Node* node = new Node(id, Node::word, let);
        nodes.push_back(node);
        Edge* edge = new Edge(sink, node, false); //add edge to connect from word to sink
		Edge* rev = new Edge(node, sink, true);	//add reverse to connect from sink to word
		edge->reverse = rev;
		rev->reverse = edge;
        node->adj.push_back(edge); //adds to adjacency list;
		sink->adj.push_back(rev);

        for(int j = 1; j <= numDice; j++){
            if(has_letter(word[i], nodes[j])){ //if a letter in the die matches the asking letter
                Edge* edge = new Edge(node, nodes[j], false); //add edges later connect to source
				Edge* rev = new Edge(nodes[j], node, true);
				edge->reverse = rev;
				rev->reverse = edge;
                nodes[j]->adj.push_back(edge);
				node->adj.push_back(rev);
            }
        }
		id += 1;
    }
    nodes.push_back(sink);
}

bool Graph::BFS(){ //spell_word() function, which uses the Edmonds-Karp algorithm to find the maximum flow in the graph.
	for(auto it = nodes.begin(); it != nodes.end(); it++){
		(*it) -> visited = 0;
		(*it) -> backedge = NULL;
	}

	queue<Node*> bfs;

	//visit the source node first
	bfs.push(source);
	source -> visited = 1;

	while(!bfs.empty()){
		Node *current_node = bfs.front();
		bfs.pop();

		for (auto it : current_node -> adj) {
            Node* next = it -> to;
            if (next -> visited == 0 && it -> original == 1) {
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

//While BFS is true, we start on sink and work our way backwards. The counter keeps track of how many letters
//are connectes with the dice. If the counter matches the word length, then we can spell the word.

bool Graph::spell_word(string word) { 
	int counter = 0;

	while(BFS()){
		Node *current_node = sink;
		
		while(current_node != source){
			Edge *back = current_node -> backedge;
			swap(back->original, back->residual);

			Edge *rev = back -> reverse;
			swap(rev->original, rev->residual);

			current_node = rev -> to;
		}

		counter++;
        if (counter == (int)word.length()) return true;
	}

	return false;
}

// We delete edges of dice, word, and sink. Then delete word and sink nodes.
void Graph::delete_word_from_graph() {

	// Delete adj list of dice
    for(int i = 1; i < (int)nodes.size(); i++) {
		for(int j = 0; j < (int)nodes[i]->adj.size(); j++) {
			delete nodes[i]->adj[j];
		}
		nodes[i]->adj.clear();
    }

	// Delete word nodes and sink
	int count = 0;
    for(int i = nodes.size() - 1; i > 1; i--) {
        if(nodes[i]->type == Node::Node_Type::sink || nodes[i]->type == Node::Node_Type::word) {
			delete nodes[i];
			count += 1;
		}
        else
            break;
    }
	nodes.resize(nodes.size() - count);
}

//This sets up spellingIds in order. We first check the reverse edges of the word nodes. If they have a flow of 1, the path was taken.
void Graph::print_node_order(string word, int numDice){
	int wordIndex;

	for(int i = 0; i < (int)word.length(); i++) {
		wordIndex = numDice + 1 + i;

		for(int j = 0; j < (int)nodes[wordIndex]->adj.size(); j++) {

			if(nodes[wordIndex]->adj[j]->original == 1 && nodes[wordIndex]->adj[j]->to->type == Node::Node_Type::dice) {
				spellingIds.push_back(nodes[wordIndex]->adj[j]->to->id);
			}
		}
	}

    // Print
    for (int i = 0; i < (int)spellingIds.size(); i++) {
        if(i == (int)spellingIds.size() - 1){
            cout << spellingIds[i] << ": ";
        }
        else {
            cout << spellingIds[i] << ",";
        }
    }
    cout << word << endl;
	spellingIds.clear();
}

void Graph::reset_edges() {
	for(int i = 0; i < (int)source->adj.size(); i++) {
		// Reset flow values of source edges
		source->adj[i]->original = 1;
        source->adj[i]->residual = 0;

		// Remake reverse edges from dice to source
		Edge *rev = new Edge(source, nodes[i + 1], true);
		source->adj[i]->reverse = rev;
		rev->reverse = source->adj[i];
		nodes[i + 1]->adj.push_back(rev);
	}
}

//We read in dice and word separately and add them to the graph. We call the corresponding functions and the while loop
//will continue until something returns false.

int main(int argc, char *argv[]) {
	string word, die;
	int id = 0;
	int numDice = 0;
	Graph *graph = new Graph();

	ifstream finD, finW;
	finD.open(argv[1]);
	finW.open(argv[2]);

	while(true) {
		finD >> die;
		if(finD.eof())
			break;
		graph->add_dice_to_graph(die, id);
		numDice += 1;
		id += 1;
	}
	finD.close();
	id = 0;

	while(true) {
		finW >> word;
		if(finW.eof())
            break;
		graph->add_word_to_graph(word, id, numDice);

		if(graph -> spell_word(word) == false) cout << "Cannot spell " << word << endl;
        else graph -> print_node_order(word, numDice);
		graph->delete_word_from_graph();
		graph->reset_edges();
	}
	finW.close();

	delete graph;

	return 0;
}
