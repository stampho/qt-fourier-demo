#include "clinfo.h"

template<typename InfoType>
struct CLInfoHelper {
    template<typename InfoFunc, typename DataType>
    static QString toString(InfoFunc func, DataType data, int info)
    {
        InfoType buffer;
        func(data, info, sizeof(buffer), &buffer, 0);
        return QString::number(buffer);
    }
};

template<>
struct CLInfoHelper<size_t *> {
    template<typename InfoFunc, typename DataType>
    static QString toString(InfoFunc func, DataType data, int info)
    {
        size_t size;
        func(data, info, 0, 0, &size);

        size_t buffer[size];
        func(data, info, size, &buffer, 0);

        QStringList items;
        for (size_t i = 0; i < size / sizeof(size_t); ++i)
            items.append(QString::number(buffer[i]));

        return items.join(" ");
    }
};

template<>
struct CLInfoHelper<char *> {
    template<typename InfoFunc, typename DataType>
    static QString toString(InfoFunc func, DataType data, int info)
    {
        size_t size;
        func(data, info, 0, 0, &size);

        char buffer[size];
        func(data, info, size, &buffer, 0);

        return QString::fromLatin1(buffer);
    }
};

QString CLInfo::keyToString(int key)
{
    switch (key) {
    case CL_PLATFORM_PROFILE: return QStringLiteral("Profile");
    case CL_PLATFORM_VERSION: return QStringLiteral("Version");
    case CL_PLATFORM_NAME: return QStringLiteral("Name");
    case CL_PLATFORM_VENDOR: return QStringLiteral("Vendor");
    case CL_PLATFORM_EXTENSIONS: return QStringLiteral("Extensions");

    case CL_DEVICE_TYPE: return QStringLiteral("Type");
    case CL_DEVICE_NAME: return QStringLiteral("Name");
    case CL_DEVICE_VENDOR: return QStringLiteral("Vendor");
    case CL_DEVICE_VERSION: return QStringLiteral("Version");
    case CL_DEVICE_MAX_COMPUTE_UNITS: return QStringLiteral("Max Compute Units");
    case CL_DEVICE_MAX_CLOCK_FREQUENCY: return QStringLiteral("Max Clock Frequency");
    case CL_DEVICE_GLOBAL_MEM_SIZE: return QStringLiteral("Global Memory");
    case CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE: return QStringLiteral("Constant Buffer");
    case CL_DEVICE_MAX_WORK_ITEM_SIZES: return QStringLiteral("Work Item Sizes");
    }

    return QString::number(key);
}

CLInfo::CLInfo(cl_platform_id id)
    : QMap<int, QString>()
{
    if (!id)
        return;

    insert(CL_PLATFORM_PROFILE, CLInfoHelper<char *>::toString(clGetPlatformInfo, id, CL_PLATFORM_PROFILE));
    insert(CL_PLATFORM_VERSION, CLInfoHelper<char *>::toString(clGetPlatformInfo, id, CL_PLATFORM_VERSION));
    insert(CL_PLATFORM_NAME, CLInfoHelper<char *>::toString(clGetPlatformInfo, id, CL_PLATFORM_NAME));
    insert(CL_PLATFORM_VENDOR, CLInfoHelper<char *>::toString(clGetPlatformInfo, id, CL_PLATFORM_VENDOR));
    insert(CL_PLATFORM_EXTENSIONS, CLInfoHelper<char *>::toString(clGetPlatformInfo, id, CL_PLATFORM_EXTENSIONS));
}

CLInfo::CLInfo(cl_device_id id)
    : QMap<int, QString>()
{
    if (!id)
        return;

    insert(CL_DEVICE_TYPE, CLInfoHelper<cl_device_type>::toString(clGetDeviceInfo, id, CL_DEVICE_TYPE));
    insert(CL_DEVICE_NAME, CLInfoHelper<char *>::toString(clGetDeviceInfo, id, CL_DEVICE_NAME));
    insert(CL_DEVICE_VENDOR, CLInfoHelper<char *>::toString(clGetDeviceInfo, id, CL_DEVICE_VENDOR));
    insert(CL_DEVICE_VERSION, CLInfoHelper<char *>::toString(clGetDeviceInfo, id, CL_DEVICE_VERSION));
    insert(CL_DEVICE_MAX_CLOCK_FREQUENCY, CLInfoHelper<cl_uint>::toString(clGetDeviceInfo, id, CL_DEVICE_MAX_CLOCK_FREQUENCY));
    insert(CL_DEVICE_MAX_COMPUTE_UNITS, CLInfoHelper<cl_uint>::toString(clGetDeviceInfo, id, CL_DEVICE_MAX_COMPUTE_UNITS));
    insert(CL_DEVICE_GLOBAL_MEM_SIZE, CLInfoHelper<cl_ulong>::toString(clGetDeviceInfo, id, CL_DEVICE_GLOBAL_MEM_SIZE));
    insert(CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE, CLInfoHelper<cl_ulong>::toString(clGetDeviceInfo, id, CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE));
    insert(CL_DEVICE_MAX_WORK_ITEM_SIZES, CLInfoHelper<size_t *>::toString(clGetDeviceInfo, id, CL_DEVICE_MAX_WORK_ITEM_SIZES));
}

CLInfo::~CLInfo()
{

}

QDebug operator<<(QDebug debug, const CLInfo &info)
{
    QDebug verbose = debug.nospace().noquote();

    Q_FOREACH (int key, info.keys()) {
        verbose << QString("%1: %2").arg(CLInfo::keyToString(key), info[key]);
        verbose << "\n";
    }

    return verbose;
}
