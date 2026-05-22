// SPDX-FileCopyrightText: 2026 Dany Petit <danypetit.be@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later
#include "dvdsource.h"

#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <QFileInfo>

extern "C" {
#include <dvdread/dvd_reader.h>
#include <dvdread/ifo_read.h>
#include <dvdread/ifo_types.h>
}

namespace {
constexpr int DVDBlockSize = 2048;
}

namespace Kmax {

DvdSource::DvdSource(QObject *parent)
    : ISource(parent)
{
}

QStringList DvdSource::supportedSchemes() const
{
    return {QStringLiteral("file"), QStringLiteral("dvd")};
}

bool DvdSource::canHandle(const QUrl &uri) const
{
    if (uri.scheme() == QLatin1String("dvd")) {
        return true;
    }
    if (!uri.isLocalFile()) {
        return false;
    }
    const QFileInfo fi(uri.toLocalFile());
    if (fi.isDir()) {
        return QDir(fi.absoluteFilePath()).exists(QStringLiteral("VIDEO_TS"))
            || fi.fileName().compare(QStringLiteral("VIDEO_TS"), Qt::CaseInsensitive) == 0;
    }
    if (fi.isFile() && fi.suffix().compare(QStringLiteral("iso"), Qt::CaseInsensitive) == 0) {
        return true;
    }
    return false;
}

QList<SourceItem> DvdSource::probe(const QUrl &uri)
{
    QString path = uri.toLocalFile();
    if (QFileInfo(path).fileName().compare(QStringLiteral("VIDEO_TS"), Qt::CaseInsensitive) == 0) {
        path = QFileInfo(path).absolutePath();
    }
    dvd_reader_t *dvd = DVDOpen(path.toUtf8().constData());
    if (!dvd) {
        setLastError(tr("Could not open DVD at %1").arg(path));
        return {};
    }

    QList<SourceItem> items;
    ifo_handle_t *vmg = ifoOpen(dvd, 0);
    if (!vmg) {
        setLastError(tr("Could not read DVD VMG"));
        DVDClose(dvd);
        return {};
    }
    const int titleCount = vmg->tt_srpt ? vmg->tt_srpt->nr_of_srpts : 0;
    for (int i = 1; i <= titleCount; ++i) {
        title_info_t *t = &vmg->tt_srpt->title[i - 1];
        ifo_handle_t *vts = ifoOpen(dvd, t->title_set_nr);
        if (!vts) {
            continue;
        }
        SourceItem item;
        item.id = path + QStringLiteral("#title=") + QString::number(i);
        item.title = tr("Title %1").arg(i);
        item.uri = QUrl::fromLocalFile(path);

        if (vts->vts_pgcit && vts->vts_pgcit->nr_of_pgci_srp > 0) {
            const auto *pgc = vts->vts_pgcit->pgci_srp[t->vts_ttn - 1].pgc;
            if (pgc) {
                const auto &dt = pgc->playback_time;
                const int frames = dt.frame_u & 0x3f;
                const int rate = ((dt.frame_u & 0xc0) >> 6) == 1 ? 25 : 30;
                item.fps = double(rate);
                item.durationMs =
                    qint64((dt.hour >> 4) * 10 + (dt.hour & 0x0f)) * 3600000
                    + qint64((dt.minute >> 4) * 10 + (dt.minute & 0x0f)) * 60000
                    + qint64((dt.second >> 4) * 10 + (dt.second & 0x0f)) * 1000
                    + qint64(frames) * 1000 / rate;
            }
        }
        if (vts->vtsi_mat) {
            item.width = vts->vtsi_mat->vts_video_attr.picture_size == 0 ? 720 : 720;
            item.height = vts->vtsi_mat->vts_video_attr.video_format == 0 ? 480 : 576;
        }
        item.extra.insert(QStringLiteral("titleNumber"), i);
        item.extra.insert(QStringLiteral("vtsNumber"), int(t->title_set_nr));
        item.extra.insert(QStringLiteral("dvdPath"), path);
        items << item;
        ifoClose(vts);
    }
    ifoClose(vmg);
    DVDClose(dvd);
    return items;
}

QString DvdSource::materialise(const SourceItem &item, const QString &workingDir)
{
    const QString path = item.extra.value(QStringLiteral("dvdPath")).toString();
    const int titleNumber = item.extra.value(QStringLiteral("titleNumber")).toInt();
    if (path.isEmpty() || titleNumber <= 0) {
        setLastError(tr("DVD item missing path/title metadata"));
        return {};
    }

    dvd_reader_t *dvd = DVDOpen(path.toUtf8().constData());
    if (!dvd) {
        setLastError(tr("Could not open DVD at %1").arg(path));
        return {};
    }
    ifo_handle_t *vmg = ifoOpen(dvd, 0);
    if (!vmg) {
        DVDClose(dvd);
        setLastError(tr("Could not read DVD VMG"));
        return {};
    }
    title_info_t *t = &vmg->tt_srpt->title[titleNumber - 1];
    dvd_file_t *file = DVDOpenFile(dvd, t->title_set_nr, DVD_READ_TITLE_VOBS);
    if (!file) {
        ifoClose(vmg);
        DVDClose(dvd);
        setLastError(tr("Could not open VOB stream"));
        return {};
    }

    const QString hash = QString::fromLatin1(
        QCryptographicHash::hash(item.id.toUtf8(), QCryptographicHash::Sha1).toHex().left(12));
    const QString outDir = workingDir + QStringLiteral("/dvd-rip/") + hash;
    QDir().mkpath(outDir);
    const QString outPath = outDir + QStringLiteral("/title.vob");

    QFile out(outPath);
    if (!out.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        DVDCloseFile(file);
        ifoClose(vmg);
        DVDClose(dvd);
        setLastError(tr("Could not write %1").arg(outPath));
        return {};
    }

    constexpr int blocksPerRead = 64;
    QByteArray buffer(blocksPerRead * DVDBlockSize, Qt::Uninitialized);
    int block = 0;
    const ssize_t totalBlocks = DVDFileSize(file);
    while (true) {
        const ssize_t got = DVDReadBlocks(file, block, blocksPerRead,
                                          reinterpret_cast<unsigned char *>(buffer.data()));
        if (got <= 0) {
            break;
        }
        out.write(buffer.constData(), got * DVDBlockSize);
        block += got;
        if (totalBlocks > 0) {
            Q_EMIT progress(int(qMin<qint64>(99, qint64(block) * 100 / totalBlocks)),
                            tr("Ripping title %1…").arg(titleNumber));
        }
        if (got < blocksPerRead) {
            break;
        }
    }

    out.close();
    DVDCloseFile(file);
    ifoClose(vmg);
    DVDClose(dvd);
    Q_EMIT progress(100, tr("Title ripped"));
    return outPath;
}

} // namespace Kmax
