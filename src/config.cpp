#include "config.h"
#include <QDir>
#include <QDate>
#include <QStandardPaths>
#include <QDebug>

Config::configStruct Config::readConfig()
{ 
    // Set the config file path on different systems
#ifdef Q_OS_WIN
    QSettings settings("config.cfg", QSettings::IniFormat);
#else
    QSettings settings(".config/chartsfinder2/config.cfg", QSettings::IniFormat);
#endif

    qDebug() << "Reading config";

    settings.beginGroup("General");

    // Read General values
    QString language = settings.value("Language", "System").toString();

    QRect mainWinGeo = settings.value("MainWinGeo", QRect()).toRect();

    QRect settingsWinGeo = settings.value("SettingsWinGeo", QRect()).toRect();

    // If not set, make it the documents folder
    QString path = settings.value("Path", QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/Charts/").toString();

    bool openCharts = settings.value("OpenCharts", true).toBool();

    bool removeFiles = settings.value("RemoveFiles", false).toBool();

    int updatePeriod = settings.value("UpdatePeriod", 3).toInt();

    // If the lastest check date plus the update period is lower than or equal to the current date, check updates
    bool checkUpdates = settings.value("CheckDate", 0).toDate().addDays(updatePeriod) <= QDate::currentDate();

    // Get the resource as QVariant
    QList<QVariant> cResources = settings.value("Resources").toList();

    QList<QStringList> resources;

    // Copy values to the string resources list
    foreach(QVariant var, cResources) resources.append(var.toStringList());

    // If no resources are exists
    if(resources.isEmpty())
    {
        resources.append({"http://imageserver.fltplan.com/merge/merge190718/%1.pdf", "0"});
        resources.append({"http://www.sia-enna.dz/PDF/AIP/AD/AD2/%1/", "1"});
        resources.append({"http://caa.gov.ly/ais/wp-content/uploads/2017/AIP/AD/%1.pdf", "0"});
        resources.append({"http://www.caiga.ru/common/AirInter/validaip/aip/ad/ad2/rus/%1/", "1"});
        resources.append({"http://vau.aero/navdb/chart/%1.pdf", "0"});
        resources.append({"http://sa-ivao.net/charts_file/%1.pdf", "0"});
        resources.append({"http://europlanet.de/vaFsP/charts/%1.pdf", "0"});
        resources.append({"http://www.fly-sea.com/charts/%1.pdf", "0"});
        resources.append({"http://uvairlines.com/admin/resources/charts/%1.pdf", "0"});
        resources.append({"https://www.virtualairlines.eu/charts/%1.pdf", "0"});
    }

    settings.endGroup();

    qDebug() << "Config is read";

    return { language, mainWinGeo, settingsWinGeo, path, openCharts, removeFiles, checkUpdates, updatePeriod, resources };
}

void Config::writeConfig(Config::configStruct config)
{
#ifdef Q_OS_WIN
    QSettings settings("config.cfg", QSettings::IniFormat);
#else
    QSettings settings(".config/chartsfinder2/config.cfg", QSettings::IniFormat);
#endif

    qDebug() << "Writing config";

    settings.beginGroup("General");

    settings.setValue("Language", config.language);

    settings.setValue("MainWinGeo", config.mainWinGeo);

    settings.setValue("SettingsWinGeo", config.settingsWinGeo);

    settings.setValue("Path", config.path);

    settings.setValue("OpenCharts", config.openCharts);

    settings.setValue("RemoveFiles", config.removeFiles);

    settings.setValue("UpdatePeriod", config.updatePeriod);

    // If updates were checked or the written date is invalid, reset the check date
    if(config.checkUpdates || !settings.value("CheckDate", 0).toDate().isValid()) settings.setValue("CheckDate", QDate::currentDate());

    QList<QVariant> resources;

    // Write each to resource to QVariant
    foreach(QStringList list, config.resources) resources.append(list);

    settings.setValue("Resources", resources);

    settings.endGroup();

    qDebug() << "Config is written";
}
