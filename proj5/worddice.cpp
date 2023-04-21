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
	 vector<string> dice; // Dev Note: Delete later, for dump_node
	 int min_nodes; //min number of dice nodes
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

// reverse_edge = true - Means we want to make an edge (auto creates reverse edge by calling itself)
// reverse_edge = false - ONLY USE INSIDE CONSTRUCTOR TO MAKE REVERSE EDGES (we want a reverse edge)
Edge::Edge(class Node *to, class Node *from, bool reverse_edge) {
	this->to = to;
	this->from = from;
	if(reverse_edge) {
		original = 1; //1
		residual = 0; //0
		reverse = new Edge(from, to, false);
	}
	else {
		original = 0; //0
		residual = 1; //1
		// Dev Note: Need to find a way to set reverse of the reverse to original
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
}

bool has_letter(char c, Node *die) {
    return die->letters[c - 65];
}

Node::~Node() {
	for(int i = 0; i < adj.size(); i++)
		delete adj[i];
}

Graph::Graph(){
    source = new Node(-1, Node::source, "");
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
	Edge* edge = new Edge(node, source, true);//add edges later connect to source
    source -> adj.push_back(edge); //adds to adjacency list; might need to fix
	nodes.push_back(node);//add to Nodes vector
	//node->adj.push_back(edge->reverse);
}


void Graph::add_word_to_graph(string word, int id, int numDice){ //add &id back in if things don't work
	sink = new Node(-2, Node::sink, "");
    for (int i = 0; i < word.size(); i++) {  // iterate over each index in the string 'word' and allocate each letter in the word
        string let(1, word[i]);
        Node* node = new Node(id, Node::word, let);
        nodes.push_back(node);
        Edge* edge = new Edge(sink, node, true);//add edges later connect to source
        node -> adj.push_back(edge); //adds to adjacency list;
		sink->adj.push_back(edge->reverse);

        for(int j = 1; j <= numDice; j++){
            if(has_letter(word[i], nodes[j])){ //if a letter in the die matches the asking letter
                Edge* edge = new Edge(node, nodes[j], true);//add edges later connect to source
                nodes[j] -> adj.push_back(edge); //adds to adjacency list; might need to fix
				node->adj.push_back(edge->reverse);
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

bool Graph::spell_word(string word) { //aka maxflow - NOT FINISHED
	int counter = 0; //may not need

	while(BFS()){
		counter++;
		if (counter == word.length()) return true;
		Node *current_node = sink;
		
		while(current_node != source){
			Edge *back = current_node -> backedge;

			back -> original = 0;
			back -> residual = 1;

			Edge *rev = back -> reverse;
			rev -> residual = 0;
			rev -> original = 1;

			current_node = rev -> to;
		}
	}
	//cout << "paths not found" << endl;

	//if (counter == word.length()) return true;
	//else return false;

	/*for(int i = nodes.size() - 2; i > 1; i--) {
		if(nodes[i]->type == Node::Node_Type::word) {
			if(nodes[i]->adj[0]->residual != 1)
				return false;
		}
		else
			break;
	}
	return true;*/
}

// Note: After deleting reverse edges of dice, may need to reconnect them to source in reset_edges
void Graph::delete_word_from_graph() {
	// Delete adj list of dice
    for(int i = 1; i < nodes.size(); i++) {
		//if(nodes[i]->type == Node::Node_Type::dice || nodes[i]->type == Node::Node_Type::word){
			for(int j = 0; j < nodes[i]->adj.size(); j++) {
				//delete nodes[i]->adj[j]->reverse; // May or may not need
				delete nodes[i]->adj[j];
			}
			nodes[i]->adj.clear();
		//}
		//else
			//break;
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

	//cout << "Nodes vector size: " << nodes.size() - count << endl;
	nodes.resize(nodes.size() - count);
    //cout << "Nodes vector size: " << nodes.size() << endl; // Dev Notes: For testing only
}

void Graph::print_node_order(string word, int numDice){
    // Set up spellingIds in order
	int wordIndex;
	for(int i = 0; i < word.length(); i++) {
		// 1st word index = numDice + 1
		// Iterative word index = numDice + 1 + i
		wordIndex = numDice + 1 + i;
		for(int j = 0; j < nodes[wordIndex]->adj.size(); j++) {
			if(nodes[wordIndex]->adj[j]->original == 1 && nodes[wordIndex]->adj[j]->to->type == Node::Node_Type::dice)
				spellingIds.push_back(nodes[wordIndex]->adj[j]->to->id);
		}
	}

    // Print
    for (int i = 0; i < spellingIds.size(); i++) {
        if(i == (int)spellingIds.size() - 1){
            cout << spellingIds[i] << ": "; //might need to take out min_nodes
        }
        else {
            cout << spellingIds[i] << ",";
        }
    }
    cout << word << endl;
}

void Graph::dump_nodes() {
	string nodeType = "Missing Type";
	int count = 0;
    for (int i = 0; i < nodes.size(); i++) {
        if(nodes[i]->type == Node::Node_Type::source) nodeType = "SOURCE";
        else if(nodes[i]->type == Node::Node_Type::sink) nodeType = "SINK";

        else if(nodes[i]->type == Node::Node_Type::word) {
			nodeType.clear();
			for(int x = 0; x < nodes[i]->letters.size(); x++) {
				if(nodes[i]->letters[x] == true)
					nodeType.push_back((char)(x + 65));
			}
		}

        else if(nodes[i]->type == Node::Node_Type::dice) {
			nodeType = dice[count];
			count += 1;
		}

        cout << "Node " << i << ": " << nodeType << " Edges to ";
        for(int j = 0; j < nodes[i]->adj.size(); j++) {
			int nodeNum = -1;
			for(int k = 0; k < nodes.size(); k++)
				if(nodes[i]->adj[j]->to == nodes[k]) nodeNum = k;
            cout << nodeNum << " ";
		}
        cout << endl;
    }
	
	/*int wordIndex;
	for(int i = 0; i < nodes.size() - 1; i++) {
        wordIndex = i;
        if(nodes[i]->type == Node::Node_Type::word)
            break;
    }
    cout << "Word Index: " << wordIndex << endl;*/
}

void Graph::reset_edges() {
	int index = 1;
	for(int i = 0; i < source->adj.size(); i++) {
		source->adj[i]->original = 1; //1
		source->adj[i]->residual = 0; //0
		source->adj[i]->reverse->original = 0; //0
		source->adj[i]->reverse->residual = 1; //1
		
		// Reset reverse edges of dice
		//nodes[index]->adj.push_back(source->adj[i]->reverse);
		//Edge* rev = new Edge(source, nodes[index], false);
		//nodes[index]->adj.push_back(rev);
		//index += 1;
	}
}

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
		graph->dice.push_back(die); // Dev Note: Delete later, for dump_node
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
		graph->dump_nodes(); // Dev Note: Delete later

		if(graph -> spell_word(word) == false) cout << "Cannot spell " << word << endl;
        else graph -> print_node_order(word, numDice);
		//else cout << "Can spell " << word << endl;

		//cout << endl;
		graph->delete_word_from_graph();
		//cout << "Graph after delete word:" << endl;
		//graph->dump_nodes();

		graph->reset_edges();
		cout << endl; // Dev Note: Delete later, this is for dump node
		//id += 1;
	}
	finW.close();

	delete graph;

	return 0;
}
