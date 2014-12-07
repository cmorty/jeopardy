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
#include "keyledcontrol.h"

struct settings_t Jeopardy::settings =  {false, false};

Jeopardy::Jeopardy(QWidget *parent) :
    QDialog(parent), players(),
    gameField(NULL)
{
    /* Load style File */
    QFile file("jeopardy.qss");
    file.open(QFile::ReadOnly);
    QString styleSheet = QLatin1String(file.readAll());
    this->setStyleSheet(styleSheet);

    for(int i = 0; i < 2 ; i++){
        KeyLedControl::setLed(i, false);
    }

    KeyLedControl * keyctrl = new KeyLedControl(this);
    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), keyctrl, SLOT(check()));
    timer->start(20);
}

Jeopardy::~Jeopardy()
{
    if(gameField != NULL)
    {
        delete gameField;
        gameField = NULL;
    }
}

void Jeopardy::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        break;
    default:
        break;
    }
}

void Jeopardy::init(QString folder)
{

   if(!loadRounds(folder)) close();
   initMenu();
   setFullscreen();
   setSound();
   initPlayers();
}

void Jeopardy::initMenu()
{
    grid = new QGridLayout();

    for(int i = 0;  i < rounds.length();  i++){
            Round * r = rounds[i];
            QPushButton * b = new QPushButton();
            b->setText(tr("Round %1").arg(r->getRoundNr()));
            b->setProperty("roundId", i);
            b->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
            grid->addWidget(b, 0, r->getRoundNr(), 0);
            connect(b, SIGNAL(clicked()), this, SLOT(initGameField()));
            buttons.append(b);
    }

    setLayout(grid);
    show();
}


void Jeopardy::initGameField()
{
    if(!players.length()){
        Phonon::MediaObject *music = NULL;
        if(Jeopardy::settings.sound)
        {
            music = Phonon::createPlayer(Phonon::NoCategory, Phonon::MediaSource("sound/title.ogg"));
            music->play();
        }
        initPlayers();
        if(music){
            music->stop();
            delete music;
        }
    }


    if(!players.length())
    {
        return;
    }

    QPushButton * button = qobject_cast<QPushButton *>(sender());
    button->setStyleSheet(QString("background-color: lightGray;"));

    int rid = (button->property("roundId")).toInt();

    Round * round = rounds[rid];


    gameField = new GameField(round, &players);
    gameField->init();

}

void Jeopardy::setSound()
{
    QMessageBox msgBox(this);
    msgBox.setText(tr("Do you need sound?"));
    msgBox.setWindowTitle(tr("Sound"));
    msgBox.setIcon(QMessageBox::Question);
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::Yes);

    if(msgBox.exec() == QMessageBox::Yes)
        Jeopardy::settings.sound = true;
    else
        Jeopardy::settings.sound = false;
}



void Jeopardy::setFullscreen()
{
    QMessageBox msgBox(this);
    msgBox.setText(tr("Do you want full screen mode?"));
    msgBox.setWindowTitle(tr("Full screen"));
    msgBox.setIcon(QMessageBox::Question);
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::Yes);

    if(msgBox.exec() == QMessageBox::Yes)
        Jeopardy::settings.fullscreen = true;
    else
        Jeopardy::settings.fullscreen = false;
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

    //Remove old players
    foreach(Player *p, players){
        delete p;
    }
    players.clear();


    for(int playerNr = 0;; playerNr++)
    {
        playerName = QString(tr("Player %1")).arg(playerNr + 1);
        int dialogcode;

        for(;;)
        {
            QInputDialog playerInput(this);

            if(playerNr > 0)
                playerInput.setCancelButtonText(tr("Play"));
            else
                playerInput.setCancelButtonText(tr("Cancel"));

            playerInput.setLabelText(tr("Enter name"));

            playerInput.setOkButtonText(tr("Create player"));
            dialogcode = playerInput.exec();
            text = playerInput.textValue();

            if(text.length() < 10)
                break;

            QMessageBox msgBox(this);
            msgBox.setText(tr("Choose a name shorter than 11 letters"));
            msgBox.exec();
        }

        if(text.isEmpty() || dialogcode == 0)
            break;

        key = QInputDialog::getItem(this, tr("Choose key"), tr("Choose key:"), keyList, 0, false, &ok);
        if(!ok)
            break;

        keyList.removeOne(key);

        color = QInputDialog::getItem(this, tr("Choose color "), tr("Color:"), colorList, 0, false, &ok);
        if(!ok)
            break;

        Player *p = new Player();

        p->setName(text);
        p->setPressed(0);
        p->setKey(key.at(0).toAscii());
        p->setColor(color);
        colorList.removeOne(color);
        p->setPoints(0);
        players.append(p);
    }

    return (players.length() > 0) ? true : false;
}



