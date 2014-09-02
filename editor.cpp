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

Editor::Editor(QList<Player *> *players, QWidget *parent):
    QDialog(parent), players(players)
{
}

Editor::~Editor()
{
    delete saveButton;
    delete lineGrid;
    delete saveGrid;
    delete mainGrid;
    delete window;
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
    window->exec();
}

void Editor::init()
{
    insertLayouts();
    assignPlayerNamesLines();
    assignPlayerPointsLines();
    assignKeyBoxes();
    assignSaveButton();
    showValues();
}

void Editor::insertLayouts()
{
    window = new QDialog();

    mainGrid = new QGridLayout();
    lineGrid = new QGridLayout();
    saveGrid = new QGridLayout();

    mainGrid->addLayout(lineGrid, 0, 0);
    mainGrid->addLayout(saveGrid, 1, 0);
    window->setLayout(mainGrid);
}

void Editor::assignPlayerNamesLines()
{
    for(int i = 0; i < players->length(); i++)
    {
        playerNamesLines[i] = new QLineEdit();
        lineGrid->addWidget(playerNamesLines[i], i, 0);
    }
}

void Editor::assignPlayerPointsLines()
{
    for(int i = 0; i < players->length(); i++)
    {
        playerPointsLines[i] = new QSpinBox();
        playerPointsLines[i]->setSingleStep(50);
        playerPointsLines[i]->setMinimum(-50000);
        playerPointsLines[i]->setMaximum(50000);
        lineGrid->addWidget(playerPointsLines[i], i, 1);
    }
}

void Editor::assignKeyBoxes()
{
    QStringList keyList;
    keyList << "A" << "B" << "C" << "D" << "E" << "F" << "G" << "H" << "I" << "J" << "K" << "L" << "M"
            << "N" << "O" << "P" << "Q" << "R" << "S" << "T" << "U" << "V" << "W" << "X" << "Y" << "Z";

    for(int i = 0; i < players->length(); i++)
    {
        playerKeyBox[i] = new QComboBox();
        playerKeyBox[i]->addItems(keyList);
        playerKeyBox[i]->setCurrentIndex((*players)[i]->getKey() - 0x41);
        lineGrid->addWidget(playerKeyBox[i], i, 2);
    }
}

void Editor::assignSaveButton()
{
    saveButton = new QPushButton();
    saveButton->setText("Save");
    saveGrid->addWidget(saveButton, 0, 0);
    QObject::connect(saveButton, SIGNAL(clicked()), this, SLOT(end()));
}

void Editor::showValues()
{
    for(int i = 0; i < players->length(); i++)
    {
        playerNamesLines[i]->setText((*players)[i]->getName());
        playerPointsLines[i]->setValue((*players)[i]->getPoints());
    }
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
    window->done(0);
}
