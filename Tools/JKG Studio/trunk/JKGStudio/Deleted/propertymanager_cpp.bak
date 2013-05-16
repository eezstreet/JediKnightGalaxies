#include "propertymanager.h"
#include "mdibase.h"


PropertyManager::PropertyManager(QtAbstractPropertyBrowser *browser, QObject *parent) : QObject(parent)
{
	m_browser = browser;
	m_boolManager = new QtBoolPropertyManager(this);
	m_stringManager = new QtStringPropertyManager(this);
	m_intManager = new QtIntPropertyManager(this);
	m_scriptManager = new QtScriptPropertyManager(this);

	m_labelManager = new QtStringPropertyManager(this);
	m_groupManager = new QtGroupPropertyManager(this);

	m_checkboxFactory = new QtCheckBoxFactory(this);
	m_lineEditFactory = new QtLineEditFactory(this);
	m_spinboxFactory = new QtSpinBoxFactory(this);
	m_scriptFactory = new QtScriptEditFactory(this);

	m_activeWindow = NULL;
	m_blockSignals = false;

	m_browser->setFactoryForManager(m_boolManager, m_checkboxFactory);
	m_browser->setFactoryForManager(m_stringManager, m_lineEditFactory);
	m_browser->setFactoryForManager(m_intManager, m_spinboxFactory);
	m_browser->setFactoryForManager(m_scriptManager, m_scriptFactory);

	connect (m_boolManager, SIGNAL(valueChanged(QtProperty *, bool)), this, SLOT(valueChanged(QtProperty *, bool)));
	connect (m_stringManager, SIGNAL(valueChanged(QtProperty *, const QString &)), this, SLOT(valueChanged(QtProperty *, const QString &)));
	connect (m_intManager, SIGNAL(valueChanged(QtProperty *, int)), this, SLOT(valueChanged(QtProperty *, int)));
	connect (m_scriptManager, SIGNAL(valueChanged(QtProperty *, const QString &)), this, SLOT(valueChanged(QtProperty *, const QString &)));
}

PropertyManager::~PropertyManager()
{
	clearAll();
}

QtProperty * PropertyManager::addBoolProperty(void *owner, const QString &name, bool defValue, QtProperty *parent)	
{
	m_blockSignals = true;

	QtProperty *prop = m_boolManager->addProperty(name);
	if (parent) parent->addSubProperty(prop);
	
	m_boolManager->setValue(prop, defValue);

	if (m_registeredProperties.contains(owner)) {
		m_registeredProperties[owner].append(iProperty(prop, !parent));
	} else {
		m_registeredProperties.insert(owner, QList<iProperty>());
		m_registeredProperties[owner].append(iProperty(prop, !parent));
	}

	if (!parent) {
		if (owner == m_activeOwner)
			m_browser->addProperty(prop);
	}
	
	m_blockSignals = false;

	return prop;
}

QtProperty * PropertyManager::addStringProperty(void *owner, const QString &name, const QString &defValue, QtProperty *parent)
{
	m_blockSignals = true;

	QtProperty *prop = m_stringManager->addProperty(name);
	if (parent) parent->addSubProperty(prop);

	m_stringManager->setValue(prop, defValue);


	if (m_registeredProperties.contains(owner)) {
		m_registeredProperties[owner].append(iProperty(prop, !parent));
	} else {
		m_registeredProperties.insert(owner, QList<iProperty>());
		m_registeredProperties[owner].append(iProperty(prop, !parent));
	}

	if (!parent) {
		if (owner == m_activeOwner)
			m_browser->addProperty(prop);
	}

	m_blockSignals = false;

	return prop;
}

