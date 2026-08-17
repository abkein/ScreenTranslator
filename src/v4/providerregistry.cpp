#include "providerregistry.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QObject>
#include <QSet>
#include <QStandardPaths>

#include <algorithm>
#include <utility>

namespace v4
{
namespace
{
const QStringList builtIns{
    "baidu.js",      "bing.js",   "deepl.js",  "google.js",
    "google_api.js", "papago.js", "yandex.js",
};

QString readScript(const QString& path)
{
  QFile file(path);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    return {};
  return QString::fromUtf8(file.readAll());
}
}  // namespace

ProviderRegistry::ProviderRegistry(QString userDirectory)
  : userDirectoryOverride_(std::move(userDirectory))
{
  reload();
}

void ProviderRegistry::reload()
{
  providers_.clear();
  QSet<QString> ids;

  for (const auto& id : builtIns) {
    const auto script = readScript(":/providers/" + id);
    if (script.isEmpty())
      continue;
    providers_.push_back({id, QFileInfo(id).completeBaseName(), script, true});
    ids.insert(id);
  }

  QDir userDir(userDirectory());
  if (!userDir.exists())
    QDir().mkpath(userDir.absolutePath());

  const auto files = userDir.entryInfoList({"*.js"}, QDir::Files, QDir::Name);
  for (const auto& file : files) {
    const auto id = file.fileName();
    if (ids.contains(id))
      continue;
    const auto script = readScript(file.absoluteFilePath());
    if (script.isEmpty())
      continue;
    providers_.push_back(
        {id, file.completeBaseName() + QObject::tr(" (user)"), script, false});
    ids.insert(id);
  }
}

const QVector<ProviderInfo>& ProviderRegistry::providers() const
{
  return providers_;
}

const ProviderInfo* ProviderRegistry::find(const QString& id) const
{
  const auto it = std::find_if(
      providers_.cbegin(), providers_.cend(),
      [&id](const ProviderInfo& provider) { return provider.id == id; });
  return it == providers_.cend() ? nullptr : &*it;
}

QString ProviderRegistry::userDirectory() const
{
  if (!userDirectoryOverride_.isEmpty())
    return userDirectoryOverride_;
  return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) +
         "/providers";
}
}  // namespace v4
