/**
    Utility.h的实现，包括NFA, DFA, Proxy三个类函数的实现
*/

#include"analyse.h"
#include<vector>
#include<iterator>
#include<iostream>
#include<queue>
#include<string.h>
#include<map>


NFA::NFA(char c, int id1, int id2)
{

    startNode = new NFANode(id1, NORMAL);
    endNode = new NFANode(id2, NORMAL);
    NFAEdge edge(startNode, endNode, c);
    startNode -> outEdges.push_back(edge);
}



NFA::NFA(char c)
{
    startNode = new NFANode();
    endNode = new NFANode();
    NFAEdge edge(startNode, endNode, c);
    startNode -> outEdges.push_back(edge);
}



void NFA::Or(const NFA &nfa)
{
    NFANode * newStartNode = new NFANode();
    NFANode * newEndNode = new NFANode();
    NFAEdge edge11(newStartNode, nfa.startNode, EPSILION);
    NFAEdge edge12(newStartNode, this -> startNode, EPSILION);
    NFAEdge edge21(nfa.endNode, newEndNode, EPSILION);
    NFAEdge edge22(this -> endNode, newEndNode, EPSILION);
    newStartNode -> outEdges.push_back(edge11);
    newStartNode -> outEdges.push_back(edge12);
    newEndNode -> inEdges.push_back(edge21);
    newEndNode -> inEdges.push_back(edge22);
    this -> startNode = newStartNode;
    this -> endNode = newEndNode;
}



void NFA::And(const NFA &nfa)
{
    NFAEdge newEdge(this -> endNode, nfa.startNode, EPSILION);
    this -> endNode->outEdges.push_back(newEdge);
    nfa.startNode->inEdges.push_back(newEdge);
    this -> endNode = nfa.endNode;
}

void NFA::Star()
{
    NFANode * newStartNode = new NFANode();
    NFANode * newEndNode = new NFANode();
    NFAEdge edge11(newStartNode, this -> startNode, EPSILION);
    NFAEdge edge12(newStartNode, newEndNode, EPSILION);
    NFAEdge circleEdge(this -> endNode, this -> startNode, EPSILION);
    NFAEdge endEdge(this -> endNode, newEndNode, EPSILION);
    newStartNode -> outEdges.push_back(edge11);
    newStartNode -> outEdges.push_back(edge12);
    this -> startNode -> inEdges.push_back(edge11);
    this -> startNode -> inEdges.push_back(circleEdge);
    this -> endNode -> outEdges.push_back(circleEdge);
    this -> endNode -> outEdges.push_back(endEdge);
    newEndNode -> inEdges.push_back(endEdge);
    newEndNode -> inEdges.push_back(edge12);
    this -> startNode = newStartNode;
    this -> endNode = newEndNode;
}

//判断当前NFA图是否为空
bool NFA::isEmpty() const
{
    return this -> startNode == NULL;
}

bool NFA::isEnd(int id) const
{
    return this -> endNode -> id == id;
}
int NFA::getEndId() const
{
    return this -> endNode -> id;
}
int NFA::getStartId() const
{
    return this -> startNode -> id;
}

void NFA::operator=(const NFA &nfa)
{
    this -> startNode = nfa.startNode;
    this -> endNode = nfa.endNode;
}


bool DFANode::contains(int id)
{
    if(this -> nodes.find(id) != nodes.end())return true;
    else return false;
}


void DFANode::insert(int id)
{
    nodes.insert(id);
}


void DFANode::insert(DFAEdge edge)
{
    edges.push_back(edge);
}


void DFANode::unionNode(DFANode * node)
{
    nodes.insert(node -> nodes.begin(), node -> nodes.end());
    edges.insert(edges.end(), node -> edges.begin(), node -> edges.end());
}

DFANode * DFANode::process(char c)
{
    for(DFAEdge edge : edges)
    {
        if(edge.word == c)
        {
            return edge.next;
        }
    }
    return NULL;
}

string DFANode::toString()
{
    string str = "{";
    set<int>::iterator it = nodes.begin();
    str += std::to_string(*it);
    it++;
    while(it != nodes.end())
    {
        str += ", ";
        str += std::to_string(*it);
        it++;
    }
    str += "}";
    return str;
}


