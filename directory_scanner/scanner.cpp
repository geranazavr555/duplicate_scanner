#include "scanner.h"
#include "exceptions.h"

#include <QDirIterator>
#include <QDebug>
#include <QDir>

DuplicateScanner::DuplicateScanner(QString directory):
    comparator(nullptr),
    directory(std::move(directory))
{}

void DuplicateScanner::scan()
{
    qInfo() << "Scanning: " << directory;

    fill_buckets();
    if (stop_required)
    {
        emit finished();
        return;
    }

    if (!comparator)
        comparator = std::make_unique<FileComparator>();

    for (auto const& size_bucket_pair : buckets)
    {
        process_bucket(size_bucket_pair.first, size_bucket_pair.second);

        if (stop_required)
        {
            emit finished();
            return;
        }
    }
}

void DuplicateScanner::preprocess_file(QString path)
{
    qDebug() << path;
    QFile file(path);
    buckets[file.size()].push_back(path);
}

void DuplicateScanner::fill_buckets()
{
    QDirIterator it(QString(directory), QDirIterator::Subdirectories | QDirIterator::FollowSymlinks);
    while (it.hasNext())
    {
        QString s = it.next();
        QFileInfo info(s);
        if (info.isFile())
            preprocess_file(s);

        if (stop_required)
            return;
    }
}

void DuplicateScanner::process_bucket(DuplicateScanner::bucket_file_size_type files_size,
        const DuplicateScanner::bucket_type &bucket)
{
    std::vector<bucket_type> result;
    size_t errors = 0;

    for (auto const& path : bucket)
    {
        bool unique = true;
        for (auto& result_bucket : result)
        {
            try
            {
                if (comparator->compare(path, result_bucket.back(), &stop_required)) // or front() ?
                {
                    result_bucket.push_back(path);
                    unique = false;
                    break;
                }
            }
            catch (FilesystemException const& err)
            {
                ++errors;
                qWarning() << err.what();
                if (err.get_path() != path.toStdString())
                {
                    qCritical() << "Files have been changed during procesing. Results may be incorrect.";
                    continue;
                }
                else
                {
                    unique = false;
                    break;
                }
            }
            catch (std::exception const& err)
            {
                ++errors;
                qWarning() << err.what();
            }

            if (stop_required)
                return;
        }
        if (unique)
        {
            result.emplace_back(1, path);
        }
    }

    for (auto const& result_bucket : result)
    {
        if (result_bucket.size() > 1)
            emit bucket_ready(files_size, result_bucket);

        if (stop_required)
            return;
    }
}

void DuplicateScanner::start()
{
    scan();
}

void DuplicateScanner::stop()
{
    stop_required = true;
}
