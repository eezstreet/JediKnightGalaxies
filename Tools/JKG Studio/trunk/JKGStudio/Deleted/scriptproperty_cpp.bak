#include "scriptproperty.h"
#include <QtProperty>
#include <QMap>
#include <QIcon>
#include <QMessageBox>
#include <QHBoxLayout>
#include <QApplication>
#include <QStyleOption>
#include <QPainter>
#include <QStyle>
#include <QKeyEvent>
#include "scripteditor.h"

// ---------- EditorFactoryPrivate :
// Base class for editor factory private classes. Manages mapping of properties to editors and vice versa.

template <class Editor>
class EditorFactoryPrivate
{
public:

    typedef QList<Editor *> EditorList;
    typedef QMap<QtProperty *, EditorList> PropertyToEditorListMap;
    typedef QMap<Editor *, QtProperty *> EditorToPropertyMap;

    Editor *createEditor(QtProperty *property, QWidget *parent);
    void initializeEditor(QtProperty *property, Editor *e);
    void slotEditorDestroyed(QObject *object);

    PropertyToEditorListMap  m_createdEditors;
    EditorToPropertyMap m_editorToProperty;
};

template <class Editor>
Editor *EditorFactoryPrivate<Editor>::createEditor(QtProperty *property, QWidget *parent)
{
    Editor *editor = new Editor(parent);
    initializeEditor(property, editor);
    return editor;
}

template <class Editor>
void EditorFactoryPrivate<Editor>::initializeEditor(QtProperty *property, Editor *editor)
{
    Q_TYPENAME PropertyToEditorListMap::iterator it = m_createdEditors.find(property);
    if (it == m_createdEditors.end())
        it = m_createdEditors.insert(property, EditorList());
    it.value().append(editor);
    m_editorToProperty.insert(editor, property);
}

template <class Editor>
void EditorFactoryPrivate<Editor>::slotEditorDestroyed(QObject *object)
{
    const Q_TYPENAME EditorToPropertyMap::iterator ecend = m_editorToProperty.end();
    for (Q_TYPENAME EditorToPropertyMap::iterator itEditor = m_editorToProperty.begin(); itEditor !=  ecend; ++itEditor) {
        if (itEditor.key() == object) {
            Editor *editor = itEditor.key();
            QtProperty *property = itEditor.value();
            const Q_TYPENAME PropertyToEditorListMap::iterator pit = m_createdEditors.find(property);
            if (pit != m_createdEditors.end()) {
                pit.value().removeAll(editor);
                if (pit.value().empty())
                    m_createdEditors.erase(pit);
            }
            m_editorToProperty.erase(itEditor);
            return;
        }
    }
}

// -----------------------------------


class QtScriptPropertyManagerPrivate
{
    QtScriptPropertyManager *q_ptr;
    Q_DECLARE_PUBLIC(QtScriptPropertyManager)
public:
	struct Data
    {
		Data() { val = QString(); defaultValue = QString(); changed = false; }

        QString val;
        QString defaultValue;
		bool changed;
    };

    QMap<const QtProperty *, Data> m_values;
};

// -----------------------------------

QtScriptPropertyManager::QtScriptPropertyManager(QObject *parent)
	: QtAbstractPropertyManager(parent)
{
	d_ptr = new QtScriptPropertyManagerPrivate;
	d_ptr->q_ptr = this;
}

QtScriptPropertyManager::~QtScriptPropertyManager()
{
	clear();
	delete d_ptr;
}

QString QtScriptPropertyManager::value(const QtProperty *property) const
{
	if (d_ptr->m_values.contains(property))
	{
		return d_ptr->m_values[property].val;
	}
	return QString();
}

QString QtScriptPropertyManager::defaultValue(const QtProperty *property) const
{
	if (d_ptr->m_values.contains(property))
	{
		return d_ptr->m_values[property].defaultValue;

	}
	return QString();
}

void QtScriptPropertyManager::setValue(QtProperty *property, QString val)
{
	QMap<const QtProperty *, QtScriptPropertyManagerPrivate::Data>::Iterator it = d_ptr->m_values.find(property);
	if (it == d_ptr->m_values.end())
		return;

	if (it.value().val == val)
		return;


	it.value().val = val;
	it.value().changed = (val != it.value().defaultValue);
		
	emit propertyChanged(property);
	emit valueChanged(property, val);
}

void QtScriptPropertyManager::setDefaultValue(QtProperty *property, QString defaultVal)
{
	QMap<const QtProperty *, QtScriptPropertyManagerPrivate::Data>::Iterator it = d_ptr->m_values.find(property);
	if (it == d_ptr->m_values.end())
		return;

	if (it.value().defaultValue == defaultVal)
		return;

	it.value().defaultValue = defaultVal;
}