vector<DFANode*> DFA::getNodes(int id)
{
    vector<DFANode*> v;
    for(DFANode * node : graph)
    {
        if(node->contains(id))
            v.push_back(node);
    }
    return v;
}


DFANode* DFA::crateNewNode(int id)
{
    DFANode * node = new DFANode();
    node -> nodes.insert(id);
    graph.push_back(node);
    return node;
}

//判断两个结点的转化是否等价
bool DFA::equals(DFANode *node1, DFANode *node2, set<char> words)
{
    if(node1 == node2)return true;
    for(char c : words)
    {
        if(node1 -> process(c) != node2->process(c))
            return false;
    }
    return true;
}

//判断该结点是否为结束结点
bool DFA::isEndNode(DFANode *node) const
{
    for(DFANode * n : this -> minEndNodes)
    {
        if(node == n)
            return true;
    }
    return false;
}


void DFA::delNode(DFANode *node)
{
    vector<DFANode *>::iterator it1;
    vector<DFAEdge>::iterator it2;

    it1 = this -> graph.begin();
    while(it1 != this -> graph.end())
    {
        if(node == *it1)
            it1 = this -> graph.erase(it1);
        else
            it1++;
    }

    it1 = this -> graph.begin();
    for(it1 = this -> graph.begin(); it1 != this -> graph.end(); it1++)
        for(it2 = (*it1)->edges.begin();it2 != (*it1)->edges.end();)
        {

            if((*it2).next == node)
                it2 = (*it1) -> edges.erase(it2);
            else
                it2++;
        }
}


void DFA::getCode(DFANode *n, vector<string> &lines, int tabNumber)
{
    int edgeNumber = n -> edges.size();
    if(edgeNumber > 0)
    {
        //指向下一个结点的边
        vector<DFAEdge> nextEdges;
        //指向自己的边
        vector<DFAEdge> circleEdges;

        bool flag = true;
        //先写入获取字符的代码
        lines.push_back(getTabs(tabNumber) + "char ch = getChar();");
        for(DFAEdge e : n -> edges)
        {
            if(e.next == n)//指向自己
                circleEdges.push_back(e);
            else//指向别的结点
                nextEdges.push_back(e);
        }

        if(circleEdges.size() > 0)
        {
            string value = " ch == ";
            // ch == a
            value += circleEdges[0].word;
            for(size_t i = 1;i < circleEdges.size(); i++)
            {

                value += " || ch == ";
                value += circleEdges[i].word;
            }

            string line[5];
            line[0] = getTabs(tabNumber) +"while(" + value + " )";
            line[1] = getTabs(tabNumber) + "{";
            line[2] = getTabs(tabNumber + 1) + "input(ch);";
            line[3] = getTabs(tabNumber + 1) + "ch = getChar();";
            line[4] = getTabs(tabNumber) + "}";
            for(string str : line)
            {
                lines.push_back(str);
            }
        }
        //处理指向下一个结点的边
        if(nextEdges.size() > 0)
        {

            for(DFAEdge e : nextEdges)
            {
                string line[4];

                line[0] = getTabs(tabNumber) + (flag == false ? "else " : "")
                        + "if(ch == " + e.word + ")";
                line[1] = getTabs(tabNumber) + "{";
                line[2] = getTabs(tabNumber + 1) + "input(ch);";
                line[3] = getTabs(tabNumber) + "}";

                for(int i = 0; i< 3; i++)
                {
                    lines.push_back(line[i]);
                }
                //递归
                getCode(e.next, lines, tabNumber + 1);
                lines.push_back(line[3]);

                flag = false;
            }

            lines.push_back(getTabs(tabNumber) + "else {cout << \"error\"; exit(1);}");
        }
    }

    if(isEndNode(n))
        lines.push_back(getTabs(tabNumber) + "Done();");
}

string DFA::getTabs(int tabNumber)
{
    string str = "";
    for(int i = 0; i < tabNumber; i++)
    {
        str += "    ";
    }
    return str;
}



