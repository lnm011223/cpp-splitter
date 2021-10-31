# 编译原理 XLEX词法生成器文档



## 需求：

> 一、实验内容：
> 设计一个应用软件，以实现将正则表达式-->NFA--->DFA-->DFA最小化-->词法分析程序
>
> 二、必做实验要求：
>  （1）正则表达式应该支持单个字符，运算符号有： 连接  选择 闭包  括号
>  （2）要提供一个源程序编辑界面，让用户输入正则表达式（可保存、打开源程序）
>  （3）需要提供窗口以便用户可以查看转换得到的NFA（用状态转换表呈现即可）
>  （4）需要提供窗口以便用户可以查看转换得到的DFA（用状态转换表呈现即可）
>  （5）需要提供窗口以便用户可以查看转换得到的最小化DFA（用状态转换表呈现即可）
>  （6）需要提供窗口以便用户可以查看转换得到的词法分析程序（该分析程序需要用C语言描述）
>  （7）应该书写完善的软件文档

## 编译环境：macOS,Qt 5.12.11

## 作者：

> 20192131031 梁诺明

## 流程说明

![image-20211031152847254](https://gitee.com/lnm011223/lnm011223-picture/raw/master/uPic/image-20211031152847254.png)

## 关键算法说明

### 闭包

需要新建 startNode 和 endNode，和 nfa 前后连起来，然后把 nfa 的 startNode 和 endNode 连起来。

![image-20211031153446873](https://gitee.com/lnm011223/lnm011223-picture/raw/master/uPic/image-20211031153446873.png)

### 生成NFA

扫描正则表达式，首先扫描 | 进行拆分递归，逐项建立 NFA 后，用 ‘|’ 连接，对于括号要进行进行递归处理

### 生成DFA

1. 给 NFA 的结点编号并建立初始 DFA 结点
2. 确定结点与结点之间的关系，建立 DFA 的边

### 最小化DFA

1. 求初态集合、终态集合并对这些集合进行划分，建立最后的 DFANode
2. 建立 DFANode 之间的边，形成最小 DFA 图

### C语言生成

对于指向自己的边，生成 while 语句；对于指向下一个结点的边，生成 if 语句，并且需要递归进入下一个结点，直到当前结点的状态为 END 时，结束递归，并回调。

## 关键函数，类以及结构体

### 枚举类用于标记节点状态

```c++
enum State{
    START,END,NORMAL
};
```

### NFA

NFAEdge 用于描述一条 NFA 图中的边，包括开始结点 (startNode)，结束结点 (endNode)，处理的字符 (word)

```c++
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


```

### DFA

DFAEdge用于描述一条 DFA 图的边，其属性包括指向的结点（next）和处理的字符（word）



```c++
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



```

### Change

用于生成DFA，NFA，最小化DFA

```c++
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

```



## 运行结果展示

![image-20211031150213201](https://gitee.com/lnm011223/lnm011223-picture/raw/master/uPic/image-20211031150213201.png)

![image-20211031150228539](https://gitee.com/lnm011223/lnm011223-picture/raw/master/uPic/image-20211031150228539.png)

![image-20211031150240043](https://gitee.com/lnm011223/lnm011223-picture/raw/master/uPic/image-20211031150240043.png)

![image-20211031150251343](https://gitee.com/lnm011223/lnm011223-picture/raw/master/uPic/image-20211031150251343.png)