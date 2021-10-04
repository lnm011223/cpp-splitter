#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QTextStream>
#include <QFile>
#include <QDebug>
#include "scanner.h"
#include <QMessageBox>
#include <iostream>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_clicked()
{
    QString file_path = QFileDialog::getOpenFileName(this,"选择一个代码文件", ".", "Code(*.txt *.cpp *.mycpp *.h)");
    ui->lineEdit->setText(file_path);

    ui->textEdit->clear();
    QFile file(file_path);
    if(!file.open(QFile::ReadOnly | QFile::Text))
       return;
    QTextStream in(&file);
    while(!in.atEnd())
    {
        QString line = in.readLine();
        ui->textEdit->append(line);
    }
}
void MainWindow::on_pushButton_2_clicked()
{
    ui->textEdit_2->clear();
    QString file_path = ui->lineEdit->text();
    string com1 = " ";
    string com2 = "\n";
    string fileline;
    int i = 1;
    vector<str>array(0);
    string file_name1=file_path.toStdString();
    ifstream infile(file_name1.c_str(), ios::in);
    if (!infile)
    {
        cerr << "无法打开源文件 " << file_name1.c_str() << endl;
    }
    else
    {
        cout << endl;
        while (!infile.eof())
        {
            getline(infile, fileline, '\n');
            fileline += "\n";
            lines++;
            str nowString;
            start = 0;
            over = 0;
            for (int j = 0;fileline[j] != '\0' && over != -1;j++)
            {
                start = get_nbc(fileline, start);
                nowString.Str = GetWord(fileline, start, over);
                nowString.linenum = lines;
                if (nowString.Str.compare(com1) && nowString.Str.compare(com2))
                {
                    array.push_back(nowString);
                }
                start = over + 1;
            }
        }
        vector<pair<string, string> > result;
        result = analyse(array);
        file_name=file_name1+"_out.txt";
        bool out_flag = Output(result);
        if (out_flag)
        {
            cout << "----------------------------------------------------------------------\n";
            cout << "词法分析完成，分析结果已经存储在文件" << file_name << "中\n";
            cout << "----------------------------------------------------------------------\n";
            QString file_out = QString::fromStdString(file_name);
            QFile file(file_out);
            if(!file.open(QFile::ReadOnly | QFile::Text))
               return;
            QTextStream in(&file);
            while(!in.atEnd())
            {
                QString line = in.readLine();
                ui->textEdit_2->append(line);
            }
        }
    }

}
