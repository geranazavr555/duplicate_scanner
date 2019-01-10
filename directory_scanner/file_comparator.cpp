#include "file_comparator.h"
#include "exceptions.h"

#include <vector>
#include <QFile>
#include <QFileInfo>
#include <QDebug>

bool FileComparator::compare(QString const &a, QString const &b, bool const * abort = nullptr) const // TODO: проверить, нет ли утечки памяти
{
   // qDebug() << "cmp: " << a << " " << b;

    QFile file_a(a), file_b(b);
    QFileInfo info_a(a), info_b(b);

    if (!info_a.isFile() || !info_b.isFile())
        throw std::logic_error("Both path must specify files");

    if (file_a.size() != file_b.size())
        return false;

    if (!file_a.open(QIODevice::ReadOnly))
        throw UnableToOpenFileException(a.toStdString());
    if (!file_b.open(QIODevice::ReadOnly))
        throw UnableToOpenFileException(b.toStdString());

    std::vector<char> buffer_a(static_cast<size_t>(buffer_size)), buffer_b(static_cast<size_t>(buffer_size));

    while (true)
    {
        if (abort && *abort)
            return false;

        qint64 result_a = file_a.read(buffer_a.data(), buffer_size);
        if (result_a == -1)
            throw FilesystemException(a.toStdString());

        qint64 result_b = file_b.read(buffer_b.data(), buffer_size);
        if (result_b == -1)
            throw FilesystemException(b.toStdString());

        if (result_a != result_b)
            throw std::logic_error("Files size aren't equals");

        if (result_a == 0)
            break;

//        for (size_t i = 0; i < result_a; ++i)
//            if (buffer_a[i] != buffer_b[i])
        if (memcmp(buffer_a.data(), buffer_b.data(), static_cast<size_t>(result_a)) != 0)
            return false;
    }
    return true;
}

FileComparator::FileComparator(qint64 buffer_size): buffer_size(buffer_size)
{}

FileComparator::FileComparator() : FileComparator(1048576) {}