//构造函数，用正则表达式来初始化Change代理类
Change::Change(const string regExp)
{
    memset(this -> chart, DEFAULT_WORD, sizeof(this -> chart));
    //设置正则表达式
    this -> regularExpression = regExp;
    //产生NFA
    this -> nfa = getNFA(this -> regularExpression);
    //给NFA编号并初始化DFA
    this -> serializeNFA();
}

vector<int> Change::getOrOperatorIndex(const string regExp)
{
    vector<int> index;
    int len = regExp.length();
    for(int i = 0; i < len; i++)
    {
        if(regExp[i] == '|')
            index.push_back(i);
        else if(regExp[i] == '(')
        {
            //忽略掉()内的'|'
            int temp = getRightBracketIndex(regExp, i);
            if(temp == -1)//说明不存在与之对应的')'
            {
                std::cout << "illeagal regular expression!";
                exit(1);
            }
            i = temp;
        }
    }
    return index;
}

//判断ch是不是字符（字母或数字）
bool Change::isLetter(char ch)
{
    return isdigit(ch) || isalpha(ch);
}


int Change::getRightBracketIndex(const string regExp, const int leftIndex)
{

    int newLeftBrackets = 0;
    int len = regExp.length();
    for(int i = leftIndex + 1; i < len; i++)
    {
        switch (regExp[i]){
        case ')':

            if(newLeftBrackets == 0)
                return i;
            else newLeftBrackets--;
            break;
        case '(':
            //遇到新的未匹配'('，个数 + 1
            newLeftBrackets++;
            break;
        default:
            break;
        }
    }
    return -1;
}


NFA Change::getNFA(const string regExp)
{
    //先获取'|'的索引
    vector<int> orIndex = getOrOperatorIndex(regExp);
    int len = regExp.length();
    //先按照'|'拆分正则表达式，然后以拆分好每份表达式为单位
    if(orIndex.size() != 0)
    {
        //用于存放每个单位生成的NFA，最后把每个单位用类似于or的操作连起来就行了
        vector<NFA> NFAs;
        //正则表达式字串
        string subReg;
        int curIndex = 0;
        for(size_t i = 0; i < orIndex.size(); i++)
        {
            subReg = regExp.substr(curIndex, orIndex[i] - curIndex);
            curIndex = orIndex[i] + 1;
            NFAs.push_back(getNFA(subReg));
        }
        subReg = regExp.substr(curIndex, regExp.size() - curIndex);
        NFAs.push_back(getNFA(subReg));


        NFANode * node1 = new NFANode();
        NFANode * node2 = new NFANode();
        for(NFA n : NFAs)
        {
            //对于每个叉，新增两条边
            //新节点到NFA起点的边
            NFAEdge edge1(node1, n.startNode, EPSILION);
            //NFA结束点到新结点的边
            NFAEdge edge2(n.endNode, node2, EPSILION);
            //把前后结点和边建立联系
            node1 -> outEdges.push_back(edge1);
            n.startNode -> inEdges.push_back(edge1);
            node2 -> inEdges.push_back(edge2);
            n.endNode -> outEdges.push_back(edge2);
        }

        return NFA(node1, node2);
    }


    NFA nfa;
    //记录当前字母和下一个字母
    char cur, next;

    for(int i = 0; i< len - 1; i++)
    {
        cur = regExp[i];
        next = regExp[i + 1];
        if(isLetter(cur))
        {

            NFA n(cur);

            if(next == '*')
            {
                n.Star();

                i++;
            }

            if(nfa.isEmpty())
            {
                nfa = n;
            }
            else
            {
                nfa.And(n);
            }
        }
        //遇见()需要递归
        else if(cur == '(')
        {
            int rightBacketIndex = getRightBracketIndex(regExp, i);
            //没有对应匹配的')' 程序终止
            if(rightBacketIndex == -1)
            {
                exit(1);
            }
            //括号内的内容
            string subReg = regExp.substr(i + 1, rightBacketIndex - 1 - i);
            next = regExp[rightBacketIndex + 1];
            if(next == '*')//需要先闭包后连接
            {
                if(nfa.isEmpty())
                {
                    nfa = getNFA(subReg);
                    nfa.Star();
                    i = rightBacketIndex + 1;
                }
                else
                {
                    NFA subNfa = getNFA(subReg);
                    subNfa.Star();
                    nfa.And(subNfa);
                    i = rightBacketIndex + 1;
                }
            }
            else//直接连接操作
            {
                if(nfa.isEmpty())
                {
                    nfa = getNFA(subReg);
                }
                else
                {
                    nfa.And(getNFA(subReg));
                }
                i = rightBacketIndex;
            }
        }
    }
    //处理最后一个字符
    if(isLetter(regExp[len -1]))
    {
        if(nfa.isEmpty())
        {
            nfa = NFA(regExp[len - 1]);
        }
        else
        {
            nfa.And(NFA(regExp[len - 1]));
        }
    }
    return nfa;
}



