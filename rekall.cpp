#include "rekall.h"
#include "ui_rekall.h"

Rekall::Rekall(QWidget *parent) :
    RekallBase(parent),
    ui(new Ui::Rekall) {
    ui->setupUi(this);
    setAcceptDrops(true);
    currentProject  = 0;
    currentDocument = 0;
    currentMetadata = 0;

    metaIsChanging = chutierIsUpdating = false;
    openProject    = true;

    Global::userInfos = new UserInfos();

    Global::mainWindow = this;
    QApplication::setQuitOnLastWindowClosed(false);

    Watcher *watcher  = new Watcher(this);
    Global::watcher   = watcher;
    Global::taskList->setToolbox(ui->toolBoxLeft);
    Global::feedList->setToolbox(ui->toolBoxLeft);

    Global::chutier = ui->chutier->getTree();
    Global::udp = new Udp(0, 5678);
    settings = new QSettings(Global::pathApplication.absoluteFilePath() + "/Rekall.ini", QSettings::IniFormat, this);
    if(true) {
        httpUpload = new FileUploadController();
        settings->beginGroup("listener");
        http = new HttpListener(settings, httpUpload, this);
        connect(httpUpload, SIGNAL(fileUploaded(QString,QString,QString)), SLOT(fileUploaded(QString,QString,QString)));
    }

    Global::font.setFamily("Calibri");
    Global::font.setPixelSize(11);
    Global::fontSmall.setFamily("Calibri");
    Global::fontSmall.setPixelSize(9);

    UiFileItem::configure(ui->chutier, false);
    UiFileItem::forbiddenDirs << "rekall_cache";

    currentProject = new Project(this);
    Global::currentProject = currentProject;
    connect(currentProject, SIGNAL(refreshMetadata()), SLOT(refreshAndLastMetadata()));

    Global::phases                = new Phases(0);
    Global::tagFilterCriteria     = new Sorting(tr("Filter rules"),     6);
    Global::tagSortCriteria       = new Sorting(tr("Sorting rules"),    2);
    Global::tagColorCriteria      = new Sorting(tr("Color rules"),      5);
    Global::tagTextCriteria       = new Sorting(tr("Text rules"),       0);
    Global::tagClusterCriteria    = new Sorting(tr("Hightlight rules"), 7, true);
    Global::tagHorizontalCriteria = new Sorting(tr("Horizontal rules"), 1, false, true);

    QString styleAdditionnal = "QWidget#Sorting, QWidget#Phases, QWidget#TimelineControl { background-color: transparent;}";
    Global::tagFilterCriteria    ->setStyleSheet(styleSheet() + styleAdditionnal);
    Global::tagSortCriteria      ->setStyleSheet(styleSheet() + styleAdditionnal);
    Global::tagColorCriteria     ->setStyleSheet(styleSheet() + styleAdditionnal);
    Global::tagTextCriteria      ->setStyleSheet(styleSheet() + styleAdditionnal);
    Global::tagClusterCriteria   ->setStyleSheet(styleSheet() + styleAdditionnal);
    Global::tagHorizontalCriteria->setStyleSheet(styleSheet() + styleAdditionnal);
    Global::phases               ->setStyleSheet(styleSheet() + styleAdditionnal);
    Global::phases               ->setStyleSheet2(Global::tagFilterCriteria->styleSheet2());
    ui->timeline->timelineControl->setStyleSheet(styleSheet() + styleAdditionnal);
    ui->timeline->timelineControl->setStyleSheet2(Global::tagFilterCriteria->styleSheet2());

    connect(Global::phases,                SIGNAL(displayed(bool)), (Timeline*)Global::timeline, SLOT(actionDisplayed(bool)));
    connect(Global::tagFilterCriteria,     SIGNAL(displayed(bool)), (Timeline*)Global::timeline, SLOT(actionDisplayed(bool)));
    connect(Global::tagSortCriteria,       SIGNAL(displayed(bool)), (Timeline*)Global::timeline, SLOT(actionDisplayed(bool)));
    connect(Global::tagColorCriteria,      SIGNAL(displayed(bool)), (Timeline*)Global::timeline, SLOT(actionDisplayed(bool)));
    connect(Global::tagTextCriteria,       SIGNAL(displayed(bool)), (Timeline*)Global::timeline, SLOT(actionDisplayed(bool)));
    connect(Global::tagClusterCriteria,    SIGNAL(displayed(bool)), (Timeline*)Global::timeline, SLOT(actionDisplayed(bool)));
    connect(Global::tagHorizontalCriteria, SIGNAL(displayed(bool)), (Timeline*)Global::timeline, SLOT(actionDisplayed(bool)));
    connect(ui->timeline->timelineControl, SIGNAL(displayed(bool)), (Timeline*)Global::timeline, SLOT(actionDisplayed(bool)));

    connect(Global::phases,                SIGNAL(actionned(QString,QString)), (Timeline*)Global::timeline, SLOT(actionChanged(QString,QString)));
    connect(Global::tagFilterCriteria,     SIGNAL(actionned(QString,QString)), (Timeline*)Global::timeline, SLOT(actionChanged(QString,QString)));
    connect(Global::tagSortCriteria,       SIGNAL(actionned(QString,QString)), (Timeline*)Global::timeline, SLOT(actionChanged(QString,QString)));
    connect(Global::tagColorCriteria,      SIGNAL(actionned(QString,QString)), (Timeline*)Global::timeline, SLOT(actionChanged(QString,QString)));
    connect(Global::tagTextCriteria,       SIGNAL(actionned(QString,QString)), (Timeline*)Global::timeline, SLOT(actionChanged(QString,QString)));
    connect(Global::tagClusterCriteria,    SIGNAL(actionned(QString,QString)), (Timeline*)Global::timeline, SLOT(actionChanged(QString,QString)));
    connect(Global::tagHorizontalCriteria, SIGNAL(actionned(QString,QString)), (Timeline*)Global::timeline, SLOT(actionChanged(QString,QString)));
    Global::phases               ->action();
    Global::tagFilterCriteria    ->action();
    Global::tagSortCriteria      ->action();
    Global::tagColorCriteria     ->action();
    Global::tagTextCriteria      ->action();
    Global::tagClusterCriteria   ->action();
    Global::tagHorizontalCriteria->action();


    Global::showHelp.setAction(ui->actionInlineHelp);
    connect(ui->actionSave, SIGNAL(triggered()), SLOT(action()));
    connect(ui->actionPaste, SIGNAL(triggered()), SLOT(action()));
    connect(ui->actionRemove, SIGNAL(triggered()), SLOT(action()));
    connect(ui->actionMarker, SIGNAL(triggered(bool)), SLOT(action()));
    connect(ui->actionMarkerLong, SIGNAL(triggered(bool)), SLOT(action()));
    connect(&Global::showHelp, SIGNAL(triggered(bool)), SLOT(showHelp(bool)));

    ui->metadata->setColumnWidth(0, 135);
    ui->metadataSlider->setVisible(false);
    ui->metadataSliderIcon ->setVisible(ui->metadataSlider->isVisible());
    ui->metadataSliderIcon2->setVisible(ui->metadataSlider->isVisible());
    HtmlDelegate *delegate0 = new HtmlDelegate(false);
    HtmlDelegate *delegate1 = new HtmlDelegate(true);
    connect(delegate1, SIGNAL(closeEditor(QWidget*)), SLOT(actionMetadata()));
    ui->metadata->setItemDelegateForColumn(0, delegate0);
    ui->metadata->setItemDelegateForColumn(1, delegate1);
    //ui->metadata->setItemDelegateForColumn(options.index, new UiTreeDelegate(options, ui->view->model(), this));

    ui->chutier->showNew(false);
    ui->chutier->showImport(false);
    ui->chutier->showRemove(false);
    ui->chutier->showDuplicate(false);
    connect(ui->chutier->getTree(), SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)), SLOT(chutierItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)));
    connect(ui->actionPlay, SIGNAL(triggered()), ui->timeline, SLOT(actionPlay()));
    connect(ui->actionRewind, SIGNAL(triggered()), ui->timeline, SLOT(actionRewind()));

    personsTreeWidget = ui->persons;
    personsTreeWidget->setColumnWidth(0, 40);
    personsTreeWidget->setColumnWidth(1, 100);
    personsTreeWidget->setColumnWidth(2, 100);
    personsTreeWidget->setColumnWidth(3, 100);


    //GPS
    ui->metadataOpenGps->setVisible(false);
    ui->metadataOpenFinder->setVisible(false);
    ui->metadataOpen->setVisible(false);
    gps = new QWebView(0);
    gps->setWindowFlags(Qt::Tool);
    gps->load(QUrl::fromLocalFile(Global::pathApplication.absoluteFilePath() + "/tools/maps.html"));


    //Display splash
    splash = new Splash();
    splash->show();
    QTimer::singleShot(1500, this, SLOT(closeSplash()));
    QCoreApplication::processEvents();

    startTimer(1000);
}

