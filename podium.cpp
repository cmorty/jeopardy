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

#include "podium.h"

Podium::Podium(const QList<Player *> players, QWidget *parent) :
    QDialog(parent), players(players)
{
}

Podium::~Podium()
{
    delete first;
    delete second;
    delete third;
    delete firstGrid;
    delete secondThirdGrid;
    delete window;
    delete filename;
}

void Podium::init()
{
    window = new QWidget();
    mainGrid = new QGridLayout;
    firstGrid = new QGridLayout();
    secondThirdGrid = new QGridLayout();
    first = new QLabel();
    second = new QLabel();
    third = new QLabel();

    initLabels();

    firstGrid->addWidget(first);
    mainGrid->addLayout(firstGrid, 0, 0, 0);
    mainGrid->addLayout(secondThirdGrid, 1, 0, 0);
    window->setLayout(mainGrid);
    window->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

void Podium::initLabels()
{
    first->setAlignment(Qt::AlignCenter);
    first->setFont(QFont("Helvetica [Cronyx]", 23, QFont::Bold, false));
    first->setMinimumSize(650, 200);
    first->setMaximumSize(650, 200);
    first->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    second->setAlignment(Qt::AlignCenter);
    second->setFont(QFont("Helvetica [Cronyx]", 17, -1, false));
    second->setMinimumSize(500, 150);
    second->setMaximumSize(500, 150);
    second->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    third->setAlignment(Qt::AlignCenter);
    third->setFont(QFont("Helvetica [Cronyx]", 12, -1, false));
    third->setMinimumSize(500, 150);
    third->setMaximumSize(500, 150);
    third->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

void Podium::showPodium()
{
    init();
    sort();

    first->setStyleSheet(getLabelColorString(0));
    if(players.length() == 1)
        first->setText(QString("Guess who won... :)"));
    else
        first->setText(QString("** %1 **<br>%2").arg(players[0]->getName()).arg(players[0]->getPoints()));

    if(players.length() > 1)
    {
        second->setStyleSheet(getLabelColorString(1));
        second->setText(QString("%1<br>%2").arg(players[1]->getName()).arg(players[1]->getPoints()));
        secondThirdGrid->addWidget(second, 1, 0, 0);
    }
    else
        second->setVisible(false);

    if(players.length() > 2)
    {
        third->setStyleSheet(getLabelColorString(2));
        third->setText(QString("%1<br>%2").arg(players[2]->getName()).arg(players[2]->getPoints()));
        secondThirdGrid->addWidget(third, 1, 1, 0);
    }
    else
    {
        second->setGeometry(40, 220, 662, 171);
        third->setVisible(false);
    }

    //if(players.length() > 1)
    //    saveScore();

    window->show();
}

void Podium::sort()
{
    qSort(players.begin(), players.end(), Player::compPoints);

}


QString Podium::getLabelColorString(int player)
{
    QString color;

    color = QString("QLabel { background-color : %1; }").arg(players[player]->getColor());

    return color;
}

bool Podium::setScorefile()
{
    filename = new QString("gameStates/score.jsf");
    file = new QFile(*filename);
    QDir dir;

    if (!file->open(QIODevice::ReadWrite | QIODevice::Text))
    {
        QMessageBox::critical(this, tr("Error"), tr("Could not open score file, please select one by yourself"));
        *filename = QFileDialog::getOpenFileName(this, tr("Open File"), "gameStates/", tr("Jeopardy Score File (*.jsf)"));
        *filename = dir.absoluteFilePath(*filename);
    }

    file = new QFile(*filename);
    if (!file->open(QIODevice::ReadWrite | QIODevice::Text))
    {
            QMessageBox::critical(this, tr("Error"), tr("Could not open score file"));
            return false;
    }

    return true;
}

void Podium::saveScore()
{
    int rounds = 0, total = 0, avg = 0;
    QStringList results;
    QString line, name;
    bool found;

    if(setScorefile() == false)
        return;

    QTextStream in(file);
    line = in.readLine();

    if(!line.startsWith("name     "))
        in << "name           rounds         total          avg            \n";


    for(int i = 0; i < players.length(); i++)
    {
            name = players[i]->getName();
            name = name.toLower();
            found = false;

            in.seek(0);
            // Do until file ends
            while(!in.atEnd())
            {
                    line = in.readLine();
                    // update player values
                    if(line.startsWith(name))
                    {
                            //name      rounds      total      avg
                            results = line.split("  ", QString::SkipEmptyParts);
                            rounds = results[1].toInt() + 1;
                            total = results[2].toInt() + players[i]->getPoints();
                            avg = ceil(total / rounds);
                            found = true;
                    }
            }

            // save new player's score
            if(!found)
            {
                    rounds = 1;
                    avg = total = players[i]->getPoints();
            }

            writeScore(name, rounds, total, avg);
    }
}

void Podium::writeScore(QString name, int rounds, int total, int avg)
{
    // open file and save score
    QTextStream out(file);
    out.seek(file->size());
    out << qSetFieldWidth(15)
        << left
        << QString("%1").arg(name)
        << QString("%1").arg(rounds)
        << QString("%1").arg(total)
        << QString("%1").arg(avg)
        << qSetFieldWidth(0)
        << '\n';
}
