#ifndef CONFIG_H
#define CONFIG_H

#include <QSettings>
#include <QUrl>

class Config
{
public:
    struct configStruct
    {
        QString path;
        bool openCharts, removeFiles;
        QList<QStringList> resources;
    };

    Config();

    static configStruct readConfig();

    static void writeConfig(configStruct);
};

#endif // CONFIG_H
