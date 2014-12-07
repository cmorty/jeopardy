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

#include "answer.h"
#include "keyledcontrol.h"
#include <QGraphicsView>
#include <QGraphicsPixmapItem>
#include <QTest>
#include <jeopardy.h>

#define NOTIMEOUT 1

Answer::Answer(const QString &ans, QList<Player *> *players, Player * currentPlayer, int round, QWidget *parent = NULL) :
        QDialog(parent), answer(ans), round(round), points(0), winner(NULL), keyLock(false),
        doubleJeopardy(false), result(), players(players), currentPlayer(currentPlayer), music(NULL),  dj(NULL), videoPlayer(NULL),
        pic(NULL)
{
    //Setup Window
    this->setWindowFlags(Qt::Window);
    this->showMaximized();
    if(Jeopardy::settings.fullscreen) this->showFullScreen(); //For Windows

    QFile file("answer.qss");
    file.open(QFile::ReadOnly);
    QString styleSheet = QLatin1String(file.readAll());
    this->setStyleSheet(styleSheet);


    //Migrate
    this->setCursor(QCursor(Qt::WhatsThisCursor));
    QIcon icon;
    icon.addFile(QString::fromUtf8("Initialize"), QSize(), QIcon::Normal, QIcon::Off);
    this->setWindowIcon(icon);

    mainLayout = new QVBoxLayout(this);


    //Player-Bar - time
    QHBoxLayout * playerLayout = new QHBoxLayout();
    uiPlayer = new QLabel(this);
    playerLayout->addWidget(uiPlayer);
    playerLayout->addStretch();
    uiTime = new QLabel(this);
    playerLayout->addWidget(uiTime);
    mainLayout->addLayout(playerLayout);



    QHBoxLayout *buttonLayout = new QHBoxLayout();
    uiRight = new QPushButton(tr("Right"), this);
    connect(uiRight, SIGNAL(clicked()), this, SLOT(on_uiRight_clicked()));
    buttonLayout->addWidget(uiRight);
    uiWrong = new QPushButton(tr("Wrong"), this);
    connect(uiWrong, SIGNAL(clicked()), this, SLOT(on_uiWrong_clicked()));
    buttonLayout->addWidget(uiWrong);
    uiCancel = new QPushButton(tr("Cancel"), this);
    connect(uiCancel, SIGNAL(clicked()), this, SLOT(on_uiCancel_clicked()));
    buttonLayout->addWidget(uiCancel);
    buttonLayout->addStretch();
    uiEnd = new QPushButton(tr("End"), this);
    connect(uiEnd, SIGNAL(clicked()), this, SLOT(on_uiEnd_clicked()));
    buttonLayout->addWidget(uiEnd);
    mainLayout->addLayout(buttonLayout);
    //this->setLayout(mainLayout);



    hideButtons();

    isVideo = false;

    points = points;


    QRegExp comment("##.+##");
    QRegExp imgTag("^[[]img[]]");
    QRegExp videoTag("^[[]video[]]");
    QRegExp soundTag("^[[]sound[]]");
    QRegExp doubleJeopardyTag("[[]dj[]]");
    QRegExp lineBreakTag("[[]b[]]");

    QRegExp space("[[]s[]]");

    answer.remove(comment);
    answer.replace(lineBreakTag,"<br>");
    answer.replace(space, "&nbsp;");


    if(answer.contains(doubleJeopardyTag)){
        processDoubleJeopardy(&answer);
    }


    if(answer.contains(imgTag))
    {
        if(music) music->play();
        answer.remove(imgTag);
        answer = answer.trimmed();
        processImg(&answer);
    }
    else if(answer.contains(soundTag))
    {
        answer.remove(soundTag);
        answer = answer.trimmed();
        processSound(&answer);
    }
    else if(answer.contains(videoTag))
    {
        answer.remove(videoTag);
        answer = answer.trimmed();
        processVideo(&answer);
    }
    else
    {
        if(music) music->play();

        processText(&answer);
    }
    //Setup timer
    time = new QTime();
    time->start();
    timer = new QTimer();
    timer->setInterval(1*1000);
    connect(timer, SIGNAL(timeout()), this, SLOT(updateTime()));
    updateTime();
    timer->start();


}

