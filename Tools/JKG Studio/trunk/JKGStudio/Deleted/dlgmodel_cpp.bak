
#include "dlgmodel.h"
#include <QVariant>
#include <QMimeData>

DlgModel::DlgModel(PropertyManager *propmanager, QObject *parent) : QAbstractItemModel(parent)
{
	m_propmanager = propmanager;
	m_root = NULL;
}

DlgModel::~DlgModel()
{
	if (m_root)
	{
		m_root->m_refCount--;
		Q_ASSERT_X(m_root->isUnreferenced(), "DlgModel Destructor", "Root not unreferenced!");
		delete m_root;
	}
}


void DlgModel::newModel()
{
	if (m_root)
		delete m_root;

	m_root = new DlgRootNode;
	m_root->setModelIndex(createIndex(0,0,m_root));
	m_root->m_refCount++;
}

void DlgModel::loadModel(const QString &filename)
{

}

void DlgModel::saveModel(const QString &filename)
{

}

void DlgModel::compileModel(const QString &filename)
{

}

QModelIndex DlgModel::index (int row, int column, const QModelIndex & parent) const
{
	if (parent.isValid())
	{
		DlgNode *node = reinterpret_cast<DlgNode *>(parent.internalPointer());
		DlgNode *child;

		child = node->getChild(row);
		if (!child)
			return QModelIndex();

		return createIndex(row, column, child);

	} else {
		if (row != 0) 
			return QModelIndex();

		return createIndex(row, column, m_root);
	}
}

QModelIndex DlgModel::parent (const QModelIndex & index) const
{
	if (!index.isValid())
		return QModelIndex();

	DlgNode *node = reinterpret_cast<DlgNode *>(index.internalPointer());


	if (node && node->getParent()) {
		if (node->getParent()->getModelIndex().isValid())
			return node->getParent()->getModelIndex();

		if (node->getParent()->getParent()) {
			int row = node->getParent()->getParent()->getChildIndex(node->getParent());
			if (row != -1)
				return createIndex(row, 0, node->getParent());
		} else {
			return createIndex(0, 0, node->getParent());
		}
	}

	return QModelIndex();

}

QVariant DlgModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	return QVariant();
}

QVariant DlgModel::data(const QModelIndex &index, int role) const
{
	if (!index.isValid())
		return QVariant();

	DlgNode *node = reinterpret_cast<DlgNode *>(index.internalPointer());

	switch (role)
	{
	case Qt::DisplayRole:
		return node->description();
	case Qt::DecorationRole:
		return node->icon();
	case Qt::EditRole:
		return node->getEditText();
	default:
		return QVariant();
	}
}

bool DlgModel::setData (const QModelIndex & index, const QVariant & value, int role)
{
	if (!index.isValid())
		return false;

	DlgNode *node = reinterpret_cast<DlgNode *>(index.internalPointer());
	if (!node->isEditable())
		return false;

	switch (role)
	{
	case Qt::EditRole:
		// To properly update the properties and undo/redo, route this through the dlg's undo framework
		emit labelChanged(index, node->getEditProperty(), value);
		return true;
	default:
		return false;
	}
}

Qt::ItemFlags DlgModel::flags(const QModelIndex &index) const
{
	if (!index.isValid())
		return 0;

	Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;

	if (reinterpret_cast<DlgNode *>(index.internalPointer())->isEditable())
		flags |= Qt::ItemIsEditable;

	flags |= Qt::ItemIsDropEnabled;

	if (index.parent().isValid())
		flags |= Qt::ItemIsDragEnabled;

	
	return flags;
}

Qt::DropActions DlgModel::supportedDropActions() const
{
    return Qt::CopyAction | Qt::MoveAction | Qt::LinkAction;
}


QStringList DlgModel::mimeTypes() const
{
    QStringList types;
    types << "application/jkgstudio.dlg.imove";
    return types;
}

QMimeData *DlgModel::mimeData(const QModelIndexList &indexes) const
{
	QMimeData *mimeData = new QMimeData();
	QByteArray encodedData;

	// Only 1 index is supported
	

    QDataStream stream(&encodedData, QIODevice::WriteOnly);

    if (indexes.at(0).isValid())
		stream << (quint32)indexes.at(0).internalPointer();


    mimeData->setData("application/jkgstudio.dlg.imove", encodedData);
	
    return mimeData;
}

bool DlgModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
	if (!data->hasFormat("application/jkgstudio.dlg.imove"))
		return false;

	QByteArray encodedData = data->data("application/jkgstudio.dlg.imove");
	QDataStream stream(&encodedData, QIODevice::ReadOnly);
	
	DlgNode *node = NULL;

	stream >> (quint32 &)node;

	Q_ASSERT(node);
	Q_ASSERT(node->getModelIndex().isValid());

	emit nodeMoved(node->getModelIndex(), parent, index(row, column, parent));

	return true;
}

