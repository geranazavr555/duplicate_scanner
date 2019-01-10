#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "directory_scanner/worker.h"
#include <QFileDialog>
#include <QDebug>
#include <QtWidgets/QCommonStyle>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    worker_thread(new QThread(this)),
    worker(new Worker()),
    timer()
{
    qRegisterMetaType<DuplicateScanner::bucket_type>("DuplicateScanner::bucket_type");
    qRegisterMetaType<DuplicateScanner::file_size_type>("DuplicateScanner::file_size_type");

    make_ui();
    make_connections();

    worker->moveToThread(worker_thread);
    worker_thread->start();
}

MainWindow::~MainWindow()
{
    emit stop_scan();
    worker_thread->quit();
    worker_thread->wait();
    delete worker;
    delete worker_thread;
    delete ui;
}

void MainWindow::make_ui()
{
    ui->setupUi(this);
    setWindowTitle("Duplicate scanner");

    QCommonStyle style;
    ui->actionOpenDirectory->setIcon(style.standardIcon(QCommonStyle::SP_DialogOpenButton));
    ui->actionStop->setIcon(style.standardIcon(QCommonStyle::SP_DialogDiscardButton));
    ui->actionDeleteFile->setIcon(style.standardIcon(QCommonStyle::SP_DialogCloseButton));

    ui->treeWidget->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->treeWidget->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);

    ui->progressBar->setMinimum(0);
}

void MainWindow::make_connections()
{
    connect(ui->actionOpenDirectory, SIGNAL(triggered()), this, SLOT(open_directory()));
    connect(ui->actionStop, SIGNAL(triggered()), worker, SLOT(stop()), Qt::DirectConnection);

    connect(this, &MainWindow::start_scan, worker, &Worker::start);
    connect(this, &MainWindow::stop_scan, worker, &Worker::stop, Qt::DirectConnection);
    connect(this, &MainWindow::set_directory_to_scan, worker, &Worker::set_target_directory);
    connect(worker, &Worker::finished, this, &MainWindow::scan_finished);
    connect(worker, &Worker::bucket_ready, this, &MainWindow::recieve_same_files_group);
}


void MainWindow::open_directory()
{
    QString dir = QFileDialog::getExistingDirectory(this, "Select directory to scan", QDir::currentPath());

    qDebug() << dir;

    setWindowTitle("Duplicate scanner: " + dir);
    ui->statusBar->showMessage("Scanning...");
    ui->treeWidget->clear();
    ui->progressBar->reset();
    timer.restart();

    emit set_directory_to_scan(dir);
    emit start_scan();
}

void MainWindow::scan_finished()
{
    ui->statusBar->showMessage("Scanning finished in " + QString::number(timer.elapsed()) + " ms");
}

void MainWindow::recieve_same_files_group(DuplicateScanner::file_size_type single_file_size,
        DuplicateScanner::bucket_type const & files)
{
    auto group_root_item = new QTreeWidgetItem(ui->treeWidget);
    group_root_item->setText(0, "Group size" + QString::number(files.size()));
    group_root_item->setText(1, QString::number(single_file_size * files.size()) + " bytes summary");

    for (auto const& path : files)
    {
        auto item = new QTreeWidgetItem(group_root_item);
        item->setText(0, path);
        item->setText(1, QString::number(single_file_size));
    }
    ui->treeWidget->addTopLevelItem(group_root_item);
}

void MainWindow::set_steps_count(int count)
{
    ui->progressBar->setMaximum(count);
}

void MainWindow::set_current_step(int step)
{
    ui->progressBar->setValue(step);
}
