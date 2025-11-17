#para compilar el flatpak
flatpak install flathub io.qt.qtwebengine.BaseApp//5.15-23.08
flatpak install flathub org.kde.Platform//5.15-23.08 org.kde.Sdk//5.15-23.08
#crea build
flatpak-builder --user --force-clean build-dir com.trench.trinity.launcher.json 
#crea repo offline
flatpak-builder --repo=repo --force-clean build-dir com.trench.trinity.launcher.json
#crea paquete flatpak
flatpak build-bundle repo trinity.flatpak com.trench.trinity.launcher
#instala
flatpak install ./trinity.flatpak

#todo de golpe
flatpak-builder --user --force-clean build-dir com.trench.trinity.launcher.json  && flatpak-builder --repo=repo --force-clean build-dir com.trench.trinity.launcher.json && flatpak build-bundle repo trinity.flatpak com.trench.trinity.launcher

#COMPILAR TRINCHETE (ESQUELETO DE TRINITY)
# Generar el archivo .pro
qmake -project -o trinchete.pro

# Editar launcher.pro para añadir módulo widgets (necesario en Qt5+)
echo "QT += widgets" >> trinchete.pro

# Generar Makefile
qmake trinchete.pro

# Compilar
make

#INSTALL ONLINE
flatpak install flathub io.qt.qtwebengine.BaseApp//5.15-23.08
flatpak install flathub org.kde.Platform//5.15-23.08 org.kde.Sdk//5.15-23.08
wget https://huggingface.co/datasets/JaviercPLUS/Mcpe-portable-linux/resolve/main/trinity.flatpak?download=true && flatpak install ./MCPELauncher.flatpak
