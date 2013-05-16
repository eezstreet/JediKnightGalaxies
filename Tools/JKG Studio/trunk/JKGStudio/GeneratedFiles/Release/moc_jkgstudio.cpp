/****************************************************************************
** Meta object code from reading C++ file 'jkgstudio.h'
**
** Created: Fri Nov 16 21:41:49 2012
**      by: The Qt Meta Object Compiler version 63 (Qt 4.8.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../jkgstudio.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'jkgstudio.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_JKGStudio[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
      17,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      11,   10,   10,   10, 0x08,
      25,   10,   10,   10, 0x08,
      41,   10,   10,   10, 0x08,
      54,   10,   10,   10, 0x08,
      63,   10,   10,   10, 0x08,
      72,   10,   10,   10, 0x08,
      83,   10,   10,   10, 0x08,
      95,   10,   10,   10, 0x08,
     105,   10,   10,   10, 0x08,
     114,   10,   10,   10, 0x08,
     123,   10,   10,   10, 0x08,
     132,   10,   10,   10, 0x08,
     140,   10,   10,   10, 0x08,
     149,   10,   10,   10, 0x08,
     163,  159,   10,   10, 0x08,
     179,   10,   10,   10, 0x08,
     206,  202,   10,   10, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_JKGStudio[] = {
    "JKGStudio\0\0onNewScript()\0onNewDialogue()\0"
    "onNewQuest()\0onOpen()\0onSave()\0"
    "onSaveAs()\0onCompile()\0onClose()\0"
    "onQuit()\0onUndo()\0onRedo()\0onCut()\0"
    "onCopy()\0onPaste()\0idx\0onCloseTab(int)\0"
    "onSubWindowActivated()\0cmd\0onExtCommand(int)\0"
};

void JKGStudio::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        JKGStudio *_t = static_cast<JKGStudio *>(_o);
        switch (_id) {
        case 0: _t->onNewScript(); break;
        case 1: _t->onNewDialogue(); break;
        case 2: _t->onNewQuest(); break;
        case 3: _t->onOpen(); break;
        case 4: _t->onSave(); break;
        case 5: _t->onSaveAs(); break;
        case 6: _t->onCompile(); break;
        case 7: _t->onClose(); break;
        case 8: _t->onQuit(); break;
        case 9: _t->onUndo(); break;
        case 10: _t->onRedo(); break;
        case 11: _t->onCut(); break;
        case 12: _t->onCopy(); break;
        case 13: _t->onPaste(); break;
        case 14: _t->onCloseTab((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 15: _t->onSubWindowActivated(); break;
        case 16: _t->onExtCommand((*reinterpret_cast< int(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData JKGStudio::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject JKGStudio::staticMetaObject = {
    { &QMainWindow::staticMetaObject, qt_meta_stringdata_JKGStudio,
      qt_meta_data_JKGStudio, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &JKGStudio::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *JKGStudio::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *JKGStudio::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_JKGStudio))
        return static_cast<void*>(const_cast< JKGStudio*>(this));
    return QMainWindow::qt_metacast(_clname);
}

int JKGStudio::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 17)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 17;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
