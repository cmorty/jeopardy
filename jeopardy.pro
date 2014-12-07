# -------------------------------------------------
# Project created by QtCreator 2011-11-20T15:01:54
# -------------------------------------------------
QT += xml \
    xmlpatterns \
    phonon 
CONFIG += qtestlib
CONFIG += static
TARGET = jeopardy
TEMPLATE = app
SOURCES += main.cpp \
    jeopardy.cpp \
    player.cpp \
    gamefield.cpp \
    editor.cpp \
    podium.cpp \
    doublejeopardy.cpp \
    round.cpp \
    category.cpp \
    answer.cpp \
    keyledcontrol.cpp
HEADERS += jeopardy.h \
    player.h \
    gamefield.h \
    editor.h \
    podium.h \
    doublejeopardy.h \
    round.h \
    category.h \
    answer.h \
    keyledcontrol.h \
    ui_doublejeopardy.h \
    ../jeopardy/build/ui_answer.h
FORMS += answer.ui

OTHER_FILES += \
    gamefield.qss \
    answers/2.jrf \
    answers/test.jrf \
    answers/1.jrf \
    answers/3.jrf \
    answers/4.jrf \
    answer.qss \
    jeopardy.qss

TRANSLATIONS = jeopardy.ts
