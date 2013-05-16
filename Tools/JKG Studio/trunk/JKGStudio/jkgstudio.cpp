#include "jkgstudio.h"

#include "luaeditor.h"
#include "dlgeditor.h"
#include <QDockWidget>
#include <QTextEdit>
#include <QToolBar>
#include <QMessageBox>
#include <QCloseEvent>
#include <QList>
#include <QTabBar>

#include <QtWindowListMenu>
#include <QToolButton>
#include <QFileDialog>
#include <QFileInfo>
#include <QSignalMapper>

JKGStudio::JKGStudio(QWidget *parent, Qt::WFlags flags)
	: QMainWindow(parent, flags)
{
	ui.setupUi(this);
	
	QToolBar *bar = addToolBar("Standard");
	bar->setFloatable(false);
	bar->setIconSize(QSize(16,16));
	
	QToolButton *newBtn = new QToolButton(this);
	newBtn->setMenu(ui.menuNew);
	newBtn->setIcon(QIcon(":/icons/new.png"));
	newBtn->setPopupMode(QToolButton::InstantPopup);
	newBtn->setText("New");

	ui.menuNew->menuAction()->setIcon(QIcon(":/icons/new.png"));

	bar->addWidget(newBtn);

	bar->addAction(ui.actionOpen);
	bar->addAction(ui.actionSave);
	bar->addSeparator();
	bar->addAction(ui.actionCut);
	bar->addAction(ui.actionCopy);
	bar->addAction(ui.actionPaste);
	bar->addSeparator();
	bar->addAction(ui.actionUndo);
	bar->addAction(ui.actionRedo);

	// Setup menu list
	setupMenuList();
	setupExtCommands();

	m_mdi = ui.mdiArea;

	// Create window menu
	QtWindowListMenu *windowList = new QtWindowListMenu(this);
	windowList->standardAction(QtWindowListMenu::CloseAction)->setIcon(QIcon(":/icons/close.png"));
	windowList->standardAction(QtWindowListMenu::CloseAllAction)->setIcon(QIcon(":/icons/closeall.png"));
	windowList->standardAction(QtWindowListMenu::TileAction)->setIcon(QIcon(":/icons/tile.png"));
	windowList->standardAction(QtWindowListMenu::CascadeAction)->setIcon(QIcon(":/icons/cascade.png"));
	windowList->standardAction(QtWindowListMenu::NextAction)->setIcon(QIcon(":/icons/next.png"));
	windowList->standardAction(QtWindowListMenu::PrevAction)->setIcon(QIcon(":/icons/prev.png"));
	
	windowList->attachToMdiArea(m_mdi);
	menuBar()->insertMenu(ui.menuAbout->menuAction(), windowList);

	
	this->setCentralWidget(m_mdi);

	QTabBar *tabbar = m_mdi->findChild<QTabBar *>();
	if (tabbar)
		tabbar->setExpanding(false);
	
	// Qt 4.8 merge
	m_mdi->setTabsClosable(true);
	m_mdi->setTabsMovable(true);

	delete ui.centralWidget;

	QDockWidget *propdock = new QDockWidget("Properties", this);
	m_propbrowser = new QtTreePropertyBrowser(this);
	m_propmanager = new PropertyManager(m_propbrowser, this);

	propdock->toggleViewAction()->setIcon(QIcon(":/icons/properties.png"));
	propdock->setWidget(m_propbrowser);
	addDockWidget(Qt::RightDockWidgetArea, propdock);

	QDockWidget *undodock = new QDockWidget("Undo History", this);
	m_undoview = new QUndoView(this);

	//propdock->toggleViewAction()->setIcon(QIcon(":/icons/properties.png"));
	undodock->setWidget(m_undoview);
	addDockWidget(Qt::RightDockWidgetArea, undodock);

	QAction *sep = new QAction(this);
	sep->setSeparator(true);

	windowList->insertAction(windowList->actions().at(0), sep);
	windowList->insertAction(windowList->actions().at(0), undodock->toggleViewAction());
	windowList->insertAction(windowList->actions().at(0), propdock->toggleViewAction());
	
	MdiInterface::initialize(m_mdi, m_menuList, m_propmanager, m_undoview);
	m_interface = MdiInterface::get();

	onSubWindowActivated();
}

JKGStudio::~JKGStudio()
{
	MdiInterface::destroy();
}

