#include <QApplication>
#include "overlay_editor_window.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    OverlayEditorWindow window;
    
    QApplication::setApplicationName("Crosshair ToolKit");
    QApplication::setApplicationVersion("1.0.0");

    window.setWindowTitle(QString("%1 (32x32) - v%2").arg(QApplication::applicationName(), QApplication::applicationVersion()));
    
    window.show();

    return app.exec();
}
