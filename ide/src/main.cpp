/*
 * Copyright 2017-2019 Thorben Hasenpusch <t.hasenpusch@icloud.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include <QApplication>
#include <QMainWindow>
#include <QTextEdit>

static QMainWindow *createMainWindow();
static QTextEdit *createTextEdit(QWidget *parent);

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QMainWindow *mainWindow = createMainWindow();
    mainWindow->setCentralWidget(createTextEdit(mainWindow));

    mainWindow->show();

    return app.exec();
}

QMainWindow *createMainWindow()
{
    QMainWindow *mainWindow = new QMainWindow();
    mainWindow->resize(800, 600);
    mainWindow->setWindowTitle("Duality IDE");
    return mainWindow;
}

QTextEdit *createTextEdit(QWidget *parent)
{
    QTextEdit *textEdit = new QTextEdit(parent);
    textEdit->setAcceptRichText(false);
    return textEdit;
}
