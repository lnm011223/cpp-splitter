#include "util.h"
#include "scan.h"

/* states in scanner DFA */
typedef enum
{
    START, INCOMMENT, INNUM, INID, DONE,INREASSIGN
} StateType;

/* lexeme of identifier or reserved word */
char tokenString[MAXTOKENLEN + 1];

/* BUFLEN = length of the input buffer for
   source code lines */
#define BUFLEN 256

static char lineBuf[BUFLEN]; /* holds the current line */
static int linepos = 0; /* current position in LineBuf */
static int bufsize = 0; /* current size of buffer string */
static int EOF_flag = FALSE; /* corrects ungetNextChar behavior on EOF */

/* getNextChar fetches the next non-blank character
   from lineBuf, reading in a new line if lineBuf is
   exhausted */
static int getNextChar(void)
{
    if (!(linepos < bufsize))
    {
        lineno++;
        if (fgets(lineBuf, BUFLEN-1, source))
        {
            if (EchoSource)
                fprintf(listing, "%4d: %s", lineno, lineBuf);
            bufsize = strlen(lineBuf);
            linepos = 0;
            return lineBuf[linepos++];
        }
        else
        {
            EOF_flag = TRUE;
            return EOF;
        }
    }
    else
        return lineBuf[linepos++];
}

/* ungetNextChar backtracks one character
   in lineBuf */
static void ungetNextChar(void)
{
    if (!EOF_flag)
        linepos-- ;
}

/* lookup table of reserved words */
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

/* lookup an identifier to see if it is a reserved word */
/* uses linear search */
static TokenType reservedLookup (char * s)
{
    int i;
    for (i=0; i<MAXRESERVED; i++)
        if (!strcmp(s, reservedWords[i].str))
            return reservedWords[i].tok;
    return ID;
}

/****************************************/
/* the primary function of the scanner  */
/****************************************/
/* function getToken returns the
 * next token in source file
 */
TokenType getToken(void)
{
    /* index for storing into tokenString */
    int tokenStringIndex = 0;
    /* holds current token to be returned */
    TokenType currentToken;
    /* current state - always begins at START */
    StateType state = START;
    /* flag to indicate save to tokenString */
    int save;
    while (state != DONE)
    {
        int c = getNextChar();
        save = TRUE;
        switch (state)
        {
        case START:
            if (isdigit(c))
                state = INNUM;
            else if (isalpha(c))
                state = INID;
            else if (c == ':')
                      state = INREASSIGN;
            else if ((c == ' ') || (c == '\t') || (c == '\n'))
                save = FALSE;
            else if (c == '{')
            {
                save = FALSE;
                state = INCOMMENT;
            }
            else
            {
                state = DONE;
                switch (c)
                {
                case EOF:
                    save = FALSE;
                    currentToken = ENDFILE;
                    break;
                case '=':
                    //=和==的情况
                    c = getNextChar();
                    if(c == '=')
                        currentToken = EQ;
                    else
                    {
                        ungetNextChar();
                        currentToken = ASSIGN;
                    }
                    break;
                case '<':
                    //增加 <= <>
                    c = getNextChar();
                    if(c == '=')
                        currentToken = LESSEQUAL;
                    else if(c == '>')
                        currentToken = NEQ;
                    else
                    {
                        ungetNextChar();
                        currentToken = LT;
                    }
                    break;
                case '>':
                    //增加 > >=
                    c = getNextChar();
                    if(c == '=')
                        currentToken = MOREEQUAL;
                    else
                    {
                        ungetNextChar();
                        currentToken = MT;
                    }
                    break;
                case '+':
                    currentToken = PLUS;
                    break;
                case '-':
                    //增加 -=
                    c = getNextChar();
                    if(c == '=')
                        currentToken = MinusEQUAL;
                    else
                    {
                        ungetNextChar();
                        currentToken = MINUS;
                    }
                    break;
                case '*':
                    currentToken = TIMES;
                    break;
                case '/':
                    currentToken = OVER;
                    break;
                case '(':
                    currentToken = LPAREN;
                    break;
                case ')':
                    currentToken = RPAREN;
                    break;
                case ';':
                    currentToken = SEMI;
                    break;
                /* 新增特殊符号 */
                case '%':
                    currentToken = MOD;
                    break;
                case '^':
                    currentToken = POWER;
                    break;
                case '&':
                    currentToken = REAND;
                    break;
                case '|':
                    currentToken = REOR;
                    break;
                case '#':
                    currentToken = RECLO;
                    break;
                default:
                    currentToken = ERROR;
                    break;
                }
            }
            break;
        case INCOMMENT:
            save = FALSE;
            if (c == EOF)
            {
                state = DONE;
                currentToken = ENDFILE;
            }
            else if (c == '}')
                state = START;
            break;
        case INREASSIGN:
                 state = DONE;
                 if (c == '=')
                   currentToken = REASSIGN;
                 else
                 { /* backup in the input */
                   ungetNextChar();
                   save = FALSE;
                   currentToken = ERROR;
                 }
                 break;
        case INNUM:
            if (!isdigit(c))
            {
                /* backup in the input */
                ungetNextChar();
                save = FALSE;
                state = DONE;
                currentToken = NUM;
            }
            break;
        case INID:
            if (!isalpha(c))
            {
                /* backup in the input */
                ungetNextChar();
                save = FALSE;
                state = DONE;
                currentToken = ID;
            }
            break;
        case DONE:
        default:
            /* should never happen */
            fprintf(listing,"Scanner Bug: state= %d\n",state);
            state = DONE;
            currentToken = ERROR;
            break;
        }
        if ((save) && (tokenStringIndex <= MAXTOKENLEN))
            tokenString[tokenStringIndex++] = (char) c;
        if (state == DONE)
        {
            tokenString[tokenStringIndex] = '\0';
            if (currentToken == ID)
                currentToken = reservedLookup(tokenString);
            //如果下一个字符为:，说明为正则表达式的赋值语句
        }
    }
    if (TraceScan)
    {
        fprintf(listing,"\t%d: ",lineno);
        printToken(currentToken,tokenString);
    }
    return currentToken;
} /* end getToken */