void JKGStudio::setupMenuList()
{
	m_menuList.clear();
	m_menuList["save"] = ui.actionSave;
	m_menuList["saveas"] = ui.actionSave_As;
	m_menuList["undo"] = ui.actionUndo;
	m_menuList["redo"] = ui.actionRedo;
	m_menuList["cut"] = ui.actionCut;
	m_menuList["copy"] = ui.actionCopy;
	m_menuList["paste"] = ui.actionPaste;
	m_menuList["compile"] = ui.actionCompile;

	m_menuList["dlg-addentrypoint"] = ui.actionAddEntrypoint;
	m_menuList["dlg-addspeech"] = ui.actionAddSpeechNode;
	m_menuList["dlg-addoption"] = ui.actionAddOptionNode;
	m_menuList["dlg-addend"] = ui.actionAddEndNode;
	m_menuList["dlg-addscript"] = ui.actionAddScriptNode;
	m_menuList["dlg-addinterrupt"] = ui.actionAddInterruptNode;
	m_menuList["dlg-addlink"] = ui.actionAddLinkNode;
	m_menuList["dlg-adddynopt"] = ui.actionAddDynOptionNode;
	m_menuList["dlg-addtextentry"] = ui.actionAddTextEntryNode;
	m_menuList["dlg-addcinematic"] = ui.actionAddCinematicNode;
	
	m_menuList["dlg-removenode"] = ui.actionRemoveNode;
}

void JKGStudio::setupExtCommands()
{
	QSignalMapper *mapper = new QSignalMapper(this);

	mapper->setMapping(ui.actionAddEntrypoint, (int)MdiSubBase::DlgAddEntrypoint);
	connect(ui.actionAddEntrypoint, SIGNAL(triggered()), mapper, SLOT(map()));

	mapper->setMapping(ui.actionAddSpeechNode, (int)MdiSubBase::DlgAddSpeech);
	connect(ui.actionAddSpeechNode, SIGNAL(triggered()), mapper, SLOT(map()));

	mapper->setMapping(ui.actionAddOptionNode, (int)MdiSubBase::DlgAddOption);
	connect(ui.actionAddOptionNode, SIGNAL(triggered()), mapper, SLOT(map()));

	mapper->setMapping(ui.actionAddEndNode, (int)MdiSubBase::DlgAddEnd);
	connect(ui.actionAddEndNode, SIGNAL(triggered()), mapper, SLOT(map()));

	mapper->setMapping(ui.actionAddScriptNode, (int)MdiSubBase::DlgAddScript);
	connect(ui.actionAddScriptNode, SIGNAL(triggered()), mapper, SLOT(map()));

	mapper->setMapping(ui.actionAddInterruptNode, (int)MdiSubBase::DlgAddInterrupt);
	connect(ui.actionAddInterruptNode, SIGNAL(triggered()), mapper, SLOT(map()));

	mapper->setMapping(ui.actionAddLinkNode, (int)MdiSubBase::DlgAddLink);
	connect(ui.actionAddLinkNode, SIGNAL(triggered()), mapper, SLOT(map()));

	mapper->setMapping(ui.actionAddDynOptionNode, (int)MdiSubBase::DlgAddDynOpt);
	connect(ui.actionAddDynOptionNode, SIGNAL(triggered()), mapper, SLOT(map()));

	mapper->setMapping(ui.actionAddTextEntryNode, (int)MdiSubBase::DlgAddTextEntry);
	connect(ui.actionAddTextEntryNode, SIGNAL(triggered()), mapper, SLOT(map()));

	mapper->setMapping(ui.actionAddCinematicNode, (int)MdiSubBase::DlgAddCinematic);
	connect(ui.actionAddCinematicNode, SIGNAL(triggered()), mapper, SLOT(map()));

	mapper->setMapping(ui.actionRemoveNode, (int)MdiSubBase::DlgRemoveNode);
	connect(ui.actionRemoveNode, SIGNAL(triggered()), mapper, SLOT(map()));

	connect(mapper, SIGNAL(mapped(int)), this, SLOT(onExtCommand(int)));

}

MdiSubBase *JKGStudio::activeSubWindow()
{
	MdiSubBase *window = qobject_cast<MdiSubBase *>(m_mdi->currentSubWindow());
	return window;
}

