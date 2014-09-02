/*
 * Copyright (c) 2011-2014, Christian Lange
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

#ifndef GAMEFIELD_H
#define GAMEFIELD_H

#ifdef _WIN32
    #include <time.h>
#endif

#include <stdlib.h>
#include <QDir>
#include <QMenu>
#include <QAction>
#include <QLabel>
#include <QFile>
#include <QColor>
#include <QTimer>
#include <QFileDialog>
#include <QMessageBox>
#include <QShortcut>
#include <QDateTime>
#include <QGridLayout>
#include <editor.h>
#include <podium.h>
#include <answer.h>
#include "round.h"

#define WON "1"
#define NUMBER_ANSWERS 5
#define NUMBER_MAX_CATEGORIES 6
#define NUMBER_ANSWERS 5
#define NUMBER_MAX_ANSWERS ((NUMBER_MAX_CATEGORIES) * (NUMBER_ANSWERS))
#define POINTS_FACTOR 100
#define COLOR_TEXT_LENGTH 33
#define PLAYER_INDICATOR 1
#define RESULT_INDICATOR 1
#define NOT false
#define NO_WINNER -1

#define GAMEFIELD_WIDTH 1000
#define GAMEFIELD_HEIGHT 500
#define FIRST_LABEL_ROW 7
#define CATEGORY_LABEL_HEIGHT 30
#define NAME_LABEL_HEIGHT 20
#define SPLIT_FOR_TWO_LABELS 2
#define OFFSET 1

class GameField : public QDialog {
    Q_OBJECT
public:
    GameField(Round * round, QList<Player *> *players, bool sound = true, QWidget *parent = NULL);
    ~GameField();
    void init();

protected:
    void changeEvent(QEvent *e);

private:
    Round * round;
    int alreadyAnswered;
    Player * lastWinner;
    Player * currentPlayer;
    int lastPoints;
    bool sound;
    QList<Player *> *players;
    Editor *editor;
    Answer *answer;
    Podium *podium;
    QGridLayout mainGrid;
    QGridLayout categoryLabelGrid;
    QGridLayout buttonGrid;
    QGridLayout playerLabelGrid;
    QAction *randomCtx;
    QAction *editorCtx;
    QAction *loadCtx;
    QAction *saveCtx;
    QAction *endRoundCtx;
    QAction *resetRoundCtx;
    QAction *about;
    QList<QPushButton *> buttons;
    QList<QLabel *> playerNameLabels;
    QList<QLabel *> playerPointsLabels;
    QList<QLabel *> categoryLabels;


    void insertLayouts();
    void assignButtons();
    void setDefaultButtonAppearance(QPushButton *button);
    void assignPlayerNameLabels();
    void assignPlayerPointsLabels();
    void setLabelColor();
    void setPoints();
    void setNames();
    void setCurrentPlayer(Player *p);

    void updateGameFieldValues();

    QString getButtonColorByLastWinner();

    void showPodium();

    void openFileLoader();
    void openFileSaver(bool automatedBackup);
    void openEditor();
    void resetRound();

    bool eventFilter(QObject *target, QEvent *event);
    void indicateRandom();

private slots:
    /* Context Menu */
    void on_gameField_customContextMenuRequested(QPoint pos);
    Player * randomPlayer();
    void updatePointsLabels();
    void on_button_clicked();
    void updateNamesLabels();
};

#endif // GAMEFIELD_H
