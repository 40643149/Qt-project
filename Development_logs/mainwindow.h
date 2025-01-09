#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QCalendarWidget>
#include <QFileDialog>
#include <QPixmap>
#include <QListWidgetItem>
#include <QStringList>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_searchButton_clicked(); // 搜索按鈕槽函式
    void on_calendarButton_clicked(); // 顯示日曆按鈕槽函式
    void on_addImageButton_clicked(); // 添加圖片按鈕槽函式
    void on_saveLogButton_clicked(); // 保存日誌按鈕槽函式
    void loadAllLogs(); //讓logListWidget_2預設顯示所有檔案
    void handleCalendarDateSelected(const QDate &date); // 處理日曆選擇日期的槽函式
    void on_logListWidget_itemClicked(QListWidgetItem *item);    // 點擊檔案列表的槽函式
    void searchByMonth(const QString &text);//按月份篩選
    //void searchByYear(const QString &text);


private:
    Ui::MainWindow *ui;
    QString selectedImagePath; // 儲存選擇的圖片路徑
    QString selectedDate; // 儲存選擇的日期
    QStringList logFiles; // 儲存當前顯示的日誌檔案清單
    QCalendarWidget *calendarWidget; // 日曆元件

    void searchLogs(const QString &keyword); // 搜尋日誌
    void saveLog(); // 儲存日誌
    void updateLogList(const QStringList &files); // 更新檔案列表顯示
    void loadLogsForDate(const QString &date); // 載入特定日期的日誌
    void updateImagePreview(const QString &imagePath);
};

#endif // MAINWINDOW_H
