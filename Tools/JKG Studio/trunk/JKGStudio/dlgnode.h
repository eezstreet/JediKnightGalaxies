#ifndef DLGNODES_H
#define DLGNODES_H

#include <QIcon>
#include <QString>
#include <QVariant>
#include "propertymanager.h"
#include <QColor>
#include <QModelIndex>

class DlgNode
{
public:
	typedef enum {
		Root,
		Entrypoint,
		Speech,
		Option,
		End,
		Link,
		Script,
		Interrupt,
		DynOption,
		TextEntry,
		Cinematic,
	} DlgNodeType;

	DlgNode();
	~DlgNode();

	virtual DlgNodeType nodeType() = 0;
	virtual QString description() = 0;
	virtual QIcon icon() { return m_icon; };
	virtual QColor color() { return m_color; };

	virtual void setupVisuals() = 0;
	virtual void setupProperties() = 0;
	virtual void setProperty(QtProperty *prop, const QVariant &newValue, bool updateProperty = false) = 0;
	virtual QVariant getProperty(const QtProperty *prop) = 0;
	
	virtual bool childAllowed(DlgNodeType type) = 0;

	virtual bool isEditable() { return false; }
	virtual QString getEditText() { return QString(); }
	virtual QtProperty * getEditProperty() { return NULL; }

	
	bool appendChild(DlgNode *node);
	bool prependChild(DlgNode *node);
	bool insertChild(DlgNode *node, DlgNode *before);
	bool removeChild(DlgNode *node);
	void unlink();

	int getChildrenCount();
	
	DlgNode *getParent() { return m_parent; }
	DlgNode *getChild(int index);
	int getChildIndex(DlgNode *child);
	bool hasChildren() { return !!m_firstChild; }

	DlgNode *getFirstChild() { return m_firstChild; }
	DlgNode *getLastChild() { return m_firstChild; }
	DlgNode *getNextSibling() { return m_nextSibling; }
	DlgNode *getPrevSibling() { return m_prevSibling; }


	virtual bool isLinked() { return m_linked; }
	virtual bool isUnreferenced() { return m_refCount <= 0 && !m_linked; }

	int m_refCount;

	QModelIndex getModelIndex() { return m_modelidx; }
	void setModelIndex(const QModelIndex &idx) { m_modelidx = idx; }


	// Static functions
	static QString getDefaultScript(const QString &id);

protected:
	DlgNode *m_parent;
	DlgNode *m_firstChild;
	DlgNode *m_lastChild;
	DlgNode *m_nextSibling;
	DlgNode *m_prevSibling;
	bool m_linked;

	QPersistentModelIndex m_modelidx;

	void clearModelIndexes();

	PropertyManager *m_propmngr;

	QIcon m_icon;
	QColor m_color;

};


class DlgRootNode : public DlgNode
{

public:
	DlgRootNode();
	~DlgRootNode();


	virtual DlgNodeType nodeType() { return DlgNode::Root; };
	virtual QString description();

	virtual void setupVisuals();
	virtual void setupProperties();
	virtual void setProperty(QtProperty *prop, const QVariant &newValue, bool updateProperty = false);
	virtual QVariant getProperty(const QtProperty *prop);
	
	virtual bool childAllowed(DlgNodeType type);

};

class DlgEntrypointNode : public DlgNode
{

public:
	DlgEntrypointNode();
	~DlgEntrypointNode();


	virtual DlgNodeType nodeType() { return DlgNode::Entrypoint; };
	virtual QString description();

	virtual void setupVisuals();
	virtual void setupProperties();
	virtual void setProperty(QtProperty *prop, const QVariant &newValue, bool updateProperty = false);
	virtual QVariant getProperty(const QtProperty *prop);
	
	virtual bool isEditable() { return true; }
	virtual QString getEditText() { return m_description; }
	virtual QtProperty * getEditProperty() { return m_propDesc; }

	virtual bool childAllowed(DlgNodeType type);

private:
	QString m_description;
	bool m_conditional;
	QString m_conditionScript;

	//Properties
	QtProperty *m_propDesc;
	QtProperty *m_propCond;
	QtProperty *m_propScript;

};

class DlgSpeechNode : public DlgNode
{

public:
	DlgSpeechNode();
	~DlgSpeechNode();


