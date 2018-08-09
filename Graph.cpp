#include "Graph.h"
#include <iostream>
#include <iomanip>
#include <string>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <set>
#include <vector>
#include <map>

void Graph::dependGraphToString() { //a toString function to show the node dependencies. Currently broken. 
	cout << "Dependency Graph" << endl;
	for (unsigned int i = 0; i < nodes.size(); ++i) {
		cout << "R" << i << ":";
		set<int> edges = nodes[i].getEdges();
		set<int>::iterator iter;
		vector<int> edgeVec;

		for (iter = edges.begin(); iter != edges.end(); ++iter) {
			edgeVec.push_back(*iter);
		}

		for (unsigned int i = 0; i < edgeVec.size(); ++i) {
			cout << "R" << edgeVec.at(i);
			if (i < edgeVec.size() - 1) {
				cout << ",";
			}
		}
		cout << endl;
	}
}

vector<set<int>> Graph::createDependencyGraph(vector<Rule> rules) { //a main function to facilitate the creation of the nodes, connecting edges and running dfsforest to get the set of trees. 
	Graph reverseGraph;
	for (unsigned int i = 0; i < rules.size(); ++i) {
		createNode(i, rules.at(i), reverseGraph);
	}
	for (unsigned int i = 0; i < rules.size(); ++i) {
		addEdges(i, rules.at(i), rules, reverseGraph);
	}
	dfsForest(reverseGraph);
	findSCCs(reverseGraph);

	dependGraphToString();
	vector<set<int>> SCCReturnRules;
	for (unsigned int i = 0; i < SCCs.size(); ++i) {
		SCCReturnRules.push_back(SCCs.at(i).nodes);
	}
	return SCCReturnRules;
}
/*
I want to change this to return a vector of sets of ints. 
*/

void Graph::findSCCs(Graph& reverseGraph) { 
	setVisitedFalse();
	reverseGraph.setVisitedFalse();
	vector<int> sortedNodes = sortByPost(reverseGraph);
	for (unsigned int i = 0; i < sortedNodes.size(); ++i) {
		if (!nodes[sortedNodes.at(i)].isVisited()) {
			Graph::Tree newTree;
			newTree = dfs(sortedNodes.at(i), newTree);
			if (newTree.nodes.size() > 0) SCCs.push_back(newTree);
		}
	}
}
/*
I'm looking at the reverse postorder, and the normal graph. 

*/

vector<int> Graph::sortByPost(Graph& reverseGraph) {
	vector<int> sortedVec;
	for (unsigned int i = reversePostorder - 1; i > 0; --i) { //finally an excuse to use this!
		for (unsigned int j = 0; j < reverseGraph.nodes.size(); ++j) {
			unsigned int x = reverseGraph.nodes[j].getPost();
			if (x == i) {
				sortedVec.push_back(j);
			}
		}
	}
	return sortedVec;
}

void Graph::createNode(int i, Rule rule, Graph& reverseGraph) { //creates a node from a rule and adds it to the node map and the reverse graph node map. 
	Node newNode;
	newNode.setID(i);
	nodes[i] = newNode;
	reverseGraph.nodes[i] = newNode;
	
}

void Graph::addEdges(int i, Rule rule, vector<Rule> rules, Graph& reverseGraph) { //adds edges to nodes based on the rule predicates. Also adds the reverse connection on the reverse graph. 
	vector<Predicate> preds = rule.getPredicateList();
	for (unsigned int k = 0; k < preds.size(); ++k) {
		for (unsigned int j = 0; j < rules.size(); ++j) {
			if (preds.at(k).getID() == rules.at(j).getHeadPredicate().getID()) {
				nodes[i].addEdge(nodes[j].getID()); 
				reverseGraph.nodes[j].addEdge(nodes[i].getID());
			}
		}
	}
}

void Graph::dfsForest(Graph& reverseGraph) { //iteratively runs dfs() on each node. Ends up with a vector of trees called forest. 
	for (unsigned int i = 0; i < nodes.size(); ++i) {
		Tree newTree;
		newTree = dfs(i, newTree);
		Tree reverseTree;
		reverseTree = reverseDFS(i, reverseGraph, reverseTree);
		if (newTree.nodes.size() > 0) {
			forest.push_back(newTree); //I assumed I would need forest for something but I think it's useless. 
		}
		if (reverseTree.nodes.size() > 0) {
			reverseForest.push_back(reverseTree);
		}
	}
}

