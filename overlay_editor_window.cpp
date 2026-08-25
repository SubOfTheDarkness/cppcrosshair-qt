#include "overlay_editor_window.h"
#include "ui_overlay_editor_window.h"
#include "config_parser.h"
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QRegularExpression>
#include <QSysInfo>
#include <sys/types.h>
#include <signal.h>
#include "icon.xpm"

OverlayEditorWindow::OverlayEditorWindow(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::overlay_editor_window)
{
    ui->setupUi(this);
    overlayProcess = new QProcess(this);
    this->setWindowIcon(QIcon(QPixmap(app_icon_xpm)));

    ui->verticalLayout_2->setAlignment(ui->pixel_canvas, Qt::AlignCenter);

    ui->xoffset_spin->setRange(-2048, 2048);
    ui->yoffset_spin->setRange(-2048, 2048);

    Config cfg = loadConfig();
    ui->xoffset_spin->setValue(cfg.offset_x);
    ui->yoffset_spin->setValue(cfg.offset_y);

    setupPalette();
    initConnections();
    loadXpmFile();
    checkProcess();
}

OverlayEditorWindow::~OverlayEditorWindow() {
    if (overlayProcess) {
        if (overlayProcess->state() == QProcess::Running) {
            overlayProcess->disconnect();
            
            overlayProcess->closeReadChannel(QProcess::StandardOutput);
            overlayProcess->closeReadChannel(QProcess::StandardError);
            
            overlayProcess->setParent(nullptr); 
        }
    }
    delete ui;
}


void OverlayEditorWindow::setupPalette() {
    struct PaletteItem { QPushButton* btn; QString hexColor; };
    QList<PaletteItem> palette = {
        {ui->palette_red_btn, "#FF0000"}, {ui->palette_blue_btn, "#0000FF"},
        {ui->palette_green_btn, "#00FF00"}, {ui->palette_lblue_btn, "#00CED1"},
        {ui->palette_dgreen_btn, "#006400"}, {ui->palette_yellow_btn, "#FFFF00"},
        {ui->palette_white_btn, "#FFFFFF"}, {ui->palette_magenta_btn, "#FF00FF"},
        {ui->palette_black_btn, "#000000"}, {ui->palette_orange_btn, "#FF8C00"}
    };

    QString baseStyle = "QPushButton { background-color: %1; border: 1px solid #7f8c8d; border-radius: 4px; min-width: 24px; max-width: 24px; min-height: 24px; max-height: 24px; } QPushButton:hover { border: 2px solid #2c3e50; }";

    for (const auto& item : palette) {
        item.btn->setText("");
        item.btn->setStyleSheet(baseStyle.arg(item.hexColor));
        connect(item.btn, &QPushButton::clicked, this, [this, item]() {
            ui->pixel_canvas->setBrushColor(QColor(item.hexColor));
            
            ui->pixel_canvas->setCursor(Qt::CrossCursor);
        });
    }
    ui->palette_eraser_btn->setStyleSheet("QPushButton { font-weight: bold; min-height: 24px; padding: 0px 10px; }");
}

void OverlayEditorWindow::initConnections() {
    connect(ui->palette_eraser_btn, &QPushButton::clicked, this, [this]() { 
        ui->pixel_canvas->setEraserMode(true); 
        ui->pixel_canvas->setCursor(Qt::PointingHandCursor);
    });

    connect(ui->editor_save_btn, &QPushButton::clicked, this, &OverlayEditorWindow::saveXpmFile);
    connect(ui->editor_cancel_btn, &QPushButton::clicked, this, &OverlayEditorWindow::cancelChanges);
    connect(ui->editor_clear_btn, &QPushButton::clicked, ui->pixel_canvas, &PixelCanvas::clearCanvas);

    connect(ui->ctrl_toggle_btn, &QPushButton::clicked, this, &OverlayEditorWindow::toggleOverlayProcess);
    connect(overlayProcess, &QProcess::readyReadStandardOutput, this, &OverlayEditorWindow::readOverlayOutput);
    connect(overlayProcess, &QProcess::readyReadStandardError, this, &OverlayEditorWindow::readOverlayOutput);
    connect(overlayProcess, qOverload<int, QProcess::ExitStatus>(&QProcess::finished), this, &OverlayEditorWindow::handleOverlayFinished);

    connect(ui->sett_about_btn, &QPushButton::clicked, this, &OverlayEditorWindow::showAboutDialog);
}


