#include "round.h"
#include <QTextStream>
#include <QMessageBox>
#include <QObject>

Round::Round(const QString &roundfile, int roundNr) :
   roundfile(roundfile), roundNr(roundNr), nAnswers(0)
{
}

// Failing in the constructor with an exception would be much nicer,
// but as this does not seem to work in QT debug mode, this must be
// done the C way.... :(

bool Round::load(){
    QFile file(roundfile);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
      //QMessageBox::critical(this, tr("Error"), tr("Could not open file"));
        return false;
    }

    QTextStream in(&file);

    Category *curcat = NULL;

    while(!in.atEnd()) {
        QString line = in.readLine();
        if(line.startsWith('#')){
            curcat = new Category(line.mid(1));
            categories.append(curcat);
            continue;
        }
        bool convok = false;
        int points = line.section(':', 0, 0).trimmed().toUInt(&convok);
        QString answer = line.section(':', 1).trimmed();
        if(!convok || answer.length() == 0) {
            if(answer.length() != 0) qWarning() << "Ignoring Line \"" << line << '"';
            continue;
        }
        curcat->addAnswer(answer, points);
        nAnswers++;
    }
    return true;
}

Round::~Round(){
    foreach(Category *c, categories){
        delete c;
    }
}

QList<Category *> Round::getCategories(){
    return categories;
}

int Round::getAnswerCount(){
   return nAnswers;
}

int Round::getRoundNr(){
    return roundNr;
}

QString Round::getRoundFile(){
    return roundfile;
}
