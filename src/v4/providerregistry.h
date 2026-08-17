#pragma once

#include <QString>
#include <QVector>

namespace v4
{
struct ProviderInfo {
  QString id;
  QString displayName;
  QString script;
  bool builtIn{false};
};

class ProviderRegistry
{
public:
  explicit ProviderRegistry(QString userDirectory = {});

  void reload();
  const QVector<ProviderInfo>& providers() const;
  const ProviderInfo* find(const QString& id) const;
  QString userDirectory() const;

private:
  QString userDirectoryOverride_;
  QVector<ProviderInfo> providers_;
};
}  // namespace v4
