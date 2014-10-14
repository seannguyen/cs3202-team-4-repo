#include "QueryPreprocessor.h"

/* CONSTRUCTOR */
QueryPreprocessor::QueryPreprocessor() {
}

QueryPreprocessor::QueryPreprocessor(string directory) {
	this -> fileDirectory = directory;
}

/* PUBLIC METHOD */ 

// Description: this method is to start preprocess the file 
// whose directory is save in fileDirectory
void QueryPreprocessor::Preprocess() {
	readFileData();
	preprocessFileData();
}

void QueryPreprocessor::Preprocess(string query) {
	// first, reset storage (QueryRepresentator)
	QueryRepresentator::reset();

	vector<string> queryList; queryList.push_back(query);
	preprocessQuery(queryList, 0);
}

/* PRIVATE METHODS */
 
// Description: this method is to open file and 
// save only lines containing query into "fileData" list
void QueryPreprocessor::readFileData() {
	ifstream file(fileDirectory.c_str());

	if (file.is_open()) {
		// read line by line
		string line;
		int countLine = 0;
		int numOfQueries = 0;
		vector<string> query;
		while(!file.eof()) {
			getline(file, line);
			countLine++;
			// recognize first line as number of queries in file
			// change string to num here
			// set numOfQueries
			if (countLine==1) {
				numOfQueries = atoi(line.c_str());
				//cout <<"Num of queries: " << numOfQueries << endl;
			} else {

				// meet first line of query, recognize as description (useless info)
				// take no action
				if (countLine%5==3) {
				}
				// meet second line of query, recognize as declaration 
				// save into queryData
				else if (countLine%5==4) {
					query.push_back(line);
				}
				// meet third line of query, recognize as query part
				// save into queryData
				else if (countLine%5==0) {
					query.push_back(line);
				}
				// meet forth line of query, recognize as query results
				// NOTE: currently take no action
				else if (countLine%5==1) {
				}
				// meet fifth line of query, recognize as query's ending sign ("5000")
				// take no action.
				// push query to fileData. Reset query
				else if (countLine%5==2 && countLine!=2) {
					fileData.push_back(query);
					query.clear();
				}
			}
		}
		file.close();
	} else {
		cout << "Not a file directory" <<endl;
	}
}

// Description: this method is to take and preprocess query, one by one
void QueryPreprocessor::preprocessFileData() {
	// first, reset storage (QueryRepresentator)
	QueryRepresentator::reset();

	// loop: queryData, take query one by one
	for (size_t i=0; i<fileData.size(); i++) {
		vector<string> query = fileData[i];
	//	cout << i + 1 <<endl;
		preprocessQuery(query, i);
	}
}

// Description: this method is to save query data into QueryTree and SymbolTable,
// and save them into QueryRepresentator
void QueryPreprocessor::preprocessQuery(vector<string> query, int index) {
	SymbolTable table;
	QueryTree tree;
	vector<string> errors;
	string element = "";

	for (size_t i=0; i<query.size(); i++) {
		string curLine = query[i];
		istringstream ss;
		ss.str(curLine);
		while (ss >> element) {
			if (element==KEYWORD_PROG_LINE	|| element==KEYWORD_ASSIGN	||
				element==KEYWORD_STMT		|| element==KEYWORD_VAR	||
				element==KEYWORD_CONST	|| element==KEYWORD_WHILE	||
				element==KEYWORD_IF || element==KEYWORD_CALL ||
				element==KEYWORD_PROCEDURE) {
				// find ending sign of this declaration (";");
				if (curLine.find_first_of(";")!=string::npos) {
					unsigned index = curLine.find_first_of(";");
					// read declaration
					string declaration = curLine.substr(0, index+1);
					preprocessDeclaration(table, declaration, errors);
					// continue with the rest of curLine
					curLine = curLine.substr(index+1);
					ss.str(curLine);
				} else {
					errors.push_back("Error 001: Found no ending sign for symbol declaration of query " + i);
					return;
				}
			} else if (element==KEYWORD_SELECT) {
				//for (size_t i=0; i<table.getSize(); i++) 
				//	cout << i << "  "<<table.getType(i)<< " " <<table.getName(i)<<endl;
				preprocessQueryPart(tree, table, curLine, errors);
				ss.str("");
				ss.clear();
			} else {
				errors.push_back("Error 002: Unable to recognize symbol: " + element + " in line \n" + curLine);
			}
		} 
	}

	if (errors.size()==0) {
		//		tree.printTree();
		QueryRepresentator::addQuery(table, tree, true);
	} else {
		cout << "ERROR LIST:, QUERY " << index+1 <<endl;
		for (size_t i=0; i<errors.size(); i++) {
			cout << errors[i] <<endl;
		}
		cout <<endl;
		QueryRepresentator::addQuery(table, tree, false);
	}
}