Rekall::~Rekall() {
    settings->sync();
    delete ui;
}

void Rekall::fileUploaded(const QString &gpsCoord, const QString &filename, const QString &file) {
    QFileInfo fileInfo(file);
    qDebug("Upload %s @ %s = %s (%d)", qPrintable(gpsCoord), qPrintable(filename), qPrintable(fileInfo.absoluteFilePath()), fileInfo.exists());
    Document *document = new Document(Global::currentProject);
    document->updateFile(fileInfo);

    displayDocumentName(tr("Imported file"));
    displayPixmap(DocumentTypeImage, document->file.fileName(), QPixmap(document->file.fileName()));
    displayGps(QList< QPair<QString,QString> >() << qMakePair(gpsCoord, tr("Imported file")));
}

void Rekall::dragEnterEvent(QDragEnterEvent *event) {
    if(parseMimeData(event->mimeData(), "rekall", true))
        return event->acceptProposedAction();
}
void Rekall::dropEvent(QDropEvent *event) {
    if(parseMimeData(event->mimeData(), "rekall"))
        return event->acceptProposedAction();
}
bool Rekall::parseMimeData(const QMimeData *mime, const QString &source, bool test) {
    bool retour = false;
    QFileInfoList    droppedFiles;
    QList<Document*> droppedDocuments;
    QList<Person*>   droppedPersons;

    if(mime->hasFormat("application/x-qabstractitemmodeldatalist")) {
        QByteArray encoded = mime->data("application/x-qabstractitemmodeldatalist");
        QDataStream stream(&encoded, QIODevice::ReadOnly);
        while(!stream.atEnd()) {
            int row, col;
            QMap<int,  QVariant> roleDataMap;
            stream >> row >> col >> roleDataMap;
            QMapIterator<int, QVariant> roleDataMapIterator(roleDataMap);
            while(roleDataMapIterator.hasNext()) {
                roleDataMapIterator.next();
                QFileInfo file = QFileInfo(roleDataMapIterator.value().toString());
                if((col == 2) && (file.exists())) {
                    droppedFiles << file;
                    Document *document = currentProject->getDocument(file.absoluteFilePath());
                    if(document)
                        droppedDocuments << document;
                }
            }
        }
    }
    foreach(const QUrl &url, mime->urls()) {
        QFileInfo file(url.toLocalFile());
        if(file.exists()) {
            if(file.suffix().toLower() == "vcf")
                droppedPersons.append(Person::fromFile(file.absoluteFilePath(), Global::mainWindow));
            else {
                Document *document = currentProject->getDocument(file.absoluteFilePath());
                if(document)    droppedDocuments << document;
                else            droppedFiles     << file;
            }
        }
    }
    foreach(Document *droppedDocument, droppedDocuments)
        droppedFiles.removeOne(droppedDocument->file);
    if(mime->hasFormat("text/plain"))
        droppedPersons.append(Person::fromString(mime->data("text/plain"), Global::mainWindow));

    qDebug("--------------- %s -------------------", qPrintable(source));
    qDebug("Texte > %s", qPrintable(mime->text()));
    qDebug("Html  > %s", qPrintable(mime->html()));
    foreach(const QUrl &url, mime->urls())
        qDebug("URL> %s", qPrintable(url.toString()));
    foreach(const QString &format, mime->formats())
        qDebug("Data %s > %s", qPrintable(format), qPrintable(QString(mime->data(format))));
    foreach(const QFileInfo &file, droppedFiles)
        qDebug("File item  > %s", qPrintable(file.absoluteFilePath()));
    foreach(Document *document, droppedDocuments)
        qDebug("Document item > %s", qPrintable(document->file.absoluteFilePath()));
    foreach(Person *person, droppedPersons)
        qDebug("Person > %s", qPrintable(person->getFullname()));
    qDebug("--------------- %s -------------------", qPrintable(source));



    if(source == "rekall") {
        if(test) {
            if(droppedPersons.count())
                retour = true;
        }
        else {
            foreach(Person *droppedPerson, droppedPersons) {
                currentProject->addPerson(droppedPerson);
                retour = true;
            }
        }
    }
    else if(source == "timeline") {
        if(test) {
            if((droppedDocuments.count()) || (droppedFiles.count()))
                retour = true;
        }
        else {
            if(droppedFiles.count())
                currentProject->open(droppedFiles, ui->chutier);

            foreach(const QFileInfo &droppedFile, droppedFiles) {
                Document *document = currentProject->getDocument(droppedFile.absoluteFilePath());
                if(document)
                    droppedDocuments << document;
            }
            foreach(Document *droppedDocument, droppedDocuments) {
                droppedDocument->createTag(TagTypeContextualTime, currentProject->getTimelineCursorTime(Global::timelineGL->mapFromGlobal(QCursor::pos()) + Global::timelineGL->scroll), 5);
                if(droppedDocument->chutierItem)
                    Global::chutier->setCurrentItem(droppedDocument->chutierItem);
                retour = true;
            }
            Global::timelineSortChanged = Global::viewerSortChanged = Global::eventsSortChanged = Global::phases->needCalulation = true;
        }
    }
    return retour;
}





