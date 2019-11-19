/*
 * Copyright 2019 Thorben Hasenpusch <t.hasenpusch@icloud.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include "App.hpp"

#include <QFileDialog>
#include <QMenuBar>
#include <QWindow>
#include <QVBoxLayout>
#include <QPainter>
#include <QMouseEvent>
#include <QDrag>
#include <QMimeData>
#include <QApplication>
#include <QDebug>

extern "C" {
#include <duality/syntax/ast_to_core.h>
}

class ExprHole : public QWidget {
    Q_OBJECT

    bool isInDrag;

public:
    ExprHole(QWidget *parent = nullptr) : QWidget(parent)
    {
        setAcceptDrops(true);
        isInDrag = false;
    }

    void dragEnterEvent(QDragEnterEvent *event)
    {
        QByteArray data = event->mimeData()->data("expression-type");
        if (data.isNull()) {
            return;
        }

        event->acceptProposedAction();

        isInDrag = true;
        repaint();
    }

    void dropEvent(QDropEvent *event)
    {
        QByteArray data = event->mimeData()->data("expression-type");
        if (data.isNull()) {
            return;
        }

        dragDrop(QString::fromUtf8(data));

        isInDrag = false;
        repaint();
    }

    void dragLeaveEvent(QDragLeaveEvent *event)
    {
        isInDrag = false;
        repaint();
    }

    void paintEvent(QPaintEvent *event)
    {
        if (isInDrag) {
            QPainter painter(this);
            painter.setRenderHint(QPainter::Antialiasing);

            QPainterPath path;
            path.addRect(event->rect());
            QPen pen(Qt::lightGray, 5);

            painter.setPen(pen);
            painter.drawPath(path);
        }
    }

signals:
    void dragDrop(QString type);
};

#include "App.moc"

struct ListWidget : QWidget {
    QPoint dragStartPos;

    ListWidget(QWidget *parent = nullptr) : QWidget(parent) {}

    void paintEvent(QPaintEvent *event)
    {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);

        QPainterPath path;
        path.addRoundedRect(QRectF(9.5, 9.5, 100, 50), 10, 10);
        QPen pen(Qt::black, 1);

        painter.setPen(pen);
        painter.fillPath(path, Qt::red);
        painter.drawPath(path);
    }

    void mousePressEvent(QMouseEvent *event)
    {
        if (event->button() == Qt::LeftButton) {
            dragStartPos = event->pos();
        }
    }

    void mouseMoveEvent(QMouseEvent *event)
    {
        if (!(event->buttons() & Qt::LeftButton)) {
            return;
        }

        if ((event->pos() - dragStartPos).manhattanLength() < QApplication::startDragDistance()) {
            return;
        }

        QDrag *drag = new QDrag(this);

        QMimeData *mimeData = new QMimeData();
        mimeData->setData("expression-type", QString("list").toUtf8());

        drag->setMimeData(mimeData);

        drag->exec();
    }
};

App::App(QObject *parent) : QObject(parent), file(nullptr)
{
    baseWindowTitle = "Duality Program Builder";

    setupMainWindow();

    setupMenuBar(mainWindow->menuBar());

    buildingBlocksWindow = new QWidget();
    buildingBlocksWindow->setWindowTitle("Building Blocks");
    buildingBlocksWindow->move(50, 200);
    buildingBlocksWindow->resize(200, 400);

    QVBoxLayout *vbox = new QVBoxLayout(buildingBlocksWindow);
    vbox->addWidget(new ListWidget());
    vbox->addWidget(new ListWidget());

    buildingBlocksWindow->show();
    mainWindow->show();
}

void App::setupMainWindow()
{
    mainWindow = new QMainWindow();
    mainWindow->resize(800, 600);
    mainWindow->setWindowTitle(baseWindowTitle);

    ExprHole *hole = new ExprHole();
    QObject::connect(hole, &ExprHole::dragDrop, [](QString type) { qDebug() << type; });
    mainWindow->setCentralWidget(hole);
}

void App::setupMenuBar(QMenuBar *menuBar)
{
    QMenu *fileMenu = menuBar->addMenu("File");
    fileMenu->addAction("Open...", this, &App::openFile, QKeySequence::Open);
    fileMenu->addAction("Close", this, &App::closeFile, QKeySequence::Close);
    fileMenu->addAction("Save", this, &App::saveFile, QKeySequence::Save);
    fileMenu->addAction("Save As...", this, &App::saveFileAs, QKeySequence::SaveAs);
    fileMenu->addAction("Import From Text...", this, &App::importFromText);

    QMenu *windowsMenu = menuBar->addMenu("Windows");
    windowsMenu->addAction("Show Program Builder", [this]() { mainWindow->show(); mainWindow->raise(); mainWindow->activateWindow(); });
    windowsMenu->addAction("Show Building Blocks", [this]() { buildingBlocksWindow->show(); buildingBlocksWindow->raise(); buildingBlocksWindow->activateWindow(); });
}

void App::openFile()
{
    resetFile();

    QString filename = QFileDialog::getOpenFileName();
    if (filename.isNull()) {
        return;
    }

    file = new QFile(filename);
    if (!file->open(QIODevice::ReadWrite)) {
        printf("Failed to open file.\n");
        resetFile();
        return;
    }

    mainWindow->setWindowTitle(noUnsavedChangesWindowTitle());

    //textEdit->setPlainText(QString::fromUtf8(file->readAll()));
}

void App::closeFile()
{
    resetFile();
    mainWindow->setWindowTitle(baseWindowTitle);
    //textEdit->setPlainText("");
}

void App::saveFile()
{
    if (file == nullptr) {
        QString filename = QFileDialog::getSaveFileName();
        if (filename.isNull()) {
            return;
        }

        file = new QFile(filename);
        if (!file->open(QIODevice::WriteOnly)) {
            printf("Failed to open file for saving.\n");
            resetFile();
            return;
        }
    }

    /*if (!file->resize(0)) {
        printf("File resize failed.\n");
        return;
    }

    if (file->write(textEdit->toPlainText().toUtf8()) == -1) {
        printf("Error writing file.\n");
        return;
    }

    if (!file->flush()) {
        printf("Flush failed.\n");
        return;
    }*/

    mainWindow->setWindowTitle(noUnsavedChangesWindowTitle());
}

