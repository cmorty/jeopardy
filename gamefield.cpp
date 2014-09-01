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

#include "gamefield.h"

GameField::GameField(Round * round_, Player *players, int playerNr, bool sound, QWidget *parent) :
    QDialog(parent), round(round_), alreadyAnswered(0), lastWinner(NO_WINNER),
    lastPoints(0), playerNr(playerNr), sound(sound), players(players), answer(), podium(NULL),
    window(), mainGrid(), categoryLabelGrid(), buttonGrid(), playerLabelGrid(),
    randomCtx(NULL), editorCtx(NULL), loadCtx(NULL), saveCtx(NULL), endRoundCtx(NULL), about(NULL)

{
    //Init
    currentPlayer = random();

    //Setup Window
    this->setWindowFlags(Qt::Window);
    this->showMaximized();
    this->showFullScreen(); //For Windows

    /* Load style File */
    QFile file("gamefield.qss");
    file.open(QFile::ReadOnly);
    QString styleSheet = QLatin1String(file.readAll());
    this->setStyleSheet(styleSheet);


    mainGrid.setSpacing(0);

    categoryLabelGrid.setSpacing(0);

    buttonGrid.setSpacing(0);

    playerLabelGrid.setSpacing(0);

    mainGrid.addLayout(&categoryLabelGrid, 0, 0);
    mainGrid.addLayout(&buttonGrid, 1, 0);
    mainGrid.addLayout(&playerLabelGrid, 2, 0);

    mainGrid.setGeometry(QRect(0, 0, GAMEFIELD_WIDTH, GAMEFIELD_HEIGHT));
    categoryLabelGrid.setGeometry(QRect(0, 0, GAMEFIELD_WIDTH, CATEGORY_LABEL_HEIGHT));
    buttonGrid.setGeometry(QRect(0, CATEGORY_LABEL_HEIGHT, GAMEFIELD_WIDTH, GAMEFIELD_HEIGHT - CATEGORY_LABEL_HEIGHT - NAME_LABEL_HEIGHT - NAME_LABEL_HEIGHT));
    playerLabelGrid.setGeometry(QRect(0, GAMEFIELD_HEIGHT - NAME_LABEL_HEIGHT - NAME_LABEL_HEIGHT, GAMEFIELD_WIDTH, NAME_LABEL_HEIGHT + NAME_LABEL_HEIGHT));

    window.installEventFilter(this);
    window.setLayout(&mainGrid);

    //Walk round

    QList<Category *> cats = round->getCategories();
    for(int xpos = 0; xpos < cats.length(); xpos++){
        Category * cat = cats.at(xpos);
        QLabel *lab = new QLabel(&window);

        lab->setText(cat->getName());
        lab->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        lab->setProperty("iscategory", true);
        lab->setProperty("category", xpos + 1);

        categoryLabels.append(lab);
        categoryLabelGrid.addWidget(lab, 0, xpos);

        QList<answer_t *> ans = cat->getAnswers();
        for(int ypos = 0; ypos < ans.length(); ypos++){
            answer_t * a = ans[ypos];

            int currentButton = (NUMBER_MAX_CATEGORIES * ypos) + xpos;

            QPushButton * b = new QPushButton();

            b->installEventFilter(this);
            b->setProperty("ansPoints", a->points);
            b->setProperty("ansAnswer", *(a->answer));
            b->setProperty("ansCategory", xpos + 1); //Off by one....

            setDefaultButtonAppearance(b);
            categoryLabelGrid.addWidget(b, ypos + 1, xpos);

            buttons[currentButton] = b;
            connect(b, SIGNAL(clicked()), this, SLOT(on_button_clicked()));
        }

    }


    assignPlayerNameLabels();
    assignPlayerPointsLabels();

    setNames();
    setPoints();
    setLabelColor();

    /* Load style File */
    QFile file("gamefield.qss");
    file.open(QFile::ReadOnly);
    QString styleSheet = QLatin1String(file.readAll());
    window.setStyleSheet(styleSheet);

    /* Declare new context menu and connect it with the right mouse button */
    window.setContextMenuPolicy(Qt::CustomContextMenu);
    connect(&window, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(on_gameField_customContextMenuRequested(QPoint)));
}

GameField::~GameField()
{
    if(randomCtx != NULL)
        delete randomCtx;
    if(editorCtx != NULL)
        delete editorCtx;
    if(loadCtx != NULL)
        delete loadCtx;
    if(saveCtx != NULL)
        delete saveCtx;
    if(endRoundCtx != NULL)
        delete endRoundCtx;
    if(podium != NULL)
        delete podium;
    if(about != NULL)
        delete about;

    foreach(QLabel *l, playerNameLabels){
        delete l;
    }

    foreach(QLabel *l, playerPointsLabels){
        delete l;
    }

    foreach(QLabel *l, categoryLabels){
        delete l;
    }

    if(round) delete round;

}

