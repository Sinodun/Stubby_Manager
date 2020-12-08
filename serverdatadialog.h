#ifndef SERVERDATADIALOG_H
#define SERVERDATADIALOG_H

#include <QDialog>
#include <QRegExpValidator>

#include <config.h>

namespace Ui {
class serverdatadialog;
}

class ServerDataDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ServerDataDialog(Config::Server& server, QWidget *parent = nullptr);
    ~ServerDataDialog();

public slots:
    void on_applyButton_clicked();
    void on_discardButton_clicked();

private:
    Ui::serverdatadialog *ui;
    QValidator *validator;
    Config::Server& m_server;
};

#endif // SERVERDATADIALOG_H
