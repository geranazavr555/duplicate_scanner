#ifndef HW1_WORKER_H
#define HW1_WORKER_H

#include "scanner.h"
#include "file_comparator.h"

#include <memory>
#include <QThread>

class Worker : public QObject
{
    Q_OBJECT
public:
    Worker();
    Worker(QString const& target_directory);

public slots:
    void start();
    void stop();
    void set_target_directory(QString const& target_directory);

private slots:
    void scaning_finished();
    void bucket_ready_slot(DuplicateScanner::bucket_file_size_type, DuplicateScanner::bucket_type const&);

signals:
    void stop_signal();
    void started();
    void finished();
    void bucket_ready(DuplicateScanner::bucket_file_size_type, DuplicateScanner::bucket_type const&);

private:
    std::unique_ptr<DuplicateScanner> scanner;

};

#endif //HW1_WORKER_H
