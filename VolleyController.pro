QT += core
QT += gui
QT += network
QT += websockets
QT += multimedia
QT += widgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    button.cpp \
    cameratab.cpp \
    clientlistdialog.cpp \
    connection.cpp \
    directorytab.cpp \
    edit.cpp \
    fileserver.cpp \
    generalsetuparguments.cpp \
    generalsetupdialog.cpp \
    main.cpp \
    netServer.cpp \
    panelconfigurator.cpp \
    paneltab.cpp \
    scorecontroller.cpp \
    utility.cpp \
    volleycontroller.cpp \
    volleytab.cpp

HEADERS += \
    button.h \
    cameratab.h \
    clientlistdialog.h \
    connection.h \
    directorytab.h \
    edit.h \
    fileserver.h \
    generalsetuparguments.h \
    generalsetupdialog.h \
    netServer.h \
    panelconfigurator.h \
    paneldirection.h \
    paneltab.h \
    scorecontroller.h \
    utility.h \
    volleycontroller.h \
    volleytab.h

TRANSLATIONS += \
    VolleyController_en_US.ts
CONFIG += lrelease
CONFIG += embed_translations

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    VolleyController.qrc

DISTFILES += \
    android/AndroidManifest.xml \
    android/build.gradle \
    android/gradle.properties \
    android/gradle/wrapper/gradle-wrapper.jar \
    android/gradle/wrapper/gradle-wrapper.properties \
    android/gradlew \
    android/gradlew.bat \
    android/res/values/libs.xml

contains(ANDROID_TARGET_ARCH,arm64-v8a) {
    ANDROID_PACKAGE_SOURCE_DIR = \
        $$PWD/android
}
