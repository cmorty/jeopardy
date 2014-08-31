#ifndef CATEGORY_H
#define CATEGORY_H

#include <QString>
#include <answer.h>

struct answer_t{
    int points;
    QString * answer;
};


class Category
{
public:
    explicit Category(const QString &name);
    void addAnswer(const QString &answer, int points);
    QString getName();
    QList<struct answer_t *> getAnswers();

private:
    QString name;
    QList<struct answer_t *> answers;

};

#endif // CATEGORY_H
