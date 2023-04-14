#include <iostream>
#include <fstream>
#include <vector>

using namespace std;

class Node{
	public:
		Node(int id, Node_Type type, string word = ""); //constructor for nodes
		~Node(); //default destructor
		bool has_letter(char c);
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
};

int main(int argc, char *argv[]) {
	string word, die;
	vector<string> words;

	// Use argv and file streams to read input
	// Dev Note: Put dice into graph while reading input but add word when done
	ifstream finD, finW;
	finD.open(argv[1]);

	finD.close();

	return 0;
}
