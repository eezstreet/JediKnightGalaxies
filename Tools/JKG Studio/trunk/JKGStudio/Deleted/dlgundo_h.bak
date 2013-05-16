#ifndef DLGUNDO_H
#define DLGUNDO_H

#include <QUndoCommand>
#include <QModelIndex>
#include "dlgnode.h"
#include "dlgmodel.h"
#include <QTreeView>

enum
{
	Cmd_Id_AddNode = 0,
	Cmd_Id_RemoveNode,
	Cmd_Id_MoveNode,
	Cmd_Id_SetProperty,

};

class DlgAddNodeCommand : public QUndoCommand
{
public:
	DlgAddNodeCommand(DlgModel *model, QTreeView *view, const QModelIndex &parent, DlgNode::DlgNodeType type, QUndoCommand *undoParent = (QUndoCommand *)0);
	~DlgAddNodeCommand();

	int id() const { return Cmd_Id_AddNode; }
	void undo();
	void redo();
private:
	DlgModel *m_model;
	QTreeView *m_view;
	DlgNode *m_parent;
	DlgNode *m_node;
};

class DlgSetPropertyCommand : public QUndoCommand
{
public:
	DlgSetPropertyCommand(DlgModel *model, QTreeView *view, const QModelIndex &node, QtProperty *prop, const QVariant &value, bool forceUpdate = false, QUndoCommand *undoParent = (QUndoCommand *)0);
	~DlgSetPropertyCommand();

	int id() const { return Cmd_Id_SetProperty; }
	bool mergeWith(const QUndoCommand *other);
	void undo();
	void redo();
private:
	DlgModel *m_model;
	QTreeView *m_view;

	DlgNode *m_node;
	QtProperty *m_property;
	QVariant m_oldValue;
	QVariant m_newValue;
	

	bool m_undone;
	bool m_forceUpdate;
	
};

class DlgSetLabelCommand : public QUndoCommand
{
public:
	DlgSetLabelCommand(DlgModel *model, QTreeView *view, const QModelIndex &node, const QString &id, const QString &name, const QVariant &value, QUndoCommand *undoParent = (QUndoCommand *)0);
	~DlgSetLabelCommand();

	int id() const { return Cmd_Id_SetProperty; }
	bool mergeWith(const QUndoCommand *other);
	void undo();
	void redo();
private:
	DlgModel *m_model;
	QTreeView *m_view;

	DlgNode *m_node;
	QString m_id;
	QVariant m_oldValue;
	QVariant m_newValue;
	

	bool m_undone;
	
};

class DlgDeleteNodeCommand : public QUndoCommand
{
public:
	DlgDeleteNodeCommand(DlgModel *model, const QModelIndex &index, QUndoCommand *undoParent = (QUndoCommand *)0);
	~DlgDeleteNodeCommand();

	int id() const { return Cmd_Id_RemoveNode; }
	void undo();
	void redo();
private:
	DlgModel *m_model;

	DlgNode *m_parent;
	DlgNode *m_before;
	DlgNode *m_node;
};


class DlgMoveNodeCommand : public QUndoCommand
{
public:
	DlgMoveNodeCommand(DlgModel *model, const QModelIndex &node, const QModelIndex &newParent, const QModelIndex &before, QUndoCommand *undoParent = (QUndoCommand *)0);
	~DlgMoveNodeCommand();

	int id() const { return Cmd_Id_MoveNode; }
	void undo();
	void redo();
private:
	DlgModel *m_model;

	DlgNode *m_oldparent;
	DlgNode *m_oldbefore;
	DlgNode *m_newparent;
	DlgNode *m_newbefore;

	DlgNode *m_node;
};




#endif