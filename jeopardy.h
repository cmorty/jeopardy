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

#ifndef JEOPARDY_H
#define JEOPARDY_H

#include <QMainWindow>
#include <QInputDialog>
#include <QRegExp>
#include <QKeyEvent>
#include <QWidget>
#include <QString>
#include <phonon/mediaobject.h>
#include "gamefield.h"
#include "player.h"
#include "round.h"


namespace Ui {
    class Jeopardy;
}

class Jeopardy : public QDialog {
    Q_OBJECT
public:
    Jeopardy(QWidget *parent = NULL);
    ~Jeopardy();
    void init(QString folder);

protected:
    void changeEvent(QEvent *e);

private:
    int lastKey;
    int categoryNr;
    bool sound;
    bool defaultSetttings;
    QGridLayout *grid;
    QList<QPushButton *> buttons;
    QList<Round *> rounds;
    QList<Player *> players;
    Phonon::MediaObject *music;
    GameField *gameField;

    void initMenu();
    void setSound();
    bool loadRounds(QString folder);
    bool loadRound(const QString file);
    bool initPlayers();
    void startRound(int round);

    void deleteSound();

private slots:
    void initGameField();

signals:
    void deleteGameField();
};

#endif // JEOPARDY_H