int DlgModel::rowCount (const QModelIndex & parent) const
{
	if (parent.isValid()) {
		DlgNode *node = reinterpret_cast<DlgNode *>(parent.internalPointer());
		return node->getChildrenCount();
	}
	return m_root ? 1 : 0;
}

bool DlgModel::hasChildren(const QModelIndex &parent) const
{
	if (parent.isValid()) {
		DlgNode *node = reinterpret_cast<DlgNode *>(parent.internalPointer());
		return node->hasChildren();
	}
	return m_root ? true : false;
}

int DlgModel::columnCount (const QModelIndex & parent) const
{
	return 1;
}


QModelIndex DlgModel::appendNode(const QModelIndex &parent, DlgNode *node)
{
	if (!parent.isValid())
		return QModelIndex();

	DlgNode *parentnode = reinterpret_cast<DlgNode *>(parent.internalPointer());

	if (!parentnode->childAllowed(node->nodeType()) || node->isLinked())
		return QModelIndex();


	emit beginInsertRows(parent, parentnode->getChildrenCount(), parentnode->getChildrenCount());
	parentnode->appendChild(node);
	emit endInsertRows();
		
	updateModelIndexes(parentnode, false);
	updateModelIndexes(node, true);

	return node->getModelIndex();
}


QModelIndex DlgModel::insertNode(const QModelIndex &parent, const QModelIndex &before, DlgNode *node)
{
	if (!parent.isValid() || !before.isValid() || before.parent() != parent)
		return QModelIndex();

	DlgNode *parentnode = reinterpret_cast<DlgNode *>(parent.internalPointer());
	DlgNode *beforenode = reinterpret_cast<DlgNode *>(before.internalPointer());

	if (!parentnode->childAllowed(node->nodeType()) || node->isLinked())
		return QModelIndex();


	int row = parentnode->getChildIndex(beforenode);
	Q_ASSERT_X(row != -1, "insertNode", "row == -1");

	emit beginInsertRows(parent, row, row);
	parentnode->insertChild(node, beforenode);
	emit endInsertRows();
		
	updateModelIndexes(parentnode, false);
	updateModelIndexes(node, true);

	return node->getModelIndex();

}

void DlgModel::removeNode(const QModelIndex &index)
{
	if (!index.isValid())
		return;

	DlgNode *node = reinterpret_cast<DlgNode *>(index.internalPointer());
	DlgNode *parent = node->getParent();

	emit beginRemoveRows(index.parent(), index.row(), index.row());
	node->unlink();
	emit endRemoveRows();

	updateModelIndexes(parent, false);
}

QModelIndex DlgModel::moveNode(const QModelIndex &index, const QModelIndex &parent, const QModelIndex &before)
{
	if (!index.isValid() || !parent.isValid())
		return QModelIndex();

	DlgNode *node = reinterpret_cast<DlgNode *>(index.internalPointer());
	DlgNode *parentnode = reinterpret_cast<DlgNode *>(parent.internalPointer());
	DlgNode *oldparentnode = reinterpret_cast<DlgNode *>(index.parent().internalPointer());

	if (!parentnode->childAllowed(node->nodeType()))
		return QModelIndex();


	int newRow = before.isValid() ? before.row() : parentnode->getChildrenCount();

	emit beginMoveRows(index.parent(), index.row(), index.row(), parent, newRow);
	node->unlink();
	if (before.isValid()) {
		DlgNode *beforenode = reinterpret_cast<DlgNode *>(before.internalPointer());
		parentnode->insertChild(node, beforenode);
	} else {
		parentnode->appendChild(node);
	}
	emit endMoveRows();
	
	updateModelIndexes(oldparentnode, false);
	updateModelIndexes(parentnode, false);
	updateModelIndexes(node, true);

	return node->getModelIndex();
}

void DlgModel::populateProperties(const QModelIndex &index)
{
	if (!index.isValid() || !index.parent().isValid()) {
		return;
	} else {
		DlgNode *node = reinterpret_cast<DlgNode *>(index.internalPointer());
		m_propmanager->setActiveOwner(node);
	}
}

void DlgModel::propertyChanged(const QModelIndex &index, QtProperty *prop, const QVariant &value, bool forceUpdate)
{
	if (!index.isValid() || !index.parent().isValid()) {
		return;
	} else {
		DlgNode *node = reinterpret_cast<DlgNode *>(index.internalPointer());
		node->setProperty(prop, value, forceUpdate);
		emit dataChanged(index, index);
	}
}


void DlgModel::updateModelIndexes(DlgNode *node, bool recursive)
{
	// Recursively updates all model indexes of the node's children
	QModelIndex parent = node->getModelIndex();
	int i = 0;
	for (DlgNode *child = node->getFirstChild(); child; child = child->getNextSibling(), i++)
	{
		child->setModelIndex(createIndex(i, 0, child));
		if (recursive) updateModelIndexes(child, true);

	}
}