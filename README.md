# ChorusKit

ChorusKit is a plugin framework derived from Qt Creator for SVS applications.

## Dependencies

+ Qt
+ [SingleApplication](https://github.com/itay-grudev/SingleApplication)
+ [ExtensionSystem](https://github.com/qt-creator/qt-creator/tree/3.6) (ExtensionSystem, Aggregation, Utils)

## Components

### ExtensionSystem

+ [Qt Creator Plugin Overview](https://doc.qt.io/qtcreator-extending/)
<!-- 
### ChorusKit Utilities

+ [ChorusKit Plugin Loader](docs/plugin-loader.md)
+ [ChorusKit AppCore](docs/appcore/index.md) -->

## Build & Install

```sh
cmake -B build -G Ninja \
    -DCMAKE_INSTALL_PREFIX:STRING=<dir>

cmake --build build --target all

cmake --build build --target install
```