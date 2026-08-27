#pragma once

#include <QObject>
#include <QProcess>
#include <QString>
#include <QTimer>
#include <QtQml/qqmlregistration.h>
#include "../config_parser.h"

class CrosshairManager : public QObject {
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(bool isRunning READ isRunning NOTIFY isRunningChanged)
    Q_PROPERTY(int offsetX READ offsetX WRITE setOffsetX NOTIFY offsetXChanged)
    Q_PROPERTY(int offsetY READ offsetY WRITE setOffsetY NOTIFY offsetYChanged)
    Q_PROPERTY(QString hotkey READ hotkey WRITE setHotkey NOTIFY hotkeyChanged)

public:
    explicit CrosshairManager(QObject *parent = nullptr);
    ~CrosshairManager();

    bool isRunning() const;
    int offsetX() const;
    int offsetY() const;
    QString hotkey() const;

    void setOffsetX(int val);
    void setOffsetY(int val);
    void setHotkey(const QString &hk);

public slots:
    void checkProcess();   
    void toggleOverlay();  
    void saveSettings();   
    void flushCache();
    void openEditorApp();

signals:
    void isRunningChanged(bool running);
    void offsetXChanged(int val);
    void offsetYChanged(int val);
    void hotkeyChanged(const QString &hk);
    void logMessage(const QString &msg);

private slots:
    void readOverlayOutput();
    void handleOverlayFinished(int exitCode);

private:
    QProcess *overlayProcess;
    bool m_isRunning;
    Config m_cfg;
    QTimer *updateTimer;
    
    QString m_logCache; 
    bool m_qmlConnected; 

    bool checkX11HotkeyConflict(const QString &hotkeyStr);
    void sendLog(const QString &msg);
};
