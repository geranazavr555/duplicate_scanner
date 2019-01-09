#ifndef HW1_FILE_COMPARATOR_H
#define HW1_FILE_COMPARATOR_H

#include <QString>

class FileComparator
{
private:
    qint64 buffer_size;

public:
    FileComparator();
    FileComparator(qint64 buffer_size);

    bool compare(QString const& a, QString const& b, bool const * abort) const;
};

#endif //HW1_FILE_COMPARATOR_H
