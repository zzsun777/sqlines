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

#ifndef SCRIPTTAB_H
#define SCRIPTTAB_H

#include <QWidget>
#include <QSplitter>
#include <QPlainTextEdit>
#include "highlighter.h"

namespace Ui {
    class ScriptTab;
}

class ScriptTab : public QWidget
{
    Q_OBJECT

public:
    explicit ScriptTab(QWidget *parent = 0);
    ~ScriptTab();

    // Create or save if required and return the name of the source file
    QString GetSourceFile();
    // Save the source content to the file
    void SaveSource();

    // Set the source file and load its content to the source page
    void SetSourceFile(QString &file);
    // Set the target file and load its content to the target page
    void SetTargetFile(QString &file);

    void SetTabName(QString &name) { _tab_name = name; }

protected:
    void resizeEvent(QResizeEvent *event);

private:
    Ui::ScriptTab *ui;

    QString _tab_name;
    QSplitter *mainVerticalSplitter;

    QPlainTextEdit *_sourceTextEdit;
    QPlainTextEdit *_targetTextEdit;

    Highlighter *_sourceHighlighter;
    Highlighter *_targetHighlighter;

    void createScriptTab();

    void ReadFileToTextEdit(QPlainTextEdit *edit, QString &file);
};

#endif // SCRIPTTAB_H
