#include "dlgnode.h"
#include "mdibase.h"
#include <QFile>
#include <QPainter>

DlgNode::DlgNode()
{
	m_parent = NULL;
	m_firstChild = NULL;
	m_lastChild = NULL;
	m_nextSibling = NULL;
	m_prevSibling = NULL;
	m_linked = false;
	m_refCount = 0;

	m_propmngr = MdiInterface::get()->propertyManager();
}


DlgNode::~DlgNode()
{

	Q_ASSERT_X(m_refCount <= 0, "DlgNode destructor", "Refcount > 0!");

	// If still linked, unlink
	unlink();

	// Delete all children
	if (m_firstChild)
	{
		DlgNode *child, *next;

		for (child = m_firstChild; child; child = next)
		{
			next = child->m_nextSibling;
			delete child;
		}
	}

	m_propmngr->clearProperties(this);
}

void DlgNode::unlink()
{
	if (m_parent)
	{
		m_parent->removeChild(this);
	}
	clearModelIndexes();
}

void DlgNode::clearModelIndexes()
{
	setModelIndex(QModelIndex());

	for (DlgNode *child = m_firstChild; child; child = child->m_nextSibling)
	{
		child->clearModelIndexes();
	}
}
	
bool DlgNode::appendChild(DlgNode *node)
{
	if (!node || node->isLinked())
		return false;
	
	if (!childAllowed(node->nodeType()))
		return false;

	if (!m_firstChild)
	{
		m_firstChild = m_lastChild = node;
		node->m_parent = this;
		node->m_nextSibling = node->m_prevSibling = NULL;
	} else {
		m_lastChild->m_nextSibling = node;
		
		node->m_parent = this;
		node->m_nextSibling = NULL;
		node->m_prevSibling = m_lastChild;

		m_lastChild = node;
	}

	node->m_linked = true;

	return true;
}

bool DlgNode::prependChild(DlgNode *node)
{
	if (!node || node->isLinked())
		return false;
	
	if (!childAllowed(node->nodeType()))
		return false;

	if (!m_firstChild) {
		m_firstChild = m_lastChild = node;
		node->m_parent = this;
		node->m_nextSibling = node->m_prevSibling = NULL;
	} else {
		m_firstChild->m_prevSibling = node;
		
		node->m_parent = this;
		node->m_prevSibling = NULL;
		node->m_nextSibling = m_firstChild;

		m_firstChild = node;
	}

	node->m_linked = true;

	return true;
}

bool DlgNode::insertChild(DlgNode *node, DlgNode *before)
{
	if (!m_firstChild)
		return appendChild(node);

	if (!before || !node || node->isLinked())
		return false;
	
	if (!childAllowed(node->nodeType()))
		return false;


	for (DlgNode *child = m_firstChild; child; child = child->m_nextSibling)
	{
		if (child == before)
		{
			DlgNode *prev = child->m_prevSibling;

			if (prev) {
				prev->m_nextSibling = node;
				child->m_prevSibling = node;

				node->m_nextSibling = child;
				node->m_prevSibling = prev;
				
			} else {
				child->m_prevSibling = node;

				node->m_nextSibling = child;
				node->m_prevSibling = NULL;

				m_firstChild = node;
			}
			node->m_parent = this;
			node->m_linked = true;
			return true;
		}

	}
	
	return false;
}

bool DlgNode::removeChild(DlgNode *node)
{
	if (!m_firstChild)
		return false;

	if (!node || !node->isLinked() || node->m_parent != this)
		return false;

	if (node->m_prevSibling) {
		node->m_prevSibling->m_nextSibling = node->m_nextSibling;
	} else {
		m_firstChild = node->m_nextSibling;
	}

	if (node->m_nextSibling) {
		node->m_nextSibling->m_prevSibling = node->m_prevSibling;
	} else {
		m_lastChild = node->m_prevSibling;
	}

	if (m_firstChild == NULL || m_lastChild == NULL)
	{
		m_firstChild = m_lastChild = NULL;
	}

	node->m_parent = NULL;
	node->m_nextSibling = NULL;
	node->m_prevSibling = NULL;
	node->m_linked = false;

	return true;
}

int DlgNode::getChildrenCount()
{
	int count = 0;
	if (m_firstChild)
	{
		DlgNode *child;

		for (child = m_firstChild; child; child = child->m_nextSibling)
		{
			count++;
		}
	}
	
	return count;
}

