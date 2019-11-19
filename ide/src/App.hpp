/*
 * Copyright 2019 Thorben Hasenpusch <t.hasenpusch@icloud.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include <QObject>
#include <QMainWindow>
#include <QFile>

extern "C" {
#include <duality/syntax/ast.h>
}

struct App : QObject {
public:
    App(QObject *parent = nullptr);

    ~App();

private:
    dy_ast_expr program;
    QString baseWindowTitle;
    QFile *file;

    QMainWindow *mainWindow;
    QWidget *buildingBlocksWindow;

    void openFile();

    void closeFile();

    void saveFile();

    void saveFileAs();

    void importFromText();

    QString noUnsavedChangesWindowTitle();

    QString unsavedChangesWindowTitle();

    void resetFile();

    void setupMainWindow();

    void setupMenuBar(QMenuBar *menuBar);
};
