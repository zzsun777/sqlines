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

#include <QtGui>

 #include "highlighter.h"

 Highlighter::Highlighter(QTextDocument *parent)
     : QSyntaxHighlighter(parent)
 {
     HighlightingRule rule;

     keywordFormat.setForeground(Qt::blue);
     //keywordFormat.setFontWeight(QFont::Bold);
     QStringList keywordPatterns;
     keywordPatterns
             << "alter" << "and" << "as" << "autoincrement"
             << "begin" << "between" << "bigint" << "binary"
             << "cascade" << "char" << "check" << "close" << "comment" << "commit"
             << "constraint"
             << "create" << "cursor"
             << "date" << "datetime" << "day" << "deallocate "<< "declare" << "default"
             << "delete" << "double"
             << "drop"
             << "else" << "end" << "exception" << "exists" << "exit"
             << "fetch" << "for" << "foreach" << "foreign" << "from" << "function"
             << "handler"
             << "if" << "in" << "index" << "insert" << "int" << "into" << "is"
             << "go"
             << "key"
             << "like"
             << "long" << "loop"
             << "money"
             << "nchar" << "null" << "number" << "numeric" << "nvarchar" << "nvarchar2"
             << "on" << "open" << "or"
             << "primary" << "procedure"
             << "references" << "replace" << "restrict" << "return" << "returns" << "returning"
             << "select" << "set"
             << "short" << "signed" << "smallint" << "smallmoney"
             << "table" << "then" << "time" << "timestamp"
             << "union" << "unique" << "uniqueidentifier" << "unsigned" << "update"
             << "when" << "where" << "while" << "work"
             << "values" << "varchar" << "varchar2" << "view" << "volatile"
             << "xml";
     foreach (const QString &pattern, keywordPatterns) {
         rule.pattern = QRegExp(QString("\\b%1\\b").arg(pattern));
         rule.format = keywordFormat;
         rule.pattern.setCaseSensitivity(Qt::CaseInsensitive);
         highlightingRules.append(rule);
     }

     numberFormat.setForeground(Qt::darkMagenta);
     rule.pattern = QRegExp("\\b[0-9]+\\b");
     rule.format = numberFormat;
     highlightingRules.append(rule);

     singleLineCommentFormat.setForeground(Qt::darkGreen);
     rule.pattern = QRegExp("//[^\n]*");
     rule.format = singleLineCommentFormat;
     highlightingRules.append(rule);

     singleLineCommentFormat2.setForeground(Qt::darkGreen);
     rule.pattern = QRegExp("--[^\n]*");
     rule.format = singleLineCommentFormat2;
     highlightingRules.append(rule);

     multiLineCommentFormat.setForeground(Qt::darkGreen);

     quotationFormat.setForeground(Qt::darkRed);
     rule.pattern = QRegExp("\".*\"");
     rule.pattern.setMinimal(true);
     rule.format = quotationFormat;
     highlightingRules.append(rule);

     singleQuotationFormat.setForeground(Qt::darkRed);
     rule.pattern = QRegExp("'.*'");
     rule.pattern.setMinimal(true);
     rule.format = singleQuotationFormat;
     highlightingRules.append(rule);

     commentStartExpression = QRegExp("/\\*");
     commentEndExpression = QRegExp("\\*/");
 }

 void Highlighter::highlightBlock(const QString &text)
 {
     foreach (const HighlightingRule &rule, highlightingRules) {
         QRegExp expression(rule.pattern);
         int index = expression.indexIn(text);
         while (index >= 0) {
             int length = expression.matchedLength();
             setFormat(index, length, rule.format);
             index = expression.indexIn(text, index + length);
         }
     }
     setCurrentBlockState(0);

     int startIndex = 0;
     if (previousBlockState() != 1)
         startIndex = commentStartExpression.indexIn(text);

     while (startIndex >= 0) {
         int endIndex = commentEndExpression.indexIn(text, startIndex);
         int commentLength;
         if (endIndex == -1) {
             setCurrentBlockState(1);
             commentLength = text.length() - startIndex;
         } else {
             commentLength = endIndex - startIndex
                             + commentEndExpression.matchedLength();
         }
         setFormat(startIndex, commentLength, multiLineCommentFormat);
         startIndex = commentStartExpression.indexIn(text, startIndex + commentLength);
     }
 }