DlgNode *DlgNode::getChild(int index)
{
	DlgNode *child;

	for (child = m_firstChild; index && child; index--, child = child->m_nextSibling) {}

	return child;
}

int DlgNode::getChildIndex(DlgNode *child)
{
	int index = 0;
	DlgNode *current;
	for (current = m_firstChild; current && current != child; index++, current = current->m_nextSibling) {}

	if (!current)
		return -1;
	else
		return index;

}

QString DlgNode::getDefaultScript(const QString &id)
{
	QFile file(QString(":/scripts/dlg/%1.lua").arg(id));
	if (!file.open(QIODevice::ReadOnly))
		return QString();

	QString data = file.readAll();
	file.close();

	return data;
}

///////////////////////////////////////////////
/////
///// Root Node
/////
///////////////////////////////////////////////

DlgRootNode::DlgRootNode() : DlgNode()
{
	setupProperties();
	setupVisuals();
}


DlgRootNode::~DlgRootNode()
{

}


QString DlgRootNode::description()
{
	return "Dialogue";
}

void DlgRootNode::setupVisuals()
{
	m_icon = QIcon(":/icons/dlg/dialogue.png");
	m_color = QColor::fromRgb(0,0,0);
}

void DlgRootNode::setupProperties()
{
	m_propmngr->addLabelProperty("type", "Type", "Root");
}

void DlgRootNode::setProperty(QtProperty *prop, const QVariant &newValue, bool updateProperty)
{
}

QVariant DlgRootNode::getProperty(const QtProperty *prop)
{
	return QVariant();	
}
	
bool DlgRootNode::childAllowed(DlgNodeType type)
{
	switch (type)
	{
		case Entrypoint:
			return true;
		default:
			return false;

	}
}


///////////////////////////////////////////////
/////
///// Entrypoint Node
/////
///////////////////////////////////////////////


DlgEntrypointNode::DlgEntrypointNode() : DlgNode()
{
	m_conditional = false;
	m_description = "New Entrypoint";
	m_propCond = NULL;

	setupProperties();
	setupVisuals();
}

DlgEntrypointNode::~DlgEntrypointNode()
{

}


QString DlgEntrypointNode::description()
{
	return m_description;
}

void DlgEntrypointNode::setupVisuals()
{
	QPixmap icon = QPixmap(":/icons/dlg/entrypoint.png");

	if (m_conditional) {
		QPixmap overlay(":/icons/dlg/cond-overlay.png");
		QPainter painter(&icon);
		painter.drawPixmap(0, 0, 16, 16, overlay);
	}
	
	m_icon = QPixmap(icon);
	m_color = QColor(0, 0, 0);
}

void DlgEntrypointNode::setupProperties()
{
	m_propmngr->addLabelProperty(this, "Type", "Entrypoint");


	m_propDesc = m_propmngr->addStringProperty(this, "Description", m_description);

	QtProperty *cond = m_propmngr->addGroup(this, "Control Flow");

	m_propCond = m_propmngr->addBoolProperty(this, "Conditional", m_conditional, cond);
	m_propScript = m_propmngr->addScriptProperty(this, "Condition", m_conditionScript, DlgNode::getDefaultScript("condition"), cond);
	m_propScript->setEnabled(m_conditional);
}

void DlgEntrypointNode::setProperty(QtProperty *prop, const QVariant &newValue, bool updateProperty)
{
	if (prop == m_propDesc) {
		m_description = newValue.toString();
	} else if (prop == m_propCond) {
		m_conditional = newValue.toBool();
		m_propScript->setEnabled(m_conditional);
		setupVisuals();
	} else if (prop == m_propScript) {
		m_conditionScript = newValue.toString();
	}

	if (updateProperty) m_propmngr->setValue(prop, newValue);
}

QVariant DlgEntrypointNode::getProperty(const QtProperty *prop)
{
	if (prop == m_propDesc) {
		return m_description;
	} else if (prop == m_propCond) {
		return m_conditional;
	} else if (prop == m_propScript) {
		return m_conditionScript;
	}
	return QVariant();
}

bool DlgEntrypointNode::childAllowed(DlgNodeType type)
{
	switch (type)
	{
	case Root:
	case Entrypoint:
	case Option:
	case DynOption:
		return false;
	default:
		return true;
	}

}

