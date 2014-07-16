#include "o2gft.h"

static const QString GftScope = QString::fromLatin1("https://www.googleapis.com/auth/fusiontables");
static const QString GftEndpoint = QString::fromLatin1("https://accounts.google.com/o/oauth2/auth");
static const QString GftTokenUrl = QString::fromLatin1("https://accounts.google.com/o/oauth2/token");
static const QString GftRefreshUrl = QString::fromLatin1("https://accounts.google.com/o/oauth2/token");

O2Gft::O2Gft(QObject *parent): O2(parent) {
    setRequestUrl(GftEndpoint);
    setTokenUrl(GftTokenUrl);
    setRefreshTokenUrl(GftRefreshUrl);
    setScope(GftScope);
}
