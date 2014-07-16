#include <QTcpServer>
#include <QTcpSocket>
#include <QByteArray>
#include <QString>
#include <QMap>
#include <QPair>
#include <QStringList>
#include <QUrl>
#include <QDebug>
#if QT_VERSION >= 0x050000
#include <QUrlQuery>
#endif

#include "o2replyserver.h"

#define trace() if (1) qDebug()
// #define trace() if (0) qDebug()

O2ReplyServer::O2ReplyServer(QObject *parent): QTcpServer(parent) {
    connect(this, SIGNAL(newConnection()), this, SLOT(onIncomingConnection()));
}

O2ReplyServer::~O2ReplyServer() {
}

void O2ReplyServer::onIncomingConnection() {
    QTcpSocket* socket = nextPendingConnection();
    connect(socket, SIGNAL(readyRead()), this, SLOT(onBytesReady()), Qt::UniqueConnection);
    connect(socket, SIGNAL(disconnected()), socket, SLOT(deleteLater()));
}

void O2ReplyServer::onBytesReady() {
    QTcpSocket *socket = qobject_cast<QTcpSocket *>(sender());
    if (!socket) {
        return;
    }
    QByteArray reply;
    QByteArray content;
    content.append("<HTML></HTML>");
    reply.append("HTTP/1.0 200 OK \r\n");
    reply.append("Content-Type: text/html; charset=\"utf-8\"\r\n");
    reply.append(QString::fromLatin1("Content-Length: %1\r\n\r\n").arg(content.size()).toLatin1());
    reply.append(content);
    socket->write(reply);

    QByteArray data = socket->readAll();
    QMap<QString, QString> queryParams = parseQueryParams(&data);
    socket->disconnectFromHost();
    close();
    emit verificationReceived(queryParams);
}

QMap<QString, QString> O2ReplyServer::parseQueryParams(QByteArray *data) {
    trace() << "O2ReplyServer::parseQueryParams";

    QString splitGetLine = QString::fromLatin1(*data).split(QString::fromLatin1("\r\n")).first();
    splitGetLine.remove(QString::fromLatin1("GET "));
    splitGetLine.remove(QString::fromLatin1("HTTP/1.1"));
    splitGetLine.remove(QString::fromLatin1("\r\n"));
    splitGetLine.prepend(QString::fromLatin1("http://localhost"));
    QUrl getTokenUrl(splitGetLine);

    QList< QPair<QString, QString> > tokens;
#if QT_VERSION < 0x050000
    tokens = getTokenUrl.queryItems();
#else
    QUrlQuery query(getTokenUrl);
    tokens = query.queryItems();
#endif
    QMultiMap<QString, QString> queryParams;
    QPair<QString, QString> tokenPair;
    foreach (tokenPair, tokens) {
        // FIXME: We are decoding key and value again. This helps with Google OAuth, but is it mandated by the standard?
        QString key = QUrl::fromPercentEncoding(tokenPair.first.trimmed().toUtf8());
        QString value = QUrl::fromPercentEncoding(tokenPair.second.trimmed().toUtf8());
        queryParams.insert(key, value);
    }
    return queryParams;
}
