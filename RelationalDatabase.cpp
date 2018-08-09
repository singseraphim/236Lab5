#include <iostream>
#include <iomanip>
#include <string>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <set>
#include "RelationalDatabase.h"

void RelationalDatabase::getData(Parser myParser) {
	schemes = myParser.getSchemes();
	facts = myParser.getFacts();
	queries = myParser.getQueries();
	rules = myParser.getRules();
}

void RelationalDatabase::evaluate() {
	setRelations();
	addFacts();
	graphRules();
	cout << endl << "Query Evaluation" << endl;
	doQueries(); 
}

void RelationalDatabase::graphRules() {
	Graph myGraph;
	vector<set<int>> SCCs = myGraph.createDependencyGraph(rules);
	cout << endl << "Rule Evaluation" << endl;

	for (unsigned int i = 0; i < SCCs.size(); ++i) {
		evaluateSCC(SCCs.at(i));
	}

}

void RelationalDatabase::evaluateSCC(set<int> scc) { //evaluates individual scc like rules
	bool hasChanged = true;
	int passes = 0;
	vector<Rule> sccVec = intsToRules(scc);
	cout << "SCC: ";
	cout << getSCCName(scc) << endl;
	if ((scc.size() > 1) || dependentOnSelf(sccVec.at(0))) {
		while (hasChanged) {
			++passes;
			int beforeSize = currentSize;
			for (unsigned int i = 0; i < sccVec.size(); ++i) {
				cout << sccVec.at(i).toString() << endl;
				evaluateRule(sccVec.at(i));
			}
			int afterSize = currentSize;
			if (beforeSize == afterSize) {
				hasChanged = false;
			}
		}
	}
	else { //this doesn't work
			++passes;
		
			for (unsigned int i = 0; i < sccVec.size(); ++i) {
				cout << sccVec.at(i).toString() << endl;
				evaluateRule(sccVec.at(i));
			}
			
	}
	cout << passes << " passes: " << getSCCName(scc) << endl;
}

bool RelationalDatabase::dependentOnSelf(Rule myRule) {
	string headID = myRule.getHeadPredicate().getID();
	for (unsigned int i = 0; i < myRule.getPredicateList().size(); ++i) {
		if (myRule.getPredicateList().at(i).getID() == headID) {
			return true;
		}
	}
	return false;
}

string RelationalDatabase::getSCCName(set<int> scc) {
	string returnString;
	vector<int> sccIntVec;
	set<int>::iterator iter;
	for (iter = scc.begin(); iter != scc.end(); ++iter) {
		sccIntVec.push_back(*iter);
	}
	for (unsigned int i = 0; i < sccIntVec.size(); ++i) {
		returnString += "R";
		returnString += to_string(sccIntVec.at(i));
		if (i < sccIntVec.size() - 1) {
			returnString += ",";
		}
	}
	return returnString;
}

vector<Rule> RelationalDatabase::intsToRules(set<int> scc) {
	set<int>::iterator iter;
	vector<Rule> rulesVec;
	for (iter = scc.begin(); iter != scc.end(); ++iter) {
		rulesVec.push_back(rules.at(*iter));
	}
	return rulesVec;
}

void RelationalDatabase::evaluateRules() {
	
	bool hasChanged = true;
	int passes = 0;

	
	while (hasChanged) { 
		++passes;
		int beforeSize = currentSize; 
		for (unsigned int i = 0; i < rules.size(); ++i) {
			cout << rules.at(i).toString() << endl;
			evaluateRule(rules.at(i));
		}
		int afterSize = currentSize;
		if (beforeSize == afterSize) {
			hasChanged = false;
		}
	}
	cout << endl << "Schemes populated after " << passes << " passes through the Rules." << endl << endl;


}

