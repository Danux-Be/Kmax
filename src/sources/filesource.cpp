// SPDX-FileCopyrightText: 2026 Dany Petit <danypetit.be@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later
#include "filesource.h"

#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMimeDatabase>
#include <QProcess>
#include <QStandardPaths>

namespace Kmax {

FileSource::FileSource(QObject *parent)
    : ISource(parent)
{
}

QStringList FileSource::supportedSchemes() const
{
    return {QStringLiteral("file")};
}

QStringList FileSource::supportedExtensions() const
{
    return {
        QStringLiteral("avi"), QStringLiteral("mp4"), QStringLiteral("mkv"),
        QStringLiteral("mov"), QStringLiteral("webm"), QStringLiteral("m4v"),
        QStringLiteral("flv"), QStringLiteral("mpg"), QStringLiteral("mpeg"),
        QStringLiteral("ts"), QStringLiteral("m2ts"), QStringLiteral("wmv"),
        QStringLiteral("vob"), QStringLiteral("3gp"),
    };
}

bool FileSource::canHandle(const QUrl &uri) const
{
    if (!uri.isLocalFile()) {
        return false;
    }
    const auto ext = QFileInfo(uri.toLocalFile()).suffix().toLower();
    return supportedExtensions().contains(ext);
}

QList<SourceItem> FileSource::probe(const QUrl &uri)
{
    const QString path = uri.toLocalFile();
    const QString ffprobe = QStandardPaths::findExecutable(QStringLiteral("ffprobe"));
    if (ffprobe.isEmpty()) {
        setLastError(tr("ffprobe not found in PATH"));
        return {};
    }

    QProcess proc;
    const QStringList args{
        QStringLiteral("-v"), QStringLiteral("quiet"),
        QStringLiteral("-print_format"), QStringLiteral("json"),
        QStringLiteral("-show_format"),
        QStringLiteral("-show_streams"),
        path,
    };
    proc.start(ffprobe, args);
    if (!proc.waitForFinished(15000) || proc.exitCode() != 0) {
        setLastError(tr("ffprobe failed for %1").arg(path));
        return {};
    }

    const auto doc = QJsonDocument::fromJson(proc.readAllStandardOutput());
    const auto root = doc.object();

    SourceItem item;
    item.id = path;
    item.title = QFileInfo(path).completeBaseName();
    item.uri = uri;
    item.durationMs = qRound(root.value(QStringLiteral("format")).toObject()
                                 .value(QStringLiteral("duration")).toString().toDouble() * 1000.0);

    for (const auto &s : root.value(QStringLiteral("streams")).toArray()) {
        const auto stream = s.toObject();
        if (stream.value(QStringLiteral("codec_type")).toString() == QStringLiteral("video")) {
            item.width = stream.value(QStringLiteral("width")).toInt();
            item.height = stream.value(QStringLiteral("height")).toInt();
            const QString fpsStr = stream.value(QStringLiteral("r_frame_rate")).toString();
            const auto slash = fpsStr.indexOf(QLatin1Char('/'));
            if (slash > 0) {
                const double num = fpsStr.left(slash).toDouble();
                const double den = fpsStr.mid(slash + 1).toDouble();
                if (den > 0) {
                    item.fps = num / den;
                }
            }
            break;
        }
    }

    return {item};
}

QString FileSource::materialise(const SourceItem &item, const QString &workingDir)
{
    Q_UNUSED(workingDir);
    return item.uri.toLocalFile();
}

} // namespace Kmax
