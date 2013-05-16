/****************************************************************************
** Meta object code from reading C++ file 'scriptproperty.h'
**
** Created: Fri Nov 16 21:41:49 2012
**      by: The Qt Meta Object Compiler version 63 (Qt 4.8.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../scriptproperty.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'scriptproperty.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_QtScriptPropertyManager[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       4,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: signature, parameters, type, tag, flags
      38,   25,   24,   24, 0x05,

 // slots: signature, parameters, type, tag, flags
      72,   25,   24,   24, 0x0a,
     122,  102,   24,   24, 0x0a,
     168,  159,   24,   24, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_QtScriptPropertyManager[] = {
    "QtScriptPropertyManager\0\0property,val\0"
    "valueChanged(QtProperty*,QString)\0"
    "setValue(QtProperty*,QString)\0"
    "property,defaultVal\0"
    "setDefaultValue(QtProperty*,QString)\0"
    "property\0resetValue(QtProperty*)\0"
};

void QtScriptPropertyManager::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        QtScriptPropertyManager *_t = static_cast<QtScriptPropertyManager *>(_o);
        switch (_id) {
        case 0: _t->valueChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2]))); break;
        case 1: _t->setValue((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2]))); break;
        case 2: _t->setDefaultValue((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2]))); break;
        case 3: _t->resetValue((*reinterpret_cast< QtProperty*(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData QtScriptPropertyManager::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject QtScriptPropertyManager::staticMetaObject = {
    { &QtAbstractPropertyManager::staticMetaObject, qt_meta_stringdata_QtScriptPropertyManager,
      qt_meta_data_QtScriptPropertyManager, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &QtScriptPropertyManager::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *QtScriptPropertyManager::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *QtScriptPropertyManager::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_QtScriptPropertyManager))
        return static_cast<void*>(const_cast< QtScriptPropertyManager*>(this));
    return QtAbstractPropertyManager::qt_metacast(_clname);
}

int QtScriptPropertyManager::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QtAbstractPropertyManager::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 4)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 4;
    }
    return _id;
}

// SIGNAL 0
void QtScriptPropertyManager::valueChanged(QtProperty * _t1, QString _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
static const uint qt_meta_data_QScriptEditWidget[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       4,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: signature, parameters, type, tag, flags
      26,   19,   18,   18, 0x05,
      48,   18,   18,   18, 0x05,

 // slots: signature, parameters, type, tag, flags
      62,   18,   18,   18, 0x08,
      78,   18,   18,   18, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_QScriptEditWidget[] = {
    "QScriptEditWidget\0\0script\0"
    "scriptEdited(QString)\0scriptReset()\0"
    "onEditClicked()\0onResetClicked()\0"
};

void QScriptEditWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        QScriptEditWidget *_t = static_cast<QScriptEditWidget *>(_o);
        switch (_id) {
        case 0: _t->scriptEdited((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 1: _t->scriptReset(); break;
        case 2: _t->onEditClicked(); break;
        case 3: _t->onResetClicked(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData QScriptEditWidget::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject QScriptEditWidget::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_QScriptEditWidget,
      qt_meta_data_QScriptEditWidget, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &QScriptEditWidget::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *QScriptEditWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *QScriptEditWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_QScriptEditWidget))
        return static_cast<void*>(const_cast< QScriptEditWidget*>(this));
    return QWidget::qt_metacast(_clname);
}

int QScriptEditWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 4)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 4;
    }
    return _id;
}

// SIGNAL 0
void QScriptEditWidget::scriptEdited(const QString & _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void QScriptEditWidget::scriptReset()
{
    QMetaObject::activate(this, &staticMetaObject, 1, 0);
}
static const uint qt_meta_data_QtScriptEditFactory[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       4,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      35,   21,   20,   20, 0x08,
      82,   76,   20,   20, 0x08,
     105,   20,   20,   20, 0x08,
     130,  123,   20,   20, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_QtScriptEditFactory[] = {
    "QtScriptEditFactory\0\0prop,newValue\0"
    "slotPropertyChanged(QtProperty*,QString)\0"
    "value\0slotSetScript(QString)\0"
    "slotResetScript()\0editor\0"
    "slotEditorDestroyed(QObject*)\0"
};

void QtScriptEditFactory::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        QtScriptEditFactory *_t = static_cast<QtScriptEditFactory *>(_o);
        switch (_id) {
        case 0: _t->slotPropertyChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QString(*)>(_a[2]))); break;
        case 1: _t->slotSetScript((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 2: _t->slotResetScript(); break;
        case 3: _t->slotEditorDestroyed((*reinterpret_cast< QObject*(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData QtScriptEditFactory::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject QtScriptEditFactory::staticMetaObject = {
    { &QtAbstractEditorFactory<QtScriptPropertyManager>::staticMetaObject, qt_meta_stringdata_QtScriptEditFactory,
      qt_meta_data_QtScriptEditFactory, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &QtScriptEditFactory::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *QtScriptEditFactory::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *QtScriptEditFactory::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_QtScriptEditFactory))
        return static_cast<void*>(const_cast< QtScriptEditFactory*>(this));
    return QtAbstractEditorFactory<QtScriptPropertyManager>::qt_metacast(_clname);
}

int QtScriptEditFactory::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QtAbstractEditorFactory<QtScriptPropertyManager>::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 4)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 4;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
