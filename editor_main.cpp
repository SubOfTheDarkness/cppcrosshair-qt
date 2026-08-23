#include <QApplication>
#include "overlay_editor_window.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    OverlayEditorWindow window;
    
    window.setWindowTitle("XPM Crosshair Toolkit (32x32)");
    
    window.show();

    return app.exec();
}
