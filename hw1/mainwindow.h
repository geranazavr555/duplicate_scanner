#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <memory>
#include <QThread>
#include <QTime>
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

private:
    void make_connections();
    void make_ui();

private slots:
    void open_directory();

public slots:
    void scan_finished();
    void recieve_same_files_group(DuplicateScanner::file_size_type, DuplicateScanner::bucket_type const&);
    void set_steps_count(int);
    void set_current_step(int);

signals:
    void start_scan();
    void stop_scan();
    void set_directory_to_scan(QString const&);

private:
    Ui::MainWindow *ui;
    QThread *worker_thread;
    Worker *worker;
    QTime timer;
};

#endif // MAINWINDOW_H
