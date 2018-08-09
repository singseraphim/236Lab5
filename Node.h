#ifndef NODE_H
#define NODE_H

#include <iostream>
#include <iomanip>
#include <string>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <set>
#include <vector>
#include <map>

using namespace std;

class Node {
public:
	Node() {
		
	}
	void setVisited(bool vis) {
		visited = vis;
	}
	void setID(int num) {
		ID = num;
	}
	void addEdge(int newNode) {
		edges.insert(newNode);
	}
	set<int> getEdges() {
		return edges;
	}
	int getID() const {
		return ID;
	}
	bool isVisited() {
		return visited;
	}
	void setPostorder(int i) {
		postorder = i;
	}
	int getPost() const{
		return postorder;
	}
	

private:
	bool visited = false;
	int ID = -1;
	set<int> edges;
	int postorder;

};

/*
Update: Jeff is smart and I love this solution. Referring to everything via map. 

*/

#endif