//给NFA的结点编号并建立初始DFA结点
void Change::serializeNFA()
{
    //空NFA图，直接结束
    if(nfa.isEmpty())
        return;
    //先设置好NFA图的开始结点和结束结点
    nfa.startNode -> state = START;
    nfa.endNode -> state = END;
    NFANode * beginNode = nfa.startNode;
    int id = 1;
    beginNode -> id = id++;
    NFANode *nfaNode;
    DFANode *dfaNode;

    queue<NFANode*> que1,que2;
    que1.push(beginNode);
    //设置bool数组，以防止反复地以同一结点建立EPSILION闭包
    bool visited[MAX_NODE_NUMBER];
    memset(visited, false, sizeof(visited));
    while(!que1.empty())
    {
        //取出队首元素
        nfaNode = que1.front();
        que1.pop();

        dfaNode = dfa.crateNewNode(nfaNode -> id);

        visited[nfaNode -> id] = true;

        bool visited1[MAX_NODE_NUMBER];
        memset(visited1, false, sizeof(visited1));
        que2.push(nfaNode);
        while(!que2.empty())
        {
            nfaNode = que2.front();
            que2.pop();
            visited1[nfaNode -> id] = true;
            for(NFAEdge edge : nfaNode -> outEdges)
            {
                if(edge.endNode -> id == DEFAULT_ID)
                {
                    edge.endNode -> id = id++;
                }
                chart[edge.startNode -> id][edge.endNode -> id] = edge.word;
                if(edge.word == EPSILION)
                {

                    dfaNode -> insert(edge.endNode -> id);
                    if(!visited1[edge.endNode -> id])
                    {
                        que2.push(edge.endNode);
                    }
                }
                else
                {
                    wordList.insert(edge.word);
                    if(!visited[edge.endNode -> id])
                    {
                        que1.push(edge.endNode);
                        //这样可以防止队列中出现重复元素
                        visited[edge.endNode -> id] = true;
                    }
                }
            }
        }
        if(dfaNode -> contains(nfa.getEndId()))
        {
            dfaNode -> state = END;
        }
    }
    nfa.nodeNumber = id - 1;
    processDFA();
}


void Change::processDFA()
{

    for(size_t i = 0; i < dfa.graph.size(); i++)
    {
        //取出第i个DFANode
        DFANode * dfaNode = dfa.graph[i];
        map<char, vector<DFANode*>> myMap;
        //遍历当前EPSILION转换集中的所有结点
        for(int id : dfaNode ->nodes)
        {

            vector<int> v = this -> getConnections(id);
            for(int j : v)
            {

                DFANode * node = dfa.getNodes(j)[0];

                char word = this -> chart[id][j];
                DFAEdge edge(node, word);

                dfaNode -> insert(edge);
                myMap[word].push_back(node);
            }
        }



        map<char, vector<DFANode*>> :: iterator it;
        for(it = myMap.begin(); it != myMap.end(); it++)
        {
            vector<DFANode*> nodes = it -> second;

            if(nodes.size() >= 2)
            {

                for(size_t i = 1; i < nodes.size(); i++)
                {
                    nodes[0] -> unionNode(nodes[i]);
                    //从dfa图中删除被合并的node
                    dfa.delNode(nodes[i]);
                }
            }
        }
    }
    DFANode * n;
    for(size_t i = 0; i < dfa.graph.size(); i++)
    {
        n = dfa.graph[i];
    }

    dfa.wordList.insert(this -> wordList.begin(), this -> wordList.end());


}