void QtScriptPropertyManager::resetValue(QtProperty *property)
{
	QMap<const QtProperty *, QtScriptPropertyManagerPrivate::Data>::Iterator it = d_ptr->m_values.find(property);
	if (it == d_ptr->m_values.end())
		return;

	if (it.value().val == it.value().defaultValue)
		return;


	it.value().val = it.value().defaultValue;
	it.value().changed = false;
		
	emit propertyChanged(property);
	emit valueChanged(property, it.value().defaultValue);
}


QString QtScriptPropertyManager::valueText(const QtProperty *property) const
{
	const QMap<const QtProperty *, QtScriptPropertyManagerPrivate::Data>::const_iterator it = d_ptr->m_values.constFind(property);
	if (it == d_ptr->m_values.constEnd())
		return QString();

	if (it.value().changed)
		return "< Set >";
	else
		return "< Not Set >";
}

QIcon QtScriptPropertyManager::valueIcon(const QtProperty *property) const
{
	return QIcon();
}
	
void QtScriptPropertyManager::initializeProperty(QtProperty *property)
{
	d_ptr->m_values[property] = QtScriptPropertyManagerPrivate::Data();
}

void QtScriptPropertyManager::uninitializeProperty(QtProperty *property)
{
	d_ptr->m_values.remove(property);
}

// -----------------------------------


QScriptEditWidget::QScriptEditWidget(QWidget *parent)
	: QWidget(parent),
	m_label(new QLabel(this)),
	m_edit(new QToolButton(this)),
	m_reset(new QToolButton(this))
{
	QHBoxLayout *lt = new QHBoxLayout(this);
    
    if (QApplication::layoutDirection() == Qt::LeftToRight)
        lt->setContentsMargins(4, 0, 0, 0);
    else
        lt->setContentsMargins(0, 0, 4, 0);

    lt->setSpacing(0);
    lt->addWidget(m_label);
    lt->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Ignored));

    m_edit->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Ignored);
    m_edit->setFixedWidth(20);
    setFocusProxy(m_edit);
    setFocusPolicy(m_edit->focusPolicy());
    m_edit->setText(tr("..."));
	m_edit->setToolTip("Edit this script");
	m_edit->installEventFilter(this);
    
    connect(m_edit, SIGNAL(clicked()), this, SLOT(onEditClicked()));
    lt->addWidget(m_edit);

	m_reset->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Ignored);
    m_reset->setFixedWidth(20);
    m_reset->setIcon(QIcon(":/icons/reset.png"));
	m_reset->setToolTip("Reset the script back to default");
	m_reset->installEventFilter(this);
	m_reset->setEnabled(false);

	connect(m_reset, SIGNAL(clicked()), this, SLOT(onResetClicked()));
    lt->addWidget(m_reset);


    m_label->setText("< Not Set >");
}

QScriptEditWidget::~QScriptEditWidget()
{

}

void QScriptEditWidget::setValue(const QString &text)
{
	m_val = text;
	if (m_val == m_defVal) {
		m_reset->setEnabled(false);
		m_label->setText("< Not Set >");
	} else {
		m_reset->setEnabled(true);
		m_label->setText("< Set >");
	}
}

bool QScriptEditWidget::eventFilter(QObject *obj, QEvent *ev)
{
    if (obj == m_edit || obj == m_reset) {
        switch (ev->type()) {
        case QEvent::KeyPress:
        case QEvent::KeyRelease: { // Prevent the QToolButton from handling Enter/Escape meant control the delegate
            switch (static_cast<const QKeyEvent*>(ev)->key()) {
            case Qt::Key_Escape:
            case Qt::Key_Enter:
            case Qt::Key_Return:
                ev->ignore();
                return true;
            default:
                break;
            }
        }
            break;
        default:
            break;
        }
    }
    return QWidget::eventFilter(obj, ev);
}

