#include "worker.h"

#include <QDebug>

Worker::Worker() : scanner(nullptr) {}

Worker::Worker(QString const& target_directory) : Worker()
{
    set_target_directory(target_directory);
}

void Worker::set_target_directory(QString const &target_directory)
{
    if (scanner) // TODO: Is that correct?
    {
        emit stop();
    }
    scanner = std::make_unique<DuplicateScanner>(target_directory);

    connect(this, &Worker::started, scanner.get(), &DuplicateScanner::start);
    connect(this, &Worker::stop_signal, scanner.get(), &DuplicateScanner::stop);
    connect(scanner.get(), &DuplicateScanner::finished, this, &Worker::finished);
    connect(scanner.get(), &DuplicateScanner::bucket_ready, this, &Worker::bucket_ready_slot);
}

void Worker::scaning_finished()
{
    emit finished();
}

void Worker::bucket_ready_slot(DuplicateScanner::file_size_type a, DuplicateScanner::bucket_type const& b)
{
    emit bucket_ready(a, b);
}

void Worker::start()
{
    emit started();
}

void Worker::stop()
{
    emit stop_signal();
}