QtProperty * PropertyManager::addIntProperty(void *owner, const QString &name, int defValue, QtProperty *parent)
{
	m_blockSignals = true;

	QtProperty *prop = m_intManager->addProperty(name);
	if (parent) parent->addSubProperty(prop);

	m_intManager->setValue(prop, defValue);

	if (m_registeredProperties.contains(owner)) {
		m_registeredProperties[owner].append(iProperty(prop, !parent));
	} else {
		m_registeredProperties.insert(owner, QList<iProperty>());
		m_registeredProperties[owner].append(iProperty(prop, !parent));
	}

	if (!parent) {
		if (owner == m_activeOwner)
			m_browser->addProperty(prop);
	}

	m_blockSignals = false;

	return prop;
}

QtProperty * PropertyManager::addLabelProperty(void *owner, const QString &name, const QString &defValue, QtProperty *parent)
{
	m_blockSignals = true;

	QtProperty *prop = m_labelManager->addProperty(name);
	if (parent) parent->addSubProperty(prop);

	m_labelManager->setValue(prop, defValue);
	
	if (m_registeredProperties.contains(owner)) {
		m_registeredProperties[owner].append(iProperty(prop, !parent));
	} else {
		m_registeredProperties.insert(owner, QList<iProperty>());
		m_registeredProperties[owner].append(iProperty(prop, !parent));
	}

	if (!parent) {
		if (owner == m_activeOwner)
			m_browser->addProperty(prop);
	}

	m_blockSignals = false;

	return prop;
}

QtProperty * PropertyManager::addScriptProperty(void *owner, const QString &name, const QString &value, const QString &defValue, QtProperty *parent)
{
	m_blockSignals = true;
	QtProperty *prop = m_scriptManager->addProperty(name);
	if (parent) parent->addSubProperty(prop);

	m_scriptManager->setDefaultValue(prop, defValue);
	if (!value.isNull())
		m_scriptManager->setValue(prop, defValue);
	else
		m_scriptManager->resetValue(prop);

	
	if (m_registeredProperties.contains(owner)) {
		m_registeredProperties[owner].append(iProperty(prop, !parent));
	} else {
		m_registeredProperties.insert(owner, QList<iProperty>());
		m_registeredProperties[owner].append(iProperty(prop, !parent));
	}

	if (!parent) {
		if (owner == m_activeOwner)
			m_browser->addProperty(prop);
	}

	m_blockSignals = false;

	return prop;
}

QtProperty * PropertyManager::addGroup(void *owner, const QString &name, QtProperty *parent)
{
	m_blockSignals = true;
	QtProperty *prop = m_groupManager->addProperty(name);
	if (parent) parent->addSubProperty(prop);


	if (m_registeredProperties.contains(owner)) {
		m_registeredProperties[owner].append(iProperty(prop, !parent));
	} else {
		m_registeredProperties.insert(owner, QList<iProperty>());
		m_registeredProperties[owner].append(iProperty(prop, !parent));
	}

	if (!parent) {
		if (owner == m_activeOwner)
			m_browser->addProperty(prop);
	}

	m_blockSignals = false;
	return prop;
}

void PropertyManager::setBoolValue(QtProperty *prop, bool value)
{
	m_blockSignals = true;
	m_boolManager->setValue(prop, value);
	m_blockSignals = false;
}

void PropertyManager::setStringValue(QtProperty *prop, const QString &value)
{
	m_blockSignals = true;
	m_stringManager->setValue(prop, value);
	m_blockSignals = false;
}

void PropertyManager::setScriptValue(QtProperty *prop, const QString &value)
{
	m_blockSignals = true;
	m_scriptManager->setValue(prop, value);
	m_blockSignals = false;
}

void PropertyManager::setIntValue(QtProperty *prop, int value)
{
	m_blockSignals = true;
	m_intManager->setValue(prop, value);
	m_blockSignals = false;
}

void PropertyManager::setLabelValue(QtProperty *prop, const QString &value)
{
	m_blockSignals = true;
	m_labelManager->setValue(prop, value);
	m_blockSignals = false;
}