///////////////////////////////////////////////
/////
///// Speech Node
/////
///////////////////////////////////////////////


DlgSpeechNode::DlgSpeechNode() : DlgNode()
{
	m_text = "New Speech";
	m_duration = 2000;
	m_conditional = false;
	m_resolve = false;

	setupProperties();
	setupVisuals();
};

DlgSpeechNode::~DlgSpeechNode()
{

}



QString DlgSpeechNode::description()
{
	return QString("\"%1\"").arg(m_text);
}

void DlgSpeechNode::setupVisuals()
{
	QPixmap icon = QPixmap(":/icons/dlg/text.png");

	if (m_conditional) {
		QPixmap overlay(":/icons/dlg/cond-overlay.png");
		QPainter painter(&icon);
		painter.drawPixmap(0, 0, 16, 16, overlay);
	}
	
	m_icon = QPixmap(icon);
	m_color = QColor(0, 0, 0);
}

void DlgSpeechNode::setupProperties()
{
	m_propmngr->addLabelProperty(this, "Type", "Speech");
	
	m_propText = m_propmngr->addStringProperty(this, "Text", m_text);
	m_propDuration = m_propmngr->addIntProperty(this, "Duration (ms)", m_duration);

	QtProperty *cond = m_propmngr->addGroup(this, "Control Flow");

	m_propCond = m_propmngr->addBoolProperty(this, "Conditional", m_conditional, cond);
	m_propCondScript = m_propmngr->addScriptProperty(this, "Condition", m_conditionScript, DlgNode::getDefaultScript("condition"), cond);
	m_propCondScript->setEnabled(m_conditional);

	QtProperty *res = m_propmngr->addGroup(this, "Variables");

	m_propResolve = m_propmngr->addBoolProperty(this, "Use Resolver", m_conditional, res);
	m_propResolveScript = m_propmngr->addScriptProperty(this, "Resolver", m_conditionScript, DlgNode::getDefaultScript("resolver"), res);
	m_propResolveScript->setEnabled(m_conditional);
}

void DlgSpeechNode::setProperty(QtProperty *prop, const QVariant &newValue, bool updateProperty)
{
	if (prop == m_propText) {
		m_text = newValue.toString();
	} else if (prop == m_propDuration) {
		m_duration = newValue.toInt();
	} else if (prop == m_propCond) {
		m_conditional = newValue.toBool();
		m_propCondScript->setEnabled(m_conditional);
		setupVisuals();
	} else if (prop == m_propCondScript) {
		m_conditionScript = newValue.toString();
	} else if (prop == m_propResolve) {
		m_resolve = newValue.toBool();
		m_propResolveScript->setEnabled(m_resolve);
	} else if (prop == m_propResolveScript) {
		m_resolverScript = newValue.toString();
	}

	if (updateProperty) m_propmngr->setValue(prop, newValue);
}

QVariant DlgSpeechNode::getProperty(const QtProperty *prop)
{
	if (prop == m_propText) {
		return qVariantFromValue(m_text);
	} else if (prop == m_propDuration) {
		return qVariantFromValue(m_duration);
	} else if (prop == m_propCond) {
		return qVariantFromValue(m_conditional);
	} else if (prop == m_propCondScript) {
		return qVariantFromValue(m_conditionScript);
	} else if (prop == m_propResolve) {
		return qVariantFromValue(m_resolve);
	} else if (prop == m_propResolveScript) {
		return qVariantFromValue(m_resolverScript);
	}
	return QVariant();
}

	
bool DlgSpeechNode::childAllowed(DlgNodeType type)
{
	switch (type)
	{
	case Root:
	case Entrypoint:
		return false;
	default:
		return true;
	}
}

///////////////////////////////////////////////
/////
///// Option Node
/////
///////////////////////////////////////////////


DlgOptionNode::DlgOptionNode() : DlgNode()
{
	m_text = "New Option";

	m_conditional = false;
	m_propCond = false;

	setupProperties();
	setupVisuals();
}

DlgOptionNode::~DlgOptionNode()
{

}


QString DlgOptionNode::description()
{
	return QString("\"%1\"").arg(m_text);
}

