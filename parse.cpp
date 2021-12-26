#include "util.h"
#include "scan.h"
#include "parse.h"
#include <QDebug>

static TokenType token; /* holds current token */

/* function prototypes for recursive calls */
static TreeNode * stmt_sequence(void);
static TreeNode * statement(void);
static TreeNode * if_stmt(void);
static TreeNode * repeat_stmt(void);
static TreeNode * assign_stmt(void);
static TreeNode * read_stmt(void);
static TreeNode * write_stmt(void);
static TreeNode * exp(void);
static TreeNode * simple_exp(void);
static TreeNode * term(void);
static TreeNode * factor(void);

/* 新增的树节点 */
static TreeNode * dowhile_stmt(void);
static TreeNode * for_stmt(void);
static TreeNode * power_term(void); //处理乘方
//处理逻辑表达式
static TreeNode * logic_exp(void);
static TreeNode * logic_term(void);
static TreeNode * logic_factor(void);
//正则表达式
static TreeNode * re_exp(void);
static TreeNode * re_termA(void);
static TreeNode * re_termB(void);
static TreeNode * re_factor(void);

static void syntaxError(const char * message)
{
    fprintf(listing, "\n>>> ");
    fprintf(listing, "Syntax error at line %d: %s", lineno, message);
    Error = TRUE;
}

static void match(TokenType expected)
{
    if (token == expected)
        token = getToken();
    else
    {
        syntaxError("unexpected token -> ");
        printToken(token, tokenString);
        fprintf(listing, "      ");
    }
}

TreeNode * stmt_sequence(void)
{
    TreeNode * t = statement();
    TreeNode * p = t;
    while ((token!=ENDFILE) && (token!=END) && (token!=ELSE)
           && (token!=UNTIL) && (token!=WHILE) && (token!=ENDDO))
    {
        TreeNode * q;
        match(SEMI);
        q = statement();
        if (q!=NULL)
        {
            if (t==NULL)
                t = p = q;
            else /* now p cannot be NULL either */
            {
                p->sibling = q;
                p = q;
            }
        }
    }
    return t;
}


TreeNode * statement(void)
{
    TreeNode * t = NULL;
    switch (token)
    {
    case IF : t = if_stmt(); break;
    case REPEAT : t = repeat_stmt(); break;
    case ID :t = assign_stmt(); break;
    case READ : t = read_stmt(); break;
    case WRITE : t = write_stmt(); break;

    /* 扩充的文法 */
    case DO: t = dowhile_stmt(); break;
    case FOR: t = for_stmt(); break;

    default :
        syntaxError("unexpected token -> ");
        printToken(token,tokenString);
        token = getToken();
        break;
    } /* end case */
    return t;
}


/* 改写if文法 */
TreeNode * if_stmt(void)
{
    TreeNode * t = newStmtNode(IfK);
    match(IF);
    if (t != NULL)
    {
        match(LPAREN);
        t->child[0] = logic_exp();
        match(RPAREN);
    }
    if (t != NULL)
        t->child[1] = stmt_sequence();
    if (token == ELSE)
    {
        match(ELSE);
        if (t != NULL)
            t->child[2] = stmt_sequence();
    }
    return t;
}


TreeNode * repeat_stmt(void)
{
    TreeNode * t = newStmtNode(RepeatK);
    match(REPEAT);
    if (t != NULL)
        t->child[0] = stmt_sequence();
    match(UNTIL);
    if (t != NULL)
        t->child[1] = logic_exp();
    return t;
}

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

TreeNode * read_stmt(void)
{
    TreeNode * t = newStmtNode(ReadK);
    match(READ);
    if ((t != NULL) && (token == ID))
        t->attr.name = copyString(tokenString);
    match(ID);
    return t;
}

TreeNode * write_stmt(void)
{
    TreeNode * t = newStmtNode(WriteK);
    match(WRITE);
    if (t != NULL)
        t->child[0] = exp();
    return t;
}

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

TreeNode * simple_exp(void)
{
    TreeNode * t = term();
    while ((token == PLUS) || (token == MINUS)) // + -
    {
        TreeNode * p = newExpNode(OpK);
        if (p != NULL)
        {
            p->child[0] = t;
            p->attr.op = token;
            t = p;
            match(token);
            t->child[1] = term();
        }
    }
    return t;
}

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

TreeNode * factor(void)
{
    TreeNode * t = NULL;
    switch (token)
    {
    case NUM :
        t = newExpNode(ConstK);
        if ((t != NULL) && (token == NUM))
            t->attr.val = atoi(tokenString);
        match(NUM);
        break;
    case ID :
        t = newExpNode(IdK);
        if ((t!=NULL) && (token==ID))
            t->attr.name = copyString(tokenString);
        match(ID);
        break;
    case LPAREN :
        match(LPAREN);
        t = exp();
        match(RPAREN);
        break;
    default:
        syntaxError("unexpected token -> ");
        printToken(token, tokenString);
        token = getToken();
        break;
    }
    return t;
}

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

/****************************************/
/* the primary function of the parser   */
/****************************************/
/* Function parse returns the newly
 * constructed syntax tree
 */
TreeNode * parse(void)
{
    TreeNode * t;
    token = getToken();
    t = stmt_sequence();
    if (token != ENDFILE)
        syntaxError("Code ends before file\n");
    return t;
}