// Description: this method is to preprocess the declaration part of query
void QueryPreprocessor::preprocessDeclaration(SymbolTable& table, string declaration, vector<string>& errors) {
	// break string into words
	vector<string> list = breakStringIntoWords(declaration);

	// first word is type of symbol(s)
	string type=list[0];

	for(size_t i=1; i<list.size(); i++) {
		// expect to meet comma or semicolon
		if (i%2==0) {
			if (list[i]!="," && list[i]!=";") { 
				errors.push_back("Error 003: expect end sign");
				return;
			}
		}
		// expect to meet symbol
		else {
			if (list[i]=="," || list[i]==";")  {
				errors.push_back("Error 004: expect symbol");
				return;
			}
			string name = list[i];

			// save symbol into SymbolTable
			table.setSymbol(type, name);
		}
	}
}


// Description: this method is to preprocess the query part of query
void QueryPreprocessor::preprocessQueryPart(QueryTree& tree, SymbolTable table, string queryPart, vector<string>& errors) {
	// break string into words
	vector<string> list = breakStringIntoWords(queryPart);
	// create root node for Select
	TNode * root = new TNode(Select, "");
	// recognize result and create node for it
	TNode * result = preprocessResultNode(list, table, errors, 1);
	root -> addChild(result);

	Symbol currentCondition = Undefined;
	// find and preprocess conditions of query
	for (size_t i=2; i<list.size(); i++) {
		if (list[i]=="such" && list[i+1]=="that") {
			currentCondition = SuchThatCls;
			unsigned index = findFirstElement(list, i, ")");
			if ((int)index>i) {
				vector<string> suchThatCondition = subList(list, i+2, index);
				TNode * suchThat = preprocessSuchThatCondition(suchThatCondition, table, errors);
				root -> addChild(suchThat);
				// go to element after this condition
				i = index;
			}
		} else if (list[i]=="pattern") {
			currentCondition = PatternCls;
			unsigned index = findFirstElement(list, i, ")");
			if ((int)index>i) {
				vector<string> patternCondition = subList(list, i+1, index);
				TNode * pattern = preprocessPatternCondition(patternCondition, table, errors);
				root -> addChild(pattern);
				// go to element after this condition
				i = index;
			}
		} else if (list[i]=="with") {
			currentCondition = WithCls;
			TNode * withCls = preprocessWithCondition(list, table, errors, i);
			root -> addChild(withCls);
		} else if (list[i]=="and") {
			switch (currentCondition) {
			case SuchThatCls:
				{
					unsigned index = findFirstElement(list, i, ")");
					if ((int)index>i) {
						vector<string> suchThatCondition = subList(list, i+1, index);
						TNode * suchThat = preprocessSuchThatCondition(suchThatCondition, table, errors);
						root -> addChild(suchThat);
						// go to element after this condition
						i = index;
					}
					break;
				}
			case PatternCls:
				{
					unsigned index = findFirstElement(list, i, ")");
					if ((int)index>i) {
						vector<string> patternCondition = subList(list, i+1, index);
						TNode * pattern = preprocessPatternCondition(patternCondition, table, errors);
						root -> addChild(pattern);
						// go to element after this condition
						i = index;
					}
					break;
				}
			case WithCls:
				{
					currentCondition = WithCls;
					TNode * withCls = preprocessWithCondition(list, table, errors, i);
					root -> addChild(withCls);
				}
			default:
				{
					errors.push_back("Error: find \"and\" with no previous clause");
					break;
				}
			}
		} else {
		}
	}

	// final step: sort children node of root for better evaluation
	root -> sortChildrenList();

	tree.setRoot(root);
}

