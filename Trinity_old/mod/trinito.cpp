#include <QApplication>
#include <QWidget>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QFileDialog>
#include <QDir>
#include <QFileInfo>
#include <QStandardPaths>
#include <QMessageBox>

class TrinitoWindow : public QWidget {
    Q_OBJECT

public:
    TrinitoWindow(QWidget *parent = nullptr) : QWidget(parent) {
        setWindowTitle("Trinito — Gestor de Contenido para Bedrock");
        resize(500, 220);

        baseGameDir = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation)
                      + "/mcpelauncher/games/com.mojang";

        auto *layout = new QVBoxLayout(this);
        QTabWidget *tabs = new QTabWidget();
        layout->addWidget(tabs);

        tabs->addTab(createPackTab("behavior_packs", "Behavior Pack (mods)"), "Mods");
        tabs->addTab(createPackTab("resource_packs", "Resource Pack (texturas)"), "Texturas");
        tabs->addTab(createDevTab(), "Desarrollo");
        tabs->addTab(createWorldTab(), "Mundos");
    }

private:
    QString baseGameDir;

    QWidget* createPackTab(const QString &targetSubdir, const QString &labelText) {
        auto *widget = new QWidget();
        auto *layout = new QVBoxLayout(widget);

        auto *label = new QLabel(labelText);
        layout->addWidget(label);

        auto *button = new QPushButton("Seleccionar archivo...");
        layout->addWidget(button);

        connect(button, &QPushButton::clicked, this, [=]() {
            QString path = QFileDialog::getOpenFileName(
                this, "Seleccionar pack", QDir::homePath(),
                "Archivos compatibles (*.zip *.mcpack);;Todos los archivos (*)"
            );
            if (!path.isEmpty()) {
                installItem(path, targetSubdir);
            }
        });

        layout->addStretch();
        return widget;
    }

    QWidget* createDevTab() {
        auto *widget = new QWidget();
        auto *layout = new QVBoxLayout(widget);

        auto *behButton = new QPushButton("Seleccionar Development Behavior Pack (archivo)...");
        connect(behButton, &QPushButton::clicked, this, [=]() {
            QString path = QFileDialog::getOpenFileName(
                this, "Seleccionar Development Behavior Pack", QDir::homePath(),
                "Archivos compatibles (*.zip *.mcpack);;Todos los archivos (*)"
            );
            if (!path.isEmpty()) {
                installItem(path, "development_behavior_packs");
            }
        });
        layout->addWidget(new QLabel("Development Behavior Pack:"));
        layout->addWidget(behButton);

        layout->addSpacing(15);

        auto *resButton = new QPushButton("Seleccionar Development Resource Pack (archivo)...");
        connect(resButton, &QPushButton::clicked, this, [=]() {
            QString path = QFileDialog::getOpenFileName(
                this, "Seleccionar Development Resource Pack", QDir::homePath(),
                "Archivos compatibles (*.zip *.mcpack);;Todos los archivos (*)"
            );
            if (!path.isEmpty()) {
                installItem(path, "development_resource_packs");
            }
        });
        layout->addWidget(new QLabel("Development Resource Pack:"));
        layout->addWidget(resButton);

        layout->addStretch();
        return widget;
    }

    QWidget* createWorldTab() {
        auto *widget = new QWidget();
        auto *layout = new QVBoxLayout(widget);

        auto *label = new QLabel("Instalar mundo guardado (carpeta de mundo):");
        layout->addWidget(label);

        auto *button = new QPushButton("Seleccionar carpeta del mundo...");
        layout->addWidget(button);

        connect(button, &QPushButton::clicked, this, [=]() {
            QString path = QFileDialog::getExistingDirectory(
                this, "Seleccionar carpeta del mundo", QDir::homePath()
            );
            if (!path.isEmpty()) {
                installItem(path, "minecraftWorlds");
            }
        });

        layout->addStretch();
        return widget;
    }

    void installItem(const QString &sourcePath, const QString &targetSubdir) {
        QFileInfo sourceInfo(sourcePath);
        if (!sourceInfo.exists()) {
            QMessageBox::critical(this, "Error", "La ruta seleccionada no existe.");
            return;
        }

        QString targetDir = baseGameDir + "/" + targetSubdir;
        if (!QDir().mkpath(targetDir)) {
            QMessageBox::critical(this, "Error", "No se pudo crear el directorio de destino.");
            return;
        }

        QString finalDest = targetDir + "/" + sourceInfo.fileName();

        if (QFileInfo::exists(finalDest)) {
            int r = QMessageBox::warning(this, "Advertencia",
                QString("Ya existe un elemento llamado:\n%1\n\n¿Reemplazarlo?").arg(sourceInfo.fileName()),
                QMessageBox::Yes | QMessageBox::No, QMessageBox::No
            );
            if (r == QMessageBox::No) return;
            if (QFileInfo(finalDest).isDir()) {
                QDir(finalDest).removeRecursively();
            } else {
                QFile::remove(finalDest);
            }
        }

        bool success = false;
        if (sourceInfo.isDir()) {
            success = copyDirectory(sourcePath, finalDest);
        } else {
            success = QFile::copy(sourcePath, finalDest);
        }

        if (success) {
            QMessageBox::information(this, "Éxito",
                QString("¡%1 instalado correctamente en:\n%2")
                .arg(sourceInfo.fileName()).arg(targetSubdir));
        } else {
            QMessageBox::critical(this, "Error", "Falló la copia del archivo o carpeta.");
        }
    }

    bool copyDirectory(const QString &srcPath, const QString &dstPath) {
        QDir srcDir(srcPath);
        if (!srcDir.exists()) return false;
        QDir().mkpath(dstPath);
        for (const QFileInfo &info : srcDir.entryInfoList(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot)) {
            QString srcItem = srcPath + "/" + info.fileName();
            QString dstItem = dstPath + "/" + info.fileName();
            if (info.isDir()) {
                if (!copyDirectory(srcItem, dstItem)) return false;
            } else {
                if (!QFile::copy(srcItem, dstItem)) return false;
            }
        }
        return true;
    }
};

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    TrinitoWindow window;
    window.show();
    return app.exec();
}

#include "trinito.moc"
