# 编译原理 词法分析器文档



## 需求：

> 1. 把C++源代码中的各类单词（记号）进行拼装分类。
>           C++语言包含了几种类型的单词（记号）：标识符，关键字，数（包括整数、浮点数），字符串、注释、特殊符号（分界符）和运算符号等【详细的单词类别及拼装规则见另外的文件说明】。
>       
> 2. 要求应用程序应为Windows界面。
>
> 3. 打开一个C++源文件，列出所有可以拼装的单词（记号）。
>
> 4. 应该书写完善的软件设计文档。
>
>       

## 编译环境：macOS,Qt 5.12.11

## 核心算法：

读取文件后，将源程序拆分为单词，去除空格和空行后存入 vector 中，自动机一个个读入字符，后输出分析结果和统计结果存在文件里，再读取显示在程序上

## DFA图示例：

![image-20211004165146323](https://gitee.com/lnm011223/lnm011223-picture/raw/master/uPic/image-20211004165146323.png)

<!--标识符/关键字-->

![image-20211004165741462](https://gitee.com/lnm011223/lnm011223-picture/raw/master/uPic/image-20211004165741462.png)

<!--数字-->

![image-20211004170338984](https://gitee.com/lnm011223/lnm011223-picture/raw/master/uPic/image-20211004170338984.png)

![image-20211004170429845](https://gitee.com/lnm011223/lnm011223-picture/raw/master/uPic/image-20211004170429845.png)

![image-20211004170601626](https://gitee.com/lnm011223/lnm011223-picture/raw/master/uPic/image-20211004170601626.png)

![image-20211004171056774](https://gitee.com/lnm011223/lnm011223-picture/raw/master/uPic/image-20211004171056774.png)

## 重要函数实现：（scanner.h)

```c++
class str
{
public:
    int linenum;//行号
    string Str;//单词
};

//输出文件名(为原文件名+_out.txt）
string file_name;

//返回单词或符号,从位置i开始查找，引用参数j返回这个单词最后一个字符在str的位置。
string GetWord(string str, int i, int& j);

//除去字符串中连续的空格和换行。第一个参数为目标字符串，第二个参数为开始位置。返回值为第一个有效字符在字符串的位置
int get_nbc(string str, int i);

//文件输出函数，成功输出后才能被读取出来
bool Output(vector<pair<string, string> > v);

//词法分析主要算法函数，返回一个pair型数组
vector<pair<string, string> > analyse(vector<str>& vec);

//关键字判断
bool IsKey(string str);

//字母判断
bool letter(char C);

//数字判断
bool digit(char C);
```

## 软件界面展示：

![image-20211004171610796](https://gitee.com/lnm011223/lnm011223-picture/raw/master/uPic/image-20211004171610796.png)

## 测试结果：

![image-20211004171726737](https://gitee.com/lnm011223/lnm011223-picture/raw/master/uPic/image-20211004171726737.png)

![image-20211004171800343](https://gitee.com/lnm011223/lnm011223-picture/raw/master/uPic/image-20211004171800343.png)

![image-20211004171904459](https://gitee.com/lnm011223/lnm011223-picture/raw/master/uPic/image-20211004171904459.png)

## 总结：

学会了如何简单使用qt，用了DFA设计函数，但程序还不支持中文字符，有待改进
