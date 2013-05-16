/****************************************************************************
** Meta object code from reading C++ file 'dlgeditor.h'
**
** Created: Fri Nov 16 21:41:50 2012
**      by: The Qt Meta Object Compiler version 63 (Qt 4.8.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../dlgeditor.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'dlgeditor.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_DlgEditor[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       7,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      17,   11,   10,   10, 0x08,
      57,   49,   10,   10, 0x08,
      77,   49,   10,   10, 0x08,
     103,   97,   10,   10, 0x08,
     127,  124,   10,   10, 0x08,
     162,  145,   10,   10, 0x08,
     234,  211,   10,   10, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_DlgEditor[] = {
    "DlgEditor\0\0index\0onSelectionChanged(QModelIndex)\0"
    "enabled\0onUndoChanged(bool)\0"
    "onRedoChanged(bool)\0clean\0"
    "onCleanChanged(bool)\0pt\0onContext(QPoint)\0"
    "index,prop,value\0"
    "onLabelChanged(QModelIndex,QtProperty*,QVariant)\0"
    "index,newParent,before\0"
    "onNodeMove(QModelIndex,QModelIndex,QModelIndex)\0"
};

void DlgEditor::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        DlgEditor *_t = static_cast<DlgEditor *>(_o);
        switch (_id) {
        case 0: _t->onSelectionChanged((*reinterpret_cast< const QModelIndex(*)>(_a[1]))); break;
        case 1: _t->onUndoChanged((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 2: _t->onRedoChanged((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 3: _t->onCleanChanged((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 4: _t->onContext((*reinterpret_cast< const QPoint(*)>(_a[1]))); break;
        case 5: _t->onLabelChanged((*reinterpret_cast< const QModelIndex(*)>(_a[1])),(*reinterpret_cast< QtProperty*(*)>(_a[2])),(*reinterpret_cast< const QVariant(*)>(_a[3]))); break;
        case 6: _t->onNodeMove((*reinterpret_cast< const QModelIndex(*)>(_a[1])),(*reinterpret_cast< const QModelIndex(*)>(_a[2])),(*reinterpret_cast< const QModelIndex(*)>(_a[3]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData DlgEditor::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject DlgEditor::staticMetaObject = {
    { &MdiSubBase::staticMetaObject, qt_meta_stringdata_DlgEditor,
      qt_meta_data_DlgEditor, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &DlgEditor::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *DlgEditor::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *DlgEditor::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_DlgEditor))
        return static_cast<void*>(const_cast< DlgEditor*>(this));
    return MdiSubBase::qt_metacast(_clname);
}

int DlgEditor::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = MdiSubBase::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 7)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 7;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