TNode * QueryPreprocessor::preprocessResultNode(vector<string> list, SymbolTable table, vector<string>& errors, int i) {
	// FOR ITERATION 1 AND 2: BOOLEAN and single value
	if (list[i]=="BOOLEAN") {
		TNode * node = new TNode(ResultCls, "a-BOOLEAN");
		return node;
	} else {
		if (list[i]=="<") {
			//expect non-single tuple
			int j = findFirstElement(list, i, ">");
			if (j==-1) {
				errors.push_back("Error 005: invalid syntax for tuple result");
				TNode * node = new TNode();
				return node;
			}
			vector<string> tuple = subList(list, i, j);
			return preprocessTupleResult(tuple, table, errors);
		} else {
			if (table.isSymbol(list[i])) {
				TNode * node = new TNode(ResultCls, "a-single");
				TNode * nodeChild = new TNode(QuerySymbol, list[i]);
				node -> addChild(nodeChild);
				return node;
			}
			errors.push_back("Error 005: unable to find symbol: " + list[i]);
			TNode * node = new TNode();
			return node;
		}
	}
}

TNode * QueryPreprocessor::preprocessTupleResult(vector<string> list, SymbolTable table, vector<string>& errors) {
	TNode * node = new TNode(ResultCls, "a-tuple");

	for (size_t i=1; i<list.size()-1; i++) {
		if (i%2==1) {
			// expect declared symbol
			if (table.isSymbol(list[i])) {
				TNode * child = new TNode(QuerySymbol, list[i]);
				node ->addChild(child);
			} else {
				errors.push_back("Error 005: invalid symbol in tuple result: " + list[i]);
			}
		} else {
			// expect comma
			if (list[i]!=KEYWORD_COMMA) {
				errors.push_back("Error 005: invalid symbol in tuple result: " + list[i]);
			}
		}
	}

	return node;
}

// Description: this method is to preprocess the such that condition of query
TNode * QueryPreprocessor::preprocessSuchThatCondition(vector<string> list, SymbolTable table, vector<string>& errors) {
	unsigned size = list.size();
	// create first node for such that
	TNode * suchThatNode = new TNode(SuchThatCls, "c");

	//expect first value to be relationship's name
	string relation = list[0];

	if (SyntaxHelper::isRelation(relation)) {
		Symbol relationType = SyntaxHelper::getSymbolType(relation);
		TNode * relationNode = new TNode(relationType);
		
		if (list[1]!="(") {
			errors.push_back("Error 006: expect symbol ( after relation name");
		}

		int commaIndex=0;
		if (list[3]==",") {
			commaIndex=3;
			string arg1 = list[2];
			if (isNumber(arg1)) {
				TNode * arg1Node = new TNode(Const, arg1);
				relationNode -> addChild(arg1Node);
			} else {
				// expect a symbol of type procedure or any stmt type
				// depending on relation's type
				string arg1Type = table.getType(arg1);
				switch (relationType) {
				case Follows:
				case FollowsS:
				case Parent:
				case ParentS:
					if (arg1Type!=KEYWORD_STMT && arg1Type!=KEYWORD_PROG_LINE &&
						arg1Type!=KEYWORD_ASSIGN && arg1Type!= KEYWORD_WHILE &&
						arg1Type!=KEYWORD_CALL && arg1Type!=KEYWORD_IF) {
							errors.push_back("Error 000: not a valid symbol: " + arg1 + " for relation: " + relation);
					}
					break;
				case Uses:
				case Modifies:
					if (arg1Type!=KEYWORD_PROCEDURE && 
						arg1Type!=KEYWORD_STMT && arg1Type!=KEYWORD_PROG_LINE &&
						arg1Type!=KEYWORD_ASSIGN && arg1Type!= KEYWORD_WHILE &&
						arg1Type!=KEYWORD_CALL && arg1Type!=KEYWORD_IF) {
							errors.push_back("Error 000: not a valid symbol: " + arg1 + " for relation: " + relation);
					}
					break;
				case Calls:
				case CallsS:
					if (arg1Type!=KEYWORD_PROCEDURE) {
						errors.push_back("Error 000: not a valid symbol: " + arg1 + " for relation: " + relation);
					}
					break;
				case Nexts:
				case NextsS:
					if (arg1Type!=KEYWORD_PROG_LINE) {
						errors.push_back("Error 000: not a valid symbol: " + arg1 + " for relation: " + relation);
					}
					break;
					break;
				default:
					break;
				}
				TNode * arg1Node = new TNode(QuerySymbol, arg1);
				relationNode -> addChild(arg1Node);
			}
		} else if (list[5]==",") {
			commaIndex = 5;
			// expect a procedure name
			switch (relationType) {
			case Modifies:
			case Uses:
			case Calls:
			case CallsS:
				break;
			default:
				errors.push_back("Error 000: invalid use of: " + list[3] + " for relation: " + relation);
				break;
			}
			TNode * arg1Node = new TNode(Const, list[3]);
			relationNode -> addChild(arg1Node);
		}

		if (list[commaIndex+2]==")") {
			string arg2 = list[commaIndex+1];
			if (isNumber(arg2)) {
				TNode * arg2Node = new TNode(Const, arg2);
				relationNode -> addChild(arg2Node);
			} else {
				string arg2Type = table.getType(arg2);
				// expect a valid symbol's type
				switch (relationType) {
				case Follows:
				case FollowsS:
				case Parent:
				case ParentS:
					if (arg2Type!=KEYWORD_PROG_LINE && arg2Type!=KEYWORD_STMT &&
						arg2Type!=KEYWORD_ASSIGN && arg2Type!=KEYWORD_WHILE &&
						arg2Type!= KEYWORD_CALL && arg2Type!=KEYWORD_IF) {
						errors.push_back("Error 000: not a valid symbol: " + arg2 + " for relation: " + relation);
					}
					break;
				case Modifies:
				case Uses:
					if (arg2Type!=KEYWORD_VAR) {
						errors.push_back("Error 000: not a valid symbol: " + arg2 + " for relation: " + relation);
					}
					break;
				case Calls:
				case CallsS:
					if (arg2Type!=KEYWORD_PROCEDURE) {
						errors.push_back("Error 000: not a valid symbol: " + arg2 + " for relation: " + relation);
					}
					break;
				case Nexts:
				case NextsS:
					if (arg2Type!=KEYWORD_PROG_LINE) {
						errors.push_back("Error 000: not a valid symbol: " + arg2 + " for relation: " + relation);
					}
					break;
				default:
					errors.push_back("Error 000: invalid use of: " + list[3] + " for relation: " + relation);
					break;
				}
				TNode * arg2Node = new TNode(QuerySymbol, arg2);
				relationNode -> addChild(arg2Node);
			}
		} else if (list[commaIndex+4]==")") {
			// expect a name for variable or procedure
			string arg2Value = list[commaIndex+2];
			switch (relationType) {
			case Uses:
			case Modifies:
			case Calls:
			case CallsS:
				break;
			default:
				break;
			}
			TNode * arg2Node = new TNode(Const, list[commaIndex+2]);
			relationNode -> addChild(arg2Node);
		} else {
			errors.push_back("Error 007: no sign of ending for relation");
		}

		suchThatNode -> addChild(relationNode);
	} else {
		errors.push_back("Error 008: not a relation name: " + relation);
	}

	return suchThatNode;
}

