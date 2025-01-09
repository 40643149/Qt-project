#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QDir>
#include <QTextStream>
#include <QDebug>
#include <QDirIterator>

extern QString path = "C:\\";

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , calendarWidget(nullptr)
{
    ui->setupUi(this);
    loadAllLogs();

    selectedDate = QDate::currentDate().toString("yyyy-MM-dd");
    ui->MMComboBox->setCurrentText(QDate::currentDate().toString("yyyy-MM"));

    // 只保留月份下拉選單的連接
    connect(ui->MMComboBox, QOverload<const QString &>::of(&QComboBox::currentTextChanged),
            this, &MainWindow::searchByMonth);

    connect(ui->logListWidget_2, &QListWidget::itemClicked, this, &MainWindow::on_logListWidget_itemClicked);

    ui->imageLabel->setText("圖片預覽");
    ui->imageLabel->setAlignment(Qt::AlignCenter);
}

MainWindow::~MainWindow() {
    delete ui;
    delete calendarWidget;
}

void MainWindow::on_searchButton_clicked() {
    QString keyword = ui->searchLineEdit->text().trimmed();
    if (keyword.isEmpty()) {
        QMessageBox::warning(this, "警告", "請輸入關鍵字");
        return;
    }

    QStringList keywordParts = keyword.split(QRegularExpression("[\\s\\-]"), Qt::SkipEmptyParts);
    QString logDirPath = path + "logs";
    QDir logDir(logDirPath);
    QStringList matchedFiles;

    if (logDir.exists()) {
        QDirIterator it(logDirPath, QStringList() << "*.txt", QDir::Files, QDirIterator::Subdirectories);
        while (it.hasNext()) {
            QString filePath = it.next();
            QStringList fileParts = QFileInfo(filePath).baseName().split(QRegularExpression("[\\s\\-]"), Qt::SkipEmptyParts);
            if (std::all_of(keywordParts.begin(), keywordParts.end(), [&fileParts](const QString &part) { return fileParts.contains(part, Qt::CaseInsensitive); })) {
                matchedFiles.append(filePath);
            }
        }
    }
    updateLogList(matchedFiles);
}

void MainWindow::on_calendarButton_clicked() {
    if (!calendarWidget) {
        calendarWidget = new QCalendarWidget(this);
        calendarWidget->setWindowFlags(Qt::Tool);
        calendarWidget->setGeometry(this->geometry().x() + 50, this->geometry().y() + 50, 300, 300);
        connect(calendarWidget, &QCalendarWidget::clicked, this, &MainWindow::handleCalendarDateSelected);
    }
    calendarWidget->show();
}

void MainWindow::handleCalendarDateSelected(const QDate &date) {
    selectedDate = date.toString("yyyy-MM-dd");
    ui->MMComboBox->setCurrentText(date.toString("yyyy-MM"));
    loadLogsForDate(selectedDate);
    calendarWidget->hide();
}

void MainWindow::on_addImageButton_clicked() {
    QString filePath = QFileDialog::getOpenFileName(this, "選擇圖片", "", "Images (*.png *.jpg *.jpeg *.bmp)");
    if (!filePath.isEmpty()) {
        selectedImagePath = filePath;
        QPixmap pixmap(filePath);
        ui->imageLabel->setPixmap(pixmap.scaled(ui->imageLabel->size(), Qt::KeepAspectRatio));
    }
}

void MainWindow::on_saveLogButton_clicked() {
    if (selectedDate.isEmpty()) {
        QMessageBox::warning(this, "警告", "請選擇日期");
        return;
    }

    QString logDirPath = path + "logs/" + selectedDate;
    QDir().mkpath(logDirPath);

    QString logText = ui->logTextEdit->toPlainText();
    QString logFilePath = logDirPath + "/" + QDate::fromString(selectedDate, "yyyy-MM-dd").toString("yyyy-MM-dd") + ".txt";
    QFile logFile(logFilePath);
    if (logFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&logFile);
        out << logText;
        logFile.close();
    }

    if (!selectedImagePath.isEmpty()) {
        QString imageFilePath = logDirPath + "/" + QFileInfo(selectedImagePath).fileName();
        QFile::copy(selectedImagePath, imageFilePath);
    }

    QMessageBox::information(this, "成功", "日誌已儲存");
}