void OverlayEditorWindow::cancelChanges() {
    Config cfg = loadConfig();
    ui->xoffset_spin->setValue(cfg.offset_x);
    ui->yoffset_spin->setValue(cfg.offset_y);

    loadXpmFile();
}

void OverlayEditorWindow::saveXpmFile() {
    std::string dir = getConfigDir();
    std::string xpmPath = dir + "/crosshair.xpm";

    mkdir(dir.c_str(), 0755);

    QFile file(QString::fromStdString(xpmPath));
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "Error", "Cannot create crosshair.xpm file!");
        return;
    }

    QTextStream out(&file);
    out << "/* XPM */\nstatic char * crosshair_xpm[] = {\n\"32 32 11 1\",\n";
    out << "\"_ c None\",\n\"R c #FF0000\",\n\"B c #0000FF\",\n\"G c #00FF00\",\n\"L c #00CED1\",\n\"D c #006400\",\n\"Y c #FFFF00\",\n\"W c #FFFFFF\",\n\"M c #FF00FF\",\n\"K c #000000\",\n\"O c #FF8C00\",\n";

    const auto& gridData = ui->pixel_canvas->getGridData();
    for (int r = 0; r < 32; ++r) {
        out << "\"";
        for (int c = 0; c < 32; ++c) {
            if (gridData[r][c].isValid()) {
                QString hex = gridData[r][c].name().toUpper();
                if (hex == "#FF0000") out << "R";
                else if (hex == "#0000FF") out << "B";
                else if (hex == "#00FF00") out << "G";
                else if (hex == "#00CED1") out << "L";
                else if (hex == "#006400") out << "D";
                else if (hex == "#FFFF00") out << "Y";
                else if (hex == "#FFFFFF") out << "W";
                else if (hex == "#FF00FF") out << "M";
                else if (hex == "#000000") out << "K";
                else if (hex == "#FF8C00") out << "O";
                else out << "_";
            } else {
                out << "_";
            }
        }
        out << "\",\n";
    }
    out << "};";
    file.close();

    Config cfg = loadConfig();
    cfg.offset_x = ui->xoffset_spin->value();
    cfg.offset_y = ui->yoffset_spin->value();

    if (!saveConfig(cfg)) {
        QMessageBox::warning(this, "Warning", "Crosshair saved, but config.ini could not be updated!");
        return;
    }
    QMessageBox::information(this, "Success", "Crosshair successfully saved!");
}


void OverlayEditorWindow::loadXpmFile() {
    ui->pixel_canvas->clearCanvas();

    Config cfg = loadConfig();
    QFile file(QString::fromStdString(cfg.xpm_path));
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return;
    }

    QTextStream in(&file);
    QString content = in.readAll();
    file.close();

    QMap<QChar, QColor> colorMap;
    colorMap['R'] = QColor("#FF0000"); colorMap['B'] = QColor("#0000FF");
    colorMap['G'] = QColor("#00FF00"); colorMap['L'] = QColor("#00CED1");
    colorMap['D'] = QColor("#006400"); colorMap['Y'] = QColor("#FFFF00");
    colorMap['W'] = QColor("#FFFFFF"); colorMap['M'] = QColor("#FF00FF");
    colorMap['K'] = QColor("#000000"); colorMap['O'] = QColor("#FF8C00");

    QRegularExpression lineRegex("\"([R_BGLDYWMKO]{32})\"");
    QRegularExpressionMatchIterator it = lineRegex.globalMatch(content);
    
    int currentRow = 0;
    while (it.hasNext() && currentRow < 32) {
        QRegularExpressionMatch match = it.next();
        QString pixelLine = match.captured(1);
        for (int col = 0; col < 32; ++col) {
            QChar pixelChar = pixelLine.at(col);
            if (colorMap.contains(pixelChar)) {
                ui->pixel_canvas->setPixelColor(currentRow, col, colorMap[pixelChar]);
            }
        }
        currentRow++;
    }
    ui->pixel_canvas->refresh();
}

