#ifndef RECTDIALOG_H
#define RECTDIALOG_H

#include <QDialog>

namespace Ui {
class RectDialog;
}

class RectDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RectDialog(QWidget *parent = 0);
    ~RectDialog();

    QString getRectCode() const;

private slots:
    void rectCodeUpdated();
    void applyRectCode();
    void updatePreview();

private:
    Ui::RectDialog *ui;

    bool m_ignoreSignals;
    QString m_rectCode;
};

#endif // RECTDIALOG_H
