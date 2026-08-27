#include "crosshair_manager.h"
#include <QFile>
#include <QDir>
#include <sys/types.h>
#include <signal.h>
#include <fstream>

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include <QStandardPaths>

static bool x11_grab_failed = false;
static int handleX11HotkeyError(Display *, XErrorEvent *err) {
    if (err->error_code == 10) x11_grab_failed = true;
    return 0;
}

CrosshairManager::CrosshairManager(QObject *parent)
    : QObject(parent)
    , overlayProcess(new QProcess(this))
    , m_isRunning(false)
    , updateTimer(new QTimer(this))
    , m_qmlConnected(false)
{
    m_cfg = loadConfig();

    connect(overlayProcess, &QProcess::readyReadStandardOutput, this, &CrosshairManager::readOverlayOutput);
    connect(overlayProcess, &QProcess::readyReadStandardError, this, &CrosshairManager::readOverlayOutput);
    connect(overlayProcess, qOverload<int, QProcess::ExitStatus>(&QProcess::finished), this, &CrosshairManager::handleOverlayFinished);
    
    connect(updateTimer, &QTimer::timeout, this, &CrosshairManager::checkProcess);
    updateTimer->start(1000);

    checkProcess(); 
}

CrosshairManager::~CrosshairManager() {
    if (overlayProcess) {
        if (overlayProcess->state() == QProcess::Running) {
            overlayProcess->disconnect();
            overlayProcess->closeReadChannel(QProcess::StandardOutput);
            overlayProcess->closeReadChannel(QProcess::StandardError);
            overlayProcess->setParent(nullptr); 
        }
    }
}

void CrosshairManager::sendLog(const QString &msg) {
    if (m_qmlConnected) {
        emit logMessage(msg);
    } else {
        if (!m_logCache.isEmpty()) {
            m_logCache.append("\n");
        }
        m_logCache.append(msg);
    }
}

void CrosshairManager::flushCache() {
    m_qmlConnected = true;
    
    if (!m_logCache.isEmpty()) {
        emit logMessage(m_logCache);
        m_logCache.clear();
    }
}

bool CrosshairManager::isRunning() const { return m_isRunning; }
int CrosshairManager::offsetX() const { return m_cfg.offset_x; }
int CrosshairManager::offsetY() const { return m_cfg.offset_y; }
QString CrosshairManager::hotkey() const { return QString::fromStdString(m_cfg.hotkey); }

void CrosshairManager::setOffsetX(int val) { if (m_cfg.offset_x != val) { m_cfg.offset_x = val; emit offsetXChanged(val); } }
void CrosshairManager::setOffsetY(int val) { if (m_cfg.offset_y != val) { m_cfg.offset_y = val; emit offsetYChanged(val); } }
void CrosshairManager::setHotkey(const QString &hk) { if (QString::fromStdString(m_cfg.hotkey) != hk) { m_cfg.hotkey = hk.toStdString(); emit hotkeyChanged(hk); } }

void CrosshairManager::checkProcess() {
    QProcess checkProc;
    checkProc.start("pgrep", QStringList() << "-f" << "crosshair_overlay");
    if (checkProc.waitForFinished(300)) {
        QString output = QString::fromUtf8(checkProc.readAllStandardOutput()).trimmed();
        bool currentlyRunning = !output.isEmpty();
        
        if (m_isRunning != currentlyRunning) {
            m_isRunning = currentlyRunning;
            emit isRunningChanged(m_isRunning);
            
            if (m_isRunning) {
                sendLog("[*] Sync: Active crosshair_overlay detected.");
            } else {
                sendLog("[-] Sync: Overlay process is not running.");
            }
        }
    }
}

bool CrosshairManager::checkX11HotkeyConflict(const QString &hotkeyStr) {
    if (hotkeyStr.isEmpty()) return false;
    Display* dpy = XOpenDisplay(NULL);
    if (!dpy) return false;

    int screen = DefaultScreen(dpy);
    Window root = RootWindow(dpy, screen);
    QStringList tokens = hotkeyStr.split("+");
    QString keyChar = tokens.last();
    
    unsigned int modifiers = 0;
    if (hotkeyStr.contains("Ctrl"))  modifiers |= ControlMask;
    if (hotkeyStr.contains("Alt"))   modifiers |= Mod1Mask;
    if (hotkeyStr.contains("Shift")) modifiers |= ShiftMask;
    if (hotkeyStr.contains("Meta"))  modifiers |= Mod4Mask;

    KeyCode test_keycode = XKeysymToKeycode(dpy, XStringToKeysym(keyChar.toStdString().c_str()));
    if (test_keycode != 0) {
        x11_grab_failed = false;
        XSync(dpy, False);
        int (*old_handler)(Display*, XErrorEvent*) = XSetErrorHandler(handleX11HotkeyError);
        XGrabKey(dpy, test_keycode, modifiers, root, False, GrabModeAsync, GrabModeAsync);
        XSync(dpy, False);
        XSetErrorHandler(old_handler);
        
        if (x11_grab_failed) {
            XCloseDisplay(dpy);
            return true; 
        }
        XUngrabKey(dpy, test_keycode, modifiers, root);
        XFlush(dpy);
    }
    XCloseDisplay(dpy);
    return false;
}

void CrosshairManager::toggleOverlay() {
    if (overlayProcess->state() == QProcess::Running || m_isRunning) {
        sendLog("[*] Stopping overlay instance...");
        
        if (overlayProcess->state() == QProcess::Running) {
            overlayProcess->kill();
            overlayProcess->waitForFinished(400);
        } else {
            std::ifstream pid_file("/tmp/crosshair_overlay.pid");
            if (pid_file.is_open()) {
                pid_t target_pid;
                if (pid_file >> target_pid) kill(target_pid, SIGTERM);
                pid_file.close();
            } else {
                QProcess::execute("killall", QStringList() << "crosshair_overlay");
            }
        }
    } 
    else {
        sendLog("[*] Launching crosshair_overlay...");

        overlayProcess->start("crosshair_overlay", QStringList());
        
        if (!overlayProcess->waitForStarted(1000)) {
            sendLog("[!] Error: Could not execute crosshair_overlay.");
            return;
        }
    }
}

void CrosshairManager::readOverlayOutput() {
    QByteArray stdOut = overlayProcess->readAllStandardOutput();
    QByteArray stdErr = overlayProcess->readAllStandardError();
    if (!stdOut.isEmpty()) sendLog(QString::fromUtf8(stdOut).trimmed());
    if (!stdErr.isEmpty()) sendLog("[Process Error] " + QString::fromUtf8(stdErr).trimmed());
}

void CrosshairManager::handleOverlayFinished(int exitCode) {
    sendLog(QString("[-] Process finished. Exit Code: %1").arg(exitCode));
}

void CrosshairManager::saveSettings() {
    QString hk = hotkey();
    if (checkX11HotkeyConflict(hk)) {
        sendLog("[!] Error: Hotkey conflict detected. Change hotkey configuration.");
        return;
    }
    if (saveConfig(m_cfg)) {
        sendLog("[^] Configuration updated in config.ini");
    } else {
        sendLog("[!] Error: Cannot write config.ini!");
    }
}

/* Безопасный запуск графического редактора на уровне C++ */
void CrosshairManager::openEditorApp() {
    emit logMessage("[*] Requesting cppcrosshair-editor launching...");
    
    bool success = QProcess::startDetached("cppcrosshair-editor", QStringList());
    
    if (success) {
        emit logMessage("[^] Editor instance started successfully.");
    } else {
        emit logMessage("[!] Error: Failed to start cppcrosshair-editor.");
    }
}