void App::saveFileAs()
{
    resetFile();
    saveFile();
}

void App::importFromText()
{
    QString filename = QFileDialog::getOpenFileName();
    if (filename.isNull()) {
        return;
    }

    file = new QFile(filename);
    if (!file->open(QIODevice::ReadWrite)) {
        printf("Failed to open file.\n");
        delete file;
        return;
    }

    QByteArray data = file->readAll();

    size_t textSize = static_cast<size_t>(data.size());

    dy_parser_error parserError;
    size_t index;

    dy_parser_ctx parserCtx;
    parserCtx.allocator = dy_allocator_stdlib();
    parserCtx.error = &parserError;
    parserCtx.index_in = 0;
    parserCtx.index_out = &index;
    parserCtx.text = {
        data.data(),
        textSize,
    };

    dy_ast_do_block do_block;
    if (!dy_parse_file(parserCtx, &do_block)) {
        printf("Failed to parse program!\n");
        return;
    }

    program.tag = DY_AST_EXPR_DO_BLOCK;
    program.do_block = do_block;
}

QString App::noUnsavedChangesWindowTitle()
{
    if (file == nullptr) {
        return baseWindowTitle;
    } else {
        return baseWindowTitle + " (" + file->fileName() + ")";
    }
}

QString App::unsavedChangesWindowTitle()
{
    return noUnsavedChangesWindowTitle() + " *";
}

void App::resetFile()
{
    delete file;
    file = nullptr;
}

App::~App()
{
    delete file;
}
