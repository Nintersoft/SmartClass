#include "frmmain.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    frmMain w;

    if (w.getReturnCode() != -1)
        return w.getReturnCode();

    return a.exec();
}