void Rekall::action() {
    if(sender() == ui->actionSave)
        currentProject->save();
    else if(sender() == ui->actionPaste)
        parseMimeData(QApplication::clipboard()->mimeData(), "rekall");
    else if(sender() == ui->metadataOpenGps)
        gps->show();
    else if((sender() == ui->metadataOpen) && (currentMetadata)) {
        if(currentMetadata->chutierItem)
            currentMetadata->chutierItem->fileShowInOS();
        else
            UiFileItem::fileShowInOS(currentMetadata->getMetadata("Rekall", "URL").toString());
    }
    else if((sender() == ui->metadataOpenFinder) && (currentMetadata)) {
        if(currentMetadata->chutierItem)
            currentMetadata->chutierItem->fileShowInFinder();
        else if(currentMetadata->thumbnails.count())
            UiFileItem::fileShowInFinder(currentMetadata->thumbnails.first().currentFilename);
    }
    else if(sender() == ui->actionMarker) {
        Global::timeline->actionMarkerAddStart();
        Global::timeline->actionMarkerAddEnd();
    }
    else if(sender() == ui->actionMarkerLong) {
        Global::timeline->actionMarkerAddStart();
    }
    else if(sender() == ui->actionRemove) {
        if(Global::selectedTag) {
            Tag *tag = (Tag*)Global::selectedTag;
            tag->getDocument()->removeTag(tag);
            Global::timelineSortChanged = Global::viewerSortChanged = Global::eventsSortChanged = Global::phases->needCalulation = true;
            Global::selectedTag = Global::selectedTagHover = Global::selectedTagInAction = 0;
        }
    }
}
void Rekall::actionForceGL() {
    ui->toolBoxRight->resize(ui->toolBoxRight->width()+1, ui->toolBoxRight->height());
}

