/********************************************************************************
** Form generated from reading UI file 'jkgstudio.ui'
**
** Created: Fri Nov 16 21:41:49 2012
**      by: Qt User Interface Compiler version 4.8.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_JKGSTUDIO_H
#define UI_JKGSTUDIO_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QHeaderView>
#include <QtGui/QMainWindow>
#include <QtGui/QMdiArea>
#include <QtGui/QMenu>
#include <QtGui/QMenuBar>
#include <QtGui/QStatusBar>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_JKGStudioClass
{
public:
    QAction *actionOpen;
    QAction *actionSave;
    QAction *actionSave_As;
    QAction *actionCompile;
    QAction *actionQuit;
    QAction *actionUndo;
    QAction *actionCut;
    QAction *actionCopy;
    QAction *actionPaste;
    QAction *actionAbout_JKG_Studio;
    QAction *actionAbout_Qt;
    QAction *actionLua_script;
    QAction *actionDialogue;
    QAction *actionQuest;
    QAction *actionFont;
    QAction *actionShader;
    QAction *actionRedo;
    QAction *actionClose;
    QAction *actionDigitally_sign_PK3;
    QAction *actionPreferences;
    QAction *actionAddEntrypoint;
    QAction *actionAddSpeechNode;
    QAction *actionAddOptionNode;
    QAction *actionAddDynOptionNode;
    QAction *actionAddLinkNode;
    QAction *actionAddScriptNode;
    QAction *actionAddInterruptNode;
    QAction *actionAddEndNode;
    QAction *actionAddTextEntryNode;
    QAction *actionAddCinematicNode;
    QAction *actionRemoveNode;
    QWidget *centralWidget;
    QMdiArea *mdiArea;
    QMenuBar *menuBar;
    QMenu *menuFile;
    QMenu *menuNew;
    QMenu *menuEdit;
    QMenu *menuAbout;
    QMenu *menuTools;
    QMenu *menuDlgNodes;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *JKGStudioClass)
    {
        if (JKGStudioClass->objectName().isEmpty())
            JKGStudioClass->setObjectName(QString::fromUtf8("JKGStudioClass"));
        JKGStudioClass->resize(751, 491);
        QIcon icon;
        icon.addFile(QString::fromUtf8(":/icons/jkgicon.png"), QSize(), QIcon::Normal, QIcon::Off);
        JKGStudioClass->setWindowIcon(icon);
        actionOpen = new QAction(JKGStudioClass);
        actionOpen->setObjectName(QString::fromUtf8("actionOpen"));
        QIcon icon1;
        icon1.addFile(QString::fromUtf8(":/icons/open.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionOpen->setIcon(icon1);
        actionSave = new QAction(JKGStudioClass);
        actionSave->setObjectName(QString::fromUtf8("actionSave"));
        QIcon icon2;
        icon2.addFile(QString::fromUtf8(":/icons/save.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionSave->setIcon(icon2);
        actionSave_As = new QAction(JKGStudioClass);
        actionSave_As->setObjectName(QString::fromUtf8("actionSave_As"));
        QIcon icon3;
        icon3.addFile(QString::fromUtf8(":/icons/saveas.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionSave_As->setIcon(icon3);
        actionCompile = new QAction(JKGStudioClass);
        actionCompile->setObjectName(QString::fromUtf8("actionCompile"));
        QIcon icon4;
        icon4.addFile(QString::fromUtf8(":/icons/compile.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionCompile->setIcon(icon4);
        actionQuit = new QAction(JKGStudioClass);
        actionQuit->setObjectName(QString::fromUtf8("actionQuit"));
        actionUndo = new QAction(JKGStudioClass);
        actionUndo->setObjectName(QString::fromUtf8("actionUndo"));
        QIcon icon5;
        icon5.addFile(QString::fromUtf8(":/icons/undo.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionUndo->setIcon(icon5);
        actionCut = new QAction(JKGStudioClass);
        actionCut->setObjectName(QString::fromUtf8("actionCut"));
        QIcon icon6;
        icon6.addFile(QString::fromUtf8(":/icons/cut.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionCut->setIcon(icon6);
        actionCopy = new QAction(JKGStudioClass);
        actionCopy->setObjectName(QString::fromUtf8("actionCopy"));
        QIcon icon7;
        icon7.addFile(QString::fromUtf8(":/icons/copy.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionCopy->setIcon(icon7);
        actionPaste = new QAction(JKGStudioClass);
        actionPaste->setObjectName(QString::fromUtf8("actionPaste"));
        QIcon icon8;
        icon8.addFile(QString::fromUtf8(":/icons/paste.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionPaste->setIcon(icon8);
        actionAbout_JKG_Studio = new QAction(JKGStudioClass);
        actionAbout_JKG_Studio->setObjectName(QString::fromUtf8("actionAbout_JKG_Studio"));
        actionAbout_Qt = new QAction(JKGStudioClass);
        actionAbout_Qt->setObjectName(QString::fromUtf8("actionAbout_Qt"));
        actionLua_script = new QAction(JKGStudioClass);
        actionLua_script->setObjectName(QString::fromUtf8("actionLua_script"));
        QIcon icon9;
        icon9.addFile(QString::fromUtf8(":/icons/script-new.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionLua_script->setIcon(icon9);
        actionDialogue = new QAction(JKGStudioClass);
        actionDialogue->setObjectName(QString::fromUtf8("actionDialogue"));
        QIcon icon10;
        icon10.addFile(QString::fromUtf8(":/icons/dialogue-new.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionDialogue->setIcon(icon10);
        actionQuest = new QAction(JKGStudioClass);
        actionQuest->setObjectName(QString::fromUtf8("actionQuest"));
        actionFont = new QAction(JKGStudioClass);
        actionFont->setObjectName(QString::fromUtf8("actionFont"));
        QIcon icon11;
        icon11.addFile(QString::fromUtf8(":/icons/font-new.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionFont->setIcon(icon11);
        actionShader = new QAction(JKGStudioClass);
        actionShader->setObjectName(QString::fromUtf8("actionShader"));
        actionRedo = new QAction(JKGStudioClass);
        actionRedo->setObjectName(QString::fromUtf8("actionRedo"));
        QIcon icon12;
        icon12.addFile(QString::fromUtf8(":/icons/redo.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionRedo->setIcon(icon12);
        actionClose = new QAction(JKGStudioClass);
        actionClose->setObjectName(QString::fromUtf8("actionClose"));
        actionDigitally_sign_PK3 = new QAction(JKGStudioClass);
        actionDigitally_sign_PK3->setObjectName(QString::fromUtf8("actionDigitally_sign_PK3"));
        QIcon icon13;
        icon13.addFile(QString::fromUtf8(":/icons/signpk3.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionDigitally_sign_PK3->setIcon(icon13);
        actionPreferences = new QAction(JKGStudioClass);
        actionPreferences->setObjectName(QString::fromUtf8("actionPreferences"));
        actionAddEntrypoint = new QAction(JKGStudioClass);
        actionAddEntrypoint->setObjectName(QString::fromUtf8("actionAddEntrypoint"));
        QIcon icon14;
        icon14.addFile(QString::fromUtf8(":/icons/dlg/new/entrypoint.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionAddEntrypoint->setIcon(icon14);
        actionAddSpeechNode = new QAction(JKGStudioClass);
        actionAddSpeechNode->setObjectName(QString::fromUtf8("actionAddSpeechNode"));
        QIcon icon15;
        icon15.addFile(QString::fromUtf8(":/icons/dlg/new/text.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionAddSpeechNode->setIcon(icon15);
        actionAddOptionNode = new QAction(JKGStudioClass);
        actionAddOptionNode->setObjectName(QString::fromUtf8("actionAddOptionNode"));
        QIcon icon16;
        icon16.addFile(QString::fromUtf8(":/icons/dlg/new/option.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionAddOptionNode->setIcon(icon16);
        actionAddDynOptionNode = new QAction(JKGStudioClass);
        actionAddDynOptionNode->setObjectName(QString::fromUtf8("actionAddDynOptionNode"));
        QIcon icon17;
        icon17.addFile(QString::fromUtf8(":/icons/dlg/new/dynoption.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionAddDynOptionNode->setIcon(icon17);
        actionAddLinkNode = new QAction(JKGStudioClass);
        actionAddLinkNode->setObjectName(QString::fromUtf8("actionAddLinkNode"));
        QIcon icon18;
        icon18.addFile(QString::fromUtf8(":/icons/dlg/new/link.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionAddLinkNode->setIcon(icon18);
        actionAddScriptNode = new QAction(JKGStudioClass);
        actionAddScriptNode->setObjectName(QString::fromUtf8("actionAddScriptNode"));
        QIcon icon19;
        icon19.addFile(QString::fromUtf8(":/icons/dlg/new/script.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionAddScriptNode->setIcon(icon19);
        actionAddInterruptNode = new QAction(JKGStudioClass);
        actionAddInterruptNode->setObjectName(QString::fromUtf8("actionAddInterruptNode"));
        QIcon icon20;
        icon20.addFile(QString::fromUtf8(":/icons/dlg/new/interrupt.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionAddInterruptNode->setIcon(icon20);
        actionAddEndNode = new QAction(JKGStudioClass);
        actionAddEndNode->setObjectName(QString::fromUtf8("actionAddEndNode"));
        QIcon icon21;
        icon21.addFile(QString::fromUtf8(":/icons/dlg/new/end.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionAddEndNode->setIcon(icon21);
        actionAddTextEntryNode = new QAction(JKGStudioClass);
        actionAddTextEntryNode->setObjectName(QString::fromUtf8("actionAddTextEntryNode"));
        QIcon icon22;
        icon22.addFile(QString::fromUtf8(":/icons/dlg/new/textentry.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionAddTextEntryNode->setIcon(icon22);
        actionAddCinematicNode = new QAction(JKGStudioClass);
        actionAddCinematicNode->setObjectName(QString::fromUtf8("actionAddCinematicNode"));
        QIcon icon23;
        icon23.addFile(QString::fromUtf8(":/icons/dlg/new/cinematic.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionAddCinematicNode->setIcon(icon23);
        actionRemoveNode = new QAction(JKGStudioClass);
        actionRemoveNode->setObjectName(QString::fromUtf8("actionRemoveNode"));
        QIcon icon24;
        icon24.addFile(QString::fromUtf8(":/icons/dlg/node-delete.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionRemoveNode->setIcon(icon24);
        centralWidget = new QWidget(JKGStudioClass);
        centralWidget->setObjectName(QString::fromUtf8("centralWidget"));
        mdiArea = new QMdiArea(centralWidget);
        mdiArea->setObjectName(QString::fromUtf8("mdiArea"));
        mdiArea->setGeometry(QRect(10, 10, 731, 431));
        mdiArea->setStyleSheet(QString::fromUtf8(""));
        mdiArea->setViewMode(QMdiArea::TabbedView);
        mdiArea->setDocumentMode(true);
        JKGStudioClass->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(JKGStudioClass);
        menuBar->setObjectName(QString::fromUtf8("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 751, 21));
        menuFile = new QMenu(menuBar);
        menuFile->setObjectName(QString::fromUtf8("menuFile"));
        menuNew = new QMenu(menuFile);
        menuNew->setObjectName(QString::fromUtf8("menuNew"));
        menuEdit = new QMenu(menuBar);
        menuEdit->setObjectName(QString::fromUtf8("menuEdit"));
        menuAbout = new QMenu(menuBar);
        menuAbout->setObjectName(QString::fromUtf8("menuAbout"));
        menuTools = new QMenu(menuBar);
        menuTools->setObjectName(QString::fromUtf8("menuTools"));
        menuDlgNodes = new QMenu(menuBar);
        menuDlgNodes->setObjectName(QString::fromUtf8("menuDlgNodes"));
        JKGStudioClass->setMenuBar(menuBar);
        statusBar = new QStatusBar(JKGStudioClass);
        statusBar->setObjectName(QString::fromUtf8("statusBar"));
        JKGStudioClass->setStatusBar(statusBar);

        menuBar->addAction(menuFile->menuAction());
        menuBar->addAction(menuEdit->menuAction());
        menuBar->addAction(menuDlgNodes->menuAction());
        menuBar->addAction(menuTools->menuAction());
        menuBar->addAction(menuAbout->menuAction());
        menuFile->addAction(menuNew->menuAction());
        menuFile->addAction(actionOpen);
        menuFile->addAction(actionSave);
        menuFile->addAction(actionSave_As);
        menuFile->addSeparator();
        menuFile->addAction(actionCompile);
        menuFile->addSeparator();
        menuFile->addAction(actionQuit);
        menuNew->addAction(actionLua_script);
        menuNew->addSeparator();
        menuNew->addAction(actionDialogue);
        menuNew->addAction(actionQuest);
        menuNew->addSeparator();
        menuNew->addAction(actionFont);
        menuNew->addAction(actionShader);
        menuEdit->addAction(actionUndo);
        menuEdit->addAction(actionRedo);
        menuEdit->addSeparator();
        menuEdit->addAction(actionCut);
        menuEdit->addAction(actionCopy);
        menuEdit->addAction(actionPaste);
        menuAbout->addAction(actionAbout_JKG_Studio);
        menuTools->addAction(actionDigitally_sign_PK3);
        menuTools->addSeparator();
        menuTools->addAction(actionPreferences);
        menuDlgNodes->addAction(actionAddEntrypoint);
        menuDlgNodes->addAction(actionAddSpeechNode);
        menuDlgNodes->addAction(actionAddOptionNode);
        menuDlgNodes->addAction(actionAddEndNode);
        menuDlgNodes->addSeparator();
        menuDlgNodes->addAction(actionAddScriptNode);
        menuDlgNodes->addAction(actionAddInterruptNode);
        menuDlgNodes->addSeparator();
        menuDlgNodes->addAction(actionAddLinkNode);
        menuDlgNodes->addSeparator();
        menuDlgNodes->addAction(actionAddDynOptionNode);
        menuDlgNodes->addAction(actionAddTextEntryNode);
        menuDlgNodes->addAction(actionAddCinematicNode);
        menuDlgNodes->addSeparator();
        menuDlgNodes->addAction(actionRemoveNode);

        retranslateUi(JKGStudioClass);
        QObject::connect(actionOpen, SIGNAL(triggered()), JKGStudioClass, SLOT(onOpen()));
        QObject::connect(actionSave, SIGNAL(triggered()), JKGStudioClass, SLOT(onSave()));
        QObject::connect(actionSave_As, SIGNAL(triggered()), JKGStudioClass, SLOT(onSaveAs()));
        QObject::connect(actionCompile, SIGNAL(triggered()), JKGStudioClass, SLOT(onCompile()));
        QObject::connect(actionQuit, SIGNAL(triggered()), JKGStudioClass, SLOT(onQuit()));
        QObject::connect(actionLua_script, SIGNAL(triggered()), JKGStudioClass, SLOT(onNewScript()));
        QObject::connect(actionDialogue, SIGNAL(triggered()), JKGStudioClass, SLOT(onNewDialogue()));
        QObject::connect(actionQuest, SIGNAL(triggered()), JKGStudioClass, SLOT(onNewQuest()));
        QObject::connect(actionUndo, SIGNAL(triggered()), JKGStudioClass, SLOT(onUndo()));
        QObject::connect(actionRedo, SIGNAL(triggered()), JKGStudioClass, SLOT(onRedo()));
        QObject::connect(actionCut, SIGNAL(triggered()), JKGStudioClass, SLOT(onCut()));
        QObject::connect(actionCopy, SIGNAL(triggered()), JKGStudioClass, SLOT(onCopy()));
        QObject::connect(actionPaste, SIGNAL(triggered()), JKGStudioClass, SLOT(onPaste()));
        QObject::connect(actionClose, SIGNAL(triggered()), JKGStudioClass, SLOT(onClose()));
        QObject::connect(mdiArea, SIGNAL(subWindowActivated(QMdiSubWindow*)), JKGStudioClass, SLOT(onSubWindowActivated()));

        QMetaObject::connectSlotsByName(JKGStudioClass);
    } // setupUi

    void retranslateUi(QMainWindow *JKGStudioClass)
    {
        JKGStudioClass->setWindowTitle(QApplication::translate("JKGStudioClass", "JKG Studio", 0, QApplication::UnicodeUTF8));
        actionOpen->setText(QApplication::translate("JKGStudioClass", "Open", 0, QApplication::UnicodeUTF8));
        actionOpen->setShortcut(QApplication::translate("JKGStudioClass", "Ctrl+O", 0, QApplication::UnicodeUTF8));
        actionSave->setText(QApplication::translate("JKGStudioClass", "Save", 0, QApplication::UnicodeUTF8));
        actionSave->setShortcut(QApplication::translate("JKGStudioClass", "Ctrl+S", 0, QApplication::UnicodeUTF8));
        actionSave_As->setText(QApplication::translate("JKGStudioClass", "Save As...", 0, QApplication::UnicodeUTF8));
        actionCompile->setText(QApplication::translate("JKGStudioClass", "Compile", 0, QApplication::UnicodeUTF8));
        actionQuit->setText(QApplication::translate("JKGStudioClass", "Quit", 0, QApplication::UnicodeUTF8));
        actionUndo->setText(QApplication::translate("JKGStudioClass", "Undo", 0, QApplication::UnicodeUTF8));
        actionUndo->setShortcut(QApplication::translate("JKGStudioClass", "Ctrl+Z", 0, QApplication::UnicodeUTF8));
        actionCut->setText(QApplication::translate("JKGStudioClass", "Cut", 0, QApplication::UnicodeUTF8));
        actionCut->setShortcut(QApplication::translate("JKGStudioClass", "Ctrl+X", 0, QApplication::UnicodeUTF8));
        actionCopy->setText(QApplication::translate("JKGStudioClass", "Copy", 0, QApplication::UnicodeUTF8));
        actionCopy->setShortcut(QApplication::translate("JKGStudioClass", "Ctrl+C", 0, QApplication::UnicodeUTF8));
        actionPaste->setText(QApplication::translate("JKGStudioClass", "Paste", 0, QApplication::UnicodeUTF8));
        actionPaste->setShortcut(QApplication::translate("JKGStudioClass", "Ctrl+V", 0, QApplication::UnicodeUTF8));
        actionAbout_JKG_Studio->setText(QApplication::translate("JKGStudioClass", "About JKG Studio", 0, QApplication::UnicodeUTF8));
        actionAbout_Qt->setText(QApplication::translate("JKGStudioClass", "About Qt", 0, QApplication::UnicodeUTF8));
        actionLua_script->setText(QApplication::translate("JKGStudioClass", "Lua script", 0, QApplication::UnicodeUTF8));
        actionDialogue->setText(QApplication::translate("JKGStudioClass", "Dialogue", 0, QApplication::UnicodeUTF8));
        actionQuest->setText(QApplication::translate("JKGStudioClass", "Quest", 0, QApplication::UnicodeUTF8));
        actionFont->setText(QApplication::translate("JKGStudioClass", "Font", 0, QApplication::UnicodeUTF8));
        actionShader->setText(QApplication::translate("JKGStudioClass", "GPU Shader", 0, QApplication::UnicodeUTF8));
        actionRedo->setText(QApplication::translate("JKGStudioClass", "Redo", 0, QApplication::UnicodeUTF8));
        actionRedo->setShortcut(QApplication::translate("JKGStudioClass", "Ctrl+Y", 0, QApplication::UnicodeUTF8));
        actionClose->setText(QApplication::translate("JKGStudioClass", "Close", 0, QApplication::UnicodeUTF8));
        actionClose->setShortcut(QApplication::translate("JKGStudioClass", "Ctrl+F4", 0, QApplication::UnicodeUTF8));
        actionDigitally_sign_PK3->setText(QApplication::translate("JKGStudioClass", "Digitally sign PK3", 0, QApplication::UnicodeUTF8));
        actionPreferences->setText(QApplication::translate("JKGStudioClass", "Preferences", 0, QApplication::UnicodeUTF8));
        actionAddEntrypoint->setText(QApplication::translate("JKGStudioClass", "Add Entrypoint", 0, QApplication::UnicodeUTF8));
        actionAddSpeechNode->setText(QApplication::translate("JKGStudioClass", "Add Speech Node", 0, QApplication::UnicodeUTF8));
        actionAddOptionNode->setText(QApplication::translate("JKGStudioClass", "Add Option Node", 0, QApplication::UnicodeUTF8));
        actionAddDynOptionNode->setText(QApplication::translate("JKGStudioClass", "Add Dynamic Options Node", 0, QApplication::UnicodeUTF8));
        actionAddLinkNode->setText(QApplication::translate("JKGStudioClass", "Add Link Node", 0, QApplication::UnicodeUTF8));
        actionAddScriptNode->setText(QApplication::translate("JKGStudioClass", "Add Script Node", 0, QApplication::UnicodeUTF8));
        actionAddInterruptNode->setText(QApplication::translate("JKGStudioClass", "Add Interrupt Node", 0, QApplication::UnicodeUTF8));
        actionAddEndNode->setText(QApplication::translate("JKGStudioClass", "Add End Node", 0, QApplication::UnicodeUTF8));
        actionAddTextEntryNode->setText(QApplication::translate("JKGStudioClass", "Add Text Entry Node", 0, QApplication::UnicodeUTF8));
        actionAddCinematicNode->setText(QApplication::translate("JKGStudioClass", "Add Cinematic Node", 0, QApplication::UnicodeUTF8));
        actionRemoveNode->setText(QApplication::translate("JKGStudioClass", "Remove Node", 0, QApplication::UnicodeUTF8));
        actionRemoveNode->setShortcut(QApplication::translate("JKGStudioClass", "Del", 0, QApplication::UnicodeUTF8));
        menuFile->setTitle(QApplication::translate("JKGStudioClass", "&File", 0, QApplication::UnicodeUTF8));
        menuNew->setTitle(QApplication::translate("JKGStudioClass", "New", 0, QApplication::UnicodeUTF8));
        menuEdit->setTitle(QApplication::translate("JKGStudioClass", "&Edit", 0, QApplication::UnicodeUTF8));
        menuAbout->setTitle(QApplication::translate("JKGStudioClass", "&Help", 0, QApplication::UnicodeUTF8));
        menuTools->setTitle(QApplication::translate("JKGStudioClass", "&Tools", 0, QApplication::UnicodeUTF8));
        menuDlgNodes->setTitle(QApplication::translate("JKGStudioClass", "&Nodes", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class JKGStudioClass: public Ui_JKGStudioClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_JKGSTUDIO_H
