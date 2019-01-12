#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "directory_scanner/worker.h"
#include "FileItem.h"
#include <QFileDialog>
#include <QDebug>
#include <QCommonStyle>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    worker_thread(new QThread(this)),
    worker(new Worker()),
    timer(),
    ask_agreement_to_delete_file(true)
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

    ui->treeWidget->header()->setSectionResizeMode(0, QHeaderView::Interactive);
    ui->treeWidget->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    ui->treeWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->treeWidget->setSelectionBehavior(QAbstractItemView::SelectRows);

    ui->progressBar->hide();
}

void MainWindow::make_connections()
{
    connect(ui->actionOpenDirectory, SIGNAL(triggered()), this, SLOT(open_directory()));
    connect(ui->actionStop, SIGNAL(triggered()), worker, SLOT(stop()), Qt::DirectConnection);
    connect(ui->actionExit, SIGNAL(triggered()), this, SLOT(close()));
    connect(ui->actionAboutQt, SIGNAL(triggered()), this, SLOT(show_about_qt()));
    connect(ui->actionUsage, SIGNAL(triggered()), this, SLOT(show_help()));
    connect(ui->actionDeleteFile, SIGNAL(triggered()), this, SLOT(delete_selected()));

    connect(this, &MainWindow::start_scan, worker, &Worker::start);
    connect(this, &MainWindow::stop_scan, worker, &Worker::stop, Qt::DirectConnection);
    connect(this, &MainWindow::set_directory_to_scan, worker, &Worker::set_target_directory);
    connect(worker, &Worker::finished, this, &MainWindow::scan_finished);
    connect(worker, &Worker::bucket_ready, this, &MainWindow::recieve_same_files_group);
    connect(worker, &Worker::set_current_step, this, &MainWindow::set_current_step);
    connect(worker, &Worker::set_steps_count, this, &MainWindow::set_steps_count);
}


void MainWindow::open_directory()
{
    QString dir = QFileDialog::getExistingDirectory(this, "Select directory to scan", QDir::currentPath());

   // qDebug() << dir;

    if (!dir.isEmpty())
    {
        emit stop_scan();

        setWindowTitle("Duplicate scanner: " + dir);
        ui->statusBar->showMessage("Scanning...");
        ui->treeWidget->clear();
        ui->treeWidget->setSortingEnabled(false);
        ui->progressBar->setMaximum(0);
        ui->progressBar->reset();
        ui->progressBar->show();
        timer.restart();

        emit set_directory_to_scan(dir);
        emit start_scan();
    }
}

void MainWindow::scan_finished()
{
    ui->statusBar->showMessage("Scanning finished in " + QString::number(timer.elapsed()) + " ms");
    ui->progressBar->hide();
    ui->treeWidget->setSortingEnabled(true);
}

void MainWindow::recieve_same_files_group(DuplicateScanner::file_size_type single_file_size,
        DuplicateScanner::bucket_type const & files)
{
    auto group_root_item = new FileItem(ui->treeWidget);
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
    if (count > 0)
        ui->statusBar->showMessage("Scanning... Files detected: " + QString::number(count));
    else
        ui->statusBar->showMessage("Scanning...");
}

void MainWindow::set_current_step(int step)
{
    ui->progressBar->setValue(step);
}

void MainWindow::show_about_qt()
{
    QMessageBox::aboutQt(this);
}

void MainWindow::show_help()
{
    QMessageBox::about(this, "About duplicate scanner", "Usage: choose the directory to scan. "
                                                        "After scanning you will be able to choose files to delete."
                                                        "\n\n"
                                                        "Georgiy Nazarov, github.com/geranazavr555");
}

void MainWindow::delete_selected()
{
    int result = 0, errors = 0;
    auto selected = ui->treeWidget->selectedItems();
    std::vector<QTreeWidgetItem*> items_to_delete;

    for (auto const& item : selected)
    {
        if (item->childCount() == 0 && request_agreement_to_delete(item->text(0)))
        {
            QFile file(item->text(0));
            if (!file.remove())
                ++errors;
            else
            {
                ++result;
                items_to_delete.push_back(item);
            }
        }
    }

    for (auto * const item : items_to_delete)
    {
        auto parent = item->parent();
        parent->removeChild(item);
        delete item;
        if (parent->childCount() == 0)
        {
            parent->parent()->removeChild(parent);
            delete parent;
        }
    }

    ui->statusBar->showMessage("Deleted files: " + QString::number(result) + " Errors: " + QString::number(errors));
}

bool MainWindow::request_agreement_to_delete(QString const &path)
{
    if (!ask_agreement_to_delete_file)
        return true;

    auto answer = QMessageBox::warning(this, "Delete file?", "Delete " + path + " ?\n\n"
                                                                                "To prevent showing this dialog "
                                                                                "in future choose 'Yes to all'",
            QMessageBox::YesToAll |QMessageBox::Yes | QMessageBox::No);
    if (answer == QMessageBox::YesToAll)
        ask_agreement_to_delete_file = false;
    return answer == QMessageBox::YesToAll || answer == QMessageBox::Yes;
}
