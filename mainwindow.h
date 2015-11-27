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
    void showRectDialogForBench();
    void showRectDialogForCompare();

    void roundSBToPowerOfTwo(int);

    void startCompare();
    void startBench();

private:
    Ui::MainWindow *ui;
    QProgressDialog *m_progress;

    int rangeMinSBPrevValue;
    int rangeMaxSBPrevValue;
};

#endif // MAINWINDOW_H
