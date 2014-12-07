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

#ifndef ANSWER_H
#define ANSWER_H

#include <QKeyEvent>
#include <QSound>
#include <QFile>
#include <QDir>
#include <QPixmap>
#include <QTimer>
#include <QTime>
#include <QGraphicsScene>
#include <phonon/mediaobject.h>
#include <phonon/videoplayer.h>
#include <doublejeopardy.h>

#define SOME_LINE_BREAKS 10
#define MORE_LINE_BREAKS 15
#define MANY_LINE_BREAKS 20


struct result_t {
    Player * player;
    bool right;
};


class Answer : public QDialog {
    Q_OBJECT
public:
    Answer(const QString &answer, QList<Player *> *players, Player * currentPlayer, int round, QWidget *parent);
    ~Answer();
    int getPoints();
    QList<result_t> getResult();
    Player *getWinner();

private:
    QString answer;
    int round;
    int points;
    Player * winner;
    unsigned int timeStarted;
    bool keyLock;
    bool isVideo;
    bool doubleJeopardy;
    QList<struct result_t> result;
    QTime *time;
    QTimer *timer;
    QList<Player *> *players;
    Player *currentPlayer;
    Phonon::MediaObject *music;
    DoubleJeopardy *dj;

    QBoxLayout *mainLayout;
    QLabel *uiPlayer;
    QLabel *uiTime;
    QPushButton *uiRight;
    QPushButton *uiWrong;
    QPushButton *uiCancel;
    QPushButton *uiEnd;
    QWidget *uiAWidget;
    Phonon::VideoPlayer *videoPlayer;
    QGraphicsPixmapItem *gpi;
    QGraphicsView *gv;

    QPixmap * pic;

    void keyPressEvent(QKeyEvent *event);
    void processKeypress(Player * player);
    bool keyListenerIsLocked();
    void lockKeyListener();
    void releaseKeyListener();

    void showButtons();
    void hideButtons();
    void cleanup();

    QString getRoundFile();
    QFont measureFontSize(int count);

    bool getAnswer(int category, int points, QString *answer);
    void openDoubleJeopardy();

    void setAnswerWidget(QWidget * aw);

    void processText(QString *answer);
    void processImg(QString *answer);
    void processSound(QString *answer);
    void processVideo(QString *answer);
    void processDoubleJeopardy(QString *answer);

    void prependDir(QString *answer);


private slots:
    void on_uiCancel_clicked();
    void on_uiWrong_clicked();
    void on_uiRight_clicked();
    void on_uiEnd_clicked();
    void updateTime();
    void fitImg();
    void resetVid();
};

#endif // ANSWER_H
