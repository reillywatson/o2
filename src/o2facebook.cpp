#include <QDebug>
#include <QMap>
#include <QString>
#include <QStringList>
#if QT_VERSION >= 0x050000
#include <QUrlQuery>
#endif

#include "o2facebook.h"
#include "o2globals.h"

static const  QString FbEndpoint = QString::fromLatin1("https://graph.facebook.com/oauth/authorize?display=touch");
static const QString FbTokenUrl = QString::fromLatin1("https://graph.facebook.com/oauth/access_token");
static const quint16 FbLocalPort = 1965;

const QString FB_EXPIRES_KEY = QString::fromLatin1("expires");

O2Facebook::O2Facebook(QObject *parent): O2(parent) {
    setRequestUrl(FbEndpoint);
    setTokenUrl(FbTokenUrl);
    setLocalPort(FbLocalPort);
}

void O2Facebook::onVerificationReceived(const QMap<QString, QString> response) {
    emit closeBrowser();
    if (response.contains(QString::fromLatin1("error"))) {
        qWarning() << "O2Facebook::onVerificationReceived: Verification failed";
        foreach (QString key, response.keys()) {
            qWarning() << "O2Facebook::onVerificationReceived:" << key << response.value(key);
        }
        emit linkingFailed();
        return;
    }

    // Save access code
    setCode(response.value(QString::fromLatin1(O2_OAUTH2_CODE)));

    // Exchange access code for access/refresh tokens
    QUrl url(tokenUrl_);
#if QT_VERSION < 0x050000
    url.addQueryItem(QString::fromLatin1(O2_OAUTH2_CLIENT_ID), clientId_);
    url.addQueryItem(QString::fromLatin1(O2_OAUTH2_CLIENT_SECRET), clientSecret_);
    url.addQueryItem(QString::fromLatin1(O2_OAUTH2_SCOPE), scope_);
    url.addQueryItem(QString::fromLatin1(O2_OAUTH2_CODE), code());
    url.addQueryItem(QString::fromLatin1(O2_OAUTH2_REDIRECT_URI), redirectUri_);
#else
    QUrlQuery query(url);
    query.addQueryItem(QString::fromLatin1(O2_OAUTH2_CLIENT_ID), clientId_);
    query.addQueryItem(QString::fromLatin1(O2_OAUTH2_CLIENT_SECRET), clientSecret_);
    query.addQueryItem(QString::fromLatin1(O2_OAUTH2_SCOPE), scope_);
    query.addQueryItem(QString::fromLatin1(O2_OAUTH2_CODE), code());
    query.addQueryItem(QString::fromLatin1(O2_OAUTH2_REDIRECT_URI), redirectUri_);
    url.setQuery(query);
#endif

    QNetworkRequest tokenRequest(url);
    QNetworkReply *tokenReply = manager_->get(tokenRequest);
    timedReplies_.add(tokenReply);
    connect(tokenReply, SIGNAL(finished()), this, SLOT(onTokenReplyFinished()), Qt::QueuedConnection);
    connect(tokenReply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(onTokenReplyError(QNetworkReply::NetworkError)), Qt::QueuedConnection);
}

void O2Facebook::onTokenReplyFinished() {
    QNetworkReply *tokenReply = qobject_cast<QNetworkReply *>(sender());
    if (tokenReply->error() == QNetworkReply::NoError) {

        // Process reply
        QByteArray replyData = tokenReply->readAll();
        QMap<QString, QString> reply;
        foreach (QString pair, QString::fromLatin1(replyData).split(QString::fromLatin1("&"))) {
            QStringList kv = pair.split(QString::fromLatin1("="));
            if (kv.length() == 2) {
                reply.insert(kv[0], kv[1]);
            }
        }

        // Interpret reply
        setToken(reply.value(QString::fromLatin1(O2_OAUTH2_ACCESS_TOKEN), QString()));
        setExpires(reply.value(FB_EXPIRES_KEY).toInt());
        setRefreshToken(reply.value(QString::fromLatin1(O2_OAUTH2_REFRESH_TOKEN), QString()));

        timedReplies_.remove(tokenReply);
        emit linkedChanged();
        emit tokenChanged();
        emit linkingSucceeded();
    }
}

void O2Facebook::unlink() {
    O2::unlink();
    // FIXME: Delete relevant cookies, too
}
