#include "frmmain.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    frmMain w;
    if (w.returnCode != -1)
        return w.returnCode;

    return a.exec();
}
