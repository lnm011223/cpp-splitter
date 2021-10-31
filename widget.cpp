#include "widget.h"
#include "ui_widget.h"
#include"iostream"
#include"analyse.h"
using namespace std;

MainWidget::MainWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MainWidget)
{
    ui->setupUi(this);
    ui -> input -> setText("(a|b)*cde");
    this -> setWindowTitle("Author:20192131031 梁诺明");


}

MainWidget::~MainWidget()
{
    delete ui;
}




void MainWidget::on_button_clicked()
{
    //获取表达式
    string regExp = ui -> input ->text().toStdString();
    Change proxy(regExp);
    //显示NFA
    QTableWidget * nfaTable = ui -> nfaTable;
    int length = proxy.nfa.nodeNumber;
    cout<<length; 
    nfaTable -> setRowCount(length);
    nfaTable -> setColumnCount(length);
    for(int i = 0; i < length; i++)
    {
        for(int j = 0; j < length; j++)
        {
            string ch = "";
            ch += proxy.chart[i+1][j+1];
            if(ch == " ")   ch = " ";
            QString item = QString::fromStdString(ch);
            nfaTable -> setItem(i, j, new QTableWidgetItem(item));
        }
    }

    //显示DFA
    QTableWidget *dfaTable = ui-> dfaTable;
    int colCount = proxy.dfa.wordList.size();
    int rowCount = proxy.dfa.graph.size();
    dfaTable->setRowCount(rowCount);
    dfaTable->setColumnCount(colCount);

    //行标题和列标题，每个集合转成字符加进去
    QStringList colNames;
    QStringList rowNames;
    for(int j = 0; j < rowCount; j++)
    {
        string node = proxy.dfa.graph[j]->toString();
        rowNames.append(QString::fromStdString(node));
    }

    DFA dfa = proxy.dfa;
    int i = 0;
    for(char ch : proxy.dfa.wordList)
    {
        string col = "";
        col += ch;
        colNames.push_back(QString::fromStdString(col));
        for(int j = 0; j < rowCount; j++)
        {
            string transformResult;
            DFANode * node = proxy.dfa.graph[j] -> process(ch);
            if(node == NULL)
            {
                transformResult = " ";
            }
            else
            {
                transformResult = node -> toString();
            }
            dfaTable->setItem(j,i,new QTableWidgetItem(QString::fromStdString(transformResult)));
        }
        i++;
    }

    dfaTable->setHorizontalHeaderLabels(colNames);
    dfaTable->setVerticalHeaderLabels(rowNames);

    //
    string startNodeText = "";
    string endNodeText = "";
    for(DFANode *n : proxy.dfa.graph)
    {
        if(n->contains(1))
            startNodeText = n->toString();
        if(n->state == END)
        {
            endNodeText += " " + n->toString();
        }
    }

    //最小化DFA
    proxy.minimizeDFA();
    QTableWidget *minDfaTable = ui -> minDfaTable;
    rowCount = proxy.finalDFA.graph.size();
    minDfaTable->setRowCount(rowCount);
    minDfaTable->setColumnCount(colCount);
    rowNames.clear();
    for(int j = 0; j < rowCount; j++)
    {
        string node = proxy.finalDFA.graph[j]->minName;
        rowNames.append(QString::fromStdString(node));
    }
    i = 0;
    for(char ch : proxy.dfa.wordList)
    {
        for(int j = 0; j < rowCount; j++)
        {
            string transformResult;
            DFANode *node = proxy.finalDFA.graph[j]->process(ch);
            if(node == NULL)
            {
                transformResult = " ";
            }
            else
            {
                transformResult = node->minName;
            }
            minDfaTable->setItem(j, i, new QTableWidgetItem(QString::fromStdString(transformResult)));
        }
        i++;
    }
    minDfaTable->setHorizontalHeaderLabels(colNames);
    minDfaTable->setVerticalHeaderLabels(rowNames);

    startNodeText = "";
    endNodeText = "";
    for(DFANode *n : proxy.finalDFA.minEndNodes)
    {
        endNodeText += " " + n->minName;
    }
    startNodeText = proxy.finalDFA.minStartNode->minName;





    proxy.generateCode();
    QTextEdit * codeText = ui-> codeBrowser;
    codeText->setText(QString::fromStdString(proxy.code));

}
