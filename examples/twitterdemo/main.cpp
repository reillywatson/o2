#include <QApplication>
#include <QStringList>
#include <QTimer>
#include <QDebug>

#include "tweeter.h"

const QString OPT_OAUTH = QString::fromLatin1("-o");
const QString OPT_XAUTH = QString::fromLatin1("-x");
const QString OPT_USERNAME = QString::fromLatin1("-u");
const QString OPT_PASSWORD = QString::fromLatin1("-p");
const QString OPT_STATUS = QString::fromLatin1("-m");

const QString USAGE = QString::fromLatin1("\n"
                     "Usage: tweetdemo [OPTION]...\n"
                     "Get OAuth access tokens from Twitter's OAuth service and "
                     "(optionally) post a status update on a user's timeline\n"
                     "\nOptions:\n"
                     "  %1\t\tLink with Twitter OAuth service, i.e get access tokens\n"
                     "  %2\t\tLink with Twitter XAuth service, i.e get access tokens using the XAuth protocol\n"
                     "  %3 <username>\tTwitter username to be used while using XAuth (-x option)\n"
                     "  %4 <password>\tTwitter password to be used while using XAuth (-x option)\n"
                     "  %5\t\tStatus update message, enclosed in double quotes\n");


class Helper : public QObject
{
    Q_OBJECT

public:
    Helper() : QObject(), tweeter_(this), waitForMsg_(false), msg_(QString()) {}

public slots:

    void processArgs() {

        QStringList argList = qApp->arguments();

        QByteArray help = USAGE.arg(OPT_OAUTH,
                                    OPT_XAUTH,
                                    OPT_USERNAME,
                                    OPT_PASSWORD,
                                    OPT_STATUS).toLatin1();

        const char* helpText = help.constData();

        connect(&tweeter_, SIGNAL(linkingFailed()),
                this, SLOT(onLinkingFailed()));
        connect(&tweeter_, SIGNAL(linkingSucceeded()),
                this, SLOT(onLinkingSucceeded()));

        if (argList.contains(OPT_OAUTH)) {
            if (argList.contains(OPT_STATUS)) {
                waitForMsg_ = true;
                msg_ = argList.at(argList.indexOf(OPT_STATUS) + 1);
            }
            // Start OAuth
            tweeter_.doOAuth();
        } else if (argList.contains(OPT_XAUTH)) {
            if (!(argList.contains(OPT_USERNAME) && argList.contains(OPT_PASSWORD))) {
                qDebug() << "\nError: Username or Password missing!";
                qDebug() << helpText;
                qApp->exit(1);
            }

            QString username = argList.at(argList.indexOf(OPT_USERNAME) + 1);
            QString password = argList.at(argList.indexOf(OPT_PASSWORD) + 1);

            if (argList.contains(OPT_STATUS)) {
                waitForMsg_ = true;
                msg_ = argList.at(argList.indexOf(OPT_STATUS) + 1);
            }
            // Start XAuth
            tweeter_.doXAuth(username, password);
        } else if (argList.contains(OPT_STATUS)) {
            QString statusMessage = argList.at(argList.indexOf(OPT_STATUS) + 1);
            postStatusUpdate(statusMessage);
        } else {
            qDebug() << helpText;
            qApp->exit(1);
        }
    }

    void onLinkingFailed() {
        qDebug() << "Linking failed!";
        qApp->exit(1);
    }

    void onLinkingSucceeded() {
        qDebug() << "Linking succeeded!";
        if (waitForMsg_) {
            postStatusUpdate(msg_);
        } else {
            qApp->quit();
        }
    }

private slots:

    void postStatusUpdate(const QString& msg) {
        connect(&tweeter_, SIGNAL(statusPosted()),
                qApp, SLOT(quit()));
        tweeter_.postStatusUpdate(msg);
    }

private:
    Tweeter tweeter_;
    bool waitForMsg_;
    QString msg_;
};

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    Helper helper;
    QTimer::singleShot(0, &helper, SLOT(processArgs()));

    return a.exec();
}

#include "main.moc"