void JKGStudio::onSubWindowActivated()
{
	if (!activeSubWindow())
	{
		ui.actionSave->setEnabled(false);
		ui.actionSave_As->setEnabled(false);
		ui.actionUndo->setEnabled(false);
		ui.actionRedo->setEnabled(false);
		ui.actionCut->setEnabled(false);
		ui.actionCopy->setEnabled(false);
		ui.actionPaste->setEnabled(false);
		ui.actionCompile->setEnabled(false);

		ui.menuDlgNodes->menuAction()->setVisible(false);

		m_propmanager->setActiveOwner(NULL);
		m_propmanager->setActiveWindow(NULL);
		m_propbrowser->setEnabled(false);

		m_undoview->setStack(NULL);
		m_undoview->setEnabled(false);

		
		return;
	}

	switch (activeSubWindow()->mdiWindowType())
	{
		case MdiSubBase::LuaEditor:
			ui.menuDlgNodes->menuAction()->setVisible(false);
			break;
		case MdiSubBase::DlgEditor:
			ui.menuDlgNodes->menuAction()->setVisible(true);
			break;
		case MdiSubBase::QuestEditor:
			ui.menuDlgNodes->menuAction()->setVisible(false);
			break;
		default:
			ui.menuDlgNodes->menuAction()->setVisible(false);
			break;
	}

	m_propmanager->setActiveWindow(activeSubWindow());
	m_propbrowser->setEnabled(true);

	activeSubWindow()->updateMdi();
}

void JKGStudio::onNewScript()
{
	LuaEditor *edit = new LuaEditor(m_interface);
	edit->newFile();
	m_mdi->addSubWindow(edit);
	edit->show();
}

void JKGStudio::onNewDialogue()
{
	DlgEditor *dlg = new DlgEditor(m_interface);
	dlg->newFile();
	m_mdi->addSubWindow(dlg);
	dlg->show();
}

void JKGStudio::onNewQuest()
{
	QMessageBox::information(this, "Not implemented", "Quests are not yet implemented");
}

void JKGStudio::onOpen()
{
	QString path = QFileDialog::getOpenFileName(this, "Open file", QString(), "Lua Script (*.lua)");
	if (path.isEmpty())
		return;

	QFileInfo fi(path);

	if (fi.suffix() == "lua")
	{
		LuaEditor *edit = new LuaEditor(m_interface);
		edit->open(path);
		m_mdi->addSubWindow(edit);
		edit->show();
	} else {
		QMessageBox::information(this, "Unknown file type", QString("Cannot open %1.\nFile type unknown.").arg(fi.fileName()));
	}
}

void JKGStudio::onSave()
{
	if (!activeSubWindow())
		return;

	activeSubWindow()->save();
}

void JKGStudio::onSaveAs()
{
	if (!activeSubWindow())
		return;

	activeSubWindow()->saveas();
}

void JKGStudio::onCompile()
{
	if (!activeSubWindow())
		return;

	activeSubWindow()->compile();
}

void JKGStudio::onCloseTab(int idx)
{
	QList<QMdiSubWindow *> list = m_mdi->subWindowList();
	if (idx < 0 || list.count() <= idx)
		return;

	list.at(idx)->close();
}

void JKGStudio::onClose()
{
	m_mdi->closeActiveSubWindow();
}

void JKGStudio::closeEvent(QCloseEvent *ev)
{
	m_mdi->closeAllSubWindows();
	if (m_mdi->currentSubWindow())
		ev->ignore();
	else
		ev->accept();
}

void JKGStudio::onQuit()
{
	close();
}

void JKGStudio::onUndo()
{
	if (!activeSubWindow())
		return;

	activeSubWindow()->undo();
}

void JKGStudio::onRedo()
{
	if (!activeSubWindow())
		return;

	activeSubWindow()->redo();
}

void JKGStudio::onCut()
{
	if (!activeSubWindow())
		return;

	activeSubWindow()->cut();
}

void JKGStudio::onCopy()
{
	if (!activeSubWindow())
		return;

	activeSubWindow()->copy();
}

void JKGStudio::onPaste()
{
	if (!activeSubWindow())
		return;

	activeSubWindow()->paste();
}

void JKGStudio::onExtCommand(int cmd)
{
	if (!activeSubWindow())
		return;

	activeSubWindow()->extCommand((MdiSubBase::eMdiExtCommands)cmd);
}