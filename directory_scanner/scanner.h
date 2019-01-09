#ifndef HW1_SCANNER_H
#define HW1_SCANNER_H

#include "file_comparator.h"

#include <unordered_map>
#include <vector>
#include <memory>
#include <functional>
#include <QString>
#include <QHash>
#include <QObject>

class DuplicateScanner : public QObject
{
    Q_OBJECT

private:
    struct hash : public std::unary_function<QString, size_t>
    {
        size_t operator() (QString const& obj) const
        {
            return qHash(obj);
        }
    };

public:
    typedef std::vector<QString> bucket_type;
    typedef qint64 bucket_file_size_type;

private:
    void fill_buckets();
    void preprocess_file(QString path);
    void process_bucket(bucket_file_size_type file_size, bucket_type const&);

signals:
    void bucket_ready(bucket_file_size_type, bucket_type const&);
    void finished();

public slots:
    void start();
    void stop();

public:
    DuplicateScanner(QString directory);
    void scan();

private:
    std::unique_ptr<FileComparator> comparator;
    QString directory;
    std::unordered_map<qint64, bucket_type> buckets;
    bool stop_required = false;
};

#endif //HW1_SCANNER_H
