#ifndef DLGEDITOR_H
#define DLGEDITOR_H

#include "mdibase.h"
#include <QTreeView>
#include <QUndoStack>
#include "dlgmodel.h"

class DlgEditor : public MdiSubBase
{
	Q_OBJECT
public:

	DlgEditor(MdiInterface *interface, QWidget *parent = 0, Qt::WFlags flags = 0);
	~DlgEditor();
	virtual eMdiWindowType mdiWindowType() { return MdiSubBase::DlgEditor; };
	virtual void newFile();
	virtual void open(const QString &filename);
	virtual void save();
	virtual void saveas();
	virtual void compile();
	virtual void cut();
	virtual void copy();
	virtual void paste();
	virtual void undo();
	virtual void redo();
	virtual void updateMenus();
	virtual void updateProperties();
	virtual void updateUndo();
	virtual void extCommand(eMdiExtCommands cmd);
	virtual void propertyChanged(QtProperty *prop, const QVariant &value);

private:

	QString m_filename;
	QString m_filetitle;
	bool m_changed;
	bool m_untitled;

	QTreeView *m_view;
	DlgModel *m_model;
	QUndoStack *m_undostack;
	QMenu *m_viewContext;

	QtProperty *m_propTitle;
	QtProperty *m_propPath;

private slots:
	void onSelectionChanged(const QModelIndex &index);
	void onUndoChanged(bool enabled);
	void onRedoChanged(bool enabled);
	void onCleanChanged(bool clean);

	void onContext(const QPoint &pt);

	void onLabelChanged(const QModelIndex &index, QtProperty *prop, const QVariant &value);
	void onNodeMove(const QModelIndex &index, const QModelIndex &newParent, const QModelIndex &before);
};

#endif