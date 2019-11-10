#ifndef CONFIG_H
#define CONFIG_H

#include <QSettings>
#include <QUrl>
#include <QRect>
#include <QStandardPaths>

class Config
{
public:

    struct Resource
    {
        QString url;
        bool type;
        QString suffix;
    };

    struct ConfigStruct
    {
        QString language, path;
        QRect mainWinGeo, settingsWinGeo;
        bool openChart, openFolder, removeFiles, checkUpdates;
        int updatePeriod;
        QList<Resource> resources;
    };

    static ConfigStruct readConfig();

    static void writeConfig(ConfigStruct config);

    friend QDataStream& operator<<(QDataStream& out, const Resource& resource);
    friend QDataStream& operator>>(QDataStream& in, Resource& resource);
};

Q_DECLARE_METATYPE(Config::Resource);

#endif // CONFIG_H
