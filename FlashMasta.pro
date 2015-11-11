#-------------------------------------------------
#
# Project created by QtCreator 2015-07-14T10:24:19
#
#-------------------------------------------------

QT       += core gui svg

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = FlashMasta
TEMPLATE = app

CONFIG +=\
    c++11 \
    static \
    staticlib

SOURCES +=\
    src/ui/qt/main.cpp \
    src/cartridge/ngp_cartridge.cpp \
    src/cartridge/cartridge_descriptor.cpp \
    src/cartridge/ngp_chip.cpp \
    src/linkmasta_device/ngp_linkmasta_device.cpp \
    src/linkmasta_device/ngp_linkmasta_messages.cpp \
    src/tasks/forwarding_task_controller.cpp \
    src/tasks/task_controller.cpp \
    src/usb/exception/busy_exception.cpp \
    src/usb/exception/disconnected_exception.cpp \
    src/usb/exception/exception.cpp \
    src/usb/exception/interrupted_exception.cpp \
    src/usb/exception/not_found_exception.cpp \
    src/usb/exception/timeout_exception.cpp \
    src/usb/exception/unconfigured_exception.cpp \
    src/usb/exception/uninitialized_exception.cpp \
    src/usb/exception/unopen_exception.cpp \
    src/usb/libusb_usb_device.cpp \
    src/usb/usb_device.cpp \
    src/ui/qt/main_window.cpp \
    src/linkmasta_device/ws_linkmasta_device.cpp \
    src/linkmasta_device/ws_linkmasta_messages.cpp \
    src/cartridge/ws_cartridge.cpp \
    src/cartridge/ws_rom_chip.cpp \
    src/cartridge/ws_sram_chip.cpp \
    src/linkmasta_device/linkmasta_device.cpp \
    src/ui/qt/task/ngp_cartridge_backup_save_task.cpp \
    src/ui/qt/task/ngp_cartridge_backup_task.cpp \
    src/ui/qt/task/ngp_cartridge_flash_task.cpp \
    src/ui/qt/task/ngp_cartridge_restore_save_task.cpp \
    src/ui/qt/task/ngp_cartridge_task.cpp \
    src/ui/qt/task/ngp_cartridge_verify_task.cpp \
    src/ui/qt/task/ws_cartridge_backup_save_task.cpp \
    src/ui/qt/task/ws_cartridge_backup_task.cpp \
    src/ui/qt/task/ws_cartridge_flash_task.cpp \
    src/ui/qt/task/ws_cartridge_restore_save_task.cpp \
    src/ui/qt/task/ws_cartridge_task.cpp \
    src/ui/qt/task/ws_cartridge_verify_task.cpp \
    src/ui/qt/device_manager.cpp \
    src/ui/qt/libusb_device_manager.cpp \
    src/ui/qt/detail/ngp_fm_cartridge_info_widget.cpp \
    src/ui/qt/detail/fm_cartridge_slot_widget.cpp \
    src/ui/qt/flash_masta_app.cpp \
    src/ui/qt/detail/cartridge_widget.cpp \
    src/ui/qt/worker/lm_cartridge_fetching_worker.cpp \
    src/ui/qt/worker/lm_cartridge_polling_worker.cpp \
    src/ui/qt/detail/lm_detail_widget.cpp

HEADERS  +=\
    src/cartridge/cartridge.h \
    src/cartridge/ngp_cartridge.h \
    src/common/types.h \
    src/cartridge/cartridge_descriptor.h \
    src/cartridge/ngp_chip.h \
    src/linkmasta_device/linkmasta_device.h \
    src/linkmasta_device/ngp_linkmasta_device.h \
    src/linkmasta_device/ngp_linkmasta_messages.h \
    src/tasks/forwarding_task_controller.h \
    src/tasks/task_controller.h \
    src/usb/exception/busy_exception.h \
    src/usb/exception/disconnected_exception.h \
    src/usb/exception/exception.h \
    src/usb/exception/interrupted_exception.h \
    src/usb/exception/not_found_exception.h \
    src/usb/exception/timeout_exception.h \
    src/usb/exception/unconfigured_exception.h \
    src/usb/exception/uninitialized_exception.h \
    src/usb/exception/unopen_exception.h \
    src/usb/libusb_usb_device.h \
    src/usb/usb.h \
    src/usb/usb_device.h \
    src/usb/usbfwd.h \
    src/ui/qt/main_window.h \
    src/linkmasta_device/ws_linkmasta_device.h \
    src/linkmasta_device/ws_linkmasta_messages.h \
    src/cartridge/ws_cartridge.h \
    src/cartridge/ws_rom_chip.h \
    src/cartridge/ws_sram_chip.h \
    src/ui/qt/task/ngp_cartridge_backup_save_task.h \
    src/ui/qt/task/ngp_cartridge_backup_task.h \
    src/ui/qt/task/ngp_cartridge_flash_task.h \
    src/ui/qt/task/ngp_cartridge_restore_save_task.h \
    src/ui/qt/task/ngp_cartridge_task.h \
    src/ui/qt/task/ngp_cartridge_verify_task.h \
    src/ui/qt/task/ws_cartridge_backup_save_task.h \
    src/ui/qt/task/ws_cartridge_backup_task.h \
    src/ui/qt/task/ws_cartridge_flash_task.h \
    src/ui/qt/task/ws_cartridge_restore_save_task.h \
    src/ui/qt/task/ws_cartridge_task.h \
    src/ui/qt/task/ws_cartridge_verify_task.h \
    src/ui/qt/device_manager.h \
    src/ui/qt/libusb_device_manager.h \
    src/ui/qt/detail/ngp_fm_cartridge_info_widget.h \
    src/ui/qt/detail/fm_cartridge_slot_widget.h \
    src/ui/qt/flash_masta_app.h \
    src/ui/qt/detail/cartridge_widget.h \
    src/ui/qt/worker/lm_cartridge_fetching_worker.h \
    src/ui/qt/worker/lm_cartridge_polling_worker.h \
    src/ui/qt/detail/lm_detail_widget.h

FORMS    +=\
    src/ui/qt/main_window.ui \
    src/ui/qt/detail/ngp_fm_cartridge_info_widget.ui \
    src/ui/qt/detail/fm_cartridge_slot_widget.ui \
    src/ui/qt/detail/cartridge_widget.ui \
    src/ui/qt/detail/lm_detail_widget.ui

DISTFILES += \
    README.md

INCLUDEPATH +=\
    src \
    src/common

macx {
    INCLUDEPATH +=\
        includes/osx
    
    QMAKE_MAC_SDK = macosx10.11
    
    QMAKE_LFLAGS +=\
        -L"$$PWD/libs/osx"\
        -lobjc

    LIBS     +=\
        -framework IOKit \
        -framework CoreFoundation \
        "$$PWD/libs/osx/libusb-1.0.a"
    
    QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.10
    
    DEFINES  +=\
        OS_MACOSX

    APP_QML_FILES.files = $$PWD/resources.qrc
    APP_QML_FILES.path = Contents/Resources
    QMAKE_BUNDLE_DATA += APP_QML_FILES
}
win32 {
    INCLUDEPATH +=\
        includes/win

    LIBS     +=\
        "$$PWD/libs/win32/libusb-1.0.a"
		#-L"$$PWD/libs/win32" -l"usb-1.0"
		

    DEFINES  +=\
        OS_WINDOWS
}

RESOURCES += \
    resources.qrc