// Description: this method is to preprocess the pattern condition of query
TNode * QueryPreprocessor::preprocessPatternCondition(vector<string> list, SymbolTable table, vector<string>& errors) {
	unsigned size = list.size();
	// create first node for pattern
	TNode * pattern = new TNode(PatternCls, "d");

	vector<string> patternContent = subList(list, 1, size-1);
	string argName = list[0];
	string argType = table.getType(argName);
	Symbol type = SyntaxHelper::getSymbolType(argType);

	switch (type) {
	case Assign:
		{
			TNode * assignNode = preprocessAssignPattern(argName, patternContent, table, errors);
			pattern -> addChild(assignNode);
			break;
		}
	case While:
		{
			TNode * whileNode = preprocessWhilePattern(argName, patternContent, table, errors);
			pattern -> addChild(whileNode);
			break;
		}
	case If:
		{
			TNode * ifNode = preprocessIfPattern(argName, patternContent, table, errors);
			pattern -> addChild(ifNode);
			break;
		}
	default:
		{
			errors.push_back("Error 009: not a valid stmt type for pattern clause: " + argName);
			break;
		}
	}

	return pattern;
}

TNode * QueryPreprocessor::preprocessAssignPattern(string name, vector<string> list, SymbolTable table, vector<string>& errors) {
	TNode * assignNode = new TNode(QuerySymbol, name);
	unsigned size = list.size();
	if (list[0]!=KEYWORD_OPENBRACKET || list[size-1]!=KEYWORD_CLOSEBRACKET) {
		errors.push_back("Error 010: no valid bracket for pattern");
	}
	int commaIndex = findFirstElement(list, 0, ",");

	switch (commaIndex) {
	case 2:
		{
			string arg1 = list[1];
			// expect arg1 of pattern is underline or query symbol
			if (arg1==KEYWORD_UNDERLINE) {
				TNode * arg1Node = new TNode(Underline);
				assignNode -> addChild(arg1Node);
			} else {
				if (table.getIndex(arg1)==0) {
					errors.push_back("Error 011: not a declared symbol: " + arg1);
				}
				TNode * arg1Node = new TNode(QuerySymbol, arg1);
				assignNode -> addChild(arg1Node);
			}
			break;
		}
	case 4:
		{
			// expect a variable name
			if (list[1]!="\"" || list[3]!="\"") {
				errors.push_back("Error 012_01: syntax of pattern");
				return assignNode;
			}
			TNode * arg1Node = new TNode(Const, list[2]);
			assignNode -> addChild(arg1Node);
			break;
		}
	default:
		break;
	}

	// preprocess right hand side 
	switch (size-commaIndex) {
	case 3:
		{
			// expect an underline in the right of the comma
			if (list[commaIndex+1]!=KEYWORD_UNDERLINE) {
				errors.push_back("Error 012_02: Syntax of pattern");
				return assignNode;
			}
			TNode * arg2Node = new TNode(Underline);
			assignNode ->addChild(arg2Node);
			break;
		}
	default:
		{
			if (list[commaIndex+1]!=KEYWORD_UNDERLINE || list[size-2]!=KEYWORD_UNDERLINE) {
				if (list[commaIndex+1]!="\"" || list[size-2]!="\"") {
					errors.push_back("Error 012_03: Syntax of pattern");
					return assignNode;
				}
				vector<string> expression = subList(list, commaIndex+2, size-3);
				TNode * arg2Node = new TNode(No_Underline);
				TNode * exprNode = preprocessExpressionNode(expression, errors);
				arg2Node ->addChild(exprNode);
				assignNode ->addChild(arg2Node);
			} else {
				if (list[commaIndex+2]!="\"" || list[size-3]!="\"") {
					errors.push_back("Error 012_04: Syntax of pattern");
					return assignNode;
				}
				vector<string> expression = subList(list, commaIndex+3, size-4);
				TNode * arg2Node = new TNode(Underline);
				TNode * exprNode = preprocessExpressionNode(expression, errors);
				arg2Node ->addChild(exprNode);
				assignNode ->addChild(arg2Node);
			}
			break;
		}
	}
	
	return assignNode;
}

