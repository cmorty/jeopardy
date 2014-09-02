/*
 * Copyright (c) 2011-2012, Christian Lange
 * (chlange) <chlange@htwg-konstanz.de> <Christian_Lange@hotmail.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of Christian Lange nor the names of its
 *       contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL CHRISTIAN LANGE BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "editor.h"
#include <QEvent>
#include <QFile>
#include "keyledcontrol.h"

Editor::Editor(QList<Player *> *players, QWidget *parent):
    QDialog(parent), players(players)
{
    /* Load style File */
    QFile file("jeopardy.qss");
    file.open(QFile::ReadOnly);
    QString styleSheet = QLatin1String(file.readAll());
    this->setStyleSheet(styleSheet);
    KeyLedControl::setEnabled(false);
}

Editor::~Editor()
{
    KeyLedControl::setEnabled(true);
}

void Editor::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        break;
    default:
        break;
    }
}

void Editor::show()
{
    init();
    exec();
}

void Editor::init()
{
    QStringList keyList;
    keyList << "A" << "B" << "C" << "D" << "E" << "F" << "G" << "H" << "I" << "J" << "K" << "L" << "M"
            << "N" << "O" << "P" << "Q" << "R" << "S" << "T" << "U" << "V" << "W" << "X" << "Y" << "Z";




    mainGrid = new QGridLayout();
    lineGrid = new QGridLayout();
    saveGrid = new QGridLayout();

    mainGrid->addLayout(lineGrid, 0, 0);
    mainGrid->addLayout(saveGrid, 1, 0);
    setLayout(mainGrid);


    for(int i = 0; i < players->length(); i++){
        Player *p = (*players)[i];
        QLineEdit *qle = new QLineEdit();
        qle->setText(p->getName());
        playerNamesLines.append(qle);
        lineGrid->addWidget(qle, i, 0);

        QSpinBox * qsb = new QSpinBox();
        qsb->setSingleStep(50);
        qsb->setMinimum(-50000);
        qsb->setMaximum(50000);
        qsb->setValue(p->getPoints());
        lineGrid->addWidget(qsb, i, 1);
        playerPointsLines.append(qsb);

        QComboBox *cb = new QComboBox();
        cb->addItems(keyList);
        cb->setCurrentIndex(p->getKey() - 0x41);
        lineGrid->addWidget(cb, i, 2);
        playerKeyBox.append(cb);

    }

    saveButton = new QPushButton();
    saveButton->setText(tr("Save"));
    saveGrid->addWidget(saveButton, 0, 0);
    QObject::connect(saveButton, SIGNAL(clicked()), this, SLOT(end()));

}




void Editor::saveChanges()
{
    for(int i = 0; i < players->length(); i++)
    {
        (*players)[i]->setName(playerNamesLines[i]->text());
        (*players)[i]->setPoints(playerPointsLines[i]->value());
        (*players)[i]->setKey(playerKeyBox[i]->currentIndex() + 0x41);
    }
}

void Editor::end()
{
    saveChanges();
    done(0);
}
