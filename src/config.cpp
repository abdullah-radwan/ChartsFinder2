#include "config.h"
#include <QCoreApplication>
#include <QDir>

Config::Config()
{
}

Config::configStruct Config::readConfig()
{ 
    #ifdef Q_OS_WIN
    QSettings settings("config.cfg", QSettings::IniFormat);
    #else
    QSettings settings(".chartsfinder2/config.cfg", QSettings::IniFormat);;
    #endif

    settings.beginGroup("General");

    QString path = settings.value("Path", QDir::currentPath() + "/Charts/").toString();
    bool openCharts = settings.value("OpenCharts", true).toBool();
    bool removeFiles = settings.value("RemoveFiles", false).toBool();

    QList<QVariant> oResources = settings.value("Resources").toList();

    QList<QStringList> resources;

    foreach(QVariant var, oResources) { resources.append(var.toStringList()); QCoreApplication::processEvents(); }

    if(resources.isEmpty())
    {
        resources.append({"http://imageserver.fltplan.com/merge/merge190228/%1.pdf","Normal"});
        resources.append({"http://www.sia-enna.dz/PDF/AIP/AD/AD2/%1/","Folder"});
        resources.append({"http://www.caiga.ru/common/AirInter/validaip/aip/ad/ad2/rus/%1/","Folder"});
        resources.append({"http://vau.aero/navdb/chart/%1.pdf","Normal"});
        resources.append({"http://sa-ivao.net/charts_file/%1.pdf","Normal"});
        resources.append({"http://europlanet.de/vaFsP/charts/%1.pdf","Normal"});
        resources.append({"http://www.fly-sea.com/charts/%1.pdf","Normal"});
        resources.append({"http://uvairlines.com/admin/resources/charts/%1.pdf","Normal"});
        resources.append({"https://www.virtualairlines.eu/charts/%1.pdf","Normal"});
    }

    settings.endGroup();

    return {path, openCharts, removeFiles, resources};
}

void Config::writeConfig(Config::configStruct config)
{
    #ifdef Q_OS_WIN
    QSettings settings("config.cfg", QSettings::IniFormat);
    #else
    QSettings settings(".chartsfinder2/config.cfg", QSettings::IniFormat);;
    #endif

    settings.clear();

    settings.beginGroup("General");

    settings.setValue("Path", config.path);
    settings.setValue("OpenCharts", config.openCharts);
    settings.setValue("RemoveFiles", config.removeFiles);

    QList<QVariant> resources;

    foreach(QStringList list, config.resources) { resources.append(list); QCoreApplication::processEvents(); }

    settings.setValue("Resources", resources);

    settings.endGroup();
}