TNode * QueryPreprocessor::preprocessWhilePattern(string name, vector<string> list, SymbolTable table, vector<string>& errors) {
	TNode * whileNode = new TNode(QuerySymbol, name);
	unsigned size = list.size();
	if (list[0]!=KEYWORD_OPENBRACKET || list[size-1]!=KEYWORD_CLOSEBRACKET) {
		errors.push_back("Error 010: no valid bracket for pattern");
	}
	int commaIndex = findFirstElement(list, 0, ",");
	
	switch (commaIndex) {
	case 2:
		{
			string arg1 = list[1];
			// expect arg1 of pattern is underline or query symbol
			if (arg1==KEYWORD_UNDERLINE) {
				TNode * arg1Node = new TNode(Underline);
				whileNode -> addChild(arg1Node);
			} else {
				if (table.getIndex(arg1)==0) {
					errors.push_back("Error 011: not a declared symbol: " + arg1);
				}
				TNode * arg1Node = new TNode(QuerySymbol, arg1);
				whileNode -> addChild(arg1Node);
			}
			break;
		}
	case 4:
		{
			// expect a variable name
			if (list[1]!="\"" || list[3]!="\"") {
				errors.push_back("Error 012_01: syntax of pattern");
				return whileNode;
			}
			TNode * arg1Node = new TNode(Const, list[2]);
			whileNode -> addChild(arg1Node);
			break;
		}
	default:
		break;
	}

	// expect right hand side is underline
	if (commaIndex>(int)size-2 || list[commaIndex+1]!=KEYWORD_UNDERLINE) {
		errors.push_back("Error 012_05: syntax of pattern");
	} else {
		TNode * arg2Node = new TNode(Underline);
		whileNode ->addChild(arg2Node);
	}

	return whileNode;
}

