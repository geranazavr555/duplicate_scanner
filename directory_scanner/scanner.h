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
#include <string>

class DuplicateScanner : public QObject
{
    Q_OBJECT

public:
    typedef std::vector<QString> bucket_type;
    typedef qint64 file_size_type;

    typedef QByteArray hash_type;

private:
    struct bucket_info_type
    {
        file_size_type files_size;
        hash_type hash;

        bucket_info_type(file_size_type files_size, hash_type hash) : files_size(files_size), hash(hash) {}
        bool operator==(bucket_info_type const& other) const
        {
            return files_size == other.files_size && hash == other.hash;
        }
    };

    struct bucket_info_hash : public std::unary_function<bucket_info_type, size_t>
    {
        size_t operator() (bucket_info_type const& obj) const
        {
            return (static_cast<size_t>(qHash(obj.files_size)) << 1) ^
                    static_cast<size_t>(qHash(obj.hash));
        }
    };

public:
    DuplicateScanner(QString directory);
    void scan();

private:
    void fill_buckets();
    void preprocess_file(QString path);
    void process_bucket(bucket_info_type const&, bucket_type const &);

    static hash_type hash(QString const& path);

signals:
    void bucket_ready(file_size_type, bucket_type const&);
    void finished();
    void set_steps_count(int);
    void set_current_step(int);

public slots:
    void start();
    void stop();

private:
    std::unique_ptr<FileComparator> comparator;
    QString directory;
    std::unordered_map<bucket_info_type, bucket_type, bucket_info_hash> buckets;
    bool stop_required = false;
};

#endif //HW1_SCANNER_H