void GameField::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        break;
    default:
        break;
    }
}

void GameField::init()
{
    window.show();
}


/*
 * Probably usind a QSignalMapper is the cleaner solution, but the current
 * is a major improvement....
 */

void GameField::on_button_clicked()
{
    QPushButton * button = qobject_cast<QPushButton *>(sender());
    button->setDisabled(true);
    int points = (button->property("ansPoints")).toInt();
    QString answer = (button->property("ansAnswer")).toString();


    //Create UI;
    Answer ui(answer, players, playerNr, sound, currentPlayer, round->getRoundNr(), this);
    ui.exec();

    button->setText("");
    currentPlayer = lastWinner = ui.getWinner();
    QList<struct result_t> result = ui.getResult();

    /* Write player name on button */
    if(lastWinner != NO_WINNER)
    {
        button->setStyleSheet(getButtonColorByLastWinner());
        button->setText(players[lastWinner].getName());
    }
    else
    {
        currentPlayer = random();
    }

    //Handle Result
    foreach(struct result_t r, result){
        if(r.right){
            players[r.player].incPoints(points);
        } else {
            players[r.player].decPoints(points);
        }
    }

    alreadyAnswered += 1;

    //Mark active player
    updateNamesLabels();
    updatePointsLabels();

    if(alreadyAnswered < round->getAnswerCount())
    {
        /* Do backup after each answer */
        openFileSaver(true);
    }
    else
    {
        showPodium();
        window.close();
    }

}



void GameField::setDefaultButtonAppearance(QPushButton *button)
{
    int points = (button->property("ansPoints")).toInt();
    button->setText(QString("%1").arg(points));
    button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    button->setStyleSheet("");
    button->setEnabled(true);
}

void GameField::assignPlayerNameLabels()
{
    int row, column, width, height;


    height = NAME_LABEL_HEIGHT;

    for(int i = 0; i < playerNr; i++)
    {
        if((i + 1) <= NUMBER_MAX_PLAYERS / 2)
        {
            row = FIRST_LABEL_ROW;
            column = i * 2;
            width = GAMEFIELD_WIDTH / playerNr / SPLIT_FOR_TWO_LABELS;
        }
        else
        {
            row = FIRST_LABEL_ROW + 1;
            column = (i - NUMBER_MAX_PLAYERS / 2) * 2;
            width = GAMEFIELD_WIDTH / (NUMBER_MAX_PLAYERS / 2) / SPLIT_FOR_TWO_LABELS;
        }
        
        QLabel *lbl = new QLabel();
        lbl->setProperty("isname", true);
        lbl->setGeometry(0, 0, width, height);
        lbl->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        playerLabelGrid.addWidget(lbl, row, column);
        playerNameLabels.append(lbl);
    }
}

void GameField::assignPlayerPointsLabels()
{
    int row, column, width, height;

    for(int i = 0; i < NUMBER_MAX_PLAYERS; i++)


    height = NAME_LABEL_HEIGHT;

    for(int i = 0; i < playerNr; i++)
    {
        QLabel *lbl = new QLabel();

        playerPointsLabels.append(lbl);

        if((i + 1) <= NUMBER_MAX_PLAYERS / 2)
        {
            row = FIRST_LABEL_ROW;
            column = 2 * i + 1;
            width = GAMEFIELD_WIDTH / playerNr / SPLIT_FOR_TWO_LABELS;
        }
        else
        {
            row = FIRST_LABEL_ROW + 1;
            column = 2 * (i - NUMBER_MAX_PLAYERS / 2) + 1;
            width = GAMEFIELD_WIDTH / (NUMBER_MAX_PLAYERS / 2) / SPLIT_FOR_TWO_LABELS;
        }
        
        lbl->setProperty("ispoints", true);
        lbl->setGeometry(0, 0, width, height);
        lbl->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        playerLabelGrid.addWidget(lbl, row, column);
    }
}



void GameField::setLabelColor()
{
    QString color;

    for(int i = 0; i < playerNr; i++)
    {
        color = QString("* { background-color : %1; color: black;}").arg(players[i].getColor());
        playerNameLabels[i]->setStyleSheet(color);
    }
}

void GameField::setPoints()
{
    foreach(QLabel *l,  playerPointsLabels){
        l->setText("0");
    }
}

void GameField::setNames()
{
    for(int i = 0; i < playerNr; i++)
    {
        if(currentPlayer == i)
            playerNameLabels[i]->setText(QString("%1 ***").arg(players[i].getName()));
        else
            playerNameLabels[i]->setText(players[i].getName());

        QString color =  QString("* { background-color : %1; color: black;}").arg(players[i].getColor());
        playerNameLabels[i]->setStyleSheet(color);
    }
}