TNode * QueryPreprocessor::preprocessIfPattern(string name, vector<string> list, SymbolTable table, vector<string>& errors) {
	TNode * ifNode = new TNode(QuerySymbol, name);
	unsigned size = list.size();
	if (list[0]!=KEYWORD_OPENBRACKET || list[size-1]!=KEYWORD_CLOSEBRACKET) {
		errors.push_back("Error 010: no valid bracket for pattern");
	}
	int commaIndex = findFirstElement(list, 0, ",");
	switch (commaIndex) {
	case 2:
		{
			string arg1 = list[1];
			// expect arg1 of pattern is underline or query symbol
			if (arg1==KEYWORD_UNDERLINE) {
				TNode * arg1Node = new TNode(Underline);
				ifNode -> addChild(arg1Node);
			} else {
				if (table.getIndex(arg1)==0) {
					errors.push_back("Error 011: not a declared symbol: " + arg1);
				}
				TNode * arg1Node = new TNode(QuerySymbol, arg1);
				ifNode -> addChild(arg1Node);
			}
			break;
		}
	case 4:
		{
			// expect a variable name
			if (list[1]!="\"" || list[3]!="\"") {
				errors.push_back("Error 012_01: syntax of pattern");
				return ifNode;
			}
			TNode * arg1Node = new TNode(Const, list[2]);
			ifNode -> addChild(arg1Node);
			break;
		}
	default:
		break;
	}

	// expect right hand side has 2 underline signs
	if (commaIndex>(int)size-4 || list[commaIndex+1]!=KEYWORD_UNDERLINE) {
		errors.push_back("Error 012_05: syntax of pattern");
	} else {
		TNode * arg2Node = new TNode(Underline);
		ifNode ->addChild(arg2Node);
	}

	if (list[size-3]!=KEYWORD_COMMA || list[size-2]!=KEYWORD_UNDERLINE) {
		errors.push_back("Error 012_06: syntax of pattern");
	} else {
		TNode * arg3Node = new TNode(Underline);
		ifNode ->addChild(arg3Node);
	}

	return ifNode;
}

TNode * QueryPreprocessor::preprocessExpressionNode(vector<string> list, vector<string>& errors) {
	TNode * exprNode = new TNode();
	unsigned size = list.size();
	// base case
	if (size==0) {
		errors.push_back("Error 013: lack in pattern expression");
	} else if (size==1) {
		string expr = list[0];
		if (expr==KEYWORD_PLUSSIGN || expr==KEYWORD_MINUSSIGN || expr==KEYWORD_MULTIPLYSIGN ||
			expr==KEYWORD_OPENBRACKET ||expr==KEYWORD_CLOSEBRACKET) {
			errors.push_back("Error 013: lack in pattern expression");
		} else {
			if (isNumber(expr)) {
				exprNode = new TNode(Const, expr);
			} else {
				exprNode = new TNode(Var, expr);
			}
		}
	} else {
		// find last index of plus/ minus/ multiple outside of bracket
		int plusSignIndex = getLastIndexOfTokenNotInsideBracket(list, KEYWORD_PLUSSIGN);
		int minusSignIndex = getLastIndexOfTokenNotInsideBracket(list, KEYWORD_MINUSSIGN);
		int signIndex = max(plusSignIndex, minusSignIndex);
		if (signIndex < 0) {
			int multiplySignIndex = getLastIndexOfTokenNotInsideBracket(list, KEYWORD_MULTIPLYSIGN);
			// create node for multiply sign and make 2 nodes before/after it its children
			if (multiplySignIndex > 0) {
				TNode * multiplyNode = new TNode(Times);
				vector<string> sublist1 = subList(list, 0, multiplySignIndex-1);
				vector<string> sublist2 = subList(list, multiplySignIndex+1, size-1);
				TNode * node1 = preprocessExpressionNode(sublist1, errors);
				TNode * node2 = preprocessExpressionNode(sublist2, errors);
				multiplyNode ->addChild(node1);
				multiplyNode ->addChild(node2);
				return multiplyNode;
			} else {
				// there is a bracket covering all expression
				if (list[0]==KEYWORD_OPENBRACKET && list[size-1]==KEYWORD_CLOSEBRACKET) {
					list = subList(list, 1, size-2);
					return preprocessExpressionNode(list, errors);
				} else {
					errors.push_back("Error 013: invalid pattern expression");
					return exprNode;
				}
			}
		} else {
			if (plusSignIndex>minusSignIndex) {
				exprNode = new TNode(Plus);
			} else {
				exprNode = new TNode(Minus);
			}
			vector<string> sublist1 = subList(list, 0, signIndex-1);
			vector<string> sublist2 = subList(list, signIndex+1, size-1);
			TNode * node1 = preprocessExpressionNode(sublist1, errors);
			TNode * node2 = preprocessExpressionNode(sublist2, errors);
			exprNode ->addChild(node1);
			exprNode ->addChild(node2);

			return exprNode;
		}
	}

	return exprNode;
}

