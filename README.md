# ChorusKit

ChorusKit is a plugin framework derived from Qt Creator for OpenVPI SVS applications.

## Dependencies

+ Qt 5.15.2

+ [qtmediate](https://github.com/SineStriker/qtmediate)

+ [SingleApplication](https://github.com/itay-grudev/SingleApplication)

+ [qBreakpad](https://github.com/buzzySmile/qBreakpad)

+ [QtCreator 3.6](https://github.com/qt-creator/qt-creator/tree/3.6) (ExtensionSystem, Aggregation, Utils)

## Components

### ExtensionSystem

+ [Qt Creator Plugin Overview](https://doc.qt.io/qtcreator-extending/)

+ [ChorusKit Extension System](docs/extension-system.md)

### ChorusKit Utilities

+ [ChorusKit Plugin Loader](docs/plugin-loader.md)
+ [ChorusKit AppCore](docs/appcore/index.md)

## Build & Install

```sh
cmake -B build -G Ninja \
    -DCMAKE_INSTALL_PREFIX:STRING=<dir> \
    -DCHORUSKIT_BUILD_TESTS:BOOL=FALSE

cmake --build build --target all

cmake --build build --target install
```