void GameField::updateGameFieldValues()
{
    updatePointsLabels();
    updateNamesLabels();
    setLabelColor();
}

void GameField::updatePointsLabels()
{
    for(int i = 0; i < playerNr; i++)
    {
        playerPointsLabels[i]->setText(QString::number(players[i].getPoints()));
    }
}





QString GameField::getButtonColorByLastWinner()
{
    return QString("QPushButton { background-color : %1; color : black; }").arg(players[lastWinner].getColor());
}


/*TODO Reenable*/

void GameField::openFileLoader()
{
#if 0

    int lineNr = 0;
    QDir dir;
    QString fileName = QFileDialog::getOpenFileName(this, "Open File", "gameStates/", "Jeopardy Game State (*.jgs)");

    if(fileName == "")
        return;

    fileName = dir.absoluteFilePath(fileName);

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly))
    {
        QMessageBox::critical(this, tr("Error"), tr("Could not open file"));
        return;
    }

    delete window;

    QTextStream in(&file);

    QString line = in.readLine();

    for(int j = 0; j < NUMBER_MAX_PLAYERS; j++)
    {
        players[j].setName(line);
        line = in.readLine();
        lineNr++;

        players[j].setPoints(line.toInt());
        line = in.readLine();
        lineNr++;

        players[j].setColor(line);
        line = in.readLine();
        lineNr++;

        players[j].setKey(line.toInt());
        line = in.readLine();
        lineNr++;
    }

    alreadyAnswered = line.toInt();
    line = in.readLine();
    lineNr++;

    playerNr = line.toInt();
    line = in.readLine();
    lineNr++;

    int categoryNr = line.toInt();
    line = in.readLine();
    lineNr++;

    init();

    /* Already questioned answers */
    for(int i = 0; i < NUMBER_MAX_ANSWERS; i++)
    {
        if(line.toInt() == 1)
            buttons[i]->setText("");

        buttons[i]->setDisabled(line.toInt());

        line = in.readLine();
        lineNr++;
    }

    /* Color buttons with player color */
    for(int i = 0; i < NUMBER_MAX_ANSWERS; i++)
    {
        if(line == "r")
            line = "red";
        else if (line == "g")
            line = "green";
        else if(line == "y")
            line = "yellow";
        else if(line == "b")
            line = "blue";
        else if(line == "gr")
            line = "gray";
        else if(line == "m")
            line = "magenta";
        else if(line == "c")
            line = "cyan";
        else if(line == "d")
            line = "darkRed";
        else if(line == "w")
            line = "white";
        else if(line == "dM")
            line = "darkMagenta";
        else
            line = "";

        if(line != "")
            for(int j = 0; j < playerNr; j++)
                if(players[j].getColor() == line)
                    buttons[i]->setText(players[j].getName());

        line.prepend("QPushButton { background-color : ");
        line.append("; }");

        buttons[i]->setStyleSheet(line);

        line = in.readLine();
        lineNr++;
    }

    updateGameFieldValues();

    file.close();
#endif
}

/*
 * Fixme: This will only work with standard fields
 */
void GameField::openFileSaver(bool backup)
{
#if 0
    QDir dir;
    QString fileName;
    QDateTime dateTime;

    if(backup == true)
        fileName = QString("gameStates/backups/backup_%1_%2").arg(round.getRoundNr()).arg(dateTime.currentDateTime().toTime_t());
    else
        fileName = QFileDialog::getSaveFileName(this, tr("Save File"), "gameStates/", tr("Jeopardy Game State (*.jgs)"));

    if(NOT == fileName.endsWith(".jgs"))
        fileName.append(".jgs");

    fileName = dir.absoluteFilePath(fileName);

    if (fileName != "")
    {
      QFile file(fileName);
      if (!file.open(QIODevice::WriteOnly))
      {
        QMessageBox::critical(this, tr("Error"), tr("File not writeable"));
        return;
      }
      else
      {
        QTextStream stream(&file);

        for(int i = 0; i < NUMBER_MAX_PLAYERS; i++)
        {
            stream << players[i].getName() << '\n';
            stream << players[i].getPoints() << '\n';
            stream << players[i].getColor() << '\n';
            stream << players[i].getKey() << '\n';
        }

        stream << alreadyAnswered << '\n';
        stream << playerNr << '\n';
        stream << round.getCategories().length() << '\n';

        for(int i = 0; i < NUMBER_MAX_ANSWERS; i++)
            stream << !buttons[i]->isEnabled() << '\n';

        for(int i = 0; i < NUMBER_MAX_ANSWERS; i++)
        {
            /* Just save first character of color */
            QString stylesheet = buttons[i]->styleSheet();
            stylesheet.remove(0,COLOR_TEXT_LENGTH);

            if(stylesheet.startsWith("gray"))
                stylesheet = "gr";
            else if(stylesheet.startsWith("darkMagenta"))
                stylesheet = "dM";
            else
            {
                /* chop string expect for first character */
                int len = stylesheet.length();
                stylesheet.chop(--len);
            }

            stream << stylesheet << '\n';
        }

        stream.flush();
        file.close();
        }
    }
#else
        if(backup) return; //Avoid warning
#endif
}




