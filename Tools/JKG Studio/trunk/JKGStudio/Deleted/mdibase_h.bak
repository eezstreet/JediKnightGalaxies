#ifndef MDIBASE_H
#define MDIBASE_H

#include <QUndoView>
#include <QMdiArea>
#include <QMdiSubWindow>
#include <QHash>
#include <QAction>
#include "propertymanager.h"
#include <QApplication>

class MdiInterface
{
public:

	static void initialize(QMdiArea *mdiParent, const QHash<QString, QAction *> &menuList, PropertyManager *propManager, QUndoView *undoView)
	{
		Q_ASSERT(m_instance == NULL);
		m_instance = new MdiInterface(mdiParent, menuList, propManager, undoView);
	}
	
	static void destroy()
	{
		Q_ASSERT(m_instance != NULL);
		delete m_instance;
		m_instance = NULL;
	}

	static MdiInterface *get()
	{
		Q_ASSERT(m_instance != NULL);
		return m_instance;
	}

	QMdiArea *mdiArea() { return m_parent; }
	QHash<QString, QAction *> menuList() { return m_menuList; };
	PropertyManager *propertyManager() { return m_propManager; };
	QUndoView *undoView() { return m_undoView; };


private:
	MdiInterface(QMdiArea *mdiParent, const QHash<QString, QAction *> &menuList, PropertyManager *propManager, QUndoView *undoView)
	{
		m_parent = mdiParent;
		m_menuList = menuList;
		m_propManager = propManager;
		m_undoView = undoView;
	}

	~MdiInterface() {};

	static MdiInterface *m_instance;

private:
	QMdiArea *m_parent;
	QHash<QString, QAction *> m_menuList;
	PropertyManager *m_propManager;
	QUndoView *m_undoView;

};


class MdiSubBase : public QMdiSubWindow
{
	Q_OBJECT
public:

	typedef enum {
		LuaEditor,
		DlgEditor,
		QuestEditor,
	} eMdiWindowType;

	typedef enum {
		DlgAddEntrypoint,
		DlgAddSpeech,
		DlgAddOption,
		DlgAddEnd,
		DlgAddScript,
		DlgAddLink,
		DlgAddInterrupt,
		DlgAddDynOpt,
		DlgAddTextEntry,
		DlgAddCinematic,
		DlgRemoveNode,
	} eMdiExtCommands;

	virtual eMdiWindowType mdiWindowType() = 0;
	virtual void newFile() = 0;
	virtual void open(const QString &filename) = 0;
	virtual void save() = 0;
	virtual void saveas() = 0;
	virtual void compile() = 0;
	virtual void cut() = 0;
	virtual void copy() = 0;
	virtual void paste() = 0;
	virtual void undo() = 0;
	virtual void redo() = 0;
	virtual void updateMenus() = 0;
	virtual void updateProperties() = 0;
	virtual void updateUndo() = 0;
	virtual void extCommand(eMdiExtCommands cmd) {}

	virtual void propertyChanged(QtProperty *prop, const QVariant &value) = 0;

	bool isActive() { return m_mdiInterface->mdiArea()->activeSubWindow() == this; }
	
	virtual void updateMdi()
	{
		updateMenus();
		updateProperties();
		updateUndo();
	}

	void acceptClose()	// Call this when subclassing the closeEvent and accepting the event, otherwise there will be a 1 tick delay before the window is closed
	{
		if (parentWidget() && testAttribute(Qt::WA_DeleteOnClose)) {
			QChildEvent childRemoved(QEvent::ChildRemoved, this);
			QApplication::sendEvent(parentWidget(), &childRemoved);
		}
	}


protected:
	MdiSubBase(MdiInterface *interface, QWidget *parent = 0, Qt::WFlags flags = 0) : QMdiSubWindow(parent, flags) {  m_mdiInterface = interface; setAttribute(Qt::WA_DeleteOnClose); };
	~MdiSubBase() {};
	
	MdiInterface *m_mdiInterface;

private:
	MdiSubBase(const MdiSubBase &);
};

#endif