#include "dlgeditor.h"
#include "dlgundo.h"
#include <QMenu>

DlgEditor::DlgEditor(MdiInterface *interface, QWidget *parent, Qt::WFlags flags) : MdiSubBase(interface, parent, flags)
{
	setWindowIcon(QIcon(":/icons/dlg/dialogue.png"));
	m_view = new QTreeView(this);
	setWidget(m_view);

	m_view->setHeaderHidden(true);
	m_view->setAlternatingRowColors(true);
	m_view->setAnimated(true);

	m_view->setSelectionMode(QAbstractItemView::SingleSelection);
	m_view->setDragEnabled(true);
	m_view->setAcceptDrops(true);
	m_view->setDropIndicatorShown(true);
	m_view->setDragDropMode(QAbstractItemView::InternalMove);

	m_model = new DlgModel(interface->propertyManager(),  this);
	m_view->setModel(m_model);

	m_undostack = new QUndoStack(this);
	
	m_viewContext = new QMenu(this);
	QMenu *menuAdd = m_viewContext->addMenu("Add");
	
	menuAdd->addAction(interface->menuList()["dlg-addentrypoint"]);
	menuAdd->addAction(interface->menuList()["dlg-addspeech"]);
	menuAdd->addAction(interface->menuList()["dlg-addoption"]);
	menuAdd->addAction(interface->menuList()["dlg-addend"]);
	menuAdd->addSeparator();
	menuAdd->addAction(interface->menuList()["dlg-addscript"]);
	menuAdd->addAction(interface->menuList()["dlg-addinterrupt"]);
	menuAdd->addSeparator();
	menuAdd->addAction(interface->menuList()["dlg-addlink"]);
	menuAdd->addSeparator();
	menuAdd->addAction(interface->menuList()["dlg-adddynopt"]);
	menuAdd->addAction(interface->menuList()["dlg-addtextentry"]);
	menuAdd->addAction(interface->menuList()["dlg-addcinematic"]);

	m_viewContext->addSeparator();
	m_viewContext->addAction(interface->menuList()["cut"]);
	m_viewContext->addAction(interface->menuList()["copy"]);
	m_viewContext->addAction(interface->menuList()["paste"]);
	m_viewContext->addSeparator();
	m_viewContext->addAction(interface->menuList()["dlg-removenode"]);

	connect(m_view->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)), this, SLOT(onSelectionChanged(QModelIndex)));

	connect(m_undostack, SIGNAL(canUndoChanged (bool)), this, SLOT(onUndoChanged(bool)));
	connect(m_undostack, SIGNAL(canRedoChanged (bool)), this, SLOT(onRedoChanged(bool)));
	connect(m_undostack, SIGNAL(cleanChanged (bool)), this, SLOT(onCleanChanged(bool)));

	m_view->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(m_view, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(onContext(const QPoint &)));

	connect(m_model, SIGNAL(labelChanged(const QModelIndex &, QtProperty *, const QVariant &)), this, SLOT(onLabelChanged(const QModelIndex &, QtProperty *, const QVariant &)));
	connect(m_model, SIGNAL(nodeMoved(const QModelIndex &,const QModelIndex &,const QModelIndex &)), this, SLOT(onNodeMove(const QModelIndex &,const QModelIndex &,const QModelIndex &))); 

	m_propTitle = m_mdiInterface->propertyManager()->addLabelProperty(this, "Name", m_filetitle);
	m_propPath = m_mdiInterface->propertyManager()->addLabelProperty(this, "Path", m_filename);

}

DlgEditor::~DlgEditor()
{
	m_undostack->clear();

}

void DlgEditor::newFile()
{
	static int sequence = 1;

	m_undostack->clear();
	m_undostack->setClean();
	m_model->newModel();
	m_view->reset();
	
	m_filetitle = QString("untitled%1.dlg").arg(sequence++);
	m_filename = "";
	m_changed = false;
	m_untitled = true;


	m_mdiInterface->propertyManager()->setLabelValue(m_propTitle, m_filetitle);
	m_mdiInterface->propertyManager()->setLabelValue(m_propPath, m_filename);

	if (isActive())
		m_mdiInterface->undoView()->setEmptyLabel("New Dialogue");

	setWindowTitle(m_filetitle);

}

