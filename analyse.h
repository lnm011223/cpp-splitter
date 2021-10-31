#ifndef ANALYSE_H
#define ANALYSE_H
#include<vector>
#include<set>
#include<string>
using namespace std;
const int DEFAULT_ID = -1;
const char DEFAULT_WORD = ' ';
const char EPSILION = '#';
const int MAX_NODE_NUMBER = 256;


enum State{
    START,END,NORMAL
};



struct NFAEdge;


struct NFANode{
    int id;
    State state;
    vector<NFAEdge> inEdges;
    vector<NFAEdge> outEdges;
    NFANode(int i, State s) : id(i), state(s){}
    NFANode(): id(DEFAULT_ID), state(NORMAL){}
};


struct NFAEdge{
    //由startNode指向endNode
    NFANode * startNode;
    NFANode * endNode;
    char word;
    NFAEdge(NFANode * s, NFANode * e, char c): startNode(s), endNode(e), word(c){}
    NFAEdge(): startNode(NULL), endNode(NULL), word('\0'){}
};

//构建NFA图

class NFA{
public:

    NFANode * startNode;//开始结点
    NFANode * endNode;//结束结点
    int nodeNumber;

    bool isEmpty() const;//判断当前NFA图是否为空
    bool isEnd(int id) const;
    int getEndId() const;
    int getStartId() const;
    void operator=(const NFA &nfa);    
    void Or(const NFA &nfa);//  |  选择
    void And(const NFA &nfa); //  连接
    void Star();// * 闭包
    NFA():startNode(NULL), endNode(NULL), nodeNumber(2){}
    NFA(char c, int id1, int id2);
    NFA(char c);
    NFA(NFANode * s, NFANode * e): startNode(s), endNode(e){}
};




struct DFANode;


struct DFAEdge
{
    DFANode * next;   
    char word;
    DFAEdge(DFANode *n, char c): next(n), word(c){}
};

struct DFANode
{

    set<int> nodes;
    vector<DFAEdge> edges;
    string minName;
    State state = NORMAL;
    bool contains(int id);
    void insert(int id);
    void insert(DFAEdge edge);
    void unionNode(DFANode * node);
    DFANode * process(char c);
    string toString();
};

//DFA类

class DFA
{
public:
    vector<DFANode*> graph;
    vector<DFANode*> getNodes(int id);
    DFANode *minStartNode;
    vector<DFANode*> minEndNodes;
    set<char> wordList;
    DFANode* crateNewNode(int id);
    //判断两个结点的转化是否等价
    static bool equals(DFANode *node1, DFANode *node2, set<char> words);
    bool isEndNode(DFANode *node) const;
    void delNode(DFANode *node);
    void getCode(DFANode *n, vector<string> &lines, int tabNumber);
private:

    string getTabs(int tabNumber);
};



class Change
{
public:
    //正则表达式

    string regularExpression;
    void serializeNFA();
    void processDFA();
    void minimizeDFA();
    void processMinDFA();
    void generateCode();
    NFA nfa;
    DFA dfa;
    vector<vector<DFANode *>> minDFA;
    DFA finalDFA;
    char chart[MAX_NODE_NUMBER][MAX_NODE_NUMBER];
    string code;
    set<char> wordList;
    Change(const string regExp);

private:
    //获取NFA图
    NFA getNFA(const string regExp);

    vector<int> getOrOperatorIndex(const string regExp);
    bool isLetter(char ch);
    int getRightBracketIndex(const string regExp, const int leftIndex);
    vector<int> getConnections(int id);
};

#endif // ANALYSE_H
