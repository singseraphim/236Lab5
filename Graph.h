#ifndef GRAPH_H
#define GRAPH_H

#include <iostream>
#include <iomanip>
#include <string>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <set>
#include <vector>
#include <map>
#include "Node.h"
#include "RelationalDatabase.h"

using namespace std;
class Graph {
public:
	Graph() {

	}
	struct Tree {
		set<int> nodes;
		bool isInTree(int i) {
			return nodes.find(i) != nodes.end();
		}
	};
	vector<set<int>> createDependencyGraph(vector<Rule>);
	void createNode(int, Rule, Graph&);
	void addEdges(int, Rule, vector<Rule>, Graph&);
	void dependGraphToString();
	void dfsForest(Graph&);
	Tree dfs(int, Tree& newTree);
	Tree reverseDFS(int, Graph&, Tree& newTree);
	void setVisitedFalse();
	bool hasOpenConnections(int, Tree&);
	bool reverseHasOpenConnections(int, Graph&, Tree&);
	void findSCCs(Graph&);
	vector<int> sortByPost(Graph&);
	vector<Rule> getSCCRules(Tree&, vector<Rule>);
	
private:
	map<int, Node> nodes;
	vector<Tree> forest;
	vector<Tree> reverseForest;
	vector<Tree> SCCs;
	int postorder = 1;
	int reversePostorder = 1;
};

#endif