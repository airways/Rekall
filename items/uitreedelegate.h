#ifndef UITREEDELEGATE_H
#define UITREEDELEGATE_H

#include <QStyledItemDelegate>
#include <QItemDelegate>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QMouseEvent>
#include <QLineEdit>
#include <QCheckBox>
#include <QEvent>
#include <QPainter>
#include <QFileDialog>
#include <QColorDialog>
#include <QPushButton>
#include <QHeaderView>
#include <QPixmap>

class UiTreeViewOptions {
public:
    QAbstractItemModel *model;
    quint16 index;
    QString name, type, styleSheet;
    qint16 width, height, rootHeight;
    QPixmap iconCheckedOff, iconCheckedOn;
    QFileDialog *dialogFile;
    QColorDialog *dialogColor;
    QHeaderView::ResizeMode resizeMode;
public:
    UiTreeViewOptions() {
        model       = 0;
        width       = -1;
        index       = 0;
        dialogFile  = 0;
        dialogColor = 0;
    }
    explicit UiTreeViewOptions(quint16 _index, const QString &_name, const QString &_type = QString(), const QString &_styleSheet = QString(), QHeaderView::ResizeMode _resizeMode = QHeaderView::ResizeToContents, qint16 _width = -1, qint16 _rootHeight = 1, qint16 _height = 20) {
        model       = 0;
        index       = _index;
        name        = _name;
        type        = _type;
        styleSheet  = _styleSheet;
        resizeMode  = _resizeMode;
        width       = _width;
        height      = _height;
        rootHeight  = _rootHeight;
    }
};
class UiTreeDelegate : public QItemDelegate {
    Q_OBJECT

private:
    UiTreeViewOptions options;

public:
    explicit UiTreeDelegate(const UiTreeViewOptions &_options, QAbstractItemModel *_model, QObject *parent);

public:
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void setEditorData(QWidget *editor, const QModelIndex &index) const;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;
    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index);
    QSize sizeHint(const QStyleOptionViewItem &, const QModelIndex &) const;
signals:
    void action(const QModelIndex &index);
};


#endif // UITREEDELEGATE_H