void QScriptEditWidget::paintEvent(QPaintEvent *)
{
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

void QScriptEditWidget::setDefaultValue(const QString &text)
{
	m_defVal = text;
	if (m_val == m_defVal) {
		m_reset->setEnabled(false);
		m_label->setText("< Not Set >");
	} else { 
		m_reset->setEnabled(true);
		m_label->setText("< Set >");
	}
}

void QScriptEditWidget::onEditClicked()
{
	ScriptEditor editor;
	editor.setScript(m_val);

	int ret = editor.exec();

	if (ret) {
		setValue(editor.getScript());
		emit scriptEdited(m_val);
	}
}

void QScriptEditWidget::onResetClicked()
{
	if (m_val == m_defVal)
		return;

	if (QMessageBox::warning(this, "Reset script", "Are you sure you wish to reset this script?\nThis action cannot be undone!", QMessageBox::Yes | QMessageBox::No) == QMessageBox::No)
		return;

	m_label->setText("< Not Set >");
	m_reset->setEnabled(false);
	m_val = m_defVal;
	emit scriptReset();

	
}

// -----------------------------------

class QtScriptEditFactoryPrivate : public EditorFactoryPrivate<QScriptEditWidget>
{
	QtScriptEditFactory *q_ptr;
	Q_DECLARE_PUBLIC(QtScriptEditFactory)
public:
	void slotPropertyChanged(QtProperty *property, const QString &value);
	void slotSetScript(const QString &value);
	void slotResetScript();

};

void QtScriptEditFactoryPrivate::slotPropertyChanged(QtProperty *property, const QString &value)
{
	const PropertyToEditorListMap::iterator it = m_createdEditors.find(property);
	if (it == m_createdEditors.end())
		return;
	QListIterator<QScriptEditWidget *> itEditor(it.value());

	while (itEditor.hasNext())
		itEditor.next()->setValue(value);
}

void QtScriptEditFactoryPrivate::slotSetScript(const QString &value)
{
	QObject *object = q_ptr->sender();
    const EditorToPropertyMap::ConstIterator ecend = m_editorToProperty.constEnd();
    for (EditorToPropertyMap::ConstIterator itEditor = m_editorToProperty.constBegin(); itEditor != ecend; ++itEditor)
        if (itEditor.key() == object) {
            QtProperty *property = itEditor.value();
            QtScriptPropertyManager *manager = q_ptr->propertyManager(property);
            if (!manager)
                return;
            manager->setValue(property, value);
            return;
        }
}

void QtScriptEditFactoryPrivate::slotResetScript()
{
	QObject *object = q_ptr->sender();
    const EditorToPropertyMap::ConstIterator ecend = m_editorToProperty.constEnd();
    for (EditorToPropertyMap::ConstIterator itEditor = m_editorToProperty.constBegin(); itEditor != ecend; ++itEditor)
        if (itEditor.key() == object) {
            QtProperty *property = itEditor.value();
            QtScriptPropertyManager *manager = q_ptr->propertyManager(property);
            if (!manager)
                return;
            manager->resetValue(property);
            return;
        }
}


// -----------------------------------


QtScriptEditFactory::QtScriptEditFactory(QObject *parent)
	: QtAbstractEditorFactory(0)
{
	d_ptr = new QtScriptEditFactoryPrivate;
	d_ptr->q_ptr = this;
}

QtScriptEditFactory::~QtScriptEditFactory()
{
	qDeleteAll(d_ptr->m_editorToProperty.keys());
    delete d_ptr;
}

void QtScriptEditFactory::connectPropertyManager(QtScriptPropertyManager *manager)
{
	connect(manager, SIGNAL(valueChanged(QtProperty*,const QString &)),
            this, SLOT(slotPropertyChanged(QtProperty*,const QString &)));
}

QWidget *QtScriptEditFactory::createEditor(QtScriptPropertyManager *manager, QtProperty *property, QWidget *parent)
{
	QScriptEditWidget *editor = d_ptr->createEditor(property, parent);
	editor->setValue(manager->value(property));
	editor->setDefaultValue(manager->defaultValue(property));
	connect(editor, SIGNAL(scriptEdited(const QString &)), this, SLOT(slotSetScript(const QString &)));
	connect(editor, SIGNAL(scriptReset()), this, SLOT(slotResetScript()));
	connect(editor, SIGNAL(destroyed(QObject *)), this, SLOT(slotEditorDestroyed(QObject *)));
	return editor;
}

void QtScriptEditFactory::disconnectPropertyManager(QtScriptPropertyManager *manager)
{
	disconnect(manager, SIGNAL(valueChanged(QtProperty*,const QString &)), this, SLOT(slotPropertyChanged(QtProperty*,const QString &)));
}

void QtScriptEditFactory::slotSetScript(const QString &value)
{
	d_ptr->slotSetScript(value);
}

void QtScriptEditFactory::slotResetScript()
{
	d_ptr->slotResetScript();
}

void QtScriptEditFactory::slotPropertyChanged(QtProperty *prop, const QString &newValue)
{
	d_ptr->slotPropertyChanged(prop, newValue);
}

void QtScriptEditFactory::slotEditorDestroyed(QObject *editor)
{
	d_ptr->slotEditorDestroyed(editor);
}