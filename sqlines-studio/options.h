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

#ifndef OPTIONS_H
#define OPTIONS_H

#include <QDialog>
#include <QSortFilterProxyModel>
#include <QModelIndex>
#include <QSettings>

#define OPTION_WORKDIR "workdir"

namespace Ui {
    class Options;
}

class Options : public QDialog
{
    Q_OBJECT

public:
    explicit Options(QWidget *parent = 0);
    ~Options();

public slots:
    void selectWorkingDirectory();
    void saveDialogData();

private:
    Ui::Options *ui;

    QSettings *settings;
    QString workingDirectory;

    void loadOptions();
    void saveOptions();
};

#endif // OPTIONS_H
