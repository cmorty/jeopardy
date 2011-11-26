/*
 * Copyright (c) 2011, Christian Lange
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

DoubleJeopardy::DoubleJeopardy(QWidget *parent, int min, int max, Player *players[NUMBER_PLAYERS]) :
    QDialog(parent),
    ui(new Ui::DoubleJeopardy), min(min), max(max)
{
    ui->setupUi(this);
    this->insertPlayers(players);
}

DoubleJeopardy::~DoubleJeopardy()
{
    delete ui;
}

void DoubleJeopardy::changeEvent(QEvent *e)
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

void DoubleJeopardy::setLabels()
{
    if(this->min < DOUBLE_JEOPARDY_MIN_POINTS)
        this->min = DOUBLE_JEOPARDY_MIN_POINTS;

    ui->min->setText(QString("Min: %1").arg(this->min));
    ui->max->setText(QString("Max: %1").arg(this->max));

    QStringList playerList;

    for(int i = 0; i < NUMBER_PLAYERS; i++)
        playerList << this->players[i]->getName();

    ui->comboBox->addItems(playerList);
}

void DoubleJeopardy::on_button_clicked()
{
    if(this->min <= ui->points->text().toInt() && ui->points->text().toInt() <= this->max)
    {
        this->points = ui->points->text().toInt();
        done(ui->comboBox->currentIndex());
    }
    else
    {
        QMessageBox::critical(this, tr("Error"), tr("Points invalid, reenter points"));
        ui->points->setText(0);
    }
}

int DoubleJeopardy::getPoints()
{
    return this->points;
}

void DoubleJeopardy::insertPlayers(Player *players[NUMBER_PLAYERS])
{
    for(int i = 0; i < NUMBER_PLAYERS; i++)
        this->players[i] = players[i];
}
