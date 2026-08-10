#ifndef MODEL_LIST_H
#define MODEL_LIST_H

#include <QObject>

class ModelList : public QObject
{
    Q_OBJECT
public:
    ModelList();

public slots:
    void start();

signals:
    void modelList(QStringList models);
};

#endif // MODEL_LIST_H