TNode * QueryPreprocessor::preprocessWithCondition(vector<string> list, SymbolTable table, vector<string> & errors, int index) {
	TNode * withCls = new TNode(WithCls, "b");

	string arg1 = list[index+1];
	string arg1Type = table.getType(arg1);
	if (table.isSymbol(arg1)) {
		// check syntax
		if (arg1Type!=KEYWORD_PROG_LINE && list[index+2]!=".") {
			errors.push_back("Error 011: wrong expression: " + arg1 + list[index+2] + list[index+3]);
		} 
		if (arg1Type==KEYWORD_PROCEDURE && list[index+3]!="procName") {
			errors.push_back("Error 012: wrong expression: " + arg1 + list[index+2] + list[index+3]);
		}
		if (arg1Type==KEYWORD_VAR && list[index+3]!="varName") {
			errors.push_back("Error 013: wrong expression: " + arg1 + list[index+2] + list[index+3]);
		}
		if (arg1Type==KEYWORD_CONST && list[index+3]!="value") {
			errors.push_back("Error 014: wrong expression: " + arg1 + list[index+2] + list[index+3]);
		}

		if ((arg1Type==KEYWORD_STMT || arg1Type==KEYWORD_ASSIGN ||
			arg1Type==KEYWORD_WHILE || arg1Type==KEYWORD_IF || arg1Type == KEYWORD_CALL)
			&& list[index+3]!="stmt#") {
			errors.push_back("Error 015: wrong expression: " + arg1 + list[index+2] + list[index+3]);
		}
		if (arg1Type==KEYWORD_CALL && list[index+3]!="procName") {
			errors.push_back("Error 017: wrong expression: " + arg1 + list[index+2] + list[index+3]);
		}
		TNode * arg1Node = new TNode(QuerySymbol, arg1);
		withCls -> addChild(arg1Node);
		if (arg1Type==KEYWORD_PROG_LINE) {
			index +=2;
		} else {
			index +=4;
		}
	}

	if (list[index]!="=") {
		errors.push_back("Error 018: expect equality size (=) but find " + list[index] +" instead");
		return withCls;
	}
	index++;

	string arg2 = list[index];

	if (table.isSymbol(arg2)) {
		string arg2Type = table.getType(arg2);
		if (arg1Type==KEYWORD_PROCEDURE || arg1Type==KEYWORD_VAR || arg1Type==KEYWORD_CALL) { 
			if (arg2Type!=KEYWORD_PROCEDURE && arg2Type==KEYWORD_VAR && arg2Type!=KEYWORD_CALL) {
				errors.push_back("Error 019: not correct comparison (different attribute type): " + arg1 + " and " + arg2);
			} else {
				TNode * arg2Node = new TNode(QuerySymbol, arg2);
				withCls -> addChild(arg2Node);
			}
		} else {
			if (arg2Type!=KEYWORD_CONST && arg2Type!=KEYWORD_PROG_LINE &&
			arg2Type!=KEYWORD_STMT && arg2Type!=KEYWORD_ASSIGN && arg2Type!=KEYWORD_WHILE &&
			arg2Type!=KEYWORD_IF && arg2Type==KEYWORD_CALL) {
				errors.push_back("Error 020: not correct comparison (different attribute type): " + arg1 + " and " + arg2);
			} else {
				TNode * arg2Node = new TNode(QuerySymbol, arg2);
				withCls -> addChild(arg2Node);
			}
		}
	} else {
		if (arg1Type==KEYWORD_PROCEDURE || arg1Type==KEYWORD_VAR || arg1Type==KEYWORD_CALL) {
			// expect semicolon
			if (list[index]=="\"" && list[index+2]=="\"") {
				TNode * arg2Node = new TNode(Const, list[index+1]);
				withCls -> addChild(arg2Node);
			} else {
				errors.push_back("Error 021: not correct comparison (different attribute type): " + arg1 + " and " + arg2);
			}
		} else if (arg1Type==KEYWORD_CONST || arg1Type==KEYWORD_PROG_LINE ||
			arg1Type==KEYWORD_STMT || arg1Type==KEYWORD_ASSIGN || arg1Type==KEYWORD_WHILE ||
			arg1Type==KEYWORD_IF || arg1Type==KEYWORD_CALL) {
			if (isNumber(arg2)) {
				TNode * arg2Node = new TNode(Const, arg2);
				withCls -> addChild(arg2Node);
			} else {
				errors.push_back("Error 022: not correct comparison (different attribute type): " + arg1 + " and " + arg2);
			}
		} else {
			errors.push_back("Error 023: not correct comparison (different attribute type): " + arg1 + " and " + arg2);
		}
	}

	return withCls;
}

