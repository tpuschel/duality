/*
 * Copyright 2019 Thorben Hasenpusch <t.hasenpusch@icloud.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include <QApplication>
#include <QMainWindow>
#include <QPlainTextEdit>
#include <QMenuBar>
#include <QMenu>
#include <QFileDialog>
#include <QFile>
#include <QTimer>

extern "C" {
#include <duality/syntax/parser.h>
#include <duality/syntax/ast_to_core.h>
}

struct Actions : QObject {
  public:
    Actions(QObject *parent = nullptr)
        : QObject(parent), file(nullptr), timer(new QTimer(parent))
    {
        timer->setSingleShot(true);
        timer->callOnTimeout(this, &Actions::processText);
    }

    QPlainTextEdit *textEdit;

    void openFile()
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

        textEdit->setPlainText(QString::fromUtf8(file->readAll()));
    }

    void saveFile()
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

        if (!file->resize(0)) {
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
        }
    }

    void saveFileAs()
    {
        resetFile();
        saveFile();
    }

    void textChanged()
    {
        using namespace std::chrono_literals;

        timer->start(300ms);
    }

    ~Actions()
    {
        delete file;
        delete timer;
    }

  private:
    QFile *file;
    QTimer *timer;

    void resetFile()
    {
        delete file;
        file = nullptr;
    }

    void processText()
    {
        QByteArray utf8Text = textEdit->toPlainText().toUtf8();
        if (utf8Text.size() <= 0) {
            return;
        }

        size_t textSize = static_cast<size_t>(utf8Text.size());

        dy_parser_error parserError;
        size_t index;

        dy_parser_ctx parserCtx;
        parserCtx.allocator = dy_allocator_stdlib();
        parserCtx.error = &parserError;
        parserCtx.index_in = 0;
        parserCtx.index_out = &index;
        parserCtx.text = {
            utf8Text.data(),
            textSize,
        };

        dy_ast_do_block program;
        if (!dy_parse_file(parserCtx, &program)) {
            printf("Failed to parse program!\n");
            return;
        }

        size_t running_id = 0;
        dy_array_t *unbound_vars = dy_array_create(parserCtx.allocator, sizeof(dy_string_t), 2);

        struct dy_ast_to_core_ctx ast_to_core_ctx;
        ast_to_core_ctx.allocator = parserCtx.allocator;
        ast_to_core_ctx.running_id = &running_id;
        ast_to_core_ctx.bound_vars = dy_array_create(parserCtx.allocator, sizeof(struct dy_ast_to_core_bound_var), 64);
        ast_to_core_ctx.unbound_vars = unbound_vars;

        dy_core_expr core;
        if (!dy_ast_do_block_to_core(ast_to_core_ctx, program, &core)) {
            printf("Unbound variables.\n");
            return;
        }
    }
};

static QMainWindow *createMainWindow(Actions *actions);
static QPlainTextEdit *createTextEdit(QWidget *parent, Actions *actions);
static void setupMenuBar(QMenuBar *menuBar, Actions *actions);

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    Actions *actions = new Actions();

    QMainWindow *mainWindow = createMainWindow(actions);

    mainWindow->show();

    return app.exec();
}

QMainWindow *createMainWindow(Actions *actions)
{
    QMainWindow *mainWindow = new QMainWindow();
    mainWindow->resize(800, 600);
    mainWindow->setWindowTitle("Duality IDE");
    mainWindow->setCentralWidget(createTextEdit(mainWindow, actions));
    setupMenuBar(mainWindow->menuBar(), actions);
    return mainWindow;
}

void setupMenuBar(QMenuBar *menuBar, Actions *actions)
{
    QMenu *fileMenu = menuBar->addMenu("File");
    fileMenu->addAction("Open...", actions, &Actions::openFile, QKeySequence::Open);
    fileMenu->addAction("Save", actions, &Actions::saveFile, QKeySequence::Save);
    fileMenu->addAction("Save As...", actions, &Actions::saveFileAs, QKeySequence::SaveAs);
}

QPlainTextEdit *createTextEdit(QWidget *parent, Actions *actions)
{
    QPlainTextEdit *textEdit = new QPlainTextEdit(parent);

    QFont font("Menlo", 12);
    textEdit->setFont(font);

    QObject::connect(textEdit, &QPlainTextEdit::textChanged, actions, &Actions::textChanged);

    actions->textEdit = textEdit;

    return textEdit;
}
