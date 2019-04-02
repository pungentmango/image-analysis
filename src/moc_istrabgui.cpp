/****************************************************************************
** Meta object code from reading C++ file 'istrabgui.h'
**
** Created: Thu May 17 08:28:19 2012
**      by: The Qt Meta Object Compiler version 62 (Qt 4.6.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "istrabgui.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'istrabgui.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.6.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_iStrabGui[] = {

 // content:
       4,       // revision
       0,       // classname
       0,    0, // classinfo
      14,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      11,   10,   10,   10, 0x08,
      37,   10,   10,   10, 0x08,
      67,   10,   10,   10, 0x08,
      92,   10,   10,   10, 0x08,
     120,   10,   10,   10, 0x08,
     151,   10,   10,   10, 0x08,
     181,   10,   10,   10, 0x08,
     211,   10,   10,   10, 0x08,
     236,   10,   10,   10, 0x08,
     263,   10,   10,   10, 0x08,
     289,   10,   10,   10, 0x08,
     309,   10,   10,   10, 0x08,
     337,  331,   10,   10, 0x08,
     362,   10,   10,   10, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_iStrabGui[] = {
    "iStrabGui\0\0on_actionExit_triggered()\0"
    "on_reconnectButton_released()\0"
    "on_pushButton_released()\0"
    "on_analyzeButton_released()\0"
    "on_postReviewButton_released()\0"
    "on_rejectVidButton_released()\0"
    "on_acceptVidButton_released()\0"
    "on_playButton_released()\0"
    "on_recordButton_released()\0"
    "on_startButton_released()\0finishedRecording()\0"
    "clearRejectionNotes()\0state\0"
    "toggleMultipleTechs(int)\0setupDevices()\0"
};

const QMetaObject iStrabGui::staticMetaObject = {
    { &QMainWindow::staticMetaObject, qt_meta_stringdata_iStrabGui,
      qt_meta_data_iStrabGui, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &iStrabGui::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *iStrabGui::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *iStrabGui::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_iStrabGui))
        return static_cast<void*>(const_cast< iStrabGui*>(this));
    return QMainWindow::qt_metacast(_clname);
}

int iStrabGui::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: on_actionExit_triggered(); break;
        case 1: on_reconnectButton_released(); break;
        case 2: on_pushButton_released(); break;
        case 3: on_analyzeButton_released(); break;
        case 4: on_postReviewButton_released(); break;
        case 5: on_rejectVidButton_released(); break;
        case 6: on_acceptVidButton_released(); break;
        case 7: on_playButton_released(); break;
        case 8: on_recordButton_released(); break;
        case 9: on_startButton_released(); break;
        case 10: finishedRecording(); break;
        case 11: clearRejectionNotes(); break;
        case 12: toggleMultipleTechs((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 13: setupDevices(); break;
        default: ;
        }
        _id -= 14;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
