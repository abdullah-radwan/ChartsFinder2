#ifndef CONFIGEDITOR_H
#define CONFIGEDITOR_H

#include <QSettings>
#include <QRect>

class ConfigEditor
{
public:
    ConfigEditor(QString dirPath);
    void readConfig();
    void writeConfig();

    struct Resource {
        QString url;
        bool type;
        QString suffix;
    };

    struct Config {
        QString chartsPath;
        bool openCharts;
        bool openFolder;
        bool checkUpdates;
        int updatePeriod;
        QList<Resource> resources;

        QString language;
        QSize mainSize;
        QPoint mainPos;
        QSize settingsSize;
        QPoint settingsPos;
    } config;

    static void setFltPlan(ConfigEditor::Config* config);
    static void resetResources(ConfigEditor::Config* config);

private:
    friend QDataStream &operator<<(QDataStream &out, const Resource &resource);
    friend QDataStream &operator>>(QDataStream &in, Resource &resource);

    QSettings settings;
};

Q_DECLARE_METATYPE(ConfigEditor::Resource);

#endif // CONFIGEDITOR_H