void RelationalDatabase::evaluateRule(Rule myRule) {
	Relation resultRel;
	Predicate headPred = myRule.getHeadPredicate(); 
	Relation headRel; //the final relation that will be returned
	headRel.setName(headPred.getID());
	headRel.setScheme(headPred.getParameterStrings());

	vector<Predicate> rulePreds = myRule.getPredicateList();
	vector<Relation> evalRels = doRule(myRule); //evaluates the predicates like queries. evalRels = evaluatedRelations
	resultRel = join(evalRels);  //returns full joined table
	for (unsigned int i = 0; i < evalRels.size(); ++i) {
		resultRel.setScheme(joinSchemes(resultRel, evalRels.at(i))); //sets proper scheme
	}
	
	headRel = project(resultRel, headRel.getScheme()); //projects the proper columns
	
	headRel.setScheme(relationMap[headPred.getID()].getScheme()); //rename to the scheme declared in the facts section.
	
	
	
	set<Tuple> headTuples = headRel.getTuples();
	set<Tuple>::iterator headRelIter;
	for (headRelIter = headTuples.begin(); headRelIter != headTuples.end(); ++headRelIter) { //adds to facts
		Tuple currentTuple = *headRelIter;
		bool isFact = false;
		
		set<Tuple>::iterator relMapIter;
		set<Tuple> relMapTuples = relationMap[headPred.getID()].getTuples();

		for (relMapIter = relMapTuples.begin(); relMapIter != relMapTuples.end(); ++relMapIter) {
			Tuple relMapTuple = *relMapIter;
			if (relMapTuple == currentTuple) {
				isFact = true;
			}
		}
		if (!isFact) {
			relationMap[headPred.getID()].addTuple(currentTuple);
			++currentSize;
			cout << "  ";
			for (unsigned int i = 0; i < currentTuple.size(); ++i) {
				cout << headRel.getScheme().at(i) << "=" << currentTuple.at(i);
				if (i < currentTuple.size() - 1) {
					cout << ", ";
				}
			}
			cout << endl;
		}
	}
}

Relation RelationalDatabase::project(Relation& rel, Scheme headScheme) {
	Relation newRel;
	set<Tuple>::iterator iter;
	set<Tuple> relTuples = rel.getTuples(); 
	
	vector<int> headSchemeIndices = getHeadIndices(rel, headScheme);
	
	for (iter = relTuples.begin(); iter != relTuples.end(); ++iter) {
		Tuple newTuple;
		Tuple currentTuple = *iter;
		for (unsigned int i = 0; i < headSchemeIndices.size(); ++i) {
			newTuple.push_back(currentTuple.at(headSchemeIndices.at(i)));
		}
		newRel.addTuple(newTuple); 
	}

	return newRel;
}

vector<int> RelationalDatabase::getHeadIndices(Relation& rel, Scheme headScheme) {
	
	vector<int> headSchemeIndices;
	vector<string> relScheme = rel.getScheme();
	for (unsigned int j = 0; j < headScheme.size(); ++j) {
		for (unsigned int i = 0; i < relScheme.size(); ++i) {// switch order
			if (relScheme.at(i) == headScheme.at(j)) {
				headSchemeIndices.push_back(i);
			}
		}
	}
	return headSchemeIndices;
}

Relation RelationalDatabase::join(vector<Relation>& evalRels) {
	Relation newRel;
	
	Relation currentRelation = evalRels.at(0);
	for (unsigned int i = 1; i < evalRels.size(); ++i) {
		currentRelation = joinTwoRels(currentRelation, evalRels.at(i));

	}
	set<Tuple> tupleSet = currentRelation.getTuples();
	set<Tuple>::iterator iter;
	for (iter = tupleSet.begin(); iter != tupleSet.end(); ++iter) {
		newRel.addTuple(*iter);
	}

	return newRel;
}

Relation RelationalDatabase::joinTwoRels(Relation& r1, Relation& r2) { 
	Relation returnRel;
	vector<string> matchCols = getMatchCols(r1.getScheme(), r2.getScheme());
	set<Tuple> r1Tuples = r1.getTuples();
	set<Tuple> r2Tuples = r2.getTuples();
	set<Tuple>::iterator r1Iter;
	set<Tuple>::iterator r2Iter;

	returnRel.setScheme(joinSchemes(r1, r2));
	
		for (r1Iter = r1Tuples.begin(); r1Iter != r1Tuples.end(); ++r1Iter) {
			for (r2Iter = r2Tuples.begin(); r2Iter != r2Tuples.end(); ++r2Iter) {
				if (canJoin(r1, r2, *r1Iter, *r2Iter, matchCols)) {
					Tuple newTuple = combineTuples(r1, r2, *r1Iter, *r2Iter, returnRel.getScheme(), matchCols); 
					returnRel.addTuple(newTuple);
				}
			}
		}
	

	return returnRel;
}

