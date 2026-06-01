#ifndef INPUT_EDITBOX_H
#define INPUT_EDITBOX_H
#include <QPlainTextEdit>
#include <QKeyEvent>

class InputEditbox : public QPlainTextEdit
{
    Q_OBJECT

public:
    InputEditbox(QWidget *parent = nullptr);

signals:
    void enterPressed();

protected:
    void keyPressEvent(QKeyEvent *event) override;
};

#endif // INPUT_EDITBOX_H
