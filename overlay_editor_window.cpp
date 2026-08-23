#include "overlay_editor_window.h"
#include "ui_overlay_editor_window.h"
#include "config_parser.h"
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QRegularExpression>

OverlayEditorWindow::OverlayEditorWindow(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::overlay_editor_window)
{
    ui->setupUi(this);
    overlayProcess = new QProcess(this);

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
    if (overlayProcess && overlayProcess->state() == QProcess::Running) {
        overlayProcess->setParent(nullptr);
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
        QMessageBox::critical(this, "Error", "Cannot create crosshair.xpm file in user config directory!");
        return;
    }

    QTextStream out(&file);
    out << "/* XPM */\nstatic char * crosshair_xpm[] = {\n\"32 32 11 1\",\n";
    out << "\"  c None\",\n\"R c #FF0000\",\n\"B c #0000FF\",\n\"G c #00FF00\",\n\"L c #00CED1\",\n\"D c #006400\",\n\"Y c #FFFF00\",\n\"W c #FFFFFF\",\n\"M c #FF00FF\",\n\"K c #000000\",\n\"O c #FF8C00\",\n";

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
                else out << " ";
            } else {
                out << " ";
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

    QMessageBox::information(this, "Success", "Crosshair matrix and offsets successfully saved!");
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

    QRegularExpression lineRegex("\"([R BGLDYWMKO ]{32})\"");
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
        }
        
        QProcess cleanup;
        cleanup.start("pkill", QStringList() << "-f" << "crosshair_overlay");
        cleanup.waitForFinished(500);

        ui->ctrl_toggle_btn->setText("Start");
        ui->ctrl_toggle_btn->setStyleSheet("background-color: #28a745; color: white; font-weight: bold; border-radius: 4px; padding: 6px 15px;");
    } 
    else {
        ui->ctrl_log_text->clear();
        ui->ctrl_log_text->append("[*] Launching crosshair_overlay as a single independent instance...");

        overlayProcess->start("./crosshair_overlay", QStringList());
        
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
