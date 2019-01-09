#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <memory>
#include <QThread>
#include <directory_scanner/worker.h>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void open_directory();

public slots:
    void scan_finished();
    void recieve_same_files_group(DuplicateScanner::bucket_file_size_type, DuplicateScanner::bucket_type const&);

signals:
    void start_scan();
    void stop_scan();
    void set_directory_to_scan(QString const&);

private:
    Ui::MainWindow *ui;
    QThread *worker_thread;
    Worker *worker;
};

#endif // MAINWINDOW_H