//最小化DFA
void Change::minimizeDFA()
{
    //s1表示非终态集合，s2表示终态集合
    vector<DFANode *> s1, s2;
    for(DFANode * n : dfa.graph)
    {
        if(n -> state == END)
        {
            s2.push_back(n);
        }
        else
            s1.push_back(n);
    }

    queue<vector<DFANode *>> states;
    states.push(s1);
    states.push(s2);
    while(!states.empty())
    {
        vector<DFANode *> s = states.front();
        states.pop();
        if(s.size() == 0)continue;

        if(s.size() == 1)
        {
            this -> minDFA.push_back(s);
            continue;
        }

        vector<vector<DFANode *>> subStates;

        DFANode * node = s[0];

        bool flag = true;
        for(size_t i = 1; i < s.size(); i++)
        {
            //如果两个结点的转换结果不一样，就需要拆分
            if(DFA::equals(node, s[i], this -> wordList) == false)
            {

                vector<DFANode *> curStates;
                //找出s中和s[i]转换结果相同的元素
                for(vector<DFANode *>:: iterator it = s.begin() + 1; it != s.end();)
                {

                    if(DFA::equals(s[i], *it, this -> wordList))
                    {

                        curStates.push_back(*it);

                        it = s.erase(it);
                    }
                    else//不相同
                        it++;
                }

                flag = false;
                subStates.push_back(curStates);
            }

        }

        if(flag)
        {
            minDFA.push_back(s);
        }

        else
        {

            states.push(s);
            for(vector<DFANode *> s : subStates)
            {
                states.push(s);
            }
        }
    }

    processMinDFA();
}


inline bool belongs(vector<DFANode *> nodes, DFANode * node)
{
    for(DFANode * n :nodes)
    {
        if(n == node)
            return true;
    }
    return false;
}


inline bool isDifferentEdge(vector<DFAEdge> edges, DFAEdge e)
{
    for(DFAEdge edge : edges)
    {
        if(edge.word == e.word && edge.next ==e.next)
        {
            return false;
        }
    }
    return true;
}



vector<int> Change::getConnections(int id)
{
    vector<int> con;
    for(int i = 1; i <= nfa.nodeNumber; i++)
    {
        if(chart[id][i] != DEFAULT_WORD && chart[id][i] != EPSILION)
        {
            con.push_back(i);
        }
    }
    return con;
}


void Change::processMinDFA()
{

    int stateNumber = minDFA.size();
    //把处在同一集合的DFANode合成一个新的DFANode
    for(int i = 0; i < stateNumber; i++)
    {

        vector<DFANode *> curState = minDFA[i];

        vector<DFAEdge> curNewEdges;

        for(DFANode * node : curState)
        {

            for(DFAEdge edge : node ->edges)
            {

                for(int j = 0; j < stateNumber; j++)
                {
                    if(belongs(minDFA[j], edge.next))
                    {
                        //新建一条边

                        DFAEdge newEdge(minDFA[j][0], edge.word);
                        if(isDifferentEdge(curNewEdges, newEdge))
                                curNewEdges.push_back(newEdge);
                    }
                }
            }
        }

        //开始合并元素
        for(size_t j = 1; j < curState.size(); j++)
        {
            curState[0]->unionNode(curState[j]);
            if(curState[j]->state == END)
                curState[0]->state = END;
        }

        curState[0]->edges.clear();
        curState[0]->edges.assign(curNewEdges.begin(), curNewEdges.end());

        finalDFA.graph.push_back(curState[0]);
    }


    char name = 'A';
    string s = "";
    for(int i = 0; i < stateNumber; i++)
    {
        this -> finalDFA.graph[i]->minName = s + name++;

        if(this -> finalDFA.graph[i]->contains(1))
        {
            this -> finalDFA.minStartNode = this -> finalDFA.graph[i];
        }

        if(this -> finalDFA.graph[i] -> state == State::END)
        {
            this -> finalDFA.minEndNodes.push_back(this -> finalDFA.graph[i]);
        }
    }
}



void Change::generateCode()
{
    this -> code = "";
    vector<string> lines;
    DFANode * node = this -> finalDFA.minStartNode;
    finalDFA.getCode(node, lines, 0);
    for(string line : lines)
    {
        code += line;
        code += "\n";
    }
}












