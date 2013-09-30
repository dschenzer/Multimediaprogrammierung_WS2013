#include <QtGui/QGuiApplication>
#include "qtquick2applicationviewer.h"

#include <QtQuick/QQuickView>

#include "glitem.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QtQuick2ApplicationViewer viewer;

    qmlRegisterType<GlItem>("MM2013", 1, 0, "GlItem");

    viewer.setMainQmlFile(QStringLiteral("qml/hellogl/main.qml"));
    viewer.showExpanded();

    return app.exec();
}