Answer::~Answer()
{
    if(music) delete music;
    if(pic) delete pic;
    delete time;
    delete timer;
}


void Answer::setAnswerWidget(QWidget * aw){
    aw->setObjectName("answer");
    mainLayout->insertWidget(0,aw);
    QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    aw->setSizePolicy(sizePolicy);
}




void Answer::updateTime()
{
    int seconds = 31 - time->elapsed() / 1000;
    if(seconds >= 0)
        uiTime->setText(QString("%1").arg(seconds, 2));
    else
        uiTime->setText(QString("Ended..."));
}

Player * Answer::getWinner()
{
    return winner;
}

int Answer::getPoints()
{
    return points;
}

QList<struct result_t> Answer::getResult()
{
    return result;
}


void Answer::processDoubleJeopardy(QString *answer)
{
    QRegExp doubleJeopardyTag("[[]dj[]]");
    answer->remove(doubleJeopardyTag);
    openDoubleJeopardy();
}

void Answer::processImg(QString *answer)
{
    prependDir(answer);
    if(Jeopardy::settings.sound){
        music = Phonon::createPlayer(Phonon::NoCategory, Phonon::MediaSource("sound/jeopardy.wav"));
        music->play();
    }
    QGraphicsScene *scene = new QGraphicsScene(this);
    gv = new QGraphicsView(this);
    gv->setScene(scene);
    setAnswerWidget(gv);

    pic = new QPixmap(*answer);
    gpi = scene->addPixmap(*pic);
    QTimer::singleShot(200, this, SLOT(fitImg()));
}

void Answer::fitImg(){
    gv->fitInView(gpi, Qt::KeepAspectRatio);
}

void Answer::processSound(QString *answer)
{
    QLabel *lbl = new QLabel(tr("Listen..."), this);
    setAnswerWidget(lbl);
    prependDir(answer);
    music = Phonon::createPlayer(Phonon::NoCategory, Phonon::MediaSource(*answer));
    music->play();
    QTimer::singleShot(30000, music, SLOT(stop()));
}

void Answer::processVideo(QString *answer)
{
    videoPlayer = new Phonon::VideoPlayer(this);
    setAnswerWidget(videoPlayer);
    isVideo = true;
    prependDir(answer);
    videoPlayer->play(*answer);
    connect(videoPlayer, SIGNAL(finished()), this, SLOT(resetVid()));
    QTimer::singleShot(30000, videoPlayer, SLOT(stop()));
}

void Answer::resetVid(){
    videoPlayer->stop();
    videoPlayer->play();
}

void Answer::processText(QString *answer)
{
    QRegExp noEscape("[[]nE[]]");
    QRegExp alignLeftTag("[[]l[]]");

    QLabel *lbl = new QLabel(this);
    if(Jeopardy::settings.sound){
        music = Phonon::createPlayer(Phonon::NoCategory, Phonon::MediaSource("sound/jeopardy.wav"));
        music->play();
    }
    if(answer->contains(noEscape))
    {
        answer->remove(noEscape);
        lbl->setTextFormat(Qt::PlainText);
    }
    if(answer->contains(alignLeftTag)){
        answer->remove(alignLeftTag);
        lbl->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    }


    setAnswerWidget(lbl);
    int count = answer->count("<br>");
    lbl->setFont(measureFontSize(count));
    lbl->setWordWrap(true);
    answer->prepend("<p style=\"line-height:200\">");
    answer->append("</p>");
    lbl->setText(*answer);
}

void Answer::prependDir(QString *answer)
{
    answer->prepend(QString("/answers/%1/").arg(round));
    answer->prepend(QDir::currentPath());
}