void DlgOptionNode::setupVisuals()
{
	QPixmap icon = QPixmap(":/icons/dlg/option.png");

	if (m_conditional) {
		QPixmap overlay(":/icons/dlg/cond-overlay.png");
		QPainter painter(&icon);
		painter.drawPixmap(0, 0, 16, 16, overlay);
	}
	
	m_icon = QPixmap(icon);
	m_color = QColor(0, 0, 0);
}

void DlgOptionNode::setupProperties()
{
	m_propmngr->addLabelProperty(this, "Type", "Option");


	m_propText = m_propmngr->addStringProperty(this, "Text", m_text);

	QtProperty *cond = m_propmngr->addGroup(this, "Control Flow");

	m_propCond = m_propmngr->addBoolProperty(this, "Conditional", m_conditional, cond);
	m_propScript = m_propmngr->addScriptProperty(this, "Condition", m_conditionScript, DlgNode::getDefaultScript("condition"), cond);
	m_propScript->setEnabled(m_conditional);
}

void DlgOptionNode::setProperty(QtProperty *prop, const QVariant &newValue, bool updateProperty)
{
	if (prop == m_propText) {
		m_text = newValue.toString();
	} else if (prop == m_propCond) {
		m_conditional = newValue.toBool();
		m_propScript->setEnabled(m_conditional);
		setupVisuals();
	} else if (prop == m_propScript) {
		m_conditionScript = newValue.toString();
	}

	if (updateProperty) m_propmngr->setValue(prop, newValue);
}

QVariant DlgOptionNode::getProperty(const QtProperty *prop)
{
	if (prop == m_propText) {
		return qVariantFromValue(m_text);
	} else if (prop == m_propCond) {
		return qVariantFromValue(m_conditional);
	} else if (prop == m_propScript) {
		return qVariantFromValue(m_conditionScript);
	}
	return QVariant();
}
	
bool DlgOptionNode::childAllowed(DlgNodeType type)
{
	switch (type)
	{
	case Root:
	case Entrypoint:
	case Option:
	case DynOption:
		return false;
	default:
		return true;
	}
}

///////////////////////////////////////////////
/////
///// End Node
/////
///////////////////////////////////////////////

DlgEndNode::DlgEndNode() : DlgNode()
{
	setupProperties();
	setupVisuals();
}

DlgEndNode::~DlgEndNode()
{

}

QString DlgEndNode::description()
{
	return "[ End of conversation ]";
}

void DlgEndNode::setupVisuals()
{
	m_icon = QIcon(":/icons/dlg/end.png");
	m_color = QColor(0, 0, 0);
}

void DlgEndNode::setupProperties()
{
	m_propmngr->addLabelProperty(this, "Type", "End");
}

void DlgEndNode::setProperty(QtProperty *prop, const QVariant &newValue, bool updateProperty)
{
	return;
}

QVariant DlgEndNode::getProperty(const QtProperty *prop)
{
	return QVariant();
}

///////////////////////////////////////////////
/////
///// Script node
/////
///////////////////////////////////////////////

DlgScriptNode::DlgScriptNode() : DlgNode()
{
	m_description = "New Script";
	m_script = "";

	m_conditional = false;
	m_conditionScript = "";

	setupProperties();
	setupVisuals();
}

DlgScriptNode::~DlgScriptNode()
{

}

QString DlgScriptNode::description()
{
	return QString("[Script] %1").arg(m_description);
}


void DlgScriptNode::setupVisuals()
{
	QPixmap icon = QPixmap(":/icons/dlg/script.png");

	if (m_conditional) {
		QPixmap overlay(":/icons/dlg/cond-overlay.png");
		QPainter painter(&icon);
		painter.drawPixmap(0, 0, 16, 16, overlay);
	}
	
	m_icon = QPixmap(icon);
	m_color = QColor(0, 0, 0);
}

void DlgScriptNode::setupProperties()
{
	m_propmngr->addLabelProperty(this, "Type", "Script");


	m_propDescription = m_propmngr->addStringProperty(this, "Description", m_description);
	m_propScript = m_propmngr->addScriptProperty(this, "Script", m_script, DlgNode::getDefaultScript("script"));

	QtProperty *cond = m_propmngr->addGroup(this, "Control Flow");

	m_propCond = m_propmngr->addBoolProperty(this, "Conditional", m_conditional, cond);
	m_propScript = m_propmngr->addScriptProperty(this, "Condition", m_conditionScript, DlgNode::getDefaultScript("condition"), cond);
	m_propScript->setEnabled(m_conditional);
}

