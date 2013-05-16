#include "dlgundo.h"
#include "dlgnode.h"


///////////////////////////////////////////
////
//// DlgAddNodeCommand
////
///////////////////////////////////////////

DlgAddNodeCommand::DlgAddNodeCommand(DlgModel *model, QTreeView *view, const QModelIndex &parent, DlgNode::DlgNodeType type, QUndoCommand *undoParent)
	: QUndoCommand(undoParent)
{
	m_model = model;
	m_view = view;
	m_parent = reinterpret_cast<DlgNode *>(parent.internalPointer());
	Q_ASSERT_X(m_parent, "DlgAddNodeCommand", "NULL parent");

	switch (type)
	{
		case DlgNode::Entrypoint:
			m_node = new DlgEntrypointNode;
			setText("Added Entrypoint");
			break;
		case DlgNode::Speech:
			m_node = new DlgSpeechNode;
			setText("Added Speech Node");
			break;
		case DlgNode::Option:
			m_node = new DlgOptionNode;
			setText("Added Option Node");
			break;
		case DlgNode::End:
			m_node = new DlgEndNode;
			setText("Added End Node");
			break;
		case DlgNode::Script:
			m_node = new DlgScriptNode;
			setText("Added Script Node");
			break;
		case DlgNode::Interrupt:
			m_node = new DlgInterruptNode;
			setText("Added Interrupt Node");
			break;
		default:
			Q_ASSERT_X(0, "DlgAddNodeCommand", "Invalid node type!");
			setText("Added Node");
			break;
	}

	m_parent->m_refCount++;
	m_node->m_refCount++;
}

DlgAddNodeCommand::~DlgAddNodeCommand()
{
	m_node->m_refCount--;

	if (m_node->isUnreferenced())
		delete m_node;	

	m_parent->m_refCount--;

	if (m_parent->isUnreferenced())
		delete m_parent;	
}

void DlgAddNodeCommand::undo()
{
	Q_ASSERT_X(m_node->getModelIndex().isValid(), "DlgAddNodeCommand", "Model Index Invalid");  
	m_model->removeNode(m_node->getModelIndex());
}

void DlgAddNodeCommand::redo()
{
	Q_ASSERT_X(m_parent->getModelIndex().isValid(), "DlgAddNodeCommand", "Parent Model Index Invalid");  

	m_model->appendNode(m_parent->getModelIndex(), m_node);
	m_view->expand(m_parent->getModelIndex());
}


///////////////////////////////////////////
////
//// DlgSetPropertyCommand
////
///////////////////////////////////////////


DlgSetPropertyCommand::DlgSetPropertyCommand(DlgModel *model, QTreeView *view, const QModelIndex &node, QtProperty *prop, const QVariant &value, bool forceUpdate, QUndoCommand *undoParent)
	: QUndoCommand(undoParent)
{
	m_model = model;
	m_view = view;
	m_node = reinterpret_cast<DlgNode *>(node.internalPointer());
	Q_ASSERT_X(m_node, "DlgSetPropertyCommand", "m_node == NULL");
	m_node->m_refCount++;


	m_property = prop;
	m_oldValue = m_node->getProperty(prop);
	m_newValue = value;

	setText(QString("Property '%1' Changed").arg(prop->propertyName()));

	m_undone = false;
	m_forceUpdate = forceUpdate;
}

DlgSetPropertyCommand::~DlgSetPropertyCommand()
{
	m_node->m_refCount--;

	if (m_node->m_refCount <= 0 && !m_node->isLinked())
		delete m_node;
}

bool DlgSetPropertyCommand::mergeWith(const QUndoCommand *other)
{
	if (other->id() != this->id())
		return false;
	
	const DlgSetPropertyCommand *othercmd = static_cast<const DlgSetPropertyCommand *>(other);
	
	if (othercmd->m_node != this->m_node || othercmd->m_property != this->m_property)
		return false;

	m_newValue = othercmd->m_newValue;
	return true;
}

void DlgSetPropertyCommand::undo()
{
	Q_ASSERT_X(m_node->getModelIndex().isValid(), "DlgSetPropertyCommand", "Model Index Invalid");  

	m_model->propertyChanged(m_node->getModelIndex(), m_property, m_oldValue, true);
	m_undone = true;
	m_view->setCurrentIndex(m_node->getModelIndex());
}

void DlgSetPropertyCommand::redo()
{
	Q_ASSERT_X(m_node->getModelIndex().isValid(), "DlgSetPropertyCommand", "Model Index Invalid");

	m_model->propertyChanged(m_node->getModelIndex(), m_property, m_newValue, m_forceUpdate || m_undone);
	if (m_undone)
	{
		m_view->setCurrentIndex(m_node->getModelIndex());
		m_undone = false;
	}
}

///////////////////////////////////////////
////
//// DlgDeleteNodeCommand
////
///////////////////////////////////////////

