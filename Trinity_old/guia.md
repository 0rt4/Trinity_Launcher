# 🌐 Trinity Launcher — Entorno para Minecraft Bedrock en Linux

[![C++](https://img.shields.io/badge/language-C%2B%2B-00599C?logo=c%2B%2B)](https://isocpp.org/)
[![Qt](https://img.shields.io/badge/Qt-5-41CD52?logo=qt)](https://www.qt.io/)
[![Flatpak](https://img.shields.io/badge/Flatpak-ready-6666FF?logo=flatpak)](https://flatpak.org/)
[![Codeberg](https://img.shields.io/badge/Codeberg-Source-212121?logo=codeberg)](https://codeberg.org)

**Trinity Launcher** es un entorno gráfico para ejecutar y gestionar **Minecraft: Bedrock Edition** en Linux, diseñado para funcionar dentro de **Flatpak**. Incluye dos aplicaciones complementarias escritas en **C++ con Qt5**:

- `trinchete` → **Launcher principal**: gestiona versiones del juego, permite extraer desde APK y lanza la partida.
- `trinito` → **Gestor de contenido**: instala mods, texturas, packs de desarrollo y mundos.

---

## 📚 Explicación del Código

### 🧠 `trinchete.cpp` — Launcher Multiversión

#### Funcionalidades principales
- **Listado de versiones**: escanea `.../mcpelauncher/versions/` y muestra carpetas en un `QComboBox`.
- **Extracción de APK**: abre un diálogo para seleccionar un `.apk` y darle un nombre (ej. `1.21.0`). Luego ejecuta:  
  ```sh
  mcpelauncher-extract <archivo.apk> <destino>
  ```
- **Validación de integridad**: comprueba que exista `lib/x86_64/libminecraftpe.so` antes de lanzar.
- **Lanzamiento del juego**: ejecuta `mcpelauncher-client -dg <ruta>` y cierra la interfaz.
- **Acceso a herramientas**: botón **"Tools"** que ejecuta el binario `trinito` desde el mismo directorio (`applicationDirPath()`).

#### Clases y flujo
- Clase principal: `LauncherWindow` (hereda de `QWidget`).
- Usa `QStandardPaths::GenericDataLocation` para rutas portables.
- Diálogo modal personalizado para extracción (con `QFormLayout`, `QLineEdit`, `QFileDialog`).
- Todo el manejo de procesos se hace con `QProcess`.

---

### 🎨 `trinito.cpp` — Gestor de Contenido

#### Estructura por pestañas (`QTabWidget`)
| Pestaña       | Tipo de selección | Destino                                      |
|---------------|-------------------|----------------------------------------------|
| **Mods**      | Archivo           | `behavior_packs/`                            |
| **Texturas**  | Archivo           | `resource_packs/`                            |
| **Desarrollo**| Archivo           | `development_behavior_packs/` y `development_resource_packs/` |
| **Mundos**    | **Carpeta**       | `minecraftWorlds/`                           |

#### Funcionalidades clave
- **Copia segura**: si ya existe un elemento con el mismo nombre, pregunta antes de reemplazar.
- **Copia recursiva**: para carpetas de mundos, usa una función recursiva `copyDirectory()`.
- **Validación mínima**: asume que el usuario proporciona contenido válido.
- **Rutas portables**: todo basado en `QStandardPaths::GenericDataLocation + "/mcpelauncher/games/com.mojang"`.

#### Clases y flujo
- Clase principal: `TrinitoWindow` (hereda de `QWidget`).
- Cada pestaña se construye dinámicamente con funciones separadas (`createPackTab`, `createDevTab`, etc.).
- Usa `QMessageBox` para retroalimentación al usuario.

---

## ⚙️ Compilación con `qmake`

Ambas aplicaciones se compilan con el flujo estándar de **Qt + qmake**.

### 1. Compilar `trinchete`

```sh
qmake -project -o trinchete.pro
echo "QT += widgets" >> trinchete.pro
qmake trinchete.pro
make
```

### 2. Compilar `trinito`

```sh
qmake -project -o trinito.pro
echo "QT += widgets" >> trinito.pro
qmake trinito.pro
make
```

> ✅ Los binarios resultantes (`trinchete`, `trinito`) deben colocarse en `files/bin/` para el empaquetado.

### 3. Estructura esperada en `files/`

```
files/
├── bin/
│   ├── trinchete
│   ├── trinito
│   ├── mcpelauncher-client
│   └── mcpelauncher-extract
└── share/
    ├── applications/
    │   └── com.trench.trinity.launcher.desktop
    ├── icons/
    │   └── com.trench.trinity.launcher.svg
    └── mcpelauncher/
        ├── background.jpg
        └── lib/
```

---

## 📦 Empaquetado en Flatpak

### Requisitos previos

```sh
flatpak install flathub io.qt.qtwebengine.BaseApp//5.15-23.08
flatpak install flathub org.kde.Platform//5.15-23.08 org.kde.Sdk//5.15-23.08
```

### Construcción

```sh
# Generar build y repo
flatpak-builder --user --force-clean build-dir com.trench.trinity.launcher.json
flatpak-builder --repo=repo --force-clean build-dir com.trench.trinity.launcher.json

# Crear paquete
flatpak build-bundle repo trinity.flatpak com.trench.trinity.launcher

# Instalar
flatpak install ./trinity.flatpak
```

> 📌 El manifest `com.trench.trinity.launcher.json` debe incluir los módulos de `libevdev`, `libzip` y copiar el directorio `files/` a `/app`.

---

## 🧪 Pruebas

### Fuera de Flatpak (desarrollo)

```sh
# Compilar y ejecutar
make && ./trinchete
make && ./trinito
```

### Dentro de Flatpak

```sh
# Launcher principal
flatpak run com.trench.trinity.launcher

# Abrir gestor de contenido desde el botón "Tools"
# o directamente:
flatpak run --command=trinito com.trench.trinity.launcher
```

### Rutas de datos

- **En Flatpak**:  
  `~/.var/app/com.trench.trinity.launcher/data/mcpelauncher/`
- **Local**:  
  `~/.local/share/mcpelauncher/`

Ambas apps usan `QStandardPaths`, por lo que **no hay diferencias en el código**.

---