Tuple RelationalDatabase::combineTuples(const Relation& r1, const Relation& r2, Tuple t1, Tuple t2, Scheme myScheme, vector<string> matchCols) {
	Scheme scheme1 = r1.getScheme();
	Scheme scheme2 = r2.getScheme();
	Tuple returnTuple;
	for (unsigned int i = 0; i < t1.size(); ++i) {
		returnTuple.push_back(t1.at(i));
	}
	for (unsigned int i = 0; i < t2.size(); ++i) { 
		for (unsigned int j = scheme1.size(); j < myScheme.size(); ++j) {
			if (scheme2.at(i) == myScheme.at(j)) {
				returnTuple.push_back(t2.at(i));
			}
		}
	}

	return returnTuple;
}

int RelationalDatabase::getMatchIndex(string s, Scheme myScheme) {
	for (unsigned int i = 0; i < myScheme.size(); ++i) {
		if (myScheme.at(i) == s) {
			return i;
		}
	}
	return -1;
}

vector<string> RelationalDatabase::getMatchCols(Scheme s1, Scheme s2) {
	vector<string> matchVec;

	for (unsigned int i = 0; i < s1.size(); ++i) {
		for (unsigned int j = 0; j < s2.size(); ++j) {
			if (s1.at(i) == s2.at(j)) {
				matchVec.push_back(s1.at(i));
			}
		}
	}

	return matchVec;
}

vector<Relation> RelationalDatabase::doRule(Rule myRule) { //evaluates predicates of a rule like queries. 
	vector<Relation> rulePredRelations;
	myRelation.clear();
	for (unsigned int i = 0; i < myRule.getPredicateList().size(); ++i) { 
		Predicate myPred = myRule.getPredicateList().at(i);
		map<string, int> uniqueVars;
		vector<int> projectIndices;
		vector<string> renameVec;
		myRelation = select(myPred, uniqueVars, projectIndices, renameVec);
		myRelation = relationMap[myPred.getID()].project(projectIndices, myRelation);
		myRelation = myRelation.rename(renameVec, myRelation);
		rulePredRelations.push_back(myRelation);
		myRelation.clear();

	}
	return rulePredRelations;
}

Relation RelationalDatabase::select(Predicate myPred, map<string, int>& uniqueVars, vector<int>& projectIndices, vector<string>& renameVec) { //Does initial select operation on predicates. Needs more testing, haven't tried weird combinations. It looks like it's working okay though. 
	myRelation.clear();
	myRelation = relationMap[myPred.getID()];
	for (unsigned int j = 0; j < myPred.getParameters().size(); ++j) {
		if (myPred.getParameters().at(j).getType() == STRING) {
			myRelation = relationMap[myPred.getID()].selectStr(myPred.getParameters().at(j).toString(), j, myRelation);
		}
		else if (myPred.getParameters().at(j).getType() == ID) { //it selects the right column but then it changes back on the next element?
			if (uniqueVars.count(myPred.getParameters().at(j).toString()) == 0) {
				projectIndices.push_back(j);
				uniqueVars[myPred.getParameters().at(j).toString()] = j;
				renameVec.push_back(myPred.getParameters().at(j).toString());
			}
			int matchIndex = findMatchIndex(myPred.getParameters(), j);
			if (matchIndex != 0) {
				myRelation = relationMap[myPred.getID()].selectCol(myRelation, j, matchIndex);
			}
		}

	}
	return myRelation;
}

bool RelationalDatabase::canJoin(Relation& r1, Relation& r2, Tuple t1, Tuple t2, vector<string> matchCols) {
	bool canJoin = true;
	for (unsigned int i = 0; i < matchCols.size(); ++i) {
		int r1MatchIndex = getMatchIndex(matchCols.at(i), r1.getScheme());
		int r2MatchIndex = getMatchIndex(matchCols.at(i), r2.getScheme());
		if (t1.at(r1MatchIndex) != t2.at(r2MatchIndex)) {
			canJoin = false;
		}
	}
	return canJoin;
}

Scheme RelationalDatabase::joinSchemes(Scheme p1, Predicate p2) {

	vector<string> p2Params = p2.getParameterStrings();
	Scheme newScheme;
	bool isInScheme = false;
	for (unsigned int i = 0; i < p1.size(); ++i) {
		newScheme.push_back(p1.at(i));
	}
	for (unsigned int i = 0; i < p2Params.size(); ++i) {
		for (unsigned int j = 0; j < newScheme.size(); ++j) {
			if (p2Params.at(i) == newScheme.at(j)) {
				isInScheme = true;
			}
		}
		if (!isInScheme) {
			newScheme.push_back(p2Params.at(i));
		}
		else {
			isInScheme = false;
		}
	}
	return newScheme;
}