DlgDeleteNodeCommand::DlgDeleteNodeCommand(DlgModel *model, const QModelIndex &index, QUndoCommand *undoParent)
	: QUndoCommand(undoParent)
{
	m_model = model;
	m_node = reinterpret_cast<DlgNode *>(index.internalPointer());
	Q_ASSERT_X(m_node, "DlgDeleteNodeCommand", "m_node == NULL");
	m_node->m_refCount++;

	m_parent = m_node->getParent();
	Q_ASSERT_X(m_parent, "DlgDeleteNodeCommand", "m_parent == NULL");
	m_parent->m_refCount++;

	m_before = m_node->getNextSibling();
	if (m_before) m_before->m_refCount++;

	setText("Node Deleted");

}

DlgDeleteNodeCommand::~DlgDeleteNodeCommand()
{
	m_node->m_refCount--;

	if (m_node->isUnreferenced())
		delete m_node;	

	m_parent->m_refCount--;

	if (m_parent->isUnreferenced())
		delete m_parent;

	if (m_before)  {
		m_before->m_refCount--;
		
		if (m_before->isUnreferenced())
			delete m_before;
	}
}

void DlgDeleteNodeCommand::undo()
{
	Q_ASSERT_X(m_parent->getModelIndex().isValid(), "DlgDeleteNodeCommand", "Parent Model Index Invalid");  

	if (m_before) {
		Q_ASSERT_X(m_before->getModelIndex().isValid(), "DlgDeleteNodeCommand", "Before Model Index Invalid");  
		m_model->insertNode(m_parent->getModelIndex(), m_before->getModelIndex(), m_node);
	} else {
		m_model->appendNode(m_parent->getModelIndex(), m_node);
	}

}

void DlgDeleteNodeCommand::redo()
{
	Q_ASSERT_X(m_node->getModelIndex().isValid(), "DlgDeleteNodeCommand", "Node Model Index Invalid");
	m_model->removeNode(m_node->getModelIndex());
}


DlgMoveNodeCommand::DlgMoveNodeCommand(DlgModel *model, const QModelIndex &node, const QModelIndex &newParent, const QModelIndex &before, QUndoCommand *undoParent)
{
	m_model = model;
	m_node = reinterpret_cast<DlgNode *>(node.internalPointer());
	Q_ASSERT_X(m_node, "DlgMoveNodeCommand", "m_node == NULL");
	m_node->m_refCount++;

	m_oldparent = m_node->getParent();
	Q_ASSERT_X(m_oldparent, "DlgMoveNodeCommand", "m_oldparent == NULL");
	m_oldparent->m_refCount++;

	m_oldbefore = m_node->getNextSibling();
	if (m_oldbefore) m_oldbefore->m_refCount++;

	m_newparent = reinterpret_cast<DlgNode *>(newParent.internalPointer());
	Q_ASSERT_X(m_node, "DlgMoveNodeCommand", "m_newparent == NULL");
	m_newparent->m_refCount++;

	if (before.isValid()) {
		m_newbefore = reinterpret_cast<DlgNode *>(before.internalPointer()); 
		m_newbefore->m_refCount++;
	} else {
		m_newbefore = NULL;
	}

	setText("Node Moved");
}

DlgMoveNodeCommand::~DlgMoveNodeCommand()
{
	m_node->m_refCount--;

	if (m_node->isUnreferenced())
		delete m_node;	

	m_oldparent->m_refCount--;

	if (m_oldparent->isUnreferenced())
		delete m_oldparent;

	if (m_oldbefore)  {
		m_oldbefore->m_refCount--;
		
		if (m_oldbefore->isUnreferenced())
			delete m_oldbefore;
	}

	m_newparent->m_refCount--;

	if (m_newparent->isUnreferenced())
		delete m_newparent;

	if (m_newbefore)  {
		m_newbefore->m_refCount--;
		
		if (m_newbefore->isUnreferenced())
			delete m_newbefore;
	}
}

void DlgMoveNodeCommand::undo()
{
	Q_ASSERT_X(m_node->getModelIndex().isValid(), "DlgMoveNodeCommand", "Model Index Invalid");  
	Q_ASSERT_X(m_oldparent->getModelIndex().isValid(), "DlgMoveNodeCommand", "Parent Model Index Invalid");  
	Q_ASSERT_X(m_oldbefore ? m_oldbefore->getModelIndex().isValid() : true, "DlgMoveNodeCommand", "Before Model Index Invalid");  

	m_model->moveNode(m_node->getModelIndex(), m_oldparent->getModelIndex(), m_oldbefore ? m_oldbefore->getModelIndex() : QModelIndex());
}

void DlgMoveNodeCommand::redo()
{
	Q_ASSERT_X(m_node->getModelIndex().isValid(), "DlgMoveNodeCommand", "Model Index Invalid");  
	Q_ASSERT_X(m_newparent->getModelIndex().isValid(), "DlgMoveNodeCommand", "Parent Model Index Invalid");  
	Q_ASSERT_X(m_newbefore ? m_newbefore->getModelIndex().isValid() : true, "DlgMoveNodeCommand", "Before Model Index Invalid");  

	m_model->moveNode(m_node->getModelIndex(), m_newparent->getModelIndex(), m_newbefore ? m_newbefore->getModelIndex() : QModelIndex());
}