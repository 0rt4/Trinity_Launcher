#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QLineEdit>
#include <QDialog>
#include <QFileDialog>
#include <QProcess>
#include <QDir>
#include <QFileInfo>
#include <QMessageBox>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QStandardPaths>
#include <QCoreApplication> // <-- necesario para applicationDirPath()

class LauncherWindow : public QWidget {
    Q_OBJECT

public:
    LauncherWindow(QWidget *parent = nullptr) : QWidget(parent) {
        setWindowTitle("Trinity Launcher - Multiversions");
        resize(480, 220);

        mainLayout = new QVBoxLayout(this);
        mainLayout->setContentsMargins(20, 20, 20, 20);
        mainLayout->setSpacing(12);

        QLabel *titleLabel = new QLabel("Selecciona una versión instalada:");
        titleLabel->setStyleSheet("QLabel { font-weight: bold; font-size: 14px; }");
        mainLayout->addWidget(titleLabel);

        versionComboBox = new QComboBox();
        versionComboBox->setMinimumHeight(30);
        mainLayout->addWidget(versionComboBox);

        extractButton = new QPushButton("Extraer nueva versión desde APK");
        extractButton->setMinimumHeight(35);
        connect(extractButton, &QPushButton::clicked, this, &LauncherWindow::showExtractDialog);
        mainLayout->addWidget(extractButton);

        playButton = new QPushButton("Jugar");
        playButton->setEnabled(false);
        playButton->setMinimumHeight(35);
        connect(playButton, &QPushButton::clicked, this, &LauncherWindow::launchGame);
        mainLayout->addWidget(playButton);

        // --- Nuevo botón: Tools ---
        toolsButton = new QPushButton("Tools");
        toolsButton->setMinimumHeight(35);
        connect(toolsButton, &QPushButton::clicked, this, &LauncherWindow::launchTools);
        mainLayout->addWidget(toolsButton);

        connect(versionComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
                this, &LauncherWindow::onVersionSelected);

        loadInstalledVersions();
    }

private slots:
    void loadInstalledVersions() {
        versionComboBox->clear();

        QString versionsDir = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation)
                              + "/mcpelauncher/versions";

        QDir dir(versionsDir);
        if (dir.exists()) {
            QStringList versions = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
            if (!versions.isEmpty()) {
                versionComboBox->addItems(versions);
                playButton->setEnabled(true);
                return;
            }
        }
        versionComboBox->addItem("No hay versiones instaladas");
        versionComboBox->setEnabled(false);
        playButton->setEnabled(false);
    }

    void onVersionSelected(int index) {
        playButton->setEnabled(
            index >= 0 && versionComboBox->itemText(index) != "No hay versiones instaladas"
        );
    }

    void showExtractDialog() {
        QDialog dialog(this);
        dialog.setWindowTitle("Nueva versión desde APK");

        QString apkPath;
        QString versionName;

        auto *layout = new QFormLayout(&dialog);

        auto *apkButton = new QPushButton("Seleccionar APK...");
        auto *apkLabel = new QLabel("Ningún archivo seleccionado");
        apkLabel->setWordWrap(true);

        connect(apkButton, &QPushButton::clicked, [&]() {
            QString file = QFileDialog::getOpenFileName(
                &dialog, "Seleccionar APK de Minecraft", QDir::homePath(), "Archivos APK (*.apk)"
            );
            if (!file.isEmpty()) {
                apkPath = file;
                apkLabel->setText(QFileInfo(file).fileName());
            }
        });

        auto *nameEdit = new QLineEdit();
        nameEdit->setPlaceholderText("Ej: 1.21.0");
        connect(nameEdit, &QLineEdit::textChanged, [&](const QString &text) {
            versionName = text.trimmed();
        });

        layout->addRow("APK:", apkButton);
        layout->addRow(apkLabel);
        layout->addRow("Nombre de la versión:", nameEdit);

        auto *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
        layout->addWidget(buttonBox);
        dialog.setLayout(layout);

        connect(buttonBox, &QDialogButtonBox::accepted, &dialog, [&]() {
            if (apkPath.isEmpty()) {
                QMessageBox::warning(&dialog, "Error", "Debes seleccionar un archivo APK.");
                return;
            }
            if (versionName.isEmpty()) {
                QMessageBox::warning(&dialog, "Error", "Debes ingresar un nombre para la versión.");
                return;
            }
            dialog.accept();
        });
        connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

        if (dialog.exec() != QDialog::Accepted) return;

        QString destDir = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation)
                          + "/mcpelauncher/versions/" + versionName;

        if (!QDir().mkpath(destDir)) {
            QMessageBox::critical(this, "Error", "No se pudo crear el directorio de destino.");
            return;
        }

        QString extractorPath = QStandardPaths::findExecutable("mcpelauncher-extract");
        if (extractorPath.isEmpty()) {
            QMessageBox::critical(this, "Error",
                "mcpelauncher-extract no encontrado en el entorno.");
            return;
        }

        QProcess process;
        process.start(extractorPath, {apkPath, destDir});
        process.waitForFinished(-1);

        if (process.exitCode() == 0) {
            QMessageBox::information(this, "Éxito", "¡Versión extraída correctamente!");
            loadInstalledVersions();
        } else {
            QString err = process.readAllStandardError();
            if (err.isEmpty()) err = "Error desconocido durante la extracción.";
            QMessageBox::critical(this, "Error", "Falló la extracción:\n" + err);
        }
    }

    void launchGame() {
        QString selectedVersion = versionComboBox->currentText();
        if (selectedVersion == "No hay versiones instaladas") return;

        QString dataDir = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation)
                          + "/mcpelauncher/versions/" + selectedVersion;

        QString libPath = dataDir + "/lib/x86_64/libminecraftpe.so";
        if (!QFileInfo::exists(libPath)) {
            QMessageBox::warning(this, "Advertencia",
                QString("Los datos de '%1' están incompletos.\nFalta: %2")
                .arg(selectedVersion, libPath));
            return;
        }

        QString clientPath = QStandardPaths::findExecutable("mcpelauncher-client");
        if (clientPath.isEmpty()) {
            QMessageBox::critical(this, "Error", "mcpelauncher-client no encontrado.");
            return;
        }

        QProcess::startDetached(clientPath, QStringList() << "-dg" << dataDir);
        QApplication::quit();
    }

    // --- Nueva función: lanzar "trinito" ---
    void launchTools() {
        QString appDir = QCoreApplication::applicationDirPath();
        QString toolsPath = appDir + "/trinito";

        if (!QFileInfo::exists(toolsPath) || !QFileInfo(toolsPath).isExecutable()) {
            QMessageBox::critical(this, "Error",
                QString("El comando 'trinito' no se encontró en:\n%1").arg(toolsPath));
            return;
        }

        // Ejecutar trinito sin bloquear ni esperar (como herramienta externa)
        QProcess::startDetached(toolsPath);
    }

private:
    QVBoxLayout *mainLayout;
    QComboBox *versionComboBox;
    QPushButton *extractButton;
    QPushButton *playButton;
    QPushButton *toolsButton; // <-- Nuevo botón
};

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    LauncherWindow window;
    window.show();
    return app.exec();
}

#include "trinchete.moc"
