#ifndef JKGSTUDIO_H
#define JKGSTUDIO_H

#include <QtGui/QMainWindow>
#include <QMdiArea>
#include <QEvent>
#include "ui_jkgstudio.h"
#include "mdibase.h"
#include "propertymanager.h"
#include <QtTreePropertyBrowser>
#include <QUndoView>

class JKGStudio : public QMainWindow
{
	Q_OBJECT

public:
	JKGStudio(QWidget *parent = 0, Qt::WFlags flags = 0);
	~JKGStudio();

	void closeEvent(QCloseEvent *ev);

private:
	Ui::JKGStudioClass ui;

	QMdiArea *m_mdi;

	MdiSubBase *activeSubWindow();

	void setupMenuList();
	void setupExtCommands();
	QHash<QString, QAction *> m_menuList;
	QtTreePropertyBrowser *m_propbrowser;
	PropertyManager *m_propmanager;
	QUndoView *m_undoview;
	MdiInterface *m_interface;

private slots:

	void onNewScript();
	void onNewDialogue();
	void onNewQuest();
	void onOpen();
	void onSave();
	void onSaveAs();
	void onCompile();
	void onClose();
	void onQuit();
	void onUndo();
	void onRedo();
	void onCut();
	void onCopy();
	void onPaste();
	void onCloseTab(int idx);

	void onSubWindowActivated();

	void onExtCommand(int cmd);

};

#endif // JKGSTUDIO_H
