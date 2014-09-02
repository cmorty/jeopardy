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

#include "doublejeopardy.h"
#include "ui_doublejeopardy.h"

DoubleJeopardy::DoubleJeopardy(int min, int max, QList<Player *> *players, Player *player, QWidget *parent) :
    QDialog(parent), min(min), max(max), currentPlayer(player), players(players)
{
}

DoubleJeopardy::~DoubleJeopardy()
{
    delete playerComboBox;
    delete minLabel;
    delete pointsSpinBox;
    delete maxLabel;
    delete startButton;

    delete grid;
    delete window;
}

void DoubleJeopardy::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        break;
    default:
        break;
    }
}

void DoubleJeopardy::show()
{
    window->exec();
}

void DoubleJeopardy::init()
{
    insertLayouts();
    setLabels();
}

void DoubleJeopardy::insertLayouts()
{
    window = new QDialog();
    grid = new QGridLayout();

    playerComboBox = new QComboBox();
    minLabel = new QLabel();
    pointsSpinBox = new QSpinBox();
    maxLabel = new QLabel();
    startButton = new QPushButton();

    grid->addWidget(playerComboBox, 0, 0);
    grid->addWidget(minLabel, 1, 0);
    grid->addWidget(pointsSpinBox, 2, 0);
    grid->addWidget(maxLabel, 3, 0);
    grid->addWidget(startButton, 4, 0);

    window->setGeometry(0, 0, 250, 0);
    window->setLayout(grid);
}

void DoubleJeopardy::setLabels()
{
    if(min < DOUBLE_JEOPARDY_MIN_POINTS)
        min = DOUBLE_JEOPARDY_MIN_POINTS;

    QStringList playerList;

    playerList << "Choose player";

    foreach(Player *p, *players){
        playerComboBox->addItem(p->getName());
    }

    playerComboBox->setCurrentIndex(players->indexOf(currentPlayer));

    minLabel->setText(QString("Min: %1").arg(min));

    pointsSpinBox->setMinimum(-20000);
    pointsSpinBox->setMaximum(20000);
    pointsSpinBox->setSingleStep(50);
    pointsSpinBox->setValue(max);

    maxLabel->setText(QString("Max: %1").arg(max));

    startButton->setText("Save");
    QObject::connect(startButton, SIGNAL(clicked()), this, SLOT(on_button_clicked()));
}

int DoubleJeopardy::getPoints()
{
    return points;
}

Player *DoubleJeopardy::getPlayer()
{
    return players->at(index);
}

void DoubleJeopardy::on_button_clicked()
{
    if(playerComboBox->currentIndex() == 0)
    {
         QMessageBox::critical(this, tr("Error"), tr("Choose player"));
    }
    else if(min <= pointsSpinBox->value() && pointsSpinBox->value() <= max)
    {
        points = pointsSpinBox->value();
        index = playerComboBox->currentIndex();
        window->done(0);
    }
    else
    {
        QMessageBox::critical(this, tr("Error"), tr("Points out of range!"));
        pointsSpinBox->setValue(max);
    }
}
