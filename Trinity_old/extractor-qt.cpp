#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QFileDialog>
#include <QMessageBox>
#include <QStandardPaths>
#include <QProcess>
#include <QDir>
#include <QFileInfo>
#include <QPalette>
#include <QPixmap>
#include <QResizeEvent>

class ExtractorWindow : public QWidget {
    Q_OBJECT

public:
    ExtractorWindow(QWidget *parent = nullptr) : QWidget(parent) {
        setWindowTitle("MCPelauncher APK Extractor");
        resize(480, 220);

        setupBackground();

        mainLayout = new QVBoxLayout(this);
        mainLayout->setContentsMargins(12, 12, 12, 12);
        mainLayout->setSpacing(12);

        QLabel *label = new QLabel("Selecciona el APK x86_64 de Minecraft Bedrock Edition:");
        label->setStyleSheet("QLabel { background: transparent; color: white; font-weight: bold; }");
        label->setWordWrap(true);
        mainLayout->addWidget(label);

        fileChooserButton = new QPushButton("Seleccionar APK");
        fileChooserButton->setStyleSheet("QPushButton { background: rgba(0,0,0,128); color: white; }");
        connect(fileChooserButton, &QPushButton::clicked, this, &ExtractorWindow::selectApk);
        mainLayout->addWidget(fileChooserButton);

        extractButton = new QPushButton("Extraer APK");
        extractButton->setEnabled(false);
        extractButton->setStyleSheet("QPushButton { background: rgba(0,0,0,128); color: white; }");
        connect(extractButton, &QPushButton::clicked, this, &ExtractorWindow::extractApk);
        mainLayout->addWidget(extractButton);

        playButton = nullptr;
    }

private:
    void setupBackground() {
        QString bgPath = QCoreApplication::applicationDirPath() + "/../share/mcpelauncher/background.jpg";
        QPixmap bgPixmap(bgPath);

        if (!bgPixmap.isNull()) {
            bgPixmap = bgPixmap.scaled(size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
            QPalette palette;
            palette.setBrush(QPalette::Window, bgPixmap);
            setAutoFillBackground(true);
            setPalette(palette);
        }
    }

protected:
    void resizeEvent(QResizeEvent *event) override {
        QWidget::resizeEvent(event);
        setupBackground(); // Actualizar fondo al redimensionar
    }

private slots:
    void selectApk() {
        QString selected = QFileDialog::getOpenFileName(
            this,
            "Seleccionar APK",
            QDir::homePath(),
            "Archivos APK (*.apk)"
        );
        if (!selected.isEmpty()) {
            apkPath = selected;
            fileChooserButton->setText(QFileInfo(apkPath).fileName());
            extractButton->setEnabled(true);
        }
    }

    void extractApk() {
        if (apkPath.isEmpty()) return;

        QString destDir = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation)
                          + "/mcpelauncher/versions/current";

        if (!QDir().mkpath(destDir)) {
            QMessageBox::critical(this, "Error", QString("No se pudo crear el directorio:\n%1").arg(destDir));
            return;
        }

        QString appDir = QCoreApplication::applicationDirPath();
        QString extractorPath = appDir + "/mcpelauncher-extract";

        if (!QFileInfo::exists(extractorPath) || !QFileInfo(extractorPath).isExecutable()) {
            QMessageBox::critical(this, "Error",
                QString("mcpelauncher-extract no encontrado en:\n%1").arg(extractorPath));
            return;
        }

        QStringList args;
        args << apkPath << destDir;

        QProcess process;
        process.setProgram(extractorPath);
        process.setArguments(args);
        process.start();
        process.waitForFinished(-1);

        if (process.exitCode() == 0) {
            QMessageBox::information(this, "Éxito", "¡Extracción completada!");

            // Ocultar botones anteriores
            fileChooserButton->hide();
            extractButton->hide();

            // Mostrar botón "Jugar"
            if (!playButton) {
                playButton = new QPushButton("Jugar", this);
                playButton->setStyleSheet("QPushButton { background: rgba(0,0,0,128); color: white; }");
                connect(playButton, &QPushButton::clicked, this, &ExtractorWindow::launchGame);
                mainLayout->addWidget(playButton);
            }
            playButton->show();
        } else {
            QString error = process.readAllStandardError();
            if (error.isEmpty()) error = "Error desconocido.";
            QMessageBox::critical(this, "Error", QString("Falló la extracción:\n%1").arg(error));
        }
    }

    void launchGame() {
        QString appDir = QCoreApplication::applicationDirPath();
        QString clientPath = appDir + "/mcpelauncher-client";

        if (!QFileInfo::exists(clientPath) || !QFileInfo(clientPath).isExecutable()) {
            QMessageBox::critical(this, "Error", "mcpelauncher-client no encontrado.");
            return;
        }

        QString dataDir = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation)
                          + "/mcpelauncher/versions/current";

        QStringList args;
        args << "-dg" << dataDir;

        // Lanzar el juego y cerrar el extractor
        QProcess::startDetached(clientPath, args);
        QApplication::quit();
    }

private:
    QString apkPath;
    QVBoxLayout *mainLayout;
    QPushButton *fileChooserButton;
    QPushButton *extractButton;
    QPushButton *playButton;
};

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    ExtractorWindow window;
    window.show();
    return app.exec();
}

#include "extractor-qt.moc"