void OverlayEditorWindow::checkProcess(){
    QProcess checkProc;
    checkProc.start("pgrep", QStringList() << "-f" << "crosshair_overlay");
    if (checkProc.waitForFinished(500)) {
        QString output = QString::fromUtf8(checkProc.readAllStandardOutput()).trimmed();
        if (!output.isEmpty()) {
            ui->ctrl_log_text->append("[*] Detected active crosshair_overlay running in the background.");
            ui->ctrl_log_text->append("[*] Editor attached to the existing instance.");
            
            ui->ctrl_toggle_btn->setText("Stop");
            ui->ctrl_toggle_btn->setStyleSheet("background-color: #dc3545; color: white; font-weight: bold; border-radius: 4px; padding: 6px 15px;");
        }
    }
}

void OverlayEditorWindow::toggleOverlayProcess() {
    if (overlayProcess->state() == QProcess::Running || ui->ctrl_toggle_btn->text() == "Stop") {
        ui->ctrl_log_text->append("[*] Stopping overlay instance...");
        
        if (overlayProcess->state() == QProcess::Running) {
            overlayProcess->kill();
            overlayProcess->waitForFinished(500);
        } else {
            std::ifstream pid_file("/tmp/crosshair_overlay.pid");
            if (pid_file.is_open()) {
                pid_t target_pid;
                if (pid_file >> target_pid) {
                    kill(target_pid, SIGTERM);
                }
                pid_file.close();
            }
        }

        ui->ctrl_toggle_btn->setText("Start");
        ui->ctrl_toggle_btn->setStyleSheet("background-color: #28a745; color: white; font-weight: bold; border-radius: 4px; padding: 6px 15px;");
    } 
    else {
        ui->ctrl_log_text->clear();
        ui->ctrl_log_text->append("[*] Launching crosshair_overlay with console streaming...");

        QString binaryName = QFile::exists("./crosshair_overlay") ? "./crosshair_overlay" : "crosshair_overlay";
        
        overlayProcess->start(binaryName, QStringList());
        
        if (!overlayProcess->waitForStarted(1000)) {
            ui->ctrl_log_text->append("[!] Error: Could not execute binary!");
            return;
        }
        
        ui->ctrl_toggle_btn->setText("Stop");
        ui->ctrl_toggle_btn->setStyleSheet("background-color: #dc3545; color: white; font-weight: bold; border-radius: 4px; padding: 6px 15px;");
    }
}

void OverlayEditorWindow::readOverlayOutput() {
    QByteArray stdOut = overlayProcess->readAllStandardOutput();
    QByteArray stdErr = overlayProcess->readAllStandardError();
    if (!stdOut.isEmpty()) ui->ctrl_log_text->append(QString::fromUtf8(stdOut).trimmed());
    if (!stdErr.isEmpty()) ui->ctrl_log_text->append("[Process Error] " + QString::fromUtf8(stdErr).trimmed());
}

void OverlayEditorWindow::handleOverlayFinished(int exitCode) {
    ui->ctrl_log_text->append(QString("\n[-] Process finished. Exit Code: %1").arg(exitCode));
    ui->ctrl_toggle_btn->setText("Start");
    ui->ctrl_toggle_btn->setStyleSheet("background-color: #28a745; color: white; font-weight: bold; border-radius: 4px; padding: 6px 15px;");
}

