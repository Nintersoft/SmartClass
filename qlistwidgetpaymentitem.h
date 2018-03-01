#ifndef QLISTWIDGETPAYMENTITEM_H
#define QLISTWIDGETPAYMENTITEM_H

#include <QListWidgetItem>
#include <QDate>

class QListWidgetPaymentItem : public QListWidgetItem
{
public:
    QListWidgetPaymentItem(const QString &text = "", double value = 0.00);
    ~QListWidgetPaymentItem();

    void setDate(const QDate &date);
    void setInstallments(int installments);
    void setDiscount(int percentage);

    int discount();
    int installments();
    int value();
    QDate date();

private:
    QDate dateN;
    const double VALUE;
    int discountN, installmentsN;
};

#endif // QLISTWIDGETPAYMENTITEM_H
