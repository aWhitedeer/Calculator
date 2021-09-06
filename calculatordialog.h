#ifndef CALCULATORDIALOG_H
#define CALCULATORDIALOG_H

#include <QDialog>
#include <QQueue>
#include <QStack>

namespace Ui {
class CalculatorDialog;
}

class CalculatorDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CalculatorDialog(QWidget *parent = nullptr);
    ~CalculatorDialog();

    QString getExpression();
    double doCalculating(const QString &expr, QString *errStr =nullptr);

private slots:
    void receiveClickedContent(const QString &text);

    void on_pushButton_clicked();

    void on_pushButton_0_clicked();

    void on_pushButton_1_clicked();

    void on_pushButton_2_clicked();

    void on_pushButton_3_clicked();

    void on_pushButton_4_clicked();

    void on_pushButton_5_clicked();

    void on_pushButton_6_clicked();

    void on_pushButton_7_clicked();

    void on_pushButton_8_clicked();

    void on_pushButton_9_clicked();

    void on_pushButton_10_clicked();

    void on_pushButton_11_clicked();

    void on_pushButton_12_clicked();

    void on_pushButton_13_clicked();

    void on_pushButton_14_clicked();

    void on_pushButton_15_clicked();

    void on_pushButton_16_clicked();

    void on_pushButton_17_clicked();

    void on_pushButton_18_clicked();

    void on_pushButton_19_clicked();

    void on_pushButton_20_clicked();

    void on_pushButton_21_clicked();

    void on_pushButton_22_clicked();

    void on_pushButton_23_clicked();

//    void on_pushButton_24_clicked();

    void on_pushButton_26_clicked();

    void on_pushButton_27_clicked();

    void on_pushButton_25_clicked();

    void on_acceptBt_clicked();

private:
    Ui::CalculatorDialog *ui;
    QString m_errStr;
    QString m_errOutput;
    bool m_bFinish;

    void on_selectData(const QString &text);
    bool judgeExpression(const QString &expr);
    int matchCharacter(const QString &source, const char *target);
    bool isExpressionTail(const QChar &ch);
    QQueue<QString> splitExpression(const QString &expr);
    QQueue<QString> convert2Postfix(QQueue<QString> &source);
    int priority(const QString &opt);
    QString optCalculate(const QString &opt, const QString &left, const QString &right);
    QString calculate(QQueue<QString> &postfix_queue);
signals:
    void buttonClicked(const QString &text);
};

#endif // CALCULATORDIALOG_H
