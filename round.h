#ifndef ROUND_H
#define ROUND_H

#include <QFile>
#include <QList>
#include <category.h>

class Round
{
public:
    explicit Round(const QString &roundfile, int roundNr);
    QList<Category *>  getCategories();
    int getAnswerCount();
    int getRoundNr();
private:
    QString roundfile;
    QList<Category *>   categories;
    int roundNr;
    int nAnswers;


};

#endif // ROUND_H
