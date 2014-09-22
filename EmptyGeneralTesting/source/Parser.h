#pragma once
#include "PKB.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>

using namespace std;

class Parser
{

//attributes
private:
	//data
	vector<string> fileData;
	vector<string> procName;
	vector<string> varName;
	vector<string> stmtType;
	vector<int> depthLv;
	vector<pair<int, string>> modifies;
	vector<pair<int, string>> uses;
	vector<pair<string, string>> calls;

	//references
	PKB pkb;
	
	size_t currentIndex;
	vector <string> tokens;
	int currentDepth;
	vector < vector < int > > CFGNodes;
	vector <int> processedCFGStmtFlags;
	vector <int> thenStmtFlags;
	vector <string> procedureMask;

	bool isDataProcessed;
	string currentProcessingProc;
	string outputFileName;

//public methods
public:
	Parser();
	~Parser();
	void parse(string fileName);
	void buildPKB();
	void buildVarTable();
	void buildFollowTable();
	void buildModifyTable();
	void buildParentTable();
	void buildStatTable();
	void buildUseTable();
	void buildProcTable();
	void buildCallTable();
	void buildCFG();
	bool getFileData(string fileDirectory);

	//testing methods
	int getProcNumber();
	int getVarNumber();
	int getStmtNumber();
	int getThenStmtNumber();
	int getModifyPairNumber();
	int getUsePairNumber();
	int getCallPairNumber();

//private helper methods
private:
	void readFileData();
	vector<string> preprocessData(ifstream& file);
	TNode* readProcedure();
	TNode* readStmtList();
	TNode* readStmt();
	TNode* readWhileStmt();
	TNode* readCallStmt();
	TNode* readIfStmt();
	TNode* readAssignStmt();
	TNode* readExpression(vector<string> expressionTokens);
	TNode* readTerm(vector<string> termTokens);
	TNode* readFactor(vector<string> factorTokens);
	

	void match(string expectedToken);
	string getNextToken();
	string peekForwardToken (int numberOfIndexForward);
	void error();
	vector<string> breakFileDataIntoElements();
	bool isNumber(const string str);
	int getFollowedStmt(int i);
	int getFollowingStmt(int i);
	int getParentStmt(int i);
	int getLastIndexOfTokenNotIndsideBracket (vector<string> tokens, string token);
	void buildControlFlowPath(size_t statementNo);
	vector <int> getNextNodeInControlFlow(int stmtNo);
	vector <int> getChildrenStmts(int stmtNo);
};