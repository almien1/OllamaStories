QT += widgets network

CONFIG += c++17

DESTDIR = output

SOURCES += \
    input_editbox.cpp \
    main.cpp \
    llama_cpp_chat.cpp \
    llama_cpp_options.cpp \
    llama_cpp_server.cpp \
    llama_stories.cpp \
    model_list.cpp \
    ollama_cli.cpp \
    story_project.cpp

HEADERS += \
    input_editbox.h \
    llama_cpp_chat.h \
    llama_cpp_options.h \
    llama_cpp_server.h \
    llama_stories.h \
    model_list.h \
    ollama_cli.h \
    pch.h \
    story_project.h

FORMS += \
    llama_stories.ui

DISTFILES += \
    readme.md

MAKEFLAGS += -j4

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

win32 {
    LIBS += -lws2_32
}

CONFIG += precompiled_header

PRECOMPILED_HEADER = pch.h


