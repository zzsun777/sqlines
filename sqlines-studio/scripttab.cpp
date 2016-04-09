/**
 * Copyright (c) 2016 SQLines
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "QMessageBox"
#include "QPlainTextEdit"
#include "QFile"
#include "QTextStream"

#include "scripttab.h"
#include "ui_scripttab.h"

ScriptTab::ScriptTab(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ScriptTab)
{
    _sourceTextEdit = NULL;
    _targetTextEdit = NULL;

    _sourceHighlighter = NULL;
    _targetHighlighter = NULL;

    ui->setupUi(this);

    createScriptTab();
}

ScriptTab::~ScriptTab()
{
    delete ui;

    delete _sourceHighlighter;
    delete _targetHighlighter;
}

// Create or save if required and return the name of the source file
QString ScriptTab::GetSourceFile()
{
    QString name = _tab_name;

    // Save the source content to the file
    SaveSource();

    return name;
}

// Save the source content to the file
void ScriptTab::SaveSource()
{
    if(_sourceTextEdit == NULL)
        return;

    QFile file;

    file.setFileName(_tab_name);
    file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text);

    QTextStream stream(&file);
    stream << _sourceTextEdit->toPlainText() << endl;

    file.close();
}

// Set the source file and load its content to the source page
void ScriptTab::SetSourceFile(QString &source_file)
{
    ReadFileToTextEdit(_sourceTextEdit, source_file);
}

// Set the target file and load its content to the target page
void ScriptTab::SetTargetFile(QString &target_file)
{
    ReadFileToTextEdit(_targetTextEdit, target_file);
}

// Read file content to the specified text edit
void ScriptTab::ReadFileToTextEdit(QPlainTextEdit *edit, QString &in_file)
{
    if(edit == NULL)
        return;

    QFile file;

    file.setFileName(in_file);
    file.open(QIODevice::ReadOnly | QIODevice::Text);

    QTextStream stream(&file);
    edit->setPlainText(stream.readAll());
    edit->setFont(QFont("Courier"));

    file.close();
}

void ScriptTab::resizeEvent(QResizeEvent *event)
{
    ui->verticalLayoutWidget->resize(parentWidget()->size());
    mainVerticalSplitter->resize(ui->verticalLayoutWidget->size());
    QWidget::resizeEvent(event);
}

void ScriptTab::createScriptTab()
{
    // create splitter between script views and logs
    mainVerticalSplitter = new QSplitter(Qt::Vertical, ui->verticalLayoutWidget);

    // create source/target views
    QSplitter *mainHorizintalSplitter = new QSplitter();
    _sourceTextEdit = new QPlainTextEdit();
    _targetTextEdit = new QPlainTextEdit();

    mainHorizintalSplitter->addWidget(_sourceTextEdit);
    mainHorizintalSplitter->addWidget(_targetTextEdit);

    _sourceHighlighter = new Highlighter(_sourceTextEdit->document());
    _targetHighlighter = new Highlighter(_targetTextEdit->document());

    // create bottom quick template and log views
    //QSplitter *bottomHorizintalSplitter = new QSplitter();
    //QTabWidget *templateTabWidget = new QTabWidget();
    //QTabWidget *logTabWidget = new QTabWidget();
    //QPlainTextEdit *templateTextEdit = new QPlainTextEdit();
    //QPlainTextEdit *logTextEdit = new QPlainTextEdit();

    //templateTabWidget->addTab(templateTextEdit, "Quick Conversion Template");
    //logTabWidget->addTab(logTextEdit, "Log");

    //bottomHorizintalSplitter->addWidget(templateTabWidget);
    //bottomHorizintalSplitter->addWidget(logTabWidget);

    mainVerticalSplitter->addWidget(mainHorizintalSplitter);
    //mainVerticalSplitter->addWidget(bottomHorizintalSplitter);

    // set default view proporation
    QList<int> sizes; sizes.append(300); sizes.append(100);
    mainVerticalSplitter->setSizes(sizes);
}
