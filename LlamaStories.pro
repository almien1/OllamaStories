QT += widgets

CONFIG += c++17

DESTDIR = build

SOURCES += \
    conversation.cpp \
    input_editbox.cpp \
    main.cpp \
    llama_stories.cpp \
    ollama_cli.cpp \
    story_project.cpp

HEADERS += \
    conversation.h \
    input_editbox.h \
    llama_stories.h \
    ollama_cli.h \
    pch.h \
    story_project.h

FORMS += \
    llama_stories.ui

DISTFILES += \
    readme.md

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

win32 {
    LIBS += -lws2_32
}

CONFIG += precompiled_header

PRECOMPILED_HEADER = pch.h


