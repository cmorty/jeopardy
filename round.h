#ifndef ROUND_H
#define ROUND_H

#include <QFile>
#include <QList>
#include <category.h>

class Round
{
public:
    explicit Round(const QString &roundfile);
    QList<Category *>  getCategories();
    int getAnswerCount();
private:
    QString roundfile;
    QList<Category *>   categories;
    int nAnswers;


};

#endif // ROUND_H
