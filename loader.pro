#-------------------------------------------------
#
# Project created by QtCreator 2019-08-08T10:24:17
#
#-------------------------------------------------

QT  += core gui network
QMAKE_CXXFLAGS *= /std:c++17
#QMAKE_LFLAGS += -Wl,-Map=loader.map

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = loader
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++1z

SOURCES += \
    core/dll_mapper2/usermode_mapper.cpp \
    core/kdmapper/intel_driver.cpp \
    core/kdmapper/kdmapper.cpp \
    core/kdmapper/portable_executable.cpp \
    core/kdmapper/service.cpp \
    core/kdmapper/utils.cpp \
    main.cpp \
    MainWindow.cpp \
    core/translation_manager.cpp \
    util/base64.cpp \
    util/shared.cpp \
    core/authenticator.cpp \
    util/encryption.cpp \
    util/hwid.cpp \
    core/driver_mapper/driver.cpp \
    core/driver_mapper/driver_loader.cpp \
    core/driver_mapper/mapper.cpp \
    core/driver_mapper/module_manager.cpp \
    core/driver_mapper/driver_mapper.cpp \
    core/verifier/verifier.cpp \
    core/spoof/spoof.cpp \
    util/kernel.cpp \
    util/process.cpp \
    util/web_api.cpp \
    dialogs/dlg_config.cpp \
    dialogs/dlg_key.cpp \
    util/wmi.cpp \
    core/verifier/verifier2.cpp \
    core/dll_injector/injector.cpp \
    core/dll_mapper/module_mapper.cpp \
    core/dll_mapper/redirection.cpp \
    core/dll_mapper/dll_mapper.cpp \
    core/dll_mapper/mapper_process.cpp \
    dialogs/dlg_hardware.cpp

HEADERS += \
    MainWindow.h \
    core/dll_mapper2/usermode_mapper.h \
    core/kdmapper/intel_driver.hpp \
    core/kdmapper/intel_driver_resource.hpp \
    core/kdmapper/kdmapper.hpp \
    core/kdmapper/nt.hpp \
    core/kdmapper/portable_executable.hpp \
    core/kdmapper/service.hpp \
    core/kdmapper/utils.hpp \
    core/translation_manager.h \
    util/base64.h \
    util/befilter_module.h \
    util/shared.h \
    util/xorstr.h \
    core/authenticator.h \
    util/encryption.h \
    util/hwid.h \
    core/driver_mapper/driver.hpp \
    core/driver_mapper/driver_binary.hpp \
    core/driver_mapper/driver_internal.hpp \
    core/driver_mapper/driver_loader.hpp \
    core/driver_mapper/mapper.hpp \
    core/driver_mapper/module_manager.hpp \
    core/driver_mapper/driver_mapper.hpp \
    core/verifier/verifier.h \
    core/spoof/spoof.h \
    util/kernel.h \
    util/process.h \
    util/web_api.h \
    dialogs/dlg_config.h \
    dialogs/dlg_key.h \
    util/wmi.h \
    core/verifier/verifier2.h \
    core/dll_injector/injector.h \
    core/driver_mapper/module_manager.hpp \
    core/driver_mapper/driver_loader.hpp \
    core/driver_mapper/driver_binary.hpp \
    core/driver_mapper/mapper.hpp \
    core/driver_mapper/driver.hpp \
    core/driver_mapper/driver_internal.hpp \
    core/dll_mapper/module_mapper.hpp \
    core/dll_mapper/util.hpp \
    core/dll_mapper/redirection.hpp \
    core/dll_mapper/apiset_structs.hpp \
    core/dll_mapper/nt_structs.hpp \
    core/dll_mapper/dll_mapper.hpp \
    core/dll_mapper/mapper_process.hpp \
    dialogs/dlg_hardware.h

FORMS += \
    MainWindow.ui \
    dialogs/dlg_config.ui \
    dialogs/dlg_key.ui \
    dialogs/dlg_hardware.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resource.qrc

INCLUDEPATH += "C:/Program Files/VMProtect Ultimate/Include/C"
#INCLUDEPATH += "C:/Program Files/Virtualizer/Include/C"
LIBS += -L"$$_PRO_FILE_PWD_/libs" -llibcurl
LIBS += -L"$$_PRO_FILE_PWD_/libs" -lCrypt32
LIBS += -L"$$_PRO_FILE_PWD_/libs" -lVMProtectSDK64
#LIBS += -L"$$_PRO_FILE_PWD_/libs" -lVirtualizerSDK64
LIBS += -lWbemUuid


win32:RC_ICONS += icons\battlelog.ico

#DISTFILES += \
#    core/driver_mapper/cheat_driver_resource.hpp.txt \
#    core/driver_mapper/cheat_driver_resource_adrian.hpp.txt



