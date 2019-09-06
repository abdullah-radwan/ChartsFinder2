#ifndef CONFIG_H
#define CONFIG_H

#include <QSettings>
#include <QUrl>
#include <QRect>

class Config
{
public:

    typedef struct
    {
        QString language;
        QRect mainWinGeo;
        QRect settingsWinGeo;
        QString path;
        bool openCharts, removeFiles, checkUpdates;
        int updatePeriod;
        QList<QStringList> resources;
    } configStruct;

    static configStruct readConfig();

    static void writeConfig(configStruct);
};

#endif // CONFIG_H