void PropertyManager::setValue(QtProperty *prop, const QVariant &value)
{
	QtAbstractPropertyManager *mngr = prop->propertyManager();

	if (mngr == m_boolManager)
		setBoolValue(prop, value.toBool());
	else if (mngr == m_stringManager)
		setStringValue(prop, value.toString());
	else if (mngr == m_intManager)
		setIntValue(prop, value.toInt());
	else if (mngr == m_scriptManager)
		setScriptValue(prop, value.toString());
	else if (mngr == m_labelManager)
		setLabelValue(prop, value.toString());
}

bool PropertyManager::getBoolValue(QtProperty *prop)
{
	return m_boolManager->value(prop);
}

QString PropertyManager::getStringValue(QtProperty *prop)
{
	return m_stringManager->value(prop);
}

QString PropertyManager::getScriptValue(QtProperty *prop)
{
	return m_scriptManager->value(prop);
}

int PropertyManager::getIntValue(QtProperty *prop)
{
	return m_intManager->value(prop);
}

QString PropertyManager::getLabelValue(QtProperty *prop)
{
	return m_labelManager->value(prop);
}

QVariant PropertyManager::getValue(QtProperty *prop)
{
	QtAbstractPropertyManager *mngr = prop->propertyManager();
	
	if (mngr == m_boolManager)
		return qVariantFromValue(getBoolValue(prop));
	else if (mngr == m_stringManager)
		return qVariantFromValue(getStringValue(prop));
	else if (mngr == m_intManager)
		return qVariantFromValue(getIntValue(prop));
	else if (mngr == m_scriptManager)
		return qVariantFromValue(getScriptValue(prop));
	else if (mngr == m_labelManager)
		return qVariantFromValue(getLabelValue(prop));

	return QVariant();
}


void PropertyManager::setActiveWindow(MdiSubBase *wnd)
{
	m_activeWindow = wnd;
}

void PropertyManager::setActiveOwner(void *owner)
{
	m_activeOwner = owner;
	m_browser->clear();
	if (owner)
	{
		if (!m_registeredProperties.contains(owner))
			return;

		QList<iProperty> list = m_registeredProperties.value(owner);
		
		for (QList<iProperty>::const_iterator it = list.begin(); it != list.end(); it++)
		{
			if ((*it).topLevel)
				m_browser->addProperty((*it).property);
		}
	}
}

void PropertyManager::clearProperties(void *owner)
{
	if (owner == m_activeOwner)
		setActiveOwner(NULL);

	if (!m_registeredProperties.contains(owner))
		return;

	QList<iProperty> list = m_registeredProperties.value(owner);
		
	for (QList<iProperty>::iterator it = list.begin(); it != list.end(); it++)
	{
		delete (*it).property;
	}
	
	m_registeredProperties.remove(owner);
}

void PropertyManager::clearAll()
{
	m_activeOwner = NULL;

	for (QMap<void *, QList<iProperty>>::iterator it = m_registeredProperties.begin(); it != m_registeredProperties.end(); it++)
	{
		void *owner = it.key();
		QList<iProperty> list = it.value();
		
		for (QList<iProperty>::iterator it2 = list.begin(); it2 != list.end(); it2++)
		{
			delete (*it2).property;
		}
	}

	m_registeredProperties.clear();

}

void PropertyManager::valueChanged(QtProperty *prop, const QString &newvalue)
{
	if (!m_activeWindow || m_blockSignals)
		return;

	m_activeWindow->propertyChanged(prop, qVariantFromValue(newvalue));
}

void PropertyManager::valueChanged(QtProperty *prop, bool newvalue)
{
	if (!m_activeWindow || m_blockSignals)
		return;

	m_activeWindow->propertyChanged(prop, qVariantFromValue(newvalue));
}

void PropertyManager::valueChanged(QtProperty *prop, int newvalue)
{
	if (!m_activeWindow || m_blockSignals)
		return;

	m_activeWindow->propertyChanged(prop, qVariantFromValue(newvalue));
}