void Rekall::actionMetadata() {
    if((currentMetadata) && (ui->metadata->currentItem()) && (ui->metadata->currentItem()->parent())) {
        QString key      = ui->metadata->currentItem()->text(0).remove(QRegExp("<[^>]*>")).trimmed();
        QString value    = ui->metadata->currentItem()->text(1).remove(QRegExp("<[^>]*>")).trimmed();
        QString category = ui->metadata->currentItem()->parent()->text(0).remove(QRegExp("<[^>]*>")).trimmed();
        if(category == "General")   category = "Rekall";
        currentMetadata->setMetadata(category, key, value, ui->metadataSlider->value());
        Global::timelineSortChanged = Global::viewerSortChanged = Global::eventsSortChanged = Global::metaChanged = Global::phases->needCalulation = true;
    }
}
void Rekall::actionMetadata(QTreeWidgetItem *item, int) {
    if((!metaIsChanging) && (item) && (currentMetadata)) {
        QString category, meta, value;
        if(item->parent())
            category = item->parent()->text(0);
        meta  = item->text(0);
        value = item->text(1);
    }
}

void Rekall::closeSplash() {
    splash->close();
    showMaximized();
    updateGeometry();
    ui->timelineSplitter->setSizes(QList<int>() << ui->timelineSplitter->height() * 0.50 << ui->timelineSplitter->height() * 0.50);
    ui->fileSplitter->setSizes(QList<int>()     << 200                                   << ui->fileSplitter->width()      - 200);
    ui->conduiteSplitter->setSizes(QList<int>() << ui->conduiteSplitter->width()  - 300  << 300);
    //trayMenu->showMessage("Rekall", "Ready!", QSystemTrayIcon::NoIcon);
}




