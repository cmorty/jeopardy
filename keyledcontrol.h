#ifndef KEYLEDCONTROL_H
#define KEYLEDCONTROL_H

#include <QObject>

struct keys_t{
    int keycode;
    int scancode;
};

class KeyLedControl : public QObject
{
    Q_OBJECT
public:
    explicit KeyLedControl(QObject *parent = 0); 
    ~KeyLedControl();
    static void setLed(int which, bool state);
    static void setEnabled(bool on);
private:
    void setLed_l(int which, bool state);
    static bool bState[2];
    static bool startState[2];
    static struct keys_t keys[2];

    static bool enabled;

signals:

public slots:
    void check();
};

#endif // KEYLEDCONTROL_H
