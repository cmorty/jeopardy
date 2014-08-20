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

GameField::GameField(QWidget *parent, int round, int categoryNr, Player *players, int playerNr, bool sound, QString fileString) :
    QDialog(parent), round(round), alreadyAnswered(0), lastWinner(NO_WINNER),
    lastPoints(0), playerNr(playerNr), categoryNr(categoryNr), sound(sound), players(players), answer(), podium(NULL),
    randomCtx(NULL), editorCtx(NULL), loadCtx(NULL), saveCtx(NULL), endRoundCtx(NULL), about(NULL), fileString(fileString)
{
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

    delete categoryLabelGrid;
    delete buttonGrid;
    delete playerLabelGrid;
    delete mainGrid;

    delete window;
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
    insertLayouts();
    assignButtons();
    assignPlayerNameLabels();
    assignPlayerPointsLabels();
    assignCategoryLabels();
    processCategoryLabels();
    setNames();
    setPoints();
    setLabelColor();

    /* Load syle File */
    QFile file("gamefield.qss");
    file.open(QFile::ReadOnly);
    QString styleSheet = QLatin1String(file.readAll());
    window->setStyleSheet(styleSheet);

    /* Declare new context menu and connect it with the right mouse button */
    window->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(window, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(on_gameField_customContextMenuRequested(QPoint)));

    window->show();

    currentPlayer = random();
    updateCurrentPlayerLabel();
}

void GameField::setRound(int round)
{
    round = round;
}

int GameField::getRound()
{
    return round;
}

void GameField::incAlreadyAnswered(int number)
{
    alreadyAnswered += number;
}

void GameField::setAlreadyAnswered(int number)
{
    alreadyAnswered = number;
}

int GameField::getAlreadyAnswered()
{
    return alreadyAnswered;
}

void GameField::insertLayouts()
{
    window = new QWidget();
    window->setGeometry(0, 0, GAMEFIELD_WIDTH, GAMEFIELD_HEIGHT);

    mainGrid = new QGridLayout();
    mainGrid->setSpacing(0);

    categoryLabelGrid = new QGridLayout();
    categoryLabelGrid->setSpacing(0);

    buttonGrid = new QGridLayout();
    buttonGrid->setSpacing(0);

    playerLabelGrid = new QGridLayout();
    playerLabelGrid->setSpacing(0);

    mainGrid->addLayout(categoryLabelGrid, 0, 0);
    mainGrid->addLayout(buttonGrid, 1, 0);
    mainGrid->addLayout(playerLabelGrid, 2, 0);

    mainGrid->setGeometry(QRect(0, 0, GAMEFIELD_WIDTH, GAMEFIELD_HEIGHT));
    categoryLabelGrid->setGeometry(QRect(0, 0, GAMEFIELD_WIDTH, CATEGORY_LABEL_HEIGHT));
    buttonGrid->setGeometry(QRect(0, CATEGORY_LABEL_HEIGHT, GAMEFIELD_WIDTH, GAMEFIELD_HEIGHT - CATEGORY_LABEL_HEIGHT - NAME_LABEL_HEIGHT - NAME_LABEL_HEIGHT));
    playerLabelGrid->setGeometry(QRect(0, GAMEFIELD_HEIGHT - NAME_LABEL_HEIGHT - NAME_LABEL_HEIGHT, GAMEFIELD_WIDTH, NAME_LABEL_HEIGHT + NAME_LABEL_HEIGHT));

    window->installEventFilter(this);
    window->setLayout(mainGrid);
}

void GameField::assignCategoryLabels()
{
    for(int i = 0; i < NUMBER_MAX_CATEGORIES; i++)
    {
        categoryLabels[i] = new QLabel();
        categoryLabels[i]->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        categoryLabels[i]->setProperty("iscategory", true);
        categoryLabels[i]->setProperty("category", i + 1);
        categoryLabelGrid->addWidget(categoryLabels[i], 0, i);
    }
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
    int category = (button->property("ansCategory")).toInt();
    openAnswer(category, points);
}

void GameField::assignButtons()
{

    for(int category = 0; category < categoryNr; category++)
    {
        for(int i = 0; i < NUMBER_ANSWERS; i++)
        {
            int currentButton = (NUMBER_MAX_CATEGORIES * i) + category;
            int points = (i + 1) * POINTS_FACTOR;

            QPushButton * b = new QPushButton();
            buttons[currentButton] = b;

            b->installEventFilter(this);
            b->setProperty("ansPoints", points);
            b->setProperty("ansCategory", category + 1); //Off by one....

            setDefaultButtonAppearance(b);
            categoryLabelGrid->addWidget(b, i + 1, category);

            connect(b, SIGNAL(clicked()), this, SLOT(on_button_clicked()));
        }
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

    for(int i = 0; i < NUMBER_MAX_PLAYERS; i++)
        playerNameLabels[i] = new QLabel();

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
        
        playerNameLabels[i]->setProperty("isname", true);
        playerNameLabels[i]->setGeometry(0, 0, width, height);
        playerNameLabels[i]->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        playerLabelGrid->addWidget(playerNameLabels[i], row, column);
    }
}

