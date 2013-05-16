#ifndef PROPERTYMANAGER_H
#define PROPERTYMANAGER_H

#include <QObject>
#include <QtAbstractPropertyBrowser>
#include <qtpropertymanager.h>
#include <qteditorfactory.h>
#include <qttreepropertybrowser.h>
#include <QMap>
#include "scriptproperty.h"

class MdiSubBase;

class PropertyManager : public QObject
{
	Q_OBJECT
public:
	PropertyManager(QtAbstractPropertyBrowser *browser, QObject *parent = (QObject *)0);
	~PropertyManager();

	QtProperty * addBoolProperty(void *owner, const QString &name, bool defValue, QtProperty *parent = (QtProperty*)0);
	QtProperty * addStringProperty(void *owner, const QString &name, const QString &defValue, QtProperty *parent = (QtProperty*)0);
	QtProperty * addIntProperty(void *owner, const QString &name, int defValue, QtProperty *parent = (QtProperty*)0);
	QtProperty * addLabelProperty(void *owner, const QString &name, const QString &defValue, QtProperty *parent = (QtProperty*)0);
	QtProperty * addScriptProperty(void *owner, const QString &name, const QString &value, const QString &defValue, QtProperty *parent = (QtProperty*)0);
	QtProperty * addGroup(void *owner, const QString &name, QtProperty *parent = (QtProperty*)0);

	void setBoolValue(QtProperty *prop, bool value);
	void setStringValue(QtProperty *prop, const QString &value);
	void setScriptValue(QtProperty *prop, const QString &value);
	void setIntValue(QtProperty *prop, int value);
	void setLabelValue(QtProperty *prop, const QString &value);

	void setValue(QtProperty *prop, const QVariant &value);

	bool getBoolValue(QtProperty *prop);
	QString getStringValue(QtProperty *prop);
	QString getScriptValue(QtProperty *prop);
	int getIntValue(QtProperty *prop);
	QString getLabelValue(QtProperty *prop);
	
	QVariant getValue(QtProperty *prop);

	void setActiveWindow(MdiSubBase *wnd);
	
	void setActiveOwner(void *owner);
	void clearProperties(void *owner);
	void clearAll();

private:

	struct iProperty
	{
		iProperty(QtProperty *prop, bool topLevel) : property(prop), topLevel(topLevel) {};

		QtProperty *property;
		bool topLevel;
	};


	QMap<void *, QList<iProperty>> m_registeredProperties;

	void *m_activeOwner;
	bool m_blockSignals;

	QtAbstractPropertyBrowser *m_browser;
	MdiSubBase *m_activeWindow;

	// Managers
	QtBoolPropertyManager *m_boolManager;
	QtStringPropertyManager *m_stringManager;
	QtIntPropertyManager *m_intManager;
	QtScriptPropertyManager *m_scriptManager;

	QtStringPropertyManager *m_labelManager;		// Not editable
	QtGroupPropertyManager *m_groupManager;
	

	// Factories
	QtCheckBoxFactory *m_checkboxFactory;
	QtLineEditFactory *m_lineEditFactory;
	QtSpinBoxFactory *m_spinboxFactory;
	QtScriptEditFactory *m_scriptFactory;

private slots:

	void valueChanged(QtProperty *prop, const QString &newvalue);
	void valueChanged(QtProperty *prop, bool newvalue);
	void valueChanged(QtProperty *prop, int newvalue);
};

#endif