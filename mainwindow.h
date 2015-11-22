#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class QProgressDialog;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void showImageBrowser();
    void showRectDialog();
    void startCompare();

private:
    Ui::MainWindow *ui;
    QProgressDialog *m_progress;
};

#endif // MAINWINDOW_H
