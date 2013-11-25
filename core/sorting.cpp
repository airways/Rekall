#include "sorting.h"
#include "ui_sorting.h"

Sorting::Sorting(const QString &title, quint16 index, QWidget *parent) :
    QWidget(parent, Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint),
    ui(new Ui::Sorting) {

    regexp.setPatternSyntax(QRegExp::Wildcard);
    ui->setupUi(this);
    ui->title->setText(title);
    setWindowTitle(title);

    ui->filter->addItem("", "");
    ui->filter->addItem("Date (year)",  "Rekall->Date/Time | 0,4");
    ui->filter->addItem("Date (month)", "Rekall->Date/Time | 0,7");
    ui->filter->addItem("Date (day)",   "Rekall->Date/Time | 0,10");  // 1234:67:90 23:56:89
    ui->filter->addItem("Time (hours)", "Rekall->Date/Time | 11,2");  // 9876:43:10 87:54:21
    ui->filter->addItem("Type",         "Rekall->Type");
    ui->filter->addItem("Authors",      "Rekall->Author");
    ui->filter->addItem("Keywords",     "Rekall->Keywords");
    ui->filter->addItem("Fullname",     "Rekall->Name");
    ui->filter->addItem("First letter (name)", "Rekall->Name | 1");
    ui->filter->setCurrentIndex(index);
    ui->checks->setColumnWidth(0, 150);
    ui->checks->sortByColumn(1, Qt::AscendingOrder);
    isUpdating = false;
    actionSelection();
}

void Sorting::showEvent(QShowEvent *) {
    emit(displayed(true));
}
void Sorting::closeEvent(QCloseEvent *) {
    emit(displayed(false));
}
void Sorting::hideEvent(QHideEvent *) {
    emit(displayed(false));
}

void Sorting::setTagname(const QString &_tagName) {
    if(_tagName.count()) {
        QStringList tagNames = _tagName.split("->");
        if(tagNames.count()) {
            tagName       = tagNames.last();
            if(tagNames.count() > 1)
                tagNameCategory = tagNames.first();
            leftLength    = -1;
            asNumber      = false;
            sortAscending = true;
            if((tagName.toLower().contains("date")) || (tagName.toLower().contains("time")))    asDate = true;
            else                                                                                asDate = false;
        }
    }
    else
        tagName = "";
}

void Sorting::action() {
    if(isUpdating)
        return;

    if((sender() == ui->checkAll) || (sender() == ui->uncheckAll) || (sender() == ui->invertAll)) {
        QList<QTreeWidgetItem*> items;
        if(ui->checks->selectedItems().count() > 1)
            items = ui->checks->selectedItems();
        else {
            for(quint16 i = 0 ; i < ui->checks->topLevelItemCount() ; i++)
                if(!ui->checks->topLevelItem(i)->isHidden())
                    items << ui->checks->topLevelItem(i);
        }
        foreach(QTreeWidgetItem *item, items) {
            if(     (sender() == ui->checkAll)   && (item->checkState(0) == Qt::Unchecked))
                item->setCheckState(0, Qt::Checked);
            else if((sender() == ui->uncheckAll) && (item->checkState(0) == Qt::Checked))
                item->setCheckState(0, Qt::Unchecked);
            else if((sender() == ui->invertAll)  && (item->checkState(0) == Qt::Checked))
                item->setCheckState(0, Qt::Unchecked);
            else if((sender() == ui->invertAll)  && (item->checkState(0) == Qt::Unchecked))
                item->setCheckState(0, Qt::Checked);
        }
    }
    else {
        if(ui->filter->count()) {
            QString filterText = ui->filter->itemData(ui->filter->currentIndex()).toString();
            if(filterText.isEmpty())
                filterText = ui->filter->currentText();

            if(filterText.count()) {
                QStringList sortSplit = filterText.split("|");
                setTagname(sortSplit.first().trimmed());
                if(sortSplit.count() > 1) {
                    QString leftPart = sortSplit.at(1);
                    if(leftPart.endsWith("d")) {
                        leftPart.chop(1);
                        sortAscending = false;
                    }
                    if(leftPart.endsWith("f")) {
                        leftPart.chop(1);
                        asNumber = true;
                    }
                    QStringList lefts = leftPart.split(",");
                    left = lefts.at(0).toDouble();
                    if(lefts.count() > 1)
                        leftLength = lefts.at(1).toDouble();
                }
            }
        }
    }
    emit(actionned(ui->filter->currentText(), ui->matches->text()));
}
void Sorting::actionSelection() {
    QString suffix = "all";
    if(ui->checks->selectedItems().count() > 1)
        suffix = "selection";

    ui->checkAll  ->setText(tr("Select %1").arg(suffix));
    ui->uncheckAll->setText(tr("Unselect %1").arg(suffix));
    ui->invertAll ->setText(tr("Invert %1").arg(suffix));
}

