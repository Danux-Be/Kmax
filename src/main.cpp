// SPDX-FileCopyrightText: 2026 Dany Petit <danypetit.be@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later
#include <kmax_version.h>
#include "ui/mainwindow.h"

#include <KAboutData>
#include <KCrash>
#include <KDBusService>
#include <KLocalizedString>

#include <QApplication>
#include <QCommandLineParser>
#include <QIcon>
#include <QUrl>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    KLocalizedString::setApplicationDomain("kmax");
    QApplication::setWindowIcon(QIcon::fromTheme(QStringLiteral("kmax")));

    KAboutData about(
        QStringLiteral("kmax"),
        i18n("Kmax"),
        QStringLiteral(KMAX_VERSION_STRING),
        i18n("KDE video enhancement and AI upscaling application"),
        KAboutLicense::GPL_V3,
        i18n("© 2026 Dany Petit"),
        QString(),
        QStringLiteral("https://github.com/Danux-Be/Kmax"));
    about.addAuthor(i18n("Dany Petit"),
                    i18n("Maintainer"),
                    QStringLiteral("danypetit.be@gmail.com"));
    about.setHomepage(QStringLiteral("https://github.com/Danux-Be/Kmax"));
    about.setBugAddress("https://github.com/Danux-Be/Kmax/issues");
    KAboutData::setApplicationData(about);

    KCrash::initialize();

    QCommandLineParser parser;
    about.setupCommandLine(&parser);
    parser.addPositionalArgument(QStringLiteral("files"),
                                 i18n("Video files to enqueue at startup."),
                                 QStringLiteral("[files...]"));
    parser.process(app);
    about.processCommandLine(&parser);

    KDBusService service(KDBusService::Unique);

    auto *window = new Kmax::MainWindow;
    window->show();

    return app.exec();
}
