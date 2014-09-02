#include "keyledcontrol.h"
#include "windows.h"
#include <QDebug>

KeyLedControl::KeyLedControl(QObject *parent) :
    QObject(parent)
{
}


bool KeyLedControl::bState[2]={false, false};

void KeyLedControl::check(){
    for(int i = 0; i< 2; i++){
        setLed_l(i, KeyLedControl::bState[i]);
    }
}

void KeyLedControl::setLed(int which, bool state_){
    if(which < 0 || which >= 2){
        qDebug() << QString("Unsupported ID: ");
    }
        ;
    KeyLedControl::bState[which] = state_;
}


#define KEYA 1

void KeyLedControl::setLed_l(int which, bool state){
    int key = 0;
#if KEYA == 0
    int sc = 0;
#endif
    switch(which){
        case 0:
            key = VK_NUMLOCK;
#if KEYA == 0
            sc = 0x45;
#endif
            break;
        case 1:
            key = VK_CAPITAL;
#if KEYA == 0
            sc = 0x3A;
#endif
            break;
    }

    if(key == 0) return;

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
