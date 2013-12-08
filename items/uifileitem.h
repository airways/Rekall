#ifndef UIFILEITEM_H
#define UIFILEITEM_H

#include <QProcess>
#include <QDir>
#include <QDateTime>
#include <QMessageBox>
#include <QFileSystemWatcher>
#include <QTreeWidgetItem>
#include "uitreeview.h"
#include "misc/options.h"
#include "misc/global.h"

class UiFileItem : public QObject, public UiSyncItem {
    Q_OBJECT

private:
    UiBool       openInFinder, openInOs;
    qint16       currentDepth;
public:
    UiFile filename;
    bool isFile;
    bool isOpened;
    static QIcon iconFile, iconFolder;
    static bool showDateTime;

public:
    explicit UiFileItem(const QFileInfo &file, UiFileItem *_parent, WatcherBase *_watcher);
    QVariant data(int column, int role) const;
    void setData(int column, int role, const QVariant &value);
public:
    virtual void fileOpen()   { emit(askFileOpen());  qDebug("Ouverture du fichier %s", qPrintable(filename.file.absoluteFilePath()));  }
    virtual void fileSave()   { emit(askFileSave());  /*qDebug("Sauvegarde du fichier %s", qPrintable(filename.file.absoluteFilePath()));*/ }
    virtual void fileReload() { emit(askFileReload());/*qDebug("Reload du fichier %s", qPrintable(filename.file.absoluteFilePath()));*/     }
    virtual void fileClose()  { emit(askFileClose()); /*qDebug("Fermeture du fichier %s", qPrintable(filename.file.absoluteFilePath()));*/  }
signals:
    void askFileOpen();
    void askFileSave();
    void askFileReload();
    void askFileClose();
public:
    static const QString dateToString(const QDateTime &date);

public:
    static void syncWith(const QFileInfoList &files, QTreeWidget *treeWidget);
    static void configure(UiTreeView *, bool _showDateTime = true);
    UiFileItem* find(const QFileInfo &search);
    static UiFileItem* find(const QFileInfo &search, QTreeWidget *tree);
protected:
    void syncWith(qint16 depth);
    WatcherBase *watcher;
protected:
    void populate(const QFileInfo &file);
    QStringList syncWith(const QDir &dir, qint16 depth = -1);
    bool highlight(UiSyncItem *item = 0);
    static const QFileInfo fileGetName(const QFileInfo &base);
    static bool fileCopy  (const QFileInfo &source, const QFileInfo &dest);
    static bool fileRename(const QFileInfo &source, const QString &newNameWithoutSuffix, UiFileItem *item = 0);
    static bool fileRename(const QFileInfo &source, const QFileInfo &destination, UiFileItem *item = 0);
public slots:
    void fileShowInFinder();
    void fileShowInOS();
    static void fileShowInFinder(const QString &uri);
    static void fileShowInOS(const QString &uri);
    bool askForImport(UiSyncItem *i, const QFileInfoList &list);
protected slots:
    void dataChanged() { emitDataChanged(); }
    void fileRename();
    void fileWatcherDirChanged(QString);
    void fileWatcherFileChanged(QString);
    bool askForDeletion(UiSyncItem*, bool dialog = false);

public:
    static bool conformFile(const QFileInfo &file);
    static QStringList forbiddenDirs, allowedExtensions;
};

#endif // UIFILEITEM_H
