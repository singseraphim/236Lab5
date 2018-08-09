#ifndef RELATIONALDATABASE_H
#define RELATIONALDATABASE_H

#include <iostream>
#include <iomanip>
#include <string>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <set>
#include <map>
#include "Relation.h"
#include "Parser.h"
#include "Graph.h"

using namespace std;

class RelationalDatabase {
public:
	RelationalDatabase() {

	}
	void getData(Parser myParser);
	void evaluate();
	void setRelations();
	void addFacts();
	void doQueries();
	void evaluateRules();
	string relationString(Relation);
	vector<Relation> doRule(Rule);
	int findMatchIndex(vector<Parameter>, int);
	Relation join(vector<Relation>&);
	Scheme joinSchemes(Scheme, Predicate);
	Scheme joinSchemes(Relation&, Relation&);
	void evaluateRule(Rule);
	bool canJoin(Relation&, Relation&, Tuple, Tuple, vector<string>);
	Relation select(Predicate, map<string, int>&, vector<int>&, vector<string>&);
	vector<string> getMatchCols(Scheme, Scheme);
	int getMatchIndex(string, Scheme);
	Relation joinTwoRels(Relation&, Relation&);
	Tuple combineTuples(const Relation&, const Relation&, Tuple, Tuple, Scheme, vector<string>);
	Relation project(Relation&, Scheme);
	vector<int> getHeadIndices(Relation&, Scheme);
	void graphRules();
	void evaluateSCC(set<int>);
	vector<Rule> intsToRules(set<int>);
	string getSCCName(set<int>);
	bool dependentOnSelf(Rule);

private:
	vector<Predicate> facts;
	vector<Predicate> queries;
	vector<Predicate> schemes;
	vector<Relation> relationSet;
	vector<Rule> rules;
	Relation myRelation;
	map<string, Relation> relationMap;
	int currentSize;
};

#endif
