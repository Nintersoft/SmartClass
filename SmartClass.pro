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
    VERSION = 1.0.0.32

    QMAKE_TARGET_COMPANY = Nintersoft
    QMAKE_TARGET_PRODUCT = SmartClass
    QMAKE_TARGET_DESCRIPTION = SmartClass
    QMAKE_TARGET_COPYRIGHT = Copyright (c) 2017 - Nintersoft

    RC_ICONS = images\logos\Logo.ico
#    RC_LANG = 0x0416
}
else {
    VERSION = 1.0.0
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

win32: LIBS += -L$$PWD/dbmanager/ -lDBManager

INCLUDEPATH += $$PWD/nmainwindow \
               $$PWD/dbmanager \
               $$PWD/qlistwidgetpaymentitem \
               $$PWD/smartclassglobal

DEPENDPATH += $$PWD/dbmanager

TRANSLATIONS += lang/SmartClass_pt.ts \
                lang/SmartClass_en.ts

SOURCES += \
    main.cpp \
    nmainwindow/nmainwindow.cpp \
    nmainwindow/titlebar.cpp \
    smartclassglobal/smartclassglobal.cpp \
    qlistwidgetpaymentitem/qlistwidgetpaymentitem.cpp \
    frmmain.cpp \
    frmlogin.cpp \
    frmfirstrun.cpp \
    frmmanagestudent.cpp \
    frmimageviewer.cpp \
    frmabout.cpp \
    frmmanageclass.cpp \
    printpreviewform.cpp \
    frmreceipt.cpp \
    frmsettings.cpp \
    frmprintcontract.cpp \
    frmimportexportdb.cpp \
    frmmanageusers.cpp \
    frmmanageuser.cpp

HEADERS += \
    nmainwindow/nmainwindow.h \
    nmainwindow/titlebar.h \
    smartclassglobal/smartclassglobal.h \
    qlistwidgetpaymentitem/qlistwidgetpaymentitem.h \
    frmmain.h \
    frmlogin.h \
    frmfirstrun.h \
    frmmanagestudent.h \
    frmimageviewer.h \
    frmabout.h \
    printpreviewform.h \
    frmreceipt.h \
    frmsettings.h \
    frmprintcontract.h \
    frmimportexportdb.h \
    frmmanageusers.h \
    frmmanageuser.h \
    frmmanageclass.h

FORMS += \
    nmainwindow/nmainwindow.ui \
    nmainwindow/titlebar.ui \
    frmmain.ui \
    frmlogin.ui \
    frmfirstrun.ui \
    frmmanagestudent.ui \
    frmimageviewer.ui \
    frmabout.ui \
    printpreviewform.ui \
    frmreceipt.ui \
    frmsettings.ui \
    frmprintcontract.ui \
    frmimportexportdb.ui \
    frmmanageusers.ui \
    frmmanageuser.ui \
    frmmanageclass.ui

RESOURCES += \
    resources.qrc