	virtual DlgNodeType nodeType() { return DlgNode::Speech; };
	virtual QString description();

	virtual void setupVisuals();
	virtual void setupProperties();
	virtual void setProperty(QtProperty *prop, const QVariant &newValue, bool updateProperty = false);
	virtual QVariant getProperty(const QtProperty *prop);

	virtual bool isEditable() { return true; }
	virtual QString getEditText() { return m_text; }
	virtual QtProperty * getEditProperty() { return m_propText; }

	virtual bool childAllowed(DlgNodeType type);

private:
	QString m_text;
	int m_duration;
	bool m_conditional;
	QString m_conditionScript;
	bool m_resolve;
	QString m_resolverScript;

	// Properties
	QtProperty *m_propText;
	QtProperty *m_propDuration;

	QtProperty *m_propCond;
	QtProperty *m_propCondScript;

	QtProperty *m_propResolve;
	QtProperty *m_propResolveScript;
};

class DlgOptionNode : public DlgNode
{

public:
	DlgOptionNode();
	~DlgOptionNode();


	virtual DlgNodeType nodeType() { return DlgNode::Option; };
	virtual QString description();

	virtual void setupVisuals();
	virtual void setupProperties();
	virtual void setProperty(QtProperty *prop, const QVariant &newValue, bool updateProperty = false);
	virtual QVariant getProperty(const QtProperty *prop);

	virtual bool isEditable() { return true; }
	virtual QString getEditText() { return m_text; }
	virtual QtProperty * getEditProperty() { return m_propText; }
	
	
	virtual bool childAllowed(DlgNodeType type);

private:
	QString m_text;

	bool m_conditional;
	QString m_conditionScript;

	// Properties
	QtProperty *m_propText;
	QtProperty *m_propCond;
	QtProperty *m_propScript;
};

class DlgEndNode : public DlgNode
{

public:
	DlgEndNode();
	~DlgEndNode();


	virtual DlgNodeType nodeType() { return DlgNode::End; };
	virtual QString description();

	virtual void setupVisuals();
	virtual void setupProperties();
	virtual void setProperty(QtProperty *prop, const QVariant &newValue, bool updateProperty = false);
	virtual QVariant getProperty(const QtProperty *prop);
	
	virtual bool childAllowed(DlgNodeType type) { return false; }
};

class DlgScriptNode : public DlgNode
{

public:
	DlgScriptNode();
	~DlgScriptNode();


	virtual DlgNodeType nodeType() { return DlgNode::Script; };
	virtual QString description();

	virtual void setupVisuals();
	virtual void setupProperties();
	virtual void setProperty(QtProperty *prop, const QVariant &newValue, bool updateProperty = false);
	virtual QVariant getProperty(const QtProperty *prop);

	virtual bool isEditable() { return true; }
	virtual QString getEditText() { return m_description; }
	virtual QtProperty * getEditProperty() { return m_propDescription; }
	
	
	virtual bool childAllowed(DlgNodeType type);

private:
	QString m_description;

	QString m_script;

	bool m_conditional;
	QString m_conditionScript;

	// Properties
	QtProperty *m_propDescription;

	QtProperty *m_propScript;

	QtProperty *m_propCond;
	QtProperty *m_propCondScript;
};

class DlgInterruptNode : public DlgNode
{

public:
	DlgInterruptNode();
	~DlgInterruptNode();


	virtual DlgNodeType nodeType() { return DlgNode::Script; };
	virtual QString description();

	virtual void setupVisuals();
	virtual void setupProperties();
	virtual void setProperty(QtProperty *prop, const QVariant &newValue, bool updateProperty = false);
	virtual QVariant getProperty(const QtProperty *prop);

	virtual bool isEditable() { return true; }
	virtual QString getEditText() { return m_description; }
	virtual QtProperty * getEditProperty() { return m_propDescription; }
	
	
	virtual bool childAllowed(DlgNodeType type);

private:
	QString m_description;

	QString m_script;

	bool m_conditional;
	QString m_conditionScript;

	// Properties
	QtProperty *m_propDescription;

	QtProperty *m_propScript;

	QtProperty *m_propCond;
	QtProperty *m_propCondScript;
};

#endif