void OverlayEditorWindow::showAboutDialog() {
    QDialog *aboutDialog = new QDialog(this);
    aboutDialog->setWindowTitle("system_info --about");
    aboutDialog->setMinimumSize(540, 390);

    QVBoxLayout *layout = new QVBoxLayout(aboutDialog);
    layout->setContentsMargins(10, 10, 10, 10);

    QTextEdit *txtAbout = new QTextEdit(aboutDialog);
    txtAbout->setReadOnly(true);
    
    txtAbout->setStyleSheet(
        "QTextEdit {"
        "    background-color: #0c0c0c;"
        "    border: 1px solid #222222;"
        "    font-family: 'Source Code Pro', 'Fira Code', 'Courier New', monospace;"
        "    font-size: 12px;"
        "}"
    );

    QString systemUser = qgetenv("USER");
    if (systemUser.isEmpty()) systemUser = "user";
    
    QString systemHost = QSysInfo::machineHostName();
    if (systemHost.isEmpty()) systemHost = "linux";

    QString paleRed   = "#ff7675";
    QString white     = "#ffffff";
    QString brightRed = "#ff003c";
    QString softYellow= "#f1c40f";

    QString promptTop = QString("<span style='color: %1;'>╭─</span>"
                                "<span style='color: %2;'>%3</span>"
                                "<span style='color: %4;'>@</span>"
                                "<span style='color: %2;'>%5</span> "
                                "<span style='color: %4;'>in</span> "
                                "<span style='color: %4;'>~</span> "
                                "<span style='color: %4;'>took</span> "
                                "<span style='color: %6;'>0s</span>")
                        .arg(paleRed, brightRed, systemUser, white, systemHost, softYellow);

    QString promptBottom = QString("<span style='color: %1;'>╰─λ</span>").arg(paleRed);

QString globalAppName = QCoreApplication::applicationName();
    QString globalVersion = QCoreApplication::applicationVersion();

    QString htmlContent = QString(
        "%1<br>"
        "%2 ./crosshair_editor --version<br><br>"
        "--------------------------------------------------<br>"
        "<span style='color: #00FF66;'>▶ APPLICATION:</span> %3<br>"
        "<span style='color: #00FF66;'>▶ VERSION:    </span> v%4<br>"
        "<span style='color: #00FF66;'>▶ DEVELOPER:  </span> SubOfTheDarkness<br>"
        "<span style='color: #00FF66;'>▶ COMPONENT:  </span> Standalone X11 Overlay &amp; Editor<br>"
        "--------------------------------------------------<br><br>"
        "<span style='color: #E6DB74;'>[LICENSE NOTICE]</span><br>"
        "<span style='color: #888888; font-size: 11px;'>"
        "This program is free software: you can redistribute it and/or modify it "
        "under the terms of the GNU General Public License as published by the Free Software "
        "Foundation, either version 3 of the License, or (at your option) any later version.<br><br>"
        "This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; "
        "without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. "
        "See the GNU General Public License for more details.</span><br><br>"
        "%1<br>"
        "%2 <span style='color: #ffffff; background-color: #ffffff;'>&nbsp;</span>"
    ).arg(promptTop, promptBottom, globalAppName, globalVersion);

    txtAbout->setHtml(htmlContent);
    layout->addWidget(txtAbout);

    QPushButton *btnClose = new QPushButton("exit", aboutDialog);
    btnClose->setStyleSheet(
        "QPushButton {"
        "    background-color: #1e1e1e;"
        "    color: #ff7675;"
        "    border: 1px solid #333333;"
        "    font-family: monospace;"
        "    padding: 5px 15px;"
        "}"
        "QPushButton:hover {"
        "    background-color: #2a2a2a;"
        "    border-color: #ff003c;"
        "    color: #ffffff;"
        "}"
    );
    connect(btnClose, &QPushButton::clicked, aboutDialog, &QDialog::accept);
    
    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->addItem(new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum));
    btnLayout->addWidget(btnClose);
    layout->addLayout(btnLayout);

    aboutDialog->setAttribute(Qt::WA_DeleteOnClose);
    aboutDialog->exec();
}