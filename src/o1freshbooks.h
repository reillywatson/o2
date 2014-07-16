#ifndef O1FRESHBOOKS_H
#define O1FRESHBOOKS_H

#include "o1.h"

class O1Freshbooks: public O1 {
    Q_OBJECT

public:
    explicit O1Freshbooks(QObject *parent = 0): O1(parent)
    {
    }

    void setClientId(const QString &value)
    {
        O1::setClientId(value);

        setRequestTokenUrl(QUrl(QString::fromLatin1("https://%1.freshbooks.com/oauth/oauth_request.php").arg(clientId())));
        setAuthorizeUrl(QUrl(QString::fromLatin1("https://%1.freshbooks.com/oauth/oauth_authorize.php").arg(clientId())));
        setAccessTokenUrl(QUrl(QString::fromLatin1("https://%1.freshbooks.com/oauth/oauth_access.php").arg(clientId())));
    }


};

#endif // O1FRESHBOOKS_H
