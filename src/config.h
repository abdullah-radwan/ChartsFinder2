#ifndef CONFIG_H
#define CONFIG_H

#include <QSettings>
#include <QUrl>
#include <QRect>
#include <QStandardPaths>

class Config
{
public:

    typedef struct
    {
        QString language;
        QRect mainWinGeo;
        QRect settingsWinGeo;
        QString path;
        bool openChart, openFolder, removeFiles, checkUpdates;
        int updatePeriod;
        QList<QStringList> resources;
    } ConfigStruct;

    static ConfigStruct readConfig();

    static void writeConfig(ConfigStruct config);
};

#endif // CONFIG_H
