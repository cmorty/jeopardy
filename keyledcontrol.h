#ifndef KEYLEDCONTROL_H
#define KEYLEDCONTROL_H

#include <QObject>

class KeyLedControl : public QObject
{
    Q_OBJECT
public:
    explicit KeyLedControl(QObject *parent = 0);
    static bool bState[2];
    static void setLed(int which, bool state);
private:
    void setLed_l(int which, bool state);

signals:

public slots:
    void check();
};

#endif // KEYLEDCONTROL_H