void DlgEditor::open(const QString &filename)
{

}

void DlgEditor::save()
{

}

void DlgEditor::saveas()
{

}

void DlgEditor::compile()
{

}

void DlgEditor::cut()
{

}

void DlgEditor::copy()
{

}

void DlgEditor::paste()
{

}

void DlgEditor::undo()
{
	m_undostack->undo();
}

void DlgEditor::redo()
{
	m_undostack->redo();
}

void DlgEditor::updateMenus()
{
	QModelIndex index = m_view->currentIndex();

	m_mdiInterface->menuList()["saveas"]->setEnabled(true);
	m_mdiInterface->menuList()["compile"]->setEnabled(true);

	m_mdiInterface->menuList()["undo"]->setEnabled(m_undostack->canUndo());
	m_mdiInterface->menuList()["redo"]->setEnabled(m_undostack->canRedo());


	DlgNode *node = reinterpret_cast<DlgNode*>(index.internalPointer());
	if (!node) {
		m_mdiInterface->menuList()["dlg-addentrypoint"]->setEnabled(false);
		m_mdiInterface->menuList()["dlg-addspeech"]->setEnabled(false);
		m_mdiInterface->menuList()["dlg-addoption"]->setEnabled(false);
		m_mdiInterface->menuList()["dlg-addend"]->setEnabled(false);
		m_mdiInterface->menuList()["dlg-addscript"]->setEnabled(false);
		m_mdiInterface->menuList()["dlg-addinterrupt"]->setEnabled(false);
		m_mdiInterface->menuList()["dlg-addlink"]->setEnabled(false);
		m_mdiInterface->menuList()["dlg-adddynopt"]->setEnabled(false);
		m_mdiInterface->menuList()["dlg-addtextentry"]->setEnabled(false);
		m_mdiInterface->menuList()["dlg-addcinematic"]->setEnabled(false);
		m_mdiInterface->menuList()["dlg-removenode"]->setEnabled(false);
	} else {
		m_mdiInterface->menuList()["dlg-addentrypoint"]->setEnabled(node->childAllowed(DlgNode::Entrypoint));
		m_mdiInterface->menuList()["dlg-addspeech"]->setEnabled(node->childAllowed(DlgNode::Speech));
		m_mdiInterface->menuList()["dlg-addoption"]->setEnabled(node->childAllowed(DlgNode::Option));
		m_mdiInterface->menuList()["dlg-addend"]->setEnabled(node->childAllowed(DlgNode::End));
		m_mdiInterface->menuList()["dlg-addscript"]->setEnabled(node->childAllowed(DlgNode::Script));
		m_mdiInterface->menuList()["dlg-addinterrupt"]->setEnabled(node->childAllowed(DlgNode::Interrupt));
		m_mdiInterface->menuList()["dlg-addlink"]->setEnabled(node->childAllowed(DlgNode::Link));
		m_mdiInterface->menuList()["dlg-adddynopt"]->setEnabled(node->childAllowed(DlgNode::DynOption));
		m_mdiInterface->menuList()["dlg-addtextentry"]->setEnabled(node->childAllowed(DlgNode::TextEntry));
		m_mdiInterface->menuList()["dlg-addcinematic"]->setEnabled(node->childAllowed(DlgNode::Cinematic));
		m_mdiInterface->menuList()["dlg-removenode"]->setEnabled(node->nodeType() != DlgNode::Root);
	}	
}

void DlgEditor::updateProperties()
{
	if (!isActive())
		return;

	QModelIndex index = m_view->currentIndex();

	if (!index.isValid() || !index.parent().isValid()) {
		// No node or root node selected, show file properties instead
		m_mdiInterface->propertyManager()->setActiveOwner(this);
		//m_mdiInterface->propertyManager()->addLabelProperty("title", "Name", m_filetitle);
		//m_mdiInterface->propertyManager()->addLabelProperty("path", "Path", m_filename);

	} else {
		m_model->populateProperties(m_view->currentIndex());
	}
}

