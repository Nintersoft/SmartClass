#-------------------------------------------------
#
# Project developed by Nintersoft team
# Developer: Mauro Mascarenhas de Araújo
# Contact: mauro.mascarenhas@nintersoft.com
# License: Nintersoft Open Source Code Licence
# Date: 15 of June of 2017
#
#-------------------------------------------------

QT       += core gui sql printsupport network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = SmartClass
TEMPLATE = app

win32 {
    VERSION = 0.9.2.1

    QMAKE_TARGET_COMPANY = Nintersoft
    QMAKE_TARGET_PRODUCT = SmartClass
    QMAKE_TARGET_DESCRIPTION = SmartClass
    QMAKE_TARGET_COPYRIGHT = Copyright (c) 2017 - Nintersoft

    RC_ICONS = images\logos\Logo.ico
#    RC_LANG = 0x0416
}
else {
    VERSION = 0.9.2
}

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


TRANSLATIONS += lang/SmartClass_pt.ts \
                lang/SmartClass_en.ts

SOURCES += \
        main.cpp \
        frmmain.cpp \
    titlebar.cpp \
    frmlogin.cpp \
    dbmanager.cpp \
    frmfirstrun.cpp \
    frmmanagestudent.cpp \
    frmimageviewer.cpp \
    frmabout.cpp \
    frmaddclass.cpp \
    qlistwidgetpaymentitem.cpp \
    printpreviewform.cpp \
    frmreceipt.cpp \
    frmsettings.cpp \
    frmprintcontract.cpp \
    frmimportexportdb.cpp

HEADERS += \
        frmmain.h \
    titlebar.h \
    frmlogin.h \
    dbmanager.h \
    frmfirstrun.h \
    frmmanagestudent.h \
    frmimageviewer.h \
    frmabout.h \
    frmaddclass.h \
    qlistwidgetpaymentitem.h \
    printpreviewform.h \
    frmreceipt.h \
    frmsettings.h \
    frmprintcontract.h \
    frmimportexportdb.h

FORMS += \
        frmmain.ui \
    titlebar.ui \
    frmlogin.ui \
    frmfirstrun.ui \
    frmmanagestudent.ui \
    frmimageviewer.ui \
    frmabout.ui \
    frmaddclass.ui \
    printpreviewform.ui \
    frmreceipt.ui \
    frmsettings.ui \
    frmprintcontract.ui \
    frmimportexportdb.ui

RESOURCES += \
    resources.qrc