Graph::Tree Graph::reverseDFS(int n, Graph& reverseGraph, Tree& newTree) { //does same thing as dfs() but on reverse. There was probably a neater way to code this, but this way I get to just pass ints around and that is just delicious. 

	if (!reverseGraph.nodes[n].isVisited()) {

		newTree.nodes.insert(n);

		set<int> connections = reverseGraph.nodes[n].getEdges();
		set<int>::iterator iter;
		for (iter = connections.begin(); iter != connections.end(); ++iter) {
			int m = *iter;
			if (!reverseGraph.nodes[m].isVisited() && !newTree.isInTree(m)) { //recursive bit
				newTree.nodes.insert(m); 
				reverseDFS(m, reverseGraph, newTree);
			}
		}
		if (!reverseHasOpenConnections(n, reverseGraph, newTree)) { //assigns postorder
			reverseGraph.nodes[n].setPostorder(reversePostorder);
			++reversePostorder;
		}
		reverseGraph.nodes[n].setVisited(true);
	}
	return newTree;
}

Graph::Tree Graph::dfs(int n, Tree& newTree) { //recursive function that does a depth first search and creates trees. Also sets the postorder number for each node. 
	
	if (!nodes[n].isVisited()) {

		newTree.nodes.insert(n);

		set<int> connections = nodes[n].getEdges();
		set<int>::iterator iter;
		for (iter = connections.begin(); iter != connections.end(); ++iter) {
			int m = *iter;
			if ((!nodes[m].isVisited()) && !newTree.isInTree(m)) { //recursive bit. this is an issue. 
				newTree.nodes.insert(m);
				dfs(m, newTree); 
			}
		}
		if (!hasOpenConnections(n, newTree)) { //assigns postorder if has no open connections, or its open connections are on the tree already. 
			nodes[n].setPostorder(postorder);
			++postorder;
		}
		nodes[n].setVisited(true);
	}
	return newTree;
}

void Graph::setVisitedFalse() { //resets all nodes to unvisited
	for (unsigned int i = 0; i < nodes.size(); ++i) {
		nodes[i].setVisited(false);
	}
}

bool Graph::hasOpenConnections(int i, Graph::Tree& newTree) { //lookes at connected nodes to see if any of them haven't been visited.
	set<int> connections = nodes[i].getEdges();
	set<int>::iterator iter;
	bool hasOpenConnection = false;
	for (iter = connections.begin(); iter != connections.end(); ++iter) {
		int n = *iter;
		if ((!nodes[n].isVisited()) && !newTree.isInTree(n)) {
			hasOpenConnection = true;
		}
	}
	return hasOpenConnection;
}

bool Graph::reverseHasOpenConnections(int i, Graph& reverseGraph, Graph::Tree& newTree) { //looks at connected nodes of reverse graph to see if any of them haven't been visited. 
	set<int> connections = reverseGraph.nodes[i].getEdges();
	set<int>::iterator iter;
	bool hasOpenConnection = false;
	for (iter = connections.begin(); iter != connections.end(); ++iter) {
		int n = *iter;
		if (!reverseGraph.nodes[n].isVisited() &&!newTree.isInTree(n)) {
			hasOpenConnection = true;
		}
	}
	return hasOpenConnection;

}

/*
Have forest of trees done correctly. 
Postorder number is done. 
Dependency and reverse dependency graphs are correct. 
Now to find strongly connected components. Victory! Probably!
Now to run join on the connected components. Make graph interface with relationaldatabase. Oh dear. 

My postorder algorithm ends up with a switched around ordering- the reverse is correct. 
At this point I'm kind of tempted to just use it?

Okay, so I have my SCCs. What do I do with them? Right now they're just vectors of nodes. 
It would be good to end up with a vectors of rules. 



Future me:
Write sCCRules to take the trees and turn them back into vectors of rules. 
After that I think it's just making it work with your main program and formatting it properly.

Okay, pretty sure rules are good. Now to work on evaluation. 

vector of sets of ints? might be useful- ordering and no repeats. 
*/
/*
Dependency Graph
R0: R1 R2
R1: R0
R2: R1 R2
R3:
R4: R3 R4


Schemes:
A(X,Y)
B(X,Y)
C(X,Y)
D(X,Y)
E(X,Y)
F(X,Y)
G(X,Y)

Facts:
A('a','a').

Rules:
A(X,Y) :- B(X,Y), C(X,Y).
B(X,Y) :- A(X,Y), D(X,Y).
B(X,Y) :- B(Y,X).
E(X,Y) :- F(X,Y), G(X,Y).
E(X,Y) :- E(X,Y), F(X,Y).

Queries:
A(A,A)?


*/