void Rekall::refreshMetadata() {
    if(!metaIsChanging) {
        chutierIsUpdating = true;
        chutierItemChanged(ui->chutier->getTree()->currentItem(), ui->chutier->getTree()->currentItem());

        //Changemenent de méta
        if(currentDocument) {
            foreach(Tag *documentTag, currentDocument->tags) {
                if(documentTag->getDocumentVersion() == ui->metadataSlider->value()) {
                    Global::selectedTag = documentTag;
                    Global::timelineGL->ensureVisible(documentTag->getTimelineBoundingRect().translated(documentTag->timelineDestPos).topLeft());
                    Global::viewerGL  ->ensureVisible(documentTag->getViewerBoundingRect().translated(documentTag->viewerDestPos).topLeft());
                }
            }
        }
        chutierIsUpdating = false;
    }
}
void Rekall::refreshAndLastMetadata() {
    if(!metaIsChanging) {
        chutierIsUpdating = true;
        chutierItemChanged(ui->chutier->getTree()->currentItem(), 0);
        chutierIsUpdating = false;
    }
}
void Rekall::refreshMetadata(void *_tag, bool inChutier) {
    Tag *tag = (Tag*)_tag;
    if(inChutier) {
        chutierIsUpdating = true;
        Global::chutier->setCurrentItem(tag->getDocument()->chutierItem);
        chutierItemChanged(ui->chutier->getTree()->currentItem(), ui->chutier->getTree()->currentItem(), tag);
        chutierIsUpdating = false;
    }
    else
        displayMetadata(tag->getDocument(), tag, ui->chutier->getTree(), 0, 0);
}
void Rekall::chutierItemChanged(QTreeWidgetItem *item, QTreeWidgetItem *itemB, Tag *tag) {
    if(item) {
        if(tag) currentDocument = (Document*)tag->getDocument();
        else    currentDocument = currentProject->getDocumentAndSelect(((UiFileItem*)item)->filename.file.absoluteFilePath());
        displayMetadata(currentDocument, tag, ui->chutier->getTree(), item, itemB);

        if((currentMetadata) && (sender() == ui->chutier->getTree()))
            ui->metadataSlider->setValue(currentMetadata->getMetadataCountM());

        if((currentDocument) && (!chutierIsUpdating)) {
            foreach(Tag *documentTag, currentDocument->tags) {
                if(documentTag->getDocumentVersion() == ui->metadataSlider->value()) {
                    Global::selectedTag = documentTag;
                    Global::timelineGL->ensureVisible(documentTag->getTimelineBoundingRect().translated(documentTag->timelineDestPos).topLeft());
                    Global::viewerGL  ->ensureVisible(documentTag->getViewerBoundingRect().translated(documentTag->viewerDestPos).topLeft());
                }
            }
        }
    }
}
void Rekall::showPreviewTab() {
    ui->toolBoxRight->setCurrentIndex(1);
}


