#include "configeditor.h"

#include <QDataStream>
#include <QStandardPaths>
#include <QDir>
#include <QDate>
#include <QDebug>

ConfigEditor::ConfigEditor(QString dirPath) : settings(dirPath + "/config.cfg", QSettings::IniFormat) {}

void ConfigEditor::readConfig()
{
    qDebug() << "Reading config";

    settings.beginGroup("Settings");

    // If not set, make it the documents folder
    config.chartsPath = settings.value("ChartsPath",
                                       QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/Charts/").toString();

    config.openCharts = settings.value("OpenCharts", true).toBool();
    config.openFolder = settings.value("OpenFolder", true).toBool();
    config.updatePeriod = settings.value("UpdatePeriod", 7).toInt();

    if (config.updatePeriod == 3)
        config.updatePeriod = 7;

    // If the lastest check date plus the update period is lower than or equal to the current date, check updates
    config.checkUpdates = settings.value("CheckDate", 0).toDate().addDays(config.updatePeriod) <= QDate::currentDate();

    // Get the resource as QVariant
    QList<QVariant> vResources = settings.value("Resources").toList();
    config.resources = settings.value("Resources").value<QList<Resource>>();

    if (config.resources.isEmpty() && !vResources.isEmpty()) {
        // Copy values to the string resources list
        for (const QVariant &var : vResources) {
            QStringList resource = var.toStringList();

            config.resources.append({resource.first(), resource.at(1) == "1", resource.last()});
        }
        // If no resources are exists
    } else if (config.resources.isEmpty()) {
        resetResources(&config);
    }

    if (!config.resources.isEmpty() && config.resources.first().suffix.startsWith("."))
    {
        for (Resource &resource : config.resources)
        {
            resource.suffix.remove(0, 1);
        }
    }

    settings.endGroup();

    settings.beginGroup("GUI");

    config.language = settings.value("Language", "System").toString();
    config.mainSize = settings.value("MainSize", QSize(-1, -1)).toSize();
    config.mainPos = settings.value("MainPos", QPoint(-1, -1)).toPoint();
    config.settingsSize = settings.value("SettingsSize", QSize(-1, -1)).toSize();
    config.settingsPos = settings.value("SettingsPos", QPoint(-1, -1)).toPoint();

    settings.endGroup();

    qDebug() << "Config was read";
}

void ConfigEditor::writeConfig()
{
    qDebug() << "Writing config";

    settings.beginGroup("Settings");

    settings.setValue("ChartsPath", config.chartsPath);
    settings.setValue("OpenCharts", config.openCharts);
    settings.setValue("OpenFolder", config.openFolder);
    settings.setValue("UpdatePeriod", config.updatePeriod);

    // If updates were checked or the written date is invalid, reset the check date
    if (config.checkUpdates || !settings.value("CheckDate", 0).toDate().isValid())
        settings.setValue("CheckDate", QDate::currentDate());

    settings.setValue("Resources", QVariant::fromValue(config.resources));

    settings.endGroup();

    settings.beginGroup("GUI");

    settings.setValue("Language", config.language);
    settings.setValue("MainSize", config.mainSize);
    settings.setValue("MainPos", config.mainPos);
    settings.setValue("SettingsSize", config.settingsSize);
    settings.setValue("SettingsPos", config.settingsPos);

    settings.endGroup();

    qDebug() << "Config was written";
}

void ConfigEditor::resetResources(ConfigEditor::Config *config)
{
    config->resources.clear();
    config->resources.append({"http://imageserver.fltplan.com/merge/merge200521/%1", false, "pdf"});
    config->resources.append({"http://www.sia-enna.dz/PDF/AIP/AD/AD2/%1/", true, "pdf"});
    config->resources.append({"http://www.caiga.ru/common/AirInter/validaip/aip/ad/ad2/rus/%1/", true, "pdf"});
    config->resources.append({"http://vau.aero/navdb/chart/%1", false, "pdf"});
    config->resources.append({"http://europlanet.de/vaFsP/charts/%1", false, "pdf"});
    config->resources.append({"http://www.fly-sea.com/charts/%1", false, "pdf"});
    config->resources.append({"http://uvairlines.com/admin/resources/charts/%1", false, "pdf"});
    config->resources.append({"https://www.virtualairlines.eu/charts/%1", false, "pdf"});
}

QDataStream &operator<<(QDataStream &out, const ConfigEditor::Resource &resource)
{
    out << resource.url << resource.type << resource.suffix;

    return out;
}

QDataStream &operator>>(QDataStream &in, ConfigEditor::Resource &resource)
{
    in >> resource.url >> resource.type >> resource.suffix;

    return in;
}
