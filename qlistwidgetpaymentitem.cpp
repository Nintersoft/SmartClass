#include "qlistwidgetpaymentitem.h"

QListWidgetPaymentItem::QListWidgetPaymentItem(const QString &text, double value) :
    QListWidgetItem(text), VALUE(value)
{
    this->setDate(QDate(2000, 01, 01));
    this->installmentsN = 1;
    this->discountN = 0;
}

QListWidgetPaymentItem::~QListWidgetPaymentItem(){
}

void QListWidgetPaymentItem::setDate(const QDate &date){
    this->dateN = QDate(date);
}

void QListWidgetPaymentItem::setInstallments(int installments){
    this->installmentsN = installments;
}

void QListWidgetPaymentItem::setDiscount(int percentage){
    this->discountN = percentage;
}

int QListWidgetPaymentItem::discount(){
   return this->discountN;
}

int QListWidgetPaymentItem::installments(){
    return this->installmentsN;
}

int QListWidgetPaymentItem::value(){
    return this->VALUE;
}

QDate QListWidgetPaymentItem::date(){
    return this->dateN;
}