void DlgScriptNode::setProperty(QtProperty *prop, const QVariant &newValue, bool updateProperty)
{
	if (prop == m_propDescription) {
		m_description = newValue.toString();
	} else if (prop == m_propScript) {
		m_script =  newValue.toString();
	} else if (prop == m_propCond) {
		m_conditional = newValue.toBool();
		m_propScript->setEnabled(m_conditional);
		setupVisuals();
	} else if (prop == m_propScript) {
		m_conditionScript = newValue.toString();
	}

	if (updateProperty) m_propmngr->setValue(prop, newValue);
}

QVariant DlgScriptNode::getProperty(const QtProperty *prop)
{
	if (prop == m_propDescription) {
		return qVariantFromValue(m_description);
	} else if (prop == m_propScript) {
		return qVariantFromValue(m_script);
	} else if (prop == m_propCond) {
		return qVariantFromValue(m_conditional);
	} else if (prop == m_propScript) {
		return qVariantFromValue(m_conditionScript);
	}
	return QVariant();
}
	
	
bool DlgScriptNode::childAllowed(DlgNodeType type)
{
	switch (type)
	{
	case Root:
	case Entrypoint:
	case Option:
	case DynOption:
		return false;
	default:
		return true;
	}
}



///////////////////////////////////////////////
/////
///// Interrupt node
/////
///////////////////////////////////////////////

DlgInterruptNode::DlgInterruptNode() : DlgNode()
{
	m_description = "New Interrupt";
	m_script = "";

	m_conditional = false;
	m_conditionScript = "";

	setupProperties();
	setupVisuals();
}

DlgInterruptNode::~DlgInterruptNode()
{

}

QString DlgInterruptNode::description()
{
	return QString("[Interrupt] %1").arg(m_description);
}


void DlgInterruptNode::setupVisuals()
{
	QPixmap icon = QPixmap(":/icons/dlg/interrupt.png");

	if (m_conditional) {
		QPixmap overlay(":/icons/dlg/cond-overlay.png");
		QPainter painter(&icon);
		painter.drawPixmap(0, 0, 16, 16, overlay);
	}
	
	m_icon = QPixmap(icon);
	m_color = QColor(0, 0, 0);
}

void DlgInterruptNode::setupProperties()
{
	m_propmngr->addLabelProperty(this, "Type", "Interrupt");


	m_propDescription = m_propmngr->addStringProperty(this, "Description", m_description);
	m_propScript = m_propmngr->addScriptProperty(this, "Script", m_script, DlgNode::getDefaultScript("script"));

	QtProperty *cond = m_propmngr->addGroup(this, "Control Flow");

	m_propCond = m_propmngr->addBoolProperty(this, "Conditional", m_conditional, cond);
	m_propScript = m_propmngr->addScriptProperty(this, "Condition", m_conditionScript, DlgNode::getDefaultScript("condition"), cond);
	m_propScript->setEnabled(m_conditional);
}

void DlgInterruptNode::setProperty(QtProperty *prop, const QVariant &newValue, bool updateProperty)
{
	if (prop == m_propDescription) {
		m_description = newValue.toString();
	} else if (prop == m_propScript) {
		m_script =  newValue.toString();
	} else if (prop == m_propCond) {
		m_conditional = newValue.toBool();
		m_propScript->setEnabled(m_conditional);
		setupVisuals();
	} else if (prop == m_propScript) {
		m_conditionScript = newValue.toString();
	}

	if (updateProperty) m_propmngr->setValue(prop, newValue);
}

QVariant DlgInterruptNode::getProperty(const QtProperty *prop)
{
	if (prop == m_propDescription) {
		return qVariantFromValue(m_description);
	} else if (prop == m_propScript) {
		return qVariantFromValue(m_script);
	} else if (prop == m_propCond) {
		return qVariantFromValue(m_conditional);
	} else if (prop == m_propScript) {
		return qVariantFromValue(m_conditionScript);
	}
	return QVariant();
}
	
	
bool DlgInterruptNode::childAllowed(DlgNodeType type)
{
	switch (type)
	{
	case Root:
	case Entrypoint:
	case Option:
	case DynOption:
		return false;
	default:
		return true;
	}
}