/* SUPPORTING FUCNTIONS */

// Description: this method is to retrieve a sublist of input list
vector<string> QueryPreprocessor::subList(vector<string> list, int i, int j) {
	vector<string> subList;
	for (int index=i; index<=j; index++) {
		subList.push_back(list[index]);
	}
	return subList;
}

// Description: this method is to break a string into list of words in that string
// Furthermore, it separates elements (variable and expression, container and data inside, etc. )
vector<string> QueryPreprocessor::breakStringIntoWords(string str) {
	vector<string> list;
	string word;

	//separate elements
	for (size_t i=0; i<str.length(); i++) {
		char curChar = str.at(i);
		if (curChar=='(') {
			str = str.replace(i, 1, " ( ");
			i+=2;
		}
		if (curChar=='+') {
			str = str.replace(i, 1, " + ");
			i+=2;
		}
		if (curChar==')') {
			str = str.replace(i, 1, " ) ");
			i+=2;
		}
		if (curChar==';') {
			str = str.replace(i, 1, " ; ");
			i+=2;
		}
		if (curChar==',') {
				str = str.replace(i, 1, " , ");
				i+=2;
		}
		if (curChar=='_') {
			if ((int) i>3 && str.substr(i-4, 9) == "prog_line") {
			} else { 
				str = str.replace(i, 1, " _ ");
				i+=2;
			}
		}
		if (curChar==34) {
			str = str.replace(i, 1, " \" ");
			i+=2;
		}
		if (curChar=='.') {
			str = str.replace(i, 1, " . ");
			i+=2;
		}
		if (curChar=='<') {
			str = str.replace(i, 1, " < ");
			i+=2;
		}
		if (curChar=='>') {
			str = str.replace(i, 1, " > ");
			i+=2;
		}
		if (curChar=='=') {
			str = str.replace(i, 1, " = ");
			i+=2;
		}
	}

	istringstream ss;
	ss.str(str);
	while (ss >> word) {
		list.push_back(word);
	}
	return list;
}

// Description: this method is to find first index of an element in a list of string
unsigned int QueryPreprocessor::findFirstElement(vector<string> list, unsigned index, string element) {
	for (size_t i=index+1; i<list.size(); i++) {
		if (list[i]==element) 
			return i;
	}
	return -1;
}

unsigned int QueryPreprocessor::findLastElement(vector<string> list, unsigned index, string element) {
	for (size_t i=index; i>=0; i--) {
		if (list[i]==element) 
			return i;
	}
	return -1;
}


// Description: this method is to check if a string is a number or not
bool QueryPreprocessor::isNumber(string str) {
	string::const_iterator it = str.begin();
    while (it != str.end() && isdigit(*it)) ++it;
    return !str.empty() && it == str.end();
}

/* TESTING METHODS */

// Description: this method is to print fileData for testing 
void QueryPreprocessor::printFileData() {
	for (size_t i=0; i<fileData.size(); i++) {
		cout << "	Query " << i <<endl;
		vector<string> query = fileData[i];
		if (query.size()==2) {
			cout << query[0] << endl;
			cout << query[1] << endl;
		}
	}
}

int QueryPreprocessor::getLastIndexOfTokenNotInsideBracket(vector<string> list, string token) {
	vector<int> depth;
	int nestedLv = 0;
	
	for (size_t i=0; i<list.size(); i++) {
		if (list[i]==KEYWORD_OPENBRACKET) nestedLv++;
		if (list[i]==KEYWORD_CLOSEBRACKET) nestedLv--;
		depth.push_back(nestedLv);
	}

	for (size_t i=list.size()-1; i>0; i--) {
		if (list[i]==token && depth[i]==0) {
			return i;
		}
	}

	return -1;
}