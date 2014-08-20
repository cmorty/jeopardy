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

#include "jeopardy.h"
#include "ui_jeopardy.h"

Jeopardy::Jeopardy(QWidget *parent) :
    QMainWindow(parent),
    sound(false), gameField(NULL)
{

    players = new Player[NUMBER_MAX_PLAYERS];
}

Jeopardy::~Jeopardy()
{
    if(players != NULL)
        delete [] players;

    if(gameField != NULL)
    {
        delete gameField;
        gameField = NULL;
    }
}

void Jeopardy::changeEvent(QEvent *e)
{
    QMainWindow::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        break;
    default:
        break;
    }
}

void Jeopardy::init()
{
    initMenu();
}

void Jeopardy::initMenu()
{
    window = new QWidget();
    grid = new QGridLayout();

    for(int i = 0; i < NUMBER_ROUNDS; i++)
        prepareButton(i);

    window->setLayout(grid);
    window->show();
}

void Jeopardy::prepareButton(int i)
{
    buttons[i] = new QPushButton();
    buttons[i]->setText(QString("Round %1").arg(i + 1));
    buttons[i]->setFont(QFont("Helvetica [Cronyx]", 13, QFont::Bold, false));
    buttons[i]->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    grid->addWidget(buttons[i], 0, i, 0);
    grid->setSpacing(0);
    grid->setMargin(0);
    connect(buttons[i], SIGNAL(clicked()), buttons[i], SLOT(hide()));
    connect(buttons[i], SIGNAL(clicked()), this, SLOT(initGameField()));
}

void Jeopardy::initGameField()
{
    bool complete;

    round = getRound();
    setSound();

    /*
    if(sound)
    {
        music = Phonon::createPlayer(Phonon::NoCategory, Phonon::MediaSource("sound/title.ogg"));
        music->play();
    }
    */

    complete = initPlayers();

    if(NOT == complete)
    {
        deleteSound();
        return;
    }

    deleteSound();

    setCategoryNr();

    if(categoryNr > 0) {
        startRound(round);
    } else {
        QMessageBox msgBox;
        msgBox.setText("No category in round file specified!");
        msgBox.exec();
    }
}

int Jeopardy::getRound()
{
   for(int i = 0; i < NUMBER_ROUNDS; i++)
   {
       if(buttons[i]->isHidden())
       {
            round = i + 1;
            buttons[i]->setStyleSheet(QString("background-color: lightGray;"));
            buttons[i]->setHidden(false);
       }
   }

   return round;
}

void Jeopardy::setSound()
{
    QMessageBox msgBox;
    msgBox.setText("Do you need sound?");
    msgBox.setWindowTitle("Sound");
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::Yes);

    if(msgBox.exec() == QMessageBox::Yes)
        sound = true;
    else
        sound = false;
}

void Jeopardy::setCategoryNr()
{
    categoryNr = 0;
    QDir dir;
    QFile *file;

    fileString = QString("answers/%1.jrf").arg(round);
    fileString = dir.absoluteFilePath(fileString);

    file = new QFile(fileString);

    if (!file->open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QMessageBox::critical(this, tr("Error"), tr("Could not open round file, please select one by yourself"));

        fileString = QFileDialog::getOpenFileName(this, tr("Open File"), "answers/", tr("Jeopardy Round File (*.jrf)"));
        fileString = dir.absoluteFilePath(fileString);
        file = new QFile(fileString);

        if (!file->open(QIODevice::ReadOnly | QIODevice::Text))
        {
          QMessageBox::critical(this, tr("Error"), tr("Could not open file"));
          return;
        }
    }

    QTextStream in(file);
    QString line;

    for(int i = 1; i < 32; i++)
    {
        line = in.readLine();
        if(line.isNull())
            break;

        /* category lines are 1, 7, 13, 19 and 25 */
        if(i % 6 == 1)
        {
            if(line != "" && !(line.startsWith("category") && line.length() == 9) && line != "category")
                categoryNr++;
            else
                break;
        }
    }
    delete file;
}

bool Jeopardy::initPlayers()
{
    bool ok;
    QString playerName, text, key, color;
    QStringList keyList, colorList;

    colorList << "red" << "green" << "yellow" << "blue" << "gray" << "magenta" << "darkRed" << "cyan" << "white" << "darkMagenta";
    keyList << "A" << "B" << "C" << "D" << "E" << "F" << "G" << "H" << "I" << "J" << "K" << "L" << "M"
            << "N" << "O" << "P" << "Q" << "R" << "S" << "T" << "U" << "V" << "W" << "X" << "Y" << "Z";

    for(playerNr = 0; playerNr < NUMBER_MAX_PLAYERS; playerNr++)
    {
        playerName = QString("Player %1").arg(playerNr + 1);
        int dialogcode;

        for(;;)
        {
            QInputDialog playerInput;

            if(playerNr > 0)
                playerInput.setCancelButtonText("Play");
            else
                playerInput.setCancelButtonText("Cancel");

            playerInput.setLabelText("Enter name");

            playerInput.setOkButtonText("Create player");
            dialogcode = playerInput.exec();
            text = playerInput.textValue();

            if(text.length() < 10)
                break;

            QMessageBox msgBox;
            msgBox.setText("Choose a name shorter than 11 letters");
            msgBox.exec();
        }

        if(text.isEmpty() || dialogcode == 0)
            break;

        players[playerNr].setName(text);
        players[playerNr].setId(playerNr + 1);
        players[playerNr].setPressed(0);

        key = QInputDialog::getItem(this, "Choose key", "Choose key:", keyList, 0, false, &ok);
        if(!ok)
            break;

        players[playerNr].setKey(key.at(0).toAscii());
        keyList.removeOne(key);

        color = QInputDialog::getItem(this, "Choose color ", "Color:", colorList, 0, false, &ok);
        if(!ok)
            break;

        players[playerNr].setColor(color);
        colorList.removeOne(color);
        players[playerNr].setPoints(0);
    }

    return (playerNr > 0) ? true : false;
}

void Jeopardy::startRound(int round)
{
    gameField = new GameField(this, round, categoryNr, players, playerNr, sound, fileString);
    gameField->init();
}

void Jeopardy::deleteSound()
{
    if(sound)
    {
        music->stop();
        delete music;
    }
}
