#ifndef SCRIPTPROPERTY_H
#define SCRIPTPROPERTY_H

#include <QtAbstractPropertyManager>
#include <QtStringPropertyManager>
#include <QLabel>
#include <QToolButton>
#include <QObject>
#include <QEvent>


class QtScriptPropertyManagerPrivate;

class QtScriptPropertyManager : public QtAbstractPropertyManager
{
	Q_OBJECT
public:
	QtScriptPropertyManager(QObject *parent = 0);
	~QtScriptPropertyManager();

	QString value(const QtProperty *property) const;
	QString defaultValue(const QtProperty *property) const;

public slots:
    void setValue(QtProperty *property, QString val);
	void setDefaultValue(QtProperty *property, QString defaultVal);
	void resetValue(QtProperty *property);

signals:
    void valueChanged(QtProperty *property, QString val);

protected:
    QString valueText(const QtProperty *property) const;
    QIcon valueIcon(const QtProperty *property) const;
    virtual void initializeProperty(QtProperty *property);
    virtual void uninitializeProperty(QtProperty *property);

private:
    QtScriptPropertyManagerPrivate *d_ptr;
    Q_DECLARE_PRIVATE(QtScriptPropertyManager)
    Q_DISABLE_COPY(QtScriptPropertyManager)
};

class QScriptEditWidget : public QWidget
{
	Q_OBJECT
public:
	QScriptEditWidget(QWidget *parent = 0);
	~QScriptEditWidget();


	void setValue(const QString &text);
	void setDefaultValue(const QString &text);

	bool eventFilter(QObject *obj, QEvent *ev);

private slots:
	void onEditClicked();
	void onResetClicked();
signals:
	void scriptEdited(const QString &script);
	void scriptReset();

protected:
    void paintEvent(QPaintEvent *);

private:
	QLabel *m_label;
	QToolButton *m_edit;
	QToolButton *m_reset;

	QString m_val;
	QString m_defVal;
};


class QtScriptEditFactoryPrivate;

class QtScriptEditFactory : public QtAbstractEditorFactory<QtScriptPropertyManager>
{
    Q_OBJECT
public:
    QtScriptEditFactory(QObject *parent = 0);
    ~QtScriptEditFactory();
protected:
    void connectPropertyManager(QtScriptPropertyManager *manager);
    QWidget *createEditor(QtScriptPropertyManager *manager, QtProperty *property,
                QWidget *parent);
    void disconnectPropertyManager(QtScriptPropertyManager *manager);
private:
    QtScriptEditFactoryPrivate *d_ptr;
    Q_DECLARE_PRIVATE(QtScriptEditFactory)
    Q_DISABLE_COPY(QtScriptEditFactory)

private slots:
	void slotPropertyChanged(QtProperty *prop, const QString &newValue);
	void slotSetScript(const QString &value);
	void slotResetScript();
	void slotEditorDestroyed(QObject *editor);
};

#endif