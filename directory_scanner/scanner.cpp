#include "scanner.h"
#include "exceptions.h"

#include <QDirIterator>
#include <QDebug>
#include <QDir>
#include <QThread>

DuplicateScanner::DuplicateScanner(QString directory):
    comparator(nullptr),
    directory(std::move(directory))
{}

void DuplicateScanner::scan()
{
    qInfo() << "Scanning: " << directory;

    count_files();
    fill_buckets();
    if (stop_required)
    {
        emit finished();
        return;
    }

    emit set_steps_count(0);
    emit set_current_step(0);

    if (!comparator)
        comparator = std::make_unique<FileComparator>();

    for (auto const& info_bucket_pair : buckets)
    {
        process_bucket(info_bucket_pair.first, info_bucket_pair.second);

        if (stop_required)
            break;
    }

    qInfo() << "Finished";
    emit finished();
}

void DuplicateScanner::preprocess_file(QString path)
{
    //qDebug() << path;
    QFile file(path);
    buckets[bucket_info_type(file.size(), hash(path))].push_back(path);
}

void DuplicateScanner::fill_buckets()
{
    int step = 0;
    QDirIterator it(QString(directory), QDirIterator::Subdirectories | QDirIterator::FollowSymlinks);
    while (it.hasNext())
    {
        QString s = it.next();
        try
        {
            QFileInfo info(s);
            if (info.isFile())
            {
                try
                {
                    if (!info.isSymLink())
                        preprocess_file(s);
                }
                catch (FilesystemException const &err)
                {
                    qInfo() << err.what();
                }
                ++step;
                emit set_current_step(step);
            }
        }
        catch (std::exception const& err)
        {
            qWarning() << err.what();
        }

        if (stop_required)
            return;
    }
}

void DuplicateScanner::process_bucket(bucket_info_type const &bucket_info,
                                      DuplicateScanner::bucket_type const &bucket)
{
    std::vector<bucket_type> result;
    size_t errors = 0;

    //qDebug() << "processing bucket: " << bucket_info.files_size;

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
            emit bucket_ready(bucket_info.files_size, result_bucket);

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

DuplicateScanner::hash_type DuplicateScanner::hash(QString const &path)
{
    const size_t MAXLEN = 256;
    hash_type data(MAXLEN, 0);
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly))
        throw UnableToOpenFileException(path.toStdString());
    if (file.read(data.data(), MAXLEN) == -1)
        throw FilesystemException(path.toStdString());
    return data;
}

int DuplicateScanner::count_files()
{
    int result = 0, errors = 0;
    QDirIterator it(QString(directory), QDirIterator::Subdirectories | QDirIterator::FollowSymlinks);
    while (it.hasNext())
    {
        QString s = it.next();
        try
        {
            QFileInfo info(s);
            if (info.isFile())
                ++result;
        }
        catch (std::exception const& err)
        {
            qWarning() << err.what();
            ++errors;
        }

        if (stop_required)
            return result;
    }
    emit set_steps_count(result);
    qDebug() << "Files: " << result;
    return result;
}
