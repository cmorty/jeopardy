#include "keyledcontrol.h"
#include "windows.h"
#include <QDebug>


#define KEYS ((int)(sizeof(keys)/sizeof(struct keys_t)))

bool KeyLedControl::enabled = false;

struct keys_t KeyLedControl::keys[2] = {
    {VK_NUMLOCK, 0x45},
    {VK_CAPITAL, 0x3a},
};

bool KeyLedControl::bState[2]={false, false};

bool KeyLedControl::startState[2]={false, false};

KeyLedControl::KeyLedControl(QObject *parent) :
    QObject(parent)
{
    for(int i = 0; i < KEYS; i++ ){
        startState[i] = GetKeyState(keys[i].keycode);
    }
}

KeyLedControl::~KeyLedControl(){
    for(int i = 0; i < KEYS; i++ ){
        setLed(i, startState[i]);
        setLed_l(i, startState[i]);
    }
}


void KeyLedControl::check(){
    if(!enabled) return;
    for(int i = 0; i< 2; i++){
        setLed_l(i, KeyLedControl::bState[i]);
    }
}

void KeyLedControl::setLed(int which, bool state_){
    if(which < 0 || KEYS >= 2){
        qDebug() << QString("Unsupported ID: ");
    }
        ;
    KeyLedControl::bState[which] = state_;
}

void KeyLedControl::setEnabled(bool on){
    enabled = on;
}


#define KEYA 1

void KeyLedControl::setLed_l(int which, bool state){
    if(which >= KEYS) return;
    int key = keys[which].keycode;

#if 0
    if(!GetKeyboardState((LPBYTE)&keyState)){
        volatile int i = GetLastError();
        while(1);
        return;
    }
#endif

    int kstate = GetKeyState(key);

    if( (state && !(kstate & 1)) ||
        (!state && (kstate & 1)) )
    {
#if KEYA == 0

      // Simulate a key press
     keybd_event( key,
                  sc,
                  KEYEVENTF_EXTENDEDKEY | 0,
                  0 );

  // Simulate a key release
     keybd_event( key,
                  sc,
                  KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP,
                  0);
#else
        INPUT input[2];
        memset(input, 0, sizeof(input));
        input[0].type = INPUT_KEYBOARD;
        input[1].type = INPUT_KEYBOARD;

        input[0].ki.wVk = key;
        input[0].ki.dwFlags = 0;
        input[0].ki.time = 0;
        input[0].ki.dwExtraInfo = 0;

        input[1].ki.wVk = key;
        input[1].ki.dwFlags = KEYEVENTF_KEYUP;
        input[1].ki.time = 0;
        input[1].ki.dwExtraInfo = 0;

        SendInput(2,input,sizeof(INPUT));


#endif
    }
}
