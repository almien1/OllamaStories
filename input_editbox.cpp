#include "input_editbox.h"

InputEditbox::InputEditbox(QWidget *parent)
    : QPlainTextEdit(parent)
{

}

void InputEditbox::keyPressEvent(QKeyEvent *event)
{
    if ((event->key() == Qt::Key_Return) || (event->key() == Qt::Key_Enter))
    {
        Qt::KeyboardModifiers mods = event->modifiers();

        if (mods & (Qt::ControlModifier | Qt::ShiftModifier))
        {
            insertPlainText("\n");
        }
        else
        {
            emit enterPressed();
        }
    }
    else
    {
        QPlainTextEdit::keyPressEvent(event);
    }
}