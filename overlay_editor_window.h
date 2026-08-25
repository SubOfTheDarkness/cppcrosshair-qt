#pragma once

#include <QWidget>
#include <QColor>
#include <QProcess>

namespace Ui {
    class overlay_editor_window;
}

class OverlayEditorWindow : public QWidget {
    Q_OBJECT

public:
    explicit OverlayEditorWindow(QWidget *parent = nullptr);
    ~OverlayEditorWindow();

private slots:
    void toggleOverlayProcess();
    void readOverlayOutput();
    void handleOverlayFinished(int exitCode);

    void saveXpmFile();
    void cancelChanges();

    void showAboutDialog();

private:
    Ui::overlay_editor_window *ui;
    
    QProcess *overlayProcess;

    void setupPalette();
    void initConnections();
    void loadXpmFile();
    void checkProcess();
};
