#ifndef CLINFO_H
#define CLINFO_H

#include <CL/cl.h>
#include <QDebug>
#include <QMap>

class CLInfo : public QMap<int, QString> {
public:
    static QString keyToString(int);

    CLInfo(cl_platform_id);
    CLInfo(cl_device_id);
    CLInfo(cl_kernel, cl_device_id);
    virtual ~CLInfo();
};

QDebug operator<<(QDebug, const CLInfo &);

#endif // CLINFO_H
