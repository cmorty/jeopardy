#ifndef CATEGORY_H
#define CATEGORY_H

#include <QObject>
#include <QString>
#include <answer.h>

struct answer_t{
    int points;
    QString * answer;
};


class Category : public QObject
{
    Q_OBJECT
public:
    explicit Category(const QString &name, QObject *parent = 0);
    void addAnswer(const QString &answer, int points);
    QString getName();
    QList<struct answer_t *> getAnswers();

private:
    QString name;
    QList<struct answer_t *> answers;

signals:

public slots:

};

#endif // CATEGORY_H