void DlgEditor::updateUndo()
{
	m_mdiInterface->undoView()->setStack(m_undostack);
	m_mdiInterface->undoView()->setEnabled(true);
	m_mdiInterface->undoView()->setEmptyLabel("New Dialogue");
}

void DlgEditor::onSelectionChanged(const QModelIndex &index)
{
	updateMenus();
	updateProperties();
}

void DlgEditor::onUndoChanged(bool enabled)
{
	m_mdiInterface->menuList()["undo"]->setEnabled(enabled);
}

void DlgEditor::onRedoChanged(bool enabled)
{
	m_mdiInterface->menuList()["redo"]->setEnabled(enabled);
}

void DlgEditor::onCleanChanged(bool clean)
{
	m_mdiInterface->menuList()["save"]->setEnabled(!clean);
	m_changed = !clean;

	setWindowTitle(QString("%1%2").arg(m_filetitle).arg(!clean ? "*" : ""));
}

void DlgEditor::onContext(const QPoint &pt)
{
	m_viewContext->popup(m_view->mapToGlobal(pt));
}

void DlgEditor::extCommand(eMdiExtCommands cmd)
{
	QUndoCommand *command;

	switch (cmd)
	{
		case DlgAddEntrypoint:
			command = new DlgAddNodeCommand(m_model, m_view, m_view->currentIndex(), DlgNode::Entrypoint);
			m_undostack->push(command);
			break;
		case DlgAddSpeech:
			command = new DlgAddNodeCommand(m_model, m_view, m_view->currentIndex(), DlgNode::Speech);
			m_undostack->push(command);
			break;
		case DlgAddOption:
			command = new DlgAddNodeCommand(m_model, m_view, m_view->currentIndex(), DlgNode::Option);
			m_undostack->push(command);
			break;
		case DlgAddEnd:
			command = new DlgAddNodeCommand(m_model, m_view, m_view->currentIndex(), DlgNode::End);
			m_undostack->push(command);
			break;
		case DlgAddScript:
			command = new DlgAddNodeCommand(m_model, m_view, m_view->currentIndex(), DlgNode::Script);
			m_undostack->push(command);
			break;
		case DlgAddInterrupt:
			command = new DlgAddNodeCommand(m_model, m_view, m_view->currentIndex(), DlgNode::Interrupt);
			m_undostack->push(command);
			break;
		case DlgRemoveNode:
			command = new DlgDeleteNodeCommand(m_model, m_view->currentIndex());
			m_undostack->push(command);
			break;
	}
}

void DlgEditor::onLabelChanged(const QModelIndex &index, QtProperty *prop, const QVariant &value)
{
	if (!isActive())
		return;

	if (!index.isValid() || !index.parent().isValid()) {
		return;
	} else {
		DlgSetPropertyCommand *command = new DlgSetPropertyCommand(m_model, m_view, index, prop, value); 
		m_undostack->push(command);
	}
}

void DlgEditor::onNodeMove(const QModelIndex &index, const QModelIndex &newParent, const QModelIndex &before)
{
	if (!isActive())
		return;

	if (!index.isValid() || !newParent.isValid())
		return;

	DlgNode *node = reinterpret_cast<DlgNode *>(index.internalPointer());
	DlgNode *parent = reinterpret_cast<DlgNode *>(newParent.internalPointer());

	// TODO: check link nodes
	if (!parent->childAllowed(node->nodeType()))
		return;

	DlgMoveNodeCommand *command = new DlgMoveNodeCommand(m_model, index, newParent, before); 
	m_undostack->push(command);
}

void DlgEditor::propertyChanged(QtProperty *prop, const QVariant &value)
{
	if (!isActive())
		return;

	QModelIndex index = m_view->currentIndex();

	if (!index.isValid() || !index.parent().isValid()) {
		return;
	} else {
		DlgSetPropertyCommand *command = new DlgSetPropertyCommand(m_model, m_view, m_view->currentIndex(), prop, value); 
		m_undostack->push(command);
	}
}