void GameField::openEditor()
{
    editor = new Editor(this, players, playerNr);
    editor->show();

    openFileSaver(true);
    updateGameFieldValues();
}

int GameField::random()
{
    return rand() % playerNr;
}

void GameField::resetRound()
{
    alreadyAnswered = 0;
    for(int i = 0; i < playerNr; i++)
        players[i].setPoints(0);

    for(int j = 0; j < NUMBER_MAX_ANSWERS; j++)
    {
        setDefaultButtonAppearance(buttons[j]);
    }

    updatePointsLabels();
}

void GameField::on_gameField_customContextMenuRequested(QPoint pos)
{
    QPoint globalPos = window.mapToGlobal(pos);

    QMenu menu;
    randomCtx = new QAction("Random Generator", this);
    editorCtx = new QAction("Editor",this);
    loadCtx = new QAction("Load",this);
    saveCtx = new QAction("Save",this);
    endRoundCtx = new QAction("End Round", this);
    resetRoundCtx = new QAction("Reset Round", this);
    about = new QAction("About", this);

    menu.addAction(randomCtx);
    menu.addSeparator();
    menu.addAction(editorCtx);
    menu.addSeparator();
    menu.addAction(loadCtx);
    menu.addAction(saveCtx);
    menu.addSeparator();
    menu.addAction(endRoundCtx);
    menu.addSeparator();
    menu.addAction(resetRoundCtx);
    menu.addSeparator();
    menu.addAction(about);

    QAction *selectedItem = menu.exec(globalPos);

    if(selectedItem == randomCtx)
    {
        updateNamesLabels();
        currentPlayer = random();
        setNames();
    }
    else if(selectedItem == editorCtx)
        openEditor();
    else if(selectedItem == saveCtx)
        openFileSaver(false);
    else if(selectedItem == loadCtx)
        openFileLoader();
    else if(selectedItem == endRoundCtx)
    {
        showPodium();
        window.close();
    }
    else if(selectedItem == resetRoundCtx)
        resetRound();
    else if(selectedItem == about)
    {
        QMessageBox msgBox;
        msgBox.setText("Jeopardy!\n\nWritten by Christian Lange.\nChristian_Lange@hotmail.com\n\nwww.github.com/chlange/jeopardy\nwww.ganz-sicher.net/chlange");
        msgBox.exec();
    }
}

void GameField::showPodium()
{
    podium = new Podium(this, players, playerNr);
    podium->showPodium();
}

bool GameField::eventFilter(QObject *target, QEvent *event)
{
    if(target == &window && event->type() == QEvent::KeyPress)
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);

        /* Open random user picker if "r" gets pressed */
        if(keyEvent->key() == Qt::Key_R)
        {
            indicateRandom();
            updateNamesLabels();
            currentPlayer = random();
            setNames();
        }

        for(int i = 0; i < playerNr; i++)
        {
            /* Indicate key press of player over label */
            if(players[i].getKey() == keyEvent->key())
            {
                playerNameLabels[i]->setStyleSheet(QString("background-color: black; color: white;"));
                playerNameLabels[i]->setText(QString("%1 - it works").arg(players[i].getName()));
                QTimer::singleShot(200, this, SLOT(updateNamesLabels()));

                if(players[i].getPressed() > 13)
                {
                    players[i].decPoints(50);
                    QTimer::singleShot(200, this, SLOT(updatePointsLabels()));
                    QMessageBox::critical(this, tr("Error"),
                                         QString("%1 - That's enough - 50 points subtracted").arg(players[i].getName()));
                }
                else if(players[i].getPressed() > 10)
                {
                    int untilSub = 13 - players[i].getPressed() + 1;
                    QString until = (untilSub != 1) ? QString("presses") : QString("press");
                    QMessageBox::critical(this, tr("Error"),
                                         QString("%1 - You raped your key %2 times!\n%3 %4 until subtraction").arg(players[i].getName()).arg(players[i].getPressed()).arg(untilSub).arg(until));
                }

                players[i].incPressed();
            }
        }
    }

    return QDialog::eventFilter(target, event);
}

void GameField::indicateRandom()
{
    foreach(QLabel *l,  playerPointsLabels){
        l->setStyleSheet(QString("background-color: black"));
    }

    QTimer::singleShot(30, this, SLOT(updatePointsLabels()));
}



void GameField::updateNamesLabels()
{
    setNames();
    setLabelColor();
}
