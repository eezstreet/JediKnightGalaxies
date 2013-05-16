#ifndef DLGMODEL_H
#define DLGMODEL_H

#include <QAbstractItemModel>
#include "dlgnode.h"

class DlgModel : public QAbstractItemModel
{
	Q_OBJECT
public:


	DlgModel(PropertyManager *propmanager, QObject *parent = 0);
	~DlgModel();

	void newModel();
	void loadModel(const QString &filename);
	void saveModel(const QString &filename);
	void compileModel(const QString &filename);

	QModelIndex index (int row, int column, const QModelIndex & parent = QModelIndex()) const;
	QModelIndex parent (const QModelIndex & index) const;

	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
	bool setData (const QModelIndex & index, const QVariant & value, int role = Qt::EditRole);
	Qt::ItemFlags flags(const QModelIndex &index) const;
	int rowCount (const QModelIndex & parent = QModelIndex()) const;
	bool hasChildren(const QModelIndex &parent = QModelIndex()) const;

	QStringList mimeTypes() const;
	QMimeData *mimeData(const QModelIndexList &indexes) const;
	Qt::DropActions supportedDropActions() const;
	bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent);

	int columnCount (const QModelIndex & parent = QModelIndex()) const;

	QModelIndex appendNode(const QModelIndex &parent, DlgNode *node);
	QModelIndex insertNode(const QModelIndex &parent, const QModelIndex &before, DlgNode *node);
	void removeNode(const QModelIndex &index);
	QModelIndex moveNode(const QModelIndex &index, const QModelIndex &parent, const QModelIndex &before);

	void populateProperties(const QModelIndex &index);
	void propertyChanged(const QModelIndex &index, QtProperty *prop, const QVariant &value, bool forceUpdate = false);


private:

	void updateModelIndexes(DlgNode *node, bool recursive = true);

	DlgModel(const DlgModel &);

	PropertyManager *m_propmanager;
	DlgNode *m_root;

signals:

	void labelChanged(const QModelIndex &index, QtProperty *prop, const QVariant &value);
	void nodeMoved(const QModelIndex &index, const QModelIndex &newParent, const QModelIndex &before);
};

#endif