void Answer::keyPressEvent(QKeyEvent *event)
{
    int key;
    Player * player = NULL;

    if(event->key() == Qt::Key_Shift)
    {
        if(videoPlayer)
        {
            videoPlayer->stop();
            videoPlayer->seek(0);
            QTimer::singleShot(100, videoPlayer, SLOT(play()));
            QTimer::singleShot(30000, videoPlayer, SLOT(stop()));
        }
        else if(music)
        {
            music->stop();
            QTimer::singleShot(100, music, SLOT(play()));
            QTimer::singleShot(30000, music, SLOT(stop()));
        }

        time->start();
    }

    if(event->key() == Qt::Key_Escape)
        on_uiEnd_clicked();

    if(uiCancel->isVisible()){
        if(event->key() == Qt::Key_F1) on_uiRight_clicked();
        if(event->key() == Qt::Key_F2) on_uiWrong_clicked();
        if(event->key() == Qt::Key_F3) on_uiCancel_clicked();
    }


    if(keyListenerIsLocked() == true)
        return;
    else
        lockKeyListener();

    key = event->key();

    foreach(Player *p, *players)
        if(key == p->getKey())
            player = p;

    if(player){
        processKeypress(player);

    } else {
        releaseKeyListener();
    }
}

void Answer::processKeypress(Player * player)
{
#if !NOTIMEOUT
    if(time->elapsed() < time->msec() + 31000)
    {
#endif
        currentPlayer = player;
        uiPlayer->setText(player->getName());

        showButtons();
#if !NOTIMEOUT
    }
#endif
}

bool Answer::keyListenerIsLocked()
{
    return keyLock;
}

void Answer::lockKeyListener()
{
    keyLock = true;
}

void Answer::releaseKeyListener()
{
    keyLock = false;
}

void Answer::showButtons()
{
    /* Show by color and name which player should ask */
    QString color = QString("QLabel { background-color : %1; }").arg(currentPlayer->getColor());
    uiPlayer->setStyleSheet(color);

    uiCancel->setVisible(true);
    uiRight->setVisible(true);
    uiWrong->setVisible(true);
    uiPlayer->setVisible(true);
    if(videoPlayer) videoPlayer->pause();
    if(music) music->pause();

}

void Answer::hideButtons()
{
    uiCancel->setVisible(false);
    uiRight->setVisible(false);
    uiWrong->setVisible(false);
    uiPlayer->setVisible(false);


    if(videoPlayer) videoPlayer->play();
    if(music) music->play();
}

QFont Answer::measureFontSize(int count)
{
    QFont font;

    if(count > MANY_LINE_BREAKS)
        font.setPointSize(7);
    else if(count > MORE_LINE_BREAKS)
        font.setPointSize(15);
    else if(count > SOME_LINE_BREAKS)
        font.setPointSize(20);
    else
        font.setPointSize(28);

    return font;
}




void Answer::openDoubleJeopardy()
{
    lockKeyListener();
    dj = new DoubleJeopardy(points / 2, points * 2, players, currentPlayer, this);
    dj->init();
    dj->show();
    currentPlayer = dj->getPlayer();
    points = dj->getPoints();
    doubleJeopardy = true;
    processKeypress(currentPlayer);
}

void Answer::cleanup(){
    hideButtons();
    if(music) music->stop();
    if(videoPlayer) videoPlayer->stop();
    QTest::qWait(500);
    done(0);
}

void Answer::on_uiEnd_clicked()
{
    releaseKeyListener();

    QMessageBox msgBox;
    msgBox.setText(tr("Are you sure?"));
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::Abort);
    msgBox.setDefaultButton(QMessageBox::Abort);
    int ret = msgBox.exec();

    if(ret == QMessageBox::Yes)
    {
        winner = NULL;
        cleanup();
    }
}

void Answer::on_uiRight_clicked()
{
    struct result_t resultTmp;
    resultTmp.player = currentPlayer;
    resultTmp.right = true;
    result.append(resultTmp);


    releaseKeyListener();
    winner = currentPlayer;
    cleanup();
}

void Answer::on_uiWrong_clicked()
{
    struct result_t resultTmp;
    resultTmp.player = currentPlayer;
    resultTmp.right = false;
    result.append(resultTmp);

    hideButtons();
    releaseKeyListener();
    if(doubleJeopardy)
    {
        winner = NULL;
        cleanup();
    }
}

void Answer::on_uiCancel_clicked()
{
    hideButtons();
    releaseKeyListener();
}