void GameField::assignPlayerPointsLabels()
{
    int row, column, width, height;

    for(int i = 0; i < NUMBER_MAX_PLAYERS; i++)
        playerPointsLabels[i] = new QLabel();

    height = NAME_LABEL_HEIGHT;

    for(int i = 0; i < playerNr; i++)
    {
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
        
        playerPointsLabels[i]->setProperty("ispoints", true);
        playerPointsLabels[i]->setGeometry(0, 0, width, height);
        playerPointsLabels[i]->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        playerLabelGrid->addWidget(playerPointsLabels[i], row, column);
    }
}

void GameField::processCategoryLabels()
{
    int categoryLine;
    QString categoryName;

    for(int i = 0; i < categoryNr; i++)
    {
        QFile file(fileString);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
          QMessageBox::critical(this, tr("Error"), tr("Could not open file"));
          return;
        }

        QTextStream in(&file);

        /* Calculate on which line the categories in the file start */
        categoryLine = NUMBER_MAX_CATEGORIES * i + 1;

        /* Step to appropriate category section */
        for(int lineNr = 0; lineNr != categoryLine; lineNr++)
            categoryName = in.readLine();

        categoryName.replace("\\n", "\n");
        categoryLabels[i]->setText(categoryName);
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
    for(int i = 0; i < playerNr; i++)
        playerPointsLabels[i]->setText("0");
}

void GameField::setNames()
{
    for(int i = 0; i < playerNr; i++)
    {
        if(currentPlayer == i)
            playerNameLabels[i]->setText(QString("%1 ***").arg(players[i].getName()));
        else
            playerNameLabels[i]->setText(players[i].getName());
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

void GameField::updateNamesLabels()
{
    setNames();
    setLabelColor();
}

void GameField::updateLabelsAfterAnswer()
{
    updatePointsLabels();
    updateNamesLabels();
}

void GameField::updateAfterAnswer()
{
    incAlreadyAnswered(1);
    updatePointsLabels();
}

void GameField::updateCurrentPlayerLabel()
{
    updateNamesLabels();
    playerNameLabels[currentPlayer]->setText(QString("%1 ***").arg(players[currentPlayer].getName()));
}

QString GameField::getButtonColorByLastWinner()
{
    return QString("QPushButton { background-color : %1; color : black; }").arg(players[lastWinner].getColor());
}

void GameField::openAnswer(int category, int points)
{
    answer = new Answer(this, fileString, round, players, playerNr, sound, currentPlayer);
    answer->setAnswer(category, points);

    answer->exec();

    processAnswer(category, points);

    processResult();
    updateAfterAnswer();

    if(getAlreadyAnswered() < categoryNr * NUMBER_ANSWERS)
    {
        /* Do backup after each answer */
        openFileSaver(true);
    }
    else
    {
        showPodium();
        window->close();
    }
}

void GameField::processAnswer(int category, int points)
{
    QPushButton *button = buttons[NUMBER_MAX_CATEGORIES * (points / POINTS_FACTOR - OFFSET) + category - OFFSET];
    button->setText("");
    currentPlayer = lastWinner = answer->getWinner();
    lastPoints = answer->getPoints();
    result = answer->getResult();

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

    updateNamesLabels();
    updateCurrentPlayerLabel();
    delete answer;
}

void GameField::processResult()
{
    int playerId = 0;

    while(result.length() > 0)
    {
        for(int i = 0; i < NUMBER_MAX_PLAYERS; i++)
            if(result.startsWith(QString::number(i+1)))
                playerId = i;

        result.remove(0, PLAYER_INDICATOR);

        if(result.startsWith(WON))
            players[playerId].incPoints(lastPoints);
        else
            players[playerId].decPoints(lastPoints);

        result.remove(0, RESULT_INDICATOR);
    }
}

void GameField::openFileLoader()
{
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

    categoryNr = line.toInt();
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
}

void GameField::openFileSaver(bool backup)
{
    QDir dir;
    QString fileName;
    QDateTime dateTime;

    if(backup == true)
        fileName = QString("gameStates/backups/backup_%1_%2").arg(getRound()).arg(dateTime.currentDateTime().toTime_t());
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
        stream << categoryNr << '\n';

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
    srand(time(NULL));

    int rn = rand() % playerNr;

    return rn;
}

void GameField::resetRound()
{
    setAlreadyAnswered(0);
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
    QPoint globalPos = window->mapToGlobal(pos);

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
        updateCurrentPlayerLabel();
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
        window->close();
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
    if(target == window && event->type() == QEvent::KeyPress)
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);

        /* Open random user picker if "r" gets pressed */
        if(keyEvent->key() == Qt::Key_R)
        {
            indicateRandom();
            updateNamesLabels();
            currentPlayer = random();
            updateCurrentPlayerLabel();
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
    for(int i = 0; i < playerNr; i++)
        playerPointsLabels[i]->setStyleSheet(QString("background-color: black"));

    QTimer::singleShot(30, this, SLOT(updatePointsLabels()));
}
