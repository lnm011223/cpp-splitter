# TINY扩充语言的语法分析

> 一、实验内容： 
>
> - 扩充的语法规则有：实现 do while循环，for循环，扩充算术表达式的运算符号：-= 减法赋值运算符号（类似于C语言的-=）、求余%、乘方^，
> - 扩充比较运算符号：==（等于），>(大于)、<=(小于等于)、>=(大于等于)、<>(不等于)等运算符号，
> - 新增支持正则表达式以及用于repeat循环、do while循环、if条件语句作条件判断的逻辑表达式：运算符号有 and（与）、 or（或）、 not（非） 。
> - 具体文法规则自行构造。
>
>  
>
> 可参考：云盘中参考书P97及P136的文法规则。
>
>  
>
> - (1) Dowhile-stmt-->do stmt-sequence while(exp); 
> - (2) for-stmt-->for identifier:=simple-exp to simple-exp do stmt-sequence enddo  步长递增1
> - (3) for-stmt-->for identifier:=simple-exp downto simple-exp do stmt-sequence enddo  步长递减1
> - (4) -= 减法赋值运算符号、求余%、乘方^、>=(大于等于)、<=(小于等于)、>(大于)、<>(不等于)运算符号的文法规则请自行组织。
> - (5)把tiny原来的赋值运算符号(:=)改为(=),而等于的比较符号符号（=）则改为（==）
> - (6)为tiny语言增加一种新的表达式——正则表达式，其支持的运算符号有 或(|) 、连接(&)、闭包(#)、括号( ) 以及基本正则表达式 。
> - (7)为tiny语言增加一种新的语句，ID:=正则表达式 
> - (8)为tiny语言增加一种新的表达式——逻辑表达式，其支持的运算符号有 and(与) 、or (或)、非(not)。
> - (9)为了实现以上的扩充或改写功能，还需要对原tiny语言的文法规则做好相应的改造处理。 
>
> 
>
> 二、要求：
> （1）要提供一个源程序编辑界面，以让用户输入源程序（可保存、打开源程序）
> （2）可由用户选择是否生成语法树，并可查看所生成的语法树。
> （3）应该书写完善的软件文档
> （4）要求应用程序应为Windows界面。

## 测试结果展示

> - 测试文件1
>
>   ![](https://gitee.com/lnm011223/lnm011223-picture/raw/master/uPic/image-20211226193312167.png)
>
> 
>
> - 测试文件2
>
>   ![image-20211226193542072](https://gitee.com/lnm011223/lnm011223-picture/raw/master/uPic/image-20211226193542072.png)

## 软件操作流程说明

用户可以通过在左侧输入TINY源码，或者通过点击`导入文件`把需要分析的代码导入项目，用户可以点击`保存源码`来保存左侧文本框的内容。用户点击`生成语法树`后可以在右侧文本框中查看到分析后的语法树。最后，通过点击`保存语法树`可以保存语法树结果。

## 文法构造

![image-20211226194013537](https://gitee.com/lnm011223/lnm011223-picture/raw/master/uPic/image-20211226194013537.png)

![image-20211226194026341](https://gitee.com/lnm011223/lnm011223-picture/raw/master/uPic/image-20211226194026341.png)

## 部分算法和结构展示

### 1.定义保留字和语句类型

```c
/* MAXRESERVED = the number of reserved words */
#define MAXRESERVED 18 //新增7个保留字,4个特殊字符

/* book-keeping tokens */
typedef enum
{
    ENDFILE,ERROR,
    /* reserved words */
    IF,THEN,ELSE,END,REPEAT,UNTIL,READ,WRITE,
    /* multicharacter tokens */
    ID,NUM,
    /* special symbols */
    ASSIGN,EQ,LT,PLUS,MINUS,TIMES,OVER,LPAREN,RPAREN,SEMI,
    /* 扩充的保留字 */
    WHILE, DO, FOR, ENDWHILE, TO, ENDDO, DOWNTO,
    /* 扩充的特殊符号 */
    MinusEQUAL, MOD, POWER, LESSEQUAL, NEQ, MT, MOREEQUAL, AND, OR, NOT,
    REAND, REOR, RENOT, RECLO,REASSIGN, LETTER

} TokenType;
```

```c
static struct
{
    const char* str;
    TokenType tok;
} reservedWords[MAXRESERVED] =
    {
        {"if", IF}, {"then", THEN}, {"else", ELSE}, {"end", END},
        {"repeat", REPEAT}, {"until", UNTIL}, {"read", READ}, {"write", WRITE},

        /* 新增的保留字 */
        {"while", WHILE}, {"do", DO}, {"for", FOR}, {"endwhile", ENDWHILE},
        {"to", TO}, {"enddo", ENDDO}, {"downto", DOWNTO},

         /* 新增的特殊字符 */
        {"and", AND}, {"or", OR}, {"not", NOT},
    };
```

### 2.do-while

新增一个语句结点，匹配保留字DO后，左孩子指向stmt_sequence生成的子树，匹配保留字WHILE和左括号后，将该节点右孩子指向logic_exp生成的子树，最后匹配右括号

```c
/* 扩充的dowhile文法 */
TreeNode * dowhile_stmt(void)
{
    TreeNode * t = newStmtNode(DowhileK);
    match(DO);
    if(t != NULL)
        t->child[0] = stmt_sequence();
    match(WHILE);
    match(LPAREN);
    if(t != NULL)
        t->child[1] = logic_exp();
    match(RPAREN);
    return t;
}
```

### 3.for

新增一个语句结点，匹配保留字FOR后，第一个孩子指向assign_stmt生成的子树，接着判断是TO还是DOWNTO，匹配对应保留字后，将该节点第二个孩子指向simple_exp生成的子树，匹配保留字DO，然后第三个孩子指向stmt_sequence生成的子树，最后匹配ENDDO。

```c
/* 扩充for文法 */
TreeNode * for_stmt(void)
{
    TreeNode * t = newStmtNode(ForK);
    match(FOR);
    if(t != NULL)
        t->child[0] = assign_stmt();
    if(token == TO)
    {
        match(TO);
        if(t != NULL)
            t->child[1] = simple_exp();
    }
    else
    {
        match(DOWNTO);
        if(t != NULL)
            t->child[1] = simple_exp();
    }
    match(DO);
    if(t != NULL)
        t->child[2] = stmt_sequence();
    match(ENDDO);
    return t;
}
```

### 4.赋值运算

在 scan.cpp 中 TokenType getToken(void)函数中，增加扫描到‘-’时的处理：如果当前扫描到的是‘-’，使用 getNextChar()函数获取下一字符 c，如果 c 为‘=’， 此时“-=”构成减法赋值运算符；否则使用 getNextChar()函数回退一个字符， 此时为‘-’运算符。 在 parse.cpp 文件 TreeNode * assign_stmt(void) 函数中， 增加处理当前 token 为MinusEQUAL（即-=）的情况：match(MinusEQUAL)后，先生成一个运算符为减号的树 p，树的左孩子为待赋值的变量，右孩子为算术表达式。然后将树 p 作为assign 树 t 的孩子。

```c
TreeNode * assign_stmt(void)
{
    TreeNode * t = newStmtNode(AssignK);
    if ((t != NULL) && (token == ID))
        t->attr.name = copyString(tokenString);
    //生成值为token的节点，-=时用到
    TreeNode * left = newExpNode(IdK);
    left->attr.name = copyString(tokenString);
    match(ID);
    if(token == ASSIGN)  //处理=
    {
        match(ASSIGN);
        if (t != NULL)
            t->child[0] = exp();
    }
    else if(token == MinusEQUAL) //处理 -=
    {
        match(MinusEQUAL);
        //生成一个算术表达式节点
        TreeNode * p = newExpNode(OpK);
        if (p != NULL)
        {
            p->child[0] = left;
            p->attr.op = MINUS;//op为-
            p->child[1]=exp();
        }
        if(t != NULL)
            t->child[0] = p;
    }else if(token==REASSIGN){  //处理:=
        match(REASSIGN);
        if (t != NULL)
            t->child[0] = re_exp();
    }
    return t;
}
```

### 5.求%

与乘法与除法一致，生成一个新的操作符节点后，赋值为对应的符号后将左孩子指向poer_term生成的节点，将该节点赋值给t后，将右孩子设置为poer_term生成的节点。

```c
TreeNode * term(void)
{
    TreeNode * t = power_term();
    while ((token == TIMES) || (token == OVER) || (token == MOD)) // * / %
    {
        TreeNode * p = newExpNode(OpK);
        if (p != NULL)
        {
            p->child[0] = t;
            p->attr.op = token;
            t = p;
            match(token);
            p->child[1] = power_term();
        }
    }
    return t;
}
```

### 6.乘方

首先生成一个新的操作符节点p后，将p的左孩子设置为factor生成的节点t，将操作符属性设置为乘方后，将该p节点赋值给t后，将右孩子设置为factor生成的节点。

```c
/* 新增处理乘方 */
treeNode * power_term(void)
{
    TreeNode * t = factor();
    while (token == POWER) // ^
    {
        TreeNode * p = newExpNode(OpK);
        if (p != NULL)
        {
            p->child[0] = t;
            p->attr.op = token;
            t = p;
            match(token);
            p->child[1] = factor();
        }
    }
    return t;
}
```

### 7.>=(大于等于)、<=(小于等于)、>(大于)、<>(不等于)运算符号

在 scan.cpp TokenType getToken(void)函数中，增加扫描到‘<’时的处理：当前扫描到的是‘<’，使用 getNextChar()函数获取下一字符 c，如果 c 为‘=’，此时“<=”构成小于等于运算符；如果 c 为‘>’，此时“<>”构成不等于运算符； 否则使用 getNextChar()函数回退一个字符，此时为‘<’运算符。增加扫描到‘>’时的处理：当前扫描到的是‘>’，使用 getNextChar()函数获取下一字符 c，如果 c 为‘=’，此时“>=”构成大于等于运算符；否则使用 getNextChar()函数回退一个字符，此时为‘>’运算符。在 parse.c 文件中 static TreeNode * exp(void)函数中，增加处理当前 token 为 LESSEQUAL（<=）、NEQ（<>）、MT（>）、MOREEQUAL（>=）的情况。

```c
TreeNode * exp(void)
{
    TreeNode * t = simple_exp();
//     < = <= <> > >=
    if ((token == LT) || (token == EQ) || (token == LESSEQUAL)
            || (token == NEQ) ||(token == MT) || (token == MOREEQUAL))
    {
        TreeNode * p = newExpNode(OpK);
        if (p != NULL)
        {
            p->child[0] = t;
            p->attr.op = token;
            t = p;
        }
        match(token);
        if (t != NULL)
            t->child[1] = simple_exp();
    }
    return t;
}
```

### 8.把 tiny 原来的赋值运算符号(:=)改为(=),而等于的比较符号符号（=） 则改为（==） 

在 scan.cpp TokenType getToken(void)函数中，删掉扫描到‘：’时的处理，增加扫描到‘=’时的处理：当前扫描到的是‘=’，使用 getNextChar()函数获取下一字符 c，如果 c 为‘=’，此时“==”构成等于的比较符号；否则使用 getNextChar()函数回退一个字符，此时为‘=’赋值运算符。 然后在 void printToken( TokenType token, const char* tokenString)函数中，改变 ASSIGN 和 EQ 的输出：

```c
case ASSIGN: fprintf(listing, "=\n"); break;
case EQ: fprintf(listing, "==\n"); break;
```

### 9.**逻辑表达式**

在 parse.c 文件中，增加关于逻辑表达式的静态成员函数定义，用于 repeat 循环、do while 循环、if 条件语句作条件判断的逻辑表达式分别修改 TreeNode * repeat_stmt(void)、TreeNode * dowhile_stmt(void)和 TreeNode * if_stmt(void)函数，将 exp()改为 logic_exp()。

```c
//处理逻辑表达式1
TreeNode * logic_exp(void)
{
    TreeNode * t = logic_term();
    while (token == OR) //or
    {
        TreeNode * p = newExpNode(OpK);
        if (p != NULL)
        {
            p->child[0] = t;
            p->attr.op = token;
            t = p;
            match(token);
            t->child[1] = logic_term();
        }
    }
    return t;
}

//处理逻辑表达式2
TreeNode * logic_term(void)
{
    TreeNode * t = logic_factor();
    while (token == AND ) // and
    {
        TreeNode * p = newExpNode(OpK);
        if (p != NULL)
        {
            p->child[0] = t;
            p->attr.op = token;
            t = p;
            match(token);
            t->child[1] = logic_factor();
        }
    }
    return t;
}

//处理逻辑表达式3
TreeNode * logic_factor(void)
{
    TreeNode * t = NULL;
    if(token==NOT){
        t = newExpNode(OpK);
        t->attr.op = token;
        match(token);
        t->child[0] = logic_factor();
    }else{
        t=exp();
    }
    return t;
}
```

### 10.新增支持正则表达式文法规则

因为正则表达式里的字符/字符串并不是变量（ID），所以我在 globals.h 中的 ExpKind 增加了 LETTERK（即可以生成 kind.exp 为 LETTERK 节点，这样打印的时候就可以打印成 Letter 而不是 ID）。 在 scan.cpp的 TokenType getToken(void)函数中，增加扫描到‘：=’、‘&’、‘|’、‘#’时的处理。注意这里扫到字符/字符串时，还是先让它为 ID，在之后生成节点的时候将 kind.exp 变成 Letter 即可。 需要在parse.cpp 文件中增加静态成员函数定义。

```c
//处理正则表达式1
TreeNode * re_exp(void)
{
    TreeNode * t = re_termA();
    while (token == REOR) //or
    {
        TreeNode * p = newExpNode(OpK);
        if (p != NULL)
        {
            p->child[0] = t;
            p->attr.op = token;
            t = p;
            match(token);
            t->child[1] = re_termA();
        }
    }
    return t;
}

//处理正则表达式2
TreeNode * re_termA(void)
{
    TreeNode * t = re_termB();
    while (token == REAND) // and
    {
        TreeNode * p = newExpNode(OpK);
        if (p != NULL)
        {
            p->child[0] = t;
            p->attr.op = token;
            t = p;
            match(token);
            t->child[1] = re_termB();
        }
    }
    return t;
}

//处理正则表达式3
TreeNode * re_termB(void)
{
    TreeNode * t = re_factor();
    if (token == RECLO) //可选，闭包#
    {
        match(token);
        TreeNode * p = newExpNode(OpK);
        if (p != NULL)
        {
            p->child[0] = t;
            p->attr.op = RECLO;
            t=p;
        }
    }
    return t;
}

//处理正则表达式4
TreeNode * re_factor(void){
    TreeNode * t = NULL;
    if(token == LPAREN){
        match(LPAREN);
        t = re_exp();
        match(RPAREN);
    }else{
        t = newExpNode(LETTERK);
        token=LETTER;
        if ((t!=NULL) && (token==LETTER))
            t->attr.name = copyString(tokenString);
        match(LETTER);
    }
    return t;
}
```