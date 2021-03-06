#include "watcher.h"

Watcher::Watcher(QObject *parent) :
    WatcherBase(parent) {
    watcher = new QFileSystemWatcher(this);

    trayIconOff = QIcon(":/icons/res_tray_icon_black.png");
    trayIconOn  = QIcon(":/icons/res_tray_icon_color.png");
    trayMenu = new QSystemTrayIcon(this);
    connect(trayMenu, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), SLOT(trayActivated(QSystemTrayIcon::ActivationReason)));
    trayIconToOff();
    QMenu *trayElements = new QMenu(Global::mainWindow);
    trayElements->addAction(tr("Show timeline"), this, SLOT(trayActivateApp()));
    trayElements->addAction(tr("Write a note"),  this, SLOT(writeNote()));
    trayMenu->setContextMenu(trayElements);
    trayMenu->show();

    feeling = new WatcherFeeling();
    feeling->setStyleSheet(Global::mainWindow->styleSheet());
}

void Watcher::trayActivateApp() {
    Global::mainWindow->setVisbility(!Global::mainWindow->isVisible());
}

void Watcher::writeNote() {
    trayIconToOn();
}
void Watcher::trayIconToOn(Document *document) {
    if(!document)
        takeTemporarySnapshot();
    trayMenu->setIcon(trayIconOn);
    QTimer::singleShot(1000, this, SLOT(trayIconToOff()));
    feeling->display(document);
}
void Watcher::trayIconToOff() {
    trayMenu->setIcon(trayIconOff);
}

void Watcher::trayActivated(QSystemTrayIcon::ActivationReason) {
    //Global::mainWindow->show();
    //Global::mainWindow->raise();
}


void Watcher::sync(const QString &file, bool inTracker) {
    if((!watcher->directories().contains(file)) && (!watcher->files().contains(file))) {
        watcher->addPath(file);
        if(inTracker)
            watcherTracking.append(file);
    }

    /*
    qDebug("------------------------------------");
    foreach(const QString &files, watcher->directories())
        qDebug("%d %s", watcherTracking.contains(files)==true, qPrintable(files));
    foreach(const QString &files, watcher->files())
        qDebug("%d %s", watcherTracking.contains(files)==true, qPrintable(files));
    qDebug("------------------------------------");
    */
}
void Watcher::unsync(const QString &file, bool inTracker) {
    if((watcher->directories().contains(file)) || (watcher->files().contains(file))) {
        watcher->removePath(file);
        if(inTracker)
            watcherTracking.removeOne(file);
    }
}

void Watcher::takeTemporarySnapshot() {
    QRect screenSize = QApplication::desktop()->screenGeometry();
    if(lastScreenshotTimestamp.secsTo(QDateTime::currentDateTime()) > 3) {
        lastScreenshot = QPixmap();
#ifdef Q_OS_MAC
        CGRect screenRect;
        screenRect.origin.x    = screenSize.x();
        screenRect.origin.y    = screenSize.y();
        screenRect.size.width  = screenSize.width();
        screenRect.size.height = screenSize.height();
        CGImageRef screenshotMac = CGWindowListCreateImage(screenRect, kCGWindowListOptionOnScreenOnly, kCGNullWindowID, kCGWindowImageDefault);
#ifdef QT4
        lastScreenshot = QPixmap::fromMacCGImageRef(screenshotMac);
#endif
        CGImageRelease(screenshotMac);
#endif
        if(lastScreenshot.isNull())
            lastScreenshot = QPixmap::grabWindow(QApplication::desktop()->winId(), screenSize.x(), screenSize.y(), screenSize.width(), screenSize.height());
        lastScreenshotTimestamp = QDateTime::currentDateTime();
    }
    Global::temporaryScreenshot = lastScreenshot.toImage();
}

void Watcher::fileWatcherDirChanged(QString dir) {
    if(watcherTracking.contains(dir)) {
        Document *document = ((Project*)Global::currentProject)->getDocument(dir);
        if(document) {
            //trayIconToOn(document);
            //qDebug("DIR %s CHANGED", qPrintable(dir));
        }
    }
}
void Watcher::fileWatcherFileChanged(QString file) {
    if(watcherTracking.contains(file)) {
        Document *document = ((Project*)Global::currentProject)->getDocument(file);
        if(document) {
            if(document->getMetadata("File", "Hash").toString() != Global::getFileHash(file)) {
                takeTemporarySnapshot();
                trayIconToOn(document);
            }
        }
    }
}
