/****************************************************************************
** Meta object code from reading C++ file 'MainWindow.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.11.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../src/MainWindow.h"
#include <QtGui/qtextcursor.h>
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'MainWindow.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 69
#error "This file was generated using the moc from 6.11.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

#ifndef Q_CONSTINIT
#define Q_CONSTINIT
#endif

QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
QT_WARNING_DISABLE_GCC("-Wuseless-cast")
namespace {
struct qt_meta_tag_ZN11MotorWidgetE_t {};
} // unnamed namespace

template <> constexpr inline auto MotorWidget::qt_create_metaobjectdata<qt_meta_tag_ZN11MotorWidgetE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "MotorWidget"
    };

    QtMocHelpers::UintData qt_methods {
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<MotorWidget, qt_meta_tag_ZN11MotorWidgetE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject MotorWidget::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN11MotorWidgetE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN11MotorWidgetE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN11MotorWidgetE_t>.metaTypes,
    nullptr
} };

void MotorWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<MotorWidget *>(_o);
    (void)_t;
    (void)_c;
    (void)_id;
    (void)_a;
}

const QMetaObject *MotorWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *MotorWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN11MotorWidgetE_t>.strings))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int MotorWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    return _id;
}
namespace {
struct qt_meta_tag_ZN10MainWindowE_t {};
} // unnamed namespace

template <> constexpr inline auto MainWindow::qt_create_metaobjectdata<qt_meta_tag_ZN10MainWindowE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "MainWindow",
        "openPort1",
        "",
        "closePort1",
        "openPort2",
        "closePort2",
        "openPort3",
        "closePort3",
        "openPort4",
        "closePort4",
        "refreshPorts",
        "readPort1",
        "readPort2",
        "readPort3",
        "readPort4",
        "pollModbus",
        "toggleRecording",
        "recordDataLine",
        "updateUI",
        "logEventToFiles",
        "eventName",
        "changeTheme",
        "index",
        "handleError1",
        "QSerialPort::SerialPortError",
        "error",
        "handleError2",
        "handleError3",
        "handleError4",
        "startSequence",
        "updateSequence"
    };

    QtMocHelpers::UintData qt_methods {
        // Slot 'openPort1'
        QtMocHelpers::SlotData<void()>(1, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'closePort1'
        QtMocHelpers::SlotData<void()>(3, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'openPort2'
        QtMocHelpers::SlotData<void()>(4, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'closePort2'
        QtMocHelpers::SlotData<void()>(5, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'openPort3'
        QtMocHelpers::SlotData<void()>(6, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'closePort3'
        QtMocHelpers::SlotData<void()>(7, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'openPort4'
        QtMocHelpers::SlotData<void()>(8, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'closePort4'
        QtMocHelpers::SlotData<void()>(9, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'refreshPorts'
        QtMocHelpers::SlotData<void()>(10, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'readPort1'
        QtMocHelpers::SlotData<void()>(11, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'readPort2'
        QtMocHelpers::SlotData<void()>(12, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'readPort3'
        QtMocHelpers::SlotData<void()>(13, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'readPort4'
        QtMocHelpers::SlotData<void()>(14, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'pollModbus'
        QtMocHelpers::SlotData<void()>(15, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'toggleRecording'
        QtMocHelpers::SlotData<void()>(16, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'recordDataLine'
        QtMocHelpers::SlotData<void()>(17, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'updateUI'
        QtMocHelpers::SlotData<void()>(18, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'logEventToFiles'
        QtMocHelpers::SlotData<void(const QString &)>(19, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QString, 20 },
        }}),
        // Slot 'changeTheme'
        QtMocHelpers::SlotData<void(int)>(21, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 22 },
        }}),
        // Slot 'handleError1'
        QtMocHelpers::SlotData<void(QSerialPort::SerialPortError)>(23, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 24, 25 },
        }}),
        // Slot 'handleError2'
        QtMocHelpers::SlotData<void(QSerialPort::SerialPortError)>(26, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 24, 25 },
        }}),
        // Slot 'handleError3'
        QtMocHelpers::SlotData<void(QSerialPort::SerialPortError)>(27, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 24, 25 },
        }}),
        // Slot 'handleError4'
        QtMocHelpers::SlotData<void(QSerialPort::SerialPortError)>(28, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 24, 25 },
        }}),
        // Slot 'startSequence'
        QtMocHelpers::SlotData<void()>(29, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'updateSequence'
        QtMocHelpers::SlotData<void()>(30, 2, QMC::AccessPrivate, QMetaType::Void),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<MainWindow, qt_meta_tag_ZN10MainWindowE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject MainWindow::staticMetaObject = { {
    QMetaObject::SuperData::link<QMainWindow::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN10MainWindowE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN10MainWindowE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN10MainWindowE_t>.metaTypes,
    nullptr
} };

void MainWindow::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<MainWindow *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->openPort1(); break;
        case 1: _t->closePort1(); break;
        case 2: _t->openPort2(); break;
        case 3: _t->closePort2(); break;
        case 4: _t->openPort3(); break;
        case 5: _t->closePort3(); break;
        case 6: _t->openPort4(); break;
        case 7: _t->closePort4(); break;
        case 8: _t->refreshPorts(); break;
        case 9: _t->readPort1(); break;
        case 10: _t->readPort2(); break;
        case 11: _t->readPort3(); break;
        case 12: _t->readPort4(); break;
        case 13: _t->pollModbus(); break;
        case 14: _t->toggleRecording(); break;
        case 15: _t->recordDataLine(); break;
        case 16: _t->updateUI(); break;
        case 17: _t->logEventToFiles((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 18: _t->changeTheme((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 19: _t->handleError1((*reinterpret_cast<std::add_pointer_t<QSerialPort::SerialPortError>>(_a[1]))); break;
        case 20: _t->handleError2((*reinterpret_cast<std::add_pointer_t<QSerialPort::SerialPortError>>(_a[1]))); break;
        case 21: _t->handleError3((*reinterpret_cast<std::add_pointer_t<QSerialPort::SerialPortError>>(_a[1]))); break;
        case 22: _t->handleError4((*reinterpret_cast<std::add_pointer_t<QSerialPort::SerialPortError>>(_a[1]))); break;
        case 23: _t->startSequence(); break;
        case 24: _t->updateSequence(); break;
        default: ;
        }
    }
}

const QMetaObject *MainWindow::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *MainWindow::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN10MainWindowE_t>.strings))
        return static_cast<void*>(this);
    return QMainWindow::qt_metacast(_clname);
}

int MainWindow::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 25)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 25;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 25)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 25;
    }
    return _id;
}
QT_WARNING_POP
