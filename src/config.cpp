#include "config.h"
#include <QDir>
#include <QDate>
#include <QStandardPaths>
#include <QApplication>
#include <QDebug>

Config::ConfigStruct Config::readConfig()
{ 
    // This should be %appdata%/ChartsFinder2/config.cfg on Windows, and ~/.local/share/ChartsFinder2/config.cfg on Linux
    QSettings settings(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/config.cfg", QSettings::IniFormat);

    qDebug() << "Reading config";

    settings.beginGroup("General");

    // Read General values
    QString language = settings.value("Language", "System").toString();

    QRect mainWinGeo = settings.value("MainWinGeo", QRect()).toRect();

    QRect settingsWinGeo = settings.value("SettingsWinGeo", QRect()).toRect();

    // If not set, make it the documents folder
    QString path = settings.value("Path", QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/Charts/").toString();

    bool openChart = settings.value("OpenChart", true).toBool();

    bool openFolder = settings.value("OpenFolder", true).toBool();

    bool removeFiles = settings.value("RemoveFiles", false).toBool();

    int updatePeriod = settings.value("UpdatePeriod", 3).toInt();

    bool checkUpdates = false;

    // If the lastest check date plus the update period is lower than or equal to the current date, check updates
    if(updatePeriod) checkUpdates = settings.value("CheckDate", 0).toDate().addDays(updatePeriod) <= QDate::currentDate();

    // Get the resource as QVariant
    QList<QVariant> cResources = settings.value("Resources").toList();

    QList<QStringList> resources;

    // Copy values to the string resources list
    foreach(QVariant var, cResources) resources.append(var.toStringList());

    // If no resources are exists
    if(resources.isEmpty())
    {
        resources.append({"http://imageserver.fltplan.com/merge/merge191010/%1", "0", ".pdf"});
        resources.append({"http://www.sia-enna.dz/PDF/AIP/AD/AD2/%1/", "1", ".pdf"});
        resources.append({"http://caa.gov.ly/ais/wp-content/uploads/2017/AIP/AD/%1", "0", ".pdf"});
        resources.append({"http://www.caiga.ru/common/AirInter/validaip/aip/ad/ad2/rus/%1/", "1", ".pdf"});
        resources.append({"http://vau.aero/navdb/chart/%1", "0", ".pdf"});
        resources.append({"http://sa-ivao.net/charts_file/%1", "0", ".pdf"});
        resources.append({"http://europlanet.de/vaFsP/charts/%1", "0", ".pdf"});
        resources.append({"http://www.fly-sea.com/charts/%1", "0", ".pdf"});
        resources.append({"http://uvairlines.com/admin/resources/charts/%1", "0", ".pdf"});
        resources.append({"https://www.virtualairlines.eu/charts/%1", "0", ".pdf"});
    } else
    {
        // Upgrade the old resources to the new format
        for (int index = 0; index < resources.size(); index++)
        {
            QStringList resource = resources.at(index);

            // If it ends with normal (not upgraded because upgraded ends with suffix)
            if(resource.last() == "0")
            {
                // Remove the suffix from the URL
                resource.first().remove(".pdf", Qt::CaseInsensitive);

                // Add the suffix to the resource
                resource.append(".pdf");
            }
            // No need to remove the suffix as it doesn't exists if it's a folder resource
            else if(resource.last() == "1") resource.append(".pdf");

            resources.replace(index, resource);
        }
    }

    settings.endGroup();

    qDebug() << "Config is read";

    return { language, mainWinGeo, settingsWinGeo, path, openChart, openFolder, removeFiles, checkUpdates, updatePeriod, resources };
}

void Config::writeConfig(Config::ConfigStruct config)
{
    // This should be %appdata%/ChartsFinder2/config.cfg on Windows, and ~/.local/share/ChartsFinder2/config.cfg on Linux
    QSettings settings(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/config.cfg", QSettings::IniFormat);

    qDebug() << "Writing config";

    settings.beginGroup("General");

    settings.setValue("Language", config.language);

    settings.setValue("MainWinGeo", config.mainWinGeo);

    settings.setValue("SettingsWinGeo", config.settingsWinGeo);

    settings.setValue("Path", config.path);

    settings.setValue("OpenChart", config.openChart);

    settings.setValue("OpenFolder", config.openFolder);

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
