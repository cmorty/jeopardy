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
#include "ui_answer.h"

#define NOTIMEOUT 1

void Answer::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

Answer::Answer(const QString &ans, Player *players, int playerNr, bool sound, int currentPlayerId, int round, QWidget *parent = NULL) :
        QDialog(parent), ui(new Ui::Answer), answer(ans), round(round), playerNr(playerNr), points(0), currentPlayerId(currentPlayerId),
        winner(NO_WINNER), keyLock(false), sound(sound), doubleJeopardy(false), result(), players(players), currentPlayer(), dj(NULL)
{
    ui->setupUi(this);

    time = new QTime();
    time->start();
    timer = new QTimer();
    timer->setInterval(1*1000);
    connect(timer, SIGNAL(timeout()), this, SLOT(updateTime()));
    timer->start();

    hideButtons();
    ui->graphicsView->setVisible(false);
    ui->videoPlayer->setVisible(false);

    if(sound)
        music = Phonon::createPlayer(Phonon::NoCategory, Phonon::MediaSource("sound/jeopardy.wav"));

    isVideo = false;

    points = points;


    QRegExp comment("##.+##");
    QRegExp imgTag("^[[]img[]]");
    QRegExp videoTag("^[[]video[]]");
    QRegExp soundTag("^[[]sound[]]");
    QRegExp alignLeftTag("[[]l[]]");
    QRegExp doubleJeopardyTag("[[]dj[]]");
    QRegExp lineBreakTag("[[]b[]]");
    QRegExp noEscape("[[]nE[]]");
    QRegExp space("[[]s[]]");

    answer.remove(comment);
    answer.replace(lineBreakTag,"<br>");
    answer.replace(space, "&nbsp;");

    if(answer.contains(alignLeftTag))
        processAlign(&answer);

    if(answer.contains(noEscape))
    {
        answer.remove(noEscape);
        ui->answer->setTextFormat(Qt::PlainText);
    }

    if(answer.contains(doubleJeopardyTag))
        processDoubleJeopardy(&answer);

    if(answer.contains(imgTag))
    {
        if(sound)
            music->play();

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
        if(sound)
            music->play();

        processText(&answer);
    }


}

Answer::~Answer()
{
    delete ui;
    if(sound)
        delete music;

    if(dj != NULL)
        delete dj;

    delete time;
    delete timer;
}

void Answer::updateTime()
{
    int seconds = 31 - time->elapsed() / 1000;
    if(seconds >= 0)
        ui->time->setText(QString("%1").arg(seconds, 2));
    else
        ui->time->setText(QString("Ended..."));
}

int Answer::getWinner()
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


void Answer::processAlign(QString *answer)
{
    QRegExp alignLeftTag("[[]l[]]");
    answer->remove(alignLeftTag);
    ui->answer->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
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

    ui->graphicsView->setVisible(true);

    QGraphicsScene *scene = new QGraphicsScene(ui->graphicsView);
    QPixmap pic(*answer);

    if(pic.height() > ui->graphicsView->height())
        pic = pic.scaledToHeight(ui->graphicsView->height() - 10);

    if(pic.width() > ui->graphicsView->width())
        pic = pic.scaledToWidth(ui->graphicsView->width() - 10);

    scene->addPixmap(pic);
    ui->graphicsView->setScene(scene);
    ui->graphicsView->show();
}

void Answer::processSound(QString *answer)
{
    prependDir(answer);

    sound = true;
    music = Phonon::createPlayer(Phonon::NoCategory, Phonon::MediaSource(*answer));
    music->play();
    QTimer::singleShot(30000, music, SLOT(stop()));
}

void Answer::processVideo(QString *answer)
{
    isVideo = true;
    prependDir(answer);

    ui->videoPlayer->setVisible(true);
    ui->videoPlayer->play(*answer);
    QTimer::singleShot(30000, ui->videoPlayer, SLOT(stop()));
}

void Answer::processText(QString *answer)
{
    int count = answer->count("<br>");
    ui->answer->setFont(measureFontSize(count));
    ui->answer->setText(*answer);
}

void Answer::prependDir(QString *answer)
{
    answer->prepend(QString("/answers/%1/").arg(round));
    answer->prepend(QDir::currentPath());
}

void Answer::keyPressEvent(QKeyEvent *event)
{
    int key;
    int player = -1;

    if(sound && event->key() == Qt::Key_Shift)
    {
        if(isVideo == true)
        {
            ui->videoPlayer->stop();
            ui->videoPlayer->seek(0);
            QTimer::singleShot(100, ui->videoPlayer, SLOT(play()));
            QTimer::singleShot(30000, ui->videoPlayer, SLOT(stop()));
        }
        else
        {
            music->stop();
            QTimer::singleShot(100, music, SLOT(play()));
            QTimer::singleShot(30000, music, SLOT(stop()));
        }

        time->start();
    }

    if(event->key() == Qt::Key_Escape)
        on_buttonEnd_clicked();

    if(keyListenerIsLocked() == true)
        return;
    else
        lockKeyListener();

    key = event->key();

    for(int i = 0; i <  playerNr; i++)
        if(key == players[i].getKey())
            player = i;

    if(player != -1)
        processKeypress(player);
    else
        releaseKeyListener();
}

void Answer::processKeypress(int player)
{
#if NOTIMEOUT
    if(time->elapsed() < time->msec() + 31000)
    {
#endif
        currentPlayer = players[player];
        ui->currentPlayer->setText(currentPlayer.getName());

        showButtons();
#if NOTIMEOUT
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
    QString color = QString("QLabel { background-color : %1; }").arg(currentPlayer.getColor());
    ui->currentPlayer->setStyleSheet(color);

    ui->buttonCancel->setVisible(true);
    ui->buttonRight->setVisible(true);
    ui->buttonWrong->setVisible(true);
    ui->currentPlayer->setVisible(true);
}

void Answer::hideButtons()
{
    ui->buttonCancel->setVisible(false);
    ui->buttonRight->setVisible(false);
    ui->buttonWrong->setVisible(false);
    ui->currentPlayer->setVisible(false);
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
    dj = new DoubleJeopardy(this, points / 2, points * 2, players, playerNr, currentPlayerId);
    dj->init();
    dj->show();
    currentPlayerId = dj->getPlayer();
    points = dj->getPoints();
    doubleJeopardy = true;

    processKeypress(currentPlayerId);
}

void Answer::on_buttonEnd_clicked()
{
    releaseKeyListener();

    QMessageBox msgBox;
    msgBox.setText("Are you sure?");
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::Abort);
    msgBox.setDefaultButton(QMessageBox::Abort);
    int ret = msgBox.exec();

    if(ret == QMessageBox::Yes)
    {
        if(sound)
            music->stop();
        winner = NO_WINNER;
        done(0);
    }
}

void Answer::on_buttonRight_clicked()
{
    struct result_t resultTmp;
    resultTmp.player = currentPlayer.getId() - OFFSET;
    resultTmp.right = true;
    result.append(resultTmp);


    releaseKeyListener();
    if(sound)
        music->stop();
    winner = currentPlayer.getId() - OFFSET;
    done(0);
}

void Answer::on_buttonWrong_clicked()
{
    struct result_t resultTmp;
    resultTmp.player = currentPlayer.getId() - OFFSET;
    resultTmp.right = false;
    result.append(resultTmp);

    hideButtons();
    releaseKeyListener();
    if(doubleJeopardy)
    {
        if(sound)
            music->stop();
        winner = NO_WINNER;
        done(0);
    }
}

void Answer::on_buttonCancel_clicked()
{
    hideButtons();
    releaseKeyListener();
}