void Rekall::personItemChanged(QTreeWidgetItem *item, QTreeWidgetItem *itemB) {
    if(item)
        displayMetadata((Person*)item, 0, ui->chutier->getTree(), item, itemB);
}
void Rekall::displayMetadata(Metadata *metadata, Tag *tag, QTreeWidget *tree, QTreeWidgetItem *item, QTreeWidgetItem *) {
    metaIsChanging = true;
    currentMetadata = metadata;

    //Standard operations
    if((tree) && (item))
        tree->setCurrentItem(item);

    QStringList expandItems;

    QString metadataPrefix0 = "<span style='font-family: Calibri, Arial; font-size: 11px; color: #F5F8FA'>";
    QString metadataPrefix1 = "<span style='font-family: Calibri, Arial; font-size: 11px; color: %1'>";
    QString metadataSuffix = "</span>";


    for(quint16 i = 0 ; i < ui->metadata->topLevelItemCount() ; i++) {
        if(ui->metadata->topLevelItem(i)->isExpanded())
            expandItems << ui->metadata->topLevelItem(i)->text(0).remove(QRegExp("<[^>]*>")).trimmed();
        for(quint16 j = 0 ; j < ui->metadata->topLevelItem(i)->childCount() ; j++) {
            if(ui->metadata->topLevelItem(i)->child(j)->isExpanded())
                expandItems << ui->metadata->topLevelItem(i)->child(j)->text(0).remove(QRegExp("<[^>]*>")).trimmed();
        }
    }
    if(expandItems.count() == 0)
        expandItems << "General" << "Contact details";

    if(metadata) {
        ui->metadata->clear();

        QTreeWidgetItem *metadataRootItem = new QTreeWidgetItem(ui->metadata->invisibleRootItem(), QStringList() << metadataPrefix0 + tr("Details") + metadataSuffix);
        metadataRootItem->setFlags(Qt::ItemIsEnabled);
        if(expandItems.contains("Details"))
            ui->metadata->expandItem(metadataRootItem);

        //Versions
        ui->metadataSlider->setMaximum(metadata->getMetadataCountM());
        if(tag) ui->metadataSlider->setValue(tag->getDocumentVersion());

        if(metadata->getMetadataCount() > 1) ui->metadataSlider->setVisible(true);
        else                                 ui->metadataSlider->setVisible(false);
        ui->metadataSliderIcon ->setVisible(ui->metadataSlider->isVisible());
        ui->metadataSliderIcon2->setVisible(ui->metadataSlider->isVisible());

        //Sum up
        QMapIterator<QString, QMetaMap> metaIterator(metadata->getMetadata(ui->metadataSlider->value()));
        while(metaIterator.hasNext()) {
            metaIterator.next();

            QTreeWidgetItem *rootItem = 0;
            QString metaIteratorKey = metaIterator.key();
            if(metaIteratorKey == "Rekall")
                metaIteratorKey = "General";

            if(metaIteratorKey == "General")  rootItem = new QTreeWidgetItem(ui->metadata->invisibleRootItem(), QStringList() << metadataPrefix0 + "General" + metadataSuffix);
            else                              rootItem = new QTreeWidgetItem(metadataRootItem, QStringList() << metadataPrefix0 + metaIteratorKey + metadataSuffix);
            rootItem->setFlags(Qt::ItemIsEnabled);

            QMapIterator<QString,MetadataElement> ssMetaIterator(metaIterator.value());
            while(ssMetaIterator.hasNext()) {
                ssMetaIterator.next();

                QString color = "#000000";
                if(ssMetaIterator.key() == "Name") {
                    color = metadata->baseColor.name();
                    ui->metadataSlider->setStyleSheet(QString("QSlider::handle { background-color: %1; }").arg(color));
                }
                if(color == "#000000")
                    color = "#C8C8C8";
                QTreeWidgetItem *item = new QTreeWidgetItem(rootItem, QStringList()
                                                            << metadataPrefix0 + ssMetaIterator.key() + metadataSuffix
                                                            << metadataPrefix1.arg(color) + ssMetaIterator.value().toString() + metadataSuffix);
                item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled);
            }

            if(expandItems.contains(metaIteratorKey))    ui->metadata->expandItem(rootItem);
            else                                         ui->metadata->collapseItem(rootItem);
        }

        if((metadata->chutierItem) || (metadata->getType() == DocumentTypeWeb)) {
            ui->metadataOpenFinder->setVisible(true);
            ui->metadataOpen      ->setVisible(true);
        }
        else {
            ui->metadataOpenFinder->setVisible(false);
            ui->metadataOpen      ->setVisible(false);
        }

        displayDocumentName(QString("%1 (%2)").arg(metadata->getName()).arg(metadata->getMetadata("Rekall", "Folder").toString()));
        displayPixmap(metadata->getType(), metadata->getThumbnail(ui->metadataSlider->value()));
        displayGps(metadata->getGps());
    }
    metaIsChanging = false;
}
void Rekall::displayDocumentName(const QString &documentName) {
    gps->setWindowTitle(tr("Location — %1").arg(documentName));
}
void Rekall::displayPixmap(DocumentType documentType, const QPair<QString, QPixmap> &picture) {
    displayPixmap(documentType, picture.first, picture.second);
}
void Rekall::displayPixmap(DocumentType documentType, const QString &filename, const QPixmap &picture) {
    ui->preview->setMaximumHeight(ui->toolBoxRight->height() / 2.5);
    ui->preview->preview(documentType, filename, picture);
}

