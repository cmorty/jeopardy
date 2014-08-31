#include "category.h"

Category::Category(const QString &name) :
    name(name)
{
}

void Category::addAnswer(const QString &answer, int points){
    struct answer_t * ans = new struct answer_t;
    ans->answer = new QString(answer);
    ans->points = points;
    answers.append(ans);
}


QString Category::getName(){
    return name;
}

QList<struct answer_t *> Category::getAnswers(){
    return answers;
}