void MainWindow::searchByMonth(const QString &text) {
    QString year = ui->YYComboBox->currentText();
    QString month = QString::number(ui->MMComboBox->currentIndex() + 1).rightJustified(2, '0');
    QString yearMonth = year + "-" + month;
    QString logDirPath = path + "logs";
    QDir logDir(logDirPath);
    QStringList matchedFiles;

    if (logDir.exists()) {
        QDirIterator it(logDirPath, QStringList() << "*.txt", QDir::Files, QDirIterator::Subdirectories);
        while (it.hasNext()) {
            QString filePath = it.next();
            if (filePath.contains(yearMonth)) {
                matchedFiles.append(filePath);
            }
        }
    }

    updateLogList(matchedFiles);
}


void MainWindow::updateLogList(const QStringList &files) {
    ui->logListWidget_2->clear();
    for (const QString &filePath : files) {
        ui->logListWidget_2->addItem(QFileInfo(filePath).fileName());
    }
}

void MainWindow::loadLogsForDate(const QString &date) {
    QString logDirPath = path + "logs/" + date;
    QDir logDir(logDirPath);

    if (!logDir.exists()) {
        ui->logTextEdit->clear();
        ui->imageLabel->setText("圖片預覽");
        ui->logListWidget_2->clear();
        return;
    }

    ui->logListWidget_2->clear();
    QStringList logFiles = logDir.entryList({"*.txt"}, QDir::Files);
    for (const QString &file : logFiles) {
        ui->logListWidget_2->addItem(file);
    }

    if (!logFiles.isEmpty()) {
        QFile logFile(logDirPath + "/" + logFiles.first());
        if (logFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
            ui->logTextEdit->setPlainText(QTextStream(&logFile).readAll());
            logFile.close();
        }
    }

    QStringList imageFiles = logDir.entryList({"*.png", "*.jpg", "*.jpeg", "*.bmp"}, QDir::Files);
    if (!imageFiles.isEmpty()) {
        QPixmap pixmap(logDirPath + "/" + imageFiles.first());
        ui->imageLabel->setPixmap(pixmap.scaled(ui->imageLabel->size(), Qt::KeepAspectRatio));
    } else {
        ui->imageLabel->setText("圖片預覽");
    }
}

void MainWindow::on_logListWidget_itemClicked(QListWidgetItem *item) {
    QString filename = item->text();
    QString logDirPath = path + "logs";
    QString fullPath;

    QDirIterator it(logDirPath, QDir::Files | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        QString currentPath = it.next();
        if (currentPath.endsWith(filename)) {
            fullPath = currentPath;
            break;
        }
    }

    if (!fullPath.isEmpty()) {
        QFile logFile(fullPath);
        if (logFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
            ui->logTextEdit->setPlainText(QTextStream(&logFile).readAll());
            logFile.close();
        }

        QString imageDirPath = QFileInfo(fullPath).path();
        QStringList imageFiles = QDir(imageDirPath).entryList({"*.png", "*.jpg", "*.jpeg"}, QDir::Files);
        if (!imageFiles.isEmpty()) {
            QPixmap pixmap(imageDirPath + "/" + imageFiles.first());
            ui->imageLabel->setPixmap(pixmap.scaled(ui->imageLabel->size(), Qt::KeepAspectRatio));
        } else {
            ui->imageLabel->setText("圖片預覽");
        }
    }
}

void MainWindow::loadAllLogs() {
    QDirIterator it(path + "logs", {"*.txt"}, QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        ui->logListWidget_2->addItem(QFileInfo(it.next()).fileName());
    }
}