void Rekall::displayGps(const QList< QPair<QString,QString> > &gpsCoords) {
    QString mapsParam;
    for(quint16 i = 0 ; i < gpsCoords.count() ; i++) {
        QPair<QString,QString> gpsCoord = gpsCoords.at(i);
        if(gpsCoord.first.length()) {
            QStringList gpsData = gpsCoord.first.split(",");
            bool isGpsVal = false; gpsData.first().toDouble(&isGpsVal);
            if((isGpsVal) && (gpsData.count() > 1)) mapsParam += QString("['%1',%2,%3],").arg(gpsCoord.second.trimmed()).arg(gpsData.at(0).trimmed()).arg(gpsData.at(1).trimmed());
            else                                    mapsParam += QString("['%1','%2'],").arg(gpsCoord.second.trimmed()).arg(gpsCoord.first.trimmed());
        }
    }
    if(mapsParam.isEmpty())
        ui->metadataOpenGps->setVisible(false);
    else {
        mapsParam.chop(1);
        gps->page()->mainFrame()->evaluateJavaScript("initializeMaps([" + mapsParam + "])");
        ui->metadataOpenGps->setVisible(true);
    }
}



void Rekall::showHelp(bool visible) {
    ui->chutierLabel->setVisible(visible);
    ui->chutierLabel2->setVisible(visible);
    ui->tasksLabel->setVisible(visible);
    ui->feedsLabel->setVisible(visible);
    ui->peopleLabel->setVisible(visible);
    ui->previewLabel->setVisible(visible);
    ui->conduiteLabel->setVisible(visible);
}

void Rekall::timerEvent(QTimerEvent *) {
    if(Global::userInfos->updateDecounter < 0)
        Global::userInfos->update();
    Global::userInfos->updateDecounter--;

    if(openProject) {
        openProject = false;
        currentProject->open(QFileInfoList() << Global::pathCurrent, ui->chutier, true);
        ui->chutier->getTree()->collapseAll();
        for(quint16 i = 0 ; i < ui->chutier->getTree()->topLevelItemCount() ; i++)
            ui->chutier->getTree()->expandItem(ui->chutier->getTree()->topLevelItem(i));
    }

    foreach(Person *person, currentProject->persons)
        person->updateGUI();
}

void Rekall::closeEvent(QCloseEvent *) {
    /*
#ifdef Q_OS_MAC
        ProcessSerialNumber psn;
        if(GetCurrentProcess(&psn) == noErr)
            TransformProcessType(&psn, kProcessTransformToBackgroundApplication);
#endif
    */
}

void Rekall::setVisbility(bool visibility) {
    if(visibility) {
        /*
#ifdef Q_OS_MAC
        ProcessSerialNumber psn;
        if(GetCurrentProcess(&psn) == noErr)
            TransformProcessType(&psn, kProcessTransformToForegroundApplication);
#endifshow
*/
        show();
        raise();
    }
    else {
        close();
    }
}