const QString Sorting::getCriteriaFormated(const QString &_criteria) {
    if(_criteria.isEmpty())
        return _criteria;

    QString criteria = _criteria, suffix;
    qint16 index = criteria.indexOf("\n");
    if(index > 0) {
        criteria = criteria.left(index);
        suffix   = criteria.right(criteria.length() - index - 1);
    }
    if(asDate) {
        if(criteriaFormatedCache.contains(criteria))
            return criteriaFormatedCache.value(criteria);
        else {
            QString retour;
            if(     criteria.length() <= 3)   retour = criteria;
            else if(criteria.length() <= 4)   retour = tr("%1's").arg(criteria.left(4), -4, '0');
            else if(criteria.length() <= 8)   retour = QDateTime::fromString(criteria, QString("yyyy:MM:dd hh:mm:ss").left(criteria.length())).toString("MMMM yyyy");
            else if(criteria.length() <= 11)  retour = QDateTime::fromString(criteria, QString("yyyy:MM:dd hh:mm:ss").left(criteria.length())).toString("dddd dd MMMM yyyy");
            else if(criteria.length() <= 13)  retour = QDateTime::fromString(criteria, QString("yyyy:MM:dd hh:mm:ss").left(criteria.length())).toString("dddd dd MMMM yyyy, hh") + "h";
            else if(criteria.length() <= 15)  retour = QDateTime::fromString(criteria, QString("yyyy:MM:dd hh:mm:ss").left(criteria.length())).toString("dddd dd MMMM yyyy, hh:mm");
            else                              retour = QDateTime::fromString(criteria, QString("yyyy:MM:dd hh:mm:ss").left(criteria.length())).toString("dddd dd MMMM yyyy, hh:mm:ss");
            criteriaFormatedCache.insert(criteria, retour);
            return retour + suffix;
        }
    }
    else if(leftLength > 0)
        return criteria + suffix + "...";
    return criteria + suffix;
}
bool Sorting::isAcceptableWithFilters(const QString &_criteria) {
    if(!ui->matches->text().isEmpty()) {
        QStringList matches = ui->matches->text().toLower().split(",", QString::SkipEmptyParts);
        foreach(const QString &match, matches) {
            regexp.setPattern(match.trimmed());

            QStringList criterias = _criteria.toLower().split(",", QString::SkipEmptyParts);
            foreach(const QString &criteria, criterias)
                if(regexp.indexIn(criteria.trimmed()) >= 0)
                    return true;
        }
        return false;
    }
    return true;
}
const QString Sorting::getAcceptableWithFilters(const QString &_criteria) {
    QString retour;
    if(!ui->matches->text().isEmpty()) {
        QStringList matches = ui->matches->text().toLower().split(",", QString::SkipEmptyParts);
        foreach(const QString &match, matches) {
            regexp.setPattern(match.trimmed());

            QStringList criterias = _criteria.toLower().split(",", QString::SkipEmptyParts);
            foreach(const QString &criteria, criterias)
                if(regexp.indexIn(criteria.trimmed()) >= 0)
                    return criteria.trimmed();
        }
        return retour;
    }
    return _criteria;
}

const QString Sorting::getMatchName() const {
    return ui->matches->text();
}

Sorting::~Sorting() {
    delete ui;
}

void Sorting::startCheckChange() {
    qDebug("%s", qPrintable(this->windowTitle()));
    isUpdating = true;
    for(quint16 i = 0 ; i < ui->checks->topLevelItemCount() ; i++) {
        if(!ui->checks->topLevelItem(i)->isHidden())
            ui->checks->topLevelItem(i)->setHidden(true);
    }
    isUpdating = false;
}
void Sorting::addCheck(const QString &check, const QString &value) {
    isUpdating = true;
    for(quint16 i = 0 ; i < ui->checks->topLevelItemCount() ; i++) {
        if(ui->checks->topLevelItem(i)->text(0) == check) {
            if(ui->checks->topLevelItem(i)->isHidden())         ui->checks->topLevelItem(i)->setHidden(false);
            if(ui->checks->topLevelItem(i)->text(1) != value)   ui->checks->topLevelItem(i)->setText(1, value);
            isUpdating = false;
            return;
        }
    }

    QTreeWidgetItem *checkItem = new QTreeWidgetItem(ui->checks, QStringList() << check << value);
    checkItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsUserCheckable);
    checkItem->setCheckState(0, Qt::Checked);
    isUpdating = false;
}
bool Sorting::isChecked(const QString &check) const {
    for(quint16 i = 0 ; i < ui->checks->topLevelItemCount() ; i++) {
        if(ui->checks->topLevelItem(i)->text(0) == check) {
            if(ui->checks->topLevelItem(i)->checkState(0) == Qt::Checked)
                return true;
            else
                return false;
        }
    }
    return false;
}



QDomElement Sorting::serialize(QDomDocument &xmlDoc) {
    QDomElement xmlData = xmlDoc.createElement("sorting");
    xmlData.setAttribute("asNumber",        asNumber);
    xmlData.setAttribute("asDate",          asDate);
    xmlData.setAttribute("sortAscending",   sortAscending);
    xmlData.setAttribute("tagNameCategory", tagNameCategory);
    xmlData.setAttribute("tagName",         tagName);
    xmlData.setAttribute("left",            left);
    xmlData.setAttribute("leftLength",      leftLength);
    return xmlData;
}
void Sorting::deserialize(const QDomElement &xmlElement) {
    QString a = xmlElement.attribute("attribut");
}
