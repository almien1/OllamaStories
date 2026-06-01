QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    conversation.cpp \
    main.cpp \
    llama_stories.cpp \
    ollama_cli.cpp \
    story_project.cpp

HEADERS += \
    conversation.h \
    llama_stories.h \
    ollama_cli.h \
    pch.h \
    story_project.h

FORMS += \
    llama_stories.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

win32 {
    LIBS += -lws2_32
}

CONFIG += precompiled_header

PRECOMPILED_HEADER = pch.h


DISTFILES += \
    notes.md \
    readme.md