Scheme RelationalDatabase::joinSchemes(Relation& r1, Relation& r2) {
	Scheme newScheme;
	bool isInScheme = false;

	for (unsigned int i = 0; i < r1.getScheme().size(); ++i) {
		newScheme.push_back(r1.getScheme().at(i));
	}
	for (unsigned int i = 0; i < r2.getScheme().size(); ++i) {
		for (unsigned int j = 0; j < newScheme.size(); ++j) {
			if (r2.getScheme().at(i) == newScheme.at(j)) {
				isInScheme = true;
			}
		}
		if (!isInScheme) {
			newScheme.push_back(r2.getScheme().at(i));
		}
		else {
			isInScheme = false;
		}
	}


	return newScheme;
}

void RelationalDatabase::setRelations() { //puts all schemes into a map of string -> relation
	for (unsigned int i = 0; i < schemes.size(); ++i) {
		myRelation.setName(schemes.at(i).getID());
		myRelation.setScheme(schemes.at(i).getParameterStrings());
		relationMap[schemes.at(i).getID()] = myRelation;
		myRelation.clear();
	}

}

void RelationalDatabase::addFacts() { //adds facts to the relations
	for (unsigned int i = 0; i < facts.size(); ++i) {
		relationMap[facts.at(i).getID()].addTuple(facts.at(i));
		++currentSize;
	}
}

void RelationalDatabase::doQueries() { //evaluates queries. Selects, projects and renames. 
	myRelation.clear();
	for (unsigned int i = 0; i < queries.size(); ++i) {
		myRelation = relationMap[queries.at(i).getID()];
		map<string, int> uniqueVars;
		vector<int> projectIndices;
		vector<Parameter> qParams = queries.at(i).getParameters();
		vector<string> renameVec;

		for (unsigned int j = 0; j < qParams.size(); ++j) {
			if (qParams.at(j).getType() == STRING) {
				myRelation = relationMap[queries.at(i).getID()].selectStr(qParams.at(j).toString(), j, myRelation);
			}
			else if (qParams.at(j).getType() == ID) {
				if (uniqueVars.count(qParams.at(j).toString()) == 0) {
					projectIndices.push_back(j);
					uniqueVars[qParams.at(j).toString()] = j;
					renameVec.push_back(qParams.at(j).toString());
				}
				int matchIndex = findMatchIndex(qParams, j);
				if (matchIndex != 0) {
					myRelation = relationMap[queries.at(i).getID()].selectCol(myRelation, j, matchIndex);
				}
			}
		}
		myRelation = relationMap[queries.at(i).getID()].project(projectIndices, myRelation);
		myRelation = myRelation.rename(renameVec, myRelation);
		string validString;
		int size = myRelation.getSize();
		if (size > 0) {
			cout << queries.at(i).toString() << "? Yes";
			cout << "(" << size << ")" << endl;
		}
		else {
			cout << queries.at(i).toString() << "? No\n";
		}
		cout << relationString(myRelation);
	}
}

string RelationalDatabase::relationString(Relation myRel) { //returns the correctly formatted output string for a relation
	string returnString;
	set<Tuple> tupleSet = myRel.getTuples();
	set<Tuple>::iterator iter;
	Tuple myTuple;

	for (iter = tupleSet.begin(); iter != tupleSet.end(); ++iter) {
		if (myRel.getScheme().size() > 0) returnString += "  ";
		for (unsigned int i = 0; i < myRel.getScheme().size(); ++i) {
			myTuple = *iter;
			returnString += myRel.getScheme().at(i) + "=" + myTuple.at(i);
			if (i < myRel.getScheme().size() - 1) returnString += ", ";
		}
		if (myRel.getScheme().size() > 0) returnString += "\n";
	}
	
	return returnString;
}

int RelationalDatabase::findMatchIndex(vector<Parameter> paramList, int currIndex) { //determines if there are identical columns in a scheme, and if so, where they are. 
	int returnIndex = 0;
	for (unsigned int i = currIndex + 1; i < paramList.size(); ++i) {
		if (paramList.at(i).toString() == paramList.at(currIndex).toString()) {
			returnIndex = i;
			break;
		}
	}
	return returnIndex;
}




/*
Captain's log:
Everything works great, except the number of passes is off on some of them. 

Tonight: do tomorrow's homework, fix bugs and get ready to pass off
Tomorrow morning: work
Tomorrow evening: pass off lab 5


*/