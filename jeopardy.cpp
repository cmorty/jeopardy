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
    /* Load style File */
    QFile file("jeopardy.qss");
    file.open(QFile::ReadOnly);
    QString styleSheet = QLatin1String(file.readAll());
    this->setStyleSheet(styleSheet);
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

void Jeopardy::init(QString folder)
{

   if(!loadRounds(folder)) this->close();
   initMenu();
}

void Jeopardy::initMenu()
{
    window = new QWidget();
    grid = new QGridLayout();
    grid->setSpacing(0);
    grid->setMargin(0);

    for(int i = 0;  i < rounds.length();  i++){
            Round * r = rounds[i];
            QPushButton * b = new QPushButton();
            b->setText(QString("Round %1").arg(r->getRoundNr()));
            b->setProperty("roundId", i);
            b->setFont(QFont("Helvetica [Cronyx]", 13, QFont::Bold, false));
            b->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
            grid->addWidget(b, 0, r->getRoundNr(), 0);
            connect(b, SIGNAL(clicked()), this, SLOT(initGameField()));
            buttons.append(b);
    }

    window->setLayout(grid);
    window->show();
    setSound();
}


void Jeopardy::initGameField()
{
    bool complete;

    QPushButton * button = qobject_cast<QPushButton *>(sender());
    button->setDisabled(true);
    button->setStyleSheet(QString("background-color: lightGray;"));

    int rid = (button->property("roundId")).toInt();

    Round * round = rounds[rid];

    if(sound)
    {
        music = Phonon::createPlayer(Phonon::NoCategory, Phonon::MediaSource("sound/title.ogg"));
        music->play();
    }

    complete = initPlayers();

    if(NOT == complete)
    {
        deleteSound();
        return;
    }

    deleteSound();

    gameField = new GameField(round, players, playerNr, sound);
    gameField->init();

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

 bool Jeopardy::loadRounds(QString filefolder)
{

    QDir dir(filefolder);
    QString fileString;

    while(true){
        //Is there at least on round
        QFile f(dir.absoluteFilePath("1.jrf"));
        if(!f.exists()){
            QMessageBox::critical(this, tr("Error") ,  tr("Please select a file"));
        } else {
            if(loadRound(f.fileName())) break;
        }

        fileString = QFileDialog::getOpenFileName(this, tr("Open File"), dir.absolutePath(), tr("Jeopardy Round File (1.jrf)"));
        if(fileString.length() == 0) return false;

        QFileInfo file(fileString);
        dir.setPath(QString(file.absoluteDir().absolutePath()));
    }

    //Load remaining rounds
    for(int r = 2; loadRound(dir.absoluteFilePath(QString::number(r) + ".jrf")); r++);
    return true;
}

bool Jeopardy::loadRound(const QString file){
    Round *r = NULL;
    r = new Round(file, rounds.length() + 1);
    if(r->load()){
        rounds.append(r);
        return true;
    }
    return false;
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



void Jeopardy::deleteSound()
{
    if(sound)
    {
        music->stop();
        delete music;
    }
}
