# Plugin Loader

`CkLoader` is a static library helping to do essential initializations of the plugin framework and load all plugins.

You should be well informed the plugin life cycle at [Qt Creator Documentation](https://doc.qt.io/qtcreator-extending/plugin-lifecycle.html).

## About Loader

To find and load plugins is a dull and boring implementation job, we provide the common loader implementations with some hooks so that you can do some custom work while loading the plugins.

### Loader Tasks

1. Parse command line arguments
2. Initialize `ExtensionSystem::PluginManager`
3. Create a `QSplashScreen` and show messages
4. Find plugins
5. Create a `SingleApplication` guard
6. Load and initialize the plugins
7. Enter main event loop

### Loader Configuration

You need to initialize the member variables in constructor.

+ `allowRoot`: Add an `--allow-root` option to the application, if the application is launched as Root/Administrator without setting this option, the loader will display a warning and exit

+ `coreName`: The core plugin name, the default value is `Core`, it's recommended that you leave it unchanged

+ `pluginIID`: Plugin interface ID, only Qt Plugins with this IID will be recognized as the application plugin

+ `pluginPaths`: Default plugin searching paths

+ `splashSettingPath`: The splash screen configuration path

+ `extraArguments`: Extra arguments that need to be shown in help text

+ `userSettingsPath`: `QSettings` directory of `ExtensionSystem::PluginManager` (User Scope)

+ `systemSettingsPath`: `QSettings` directory of `ExtensionSystem::PluginManager` (System Scope)

You may need to override the virtual functions that will be called during the plugin loading process.

+ `bool preprocessArguments(QStringList &arguments, int *code)`
    + You should parse your custom arguments and may do some initializations before the loader creates `ExtensionSystem::PluginManager` instance
    + Params
        + `arguments`: The remaining arguments, you should remove the ones you specified in `extraArguments`
        + `code`: The return code if parse failed

+ `void beforeLoadPlugins(QSplashScreen *screen)`
    + Do something before loading plugins, you can store the splash screen handle and give it to the plugins

+ `void afterLoadPlugins()`
    + Do something before entering the main event loop

Call `LoaderConfiguration::run()` to transfer control flow, make sure the `QApplication` and `QMAppExtension` instances are created before calling it

## Example

The executable usually does nothing but loading the plugins, you should link `CKLoader` to it.

```c++
#include <QApplication>

#include <QMCoreAppExtension.h>

#include <loaderconfig.h>

class MyLoaderConfiguration : public LoaderConfiguration {
public:
    MyLoaderConfiguration() {
        QString appDir = QApplication::applicationDirPath();

        pluginIID = "org.MyOrginization.MyApplication.Plugin";
        splashSettingPath = appDir + "/config.json";
        userSettingsPath = appDir + "/settings";
        systemSettingsPath = appDir + "/settings";
        pluginPaths << (appDir + "/plugins");
    }

    bool preprocessArguments(QStringList &arguments, int *code) override {
        qDebug() << "Arguments:" << arguments;
        return true;
    }

    void beforeLoadPlugins(QSplashScreen *screen) override {
        screen->showMessage("Befor loading plugins");
    }

    void afterLoadPlugins() override {
        screen->showMessage("After loading plugins");
    }
}

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    QMAppExtension host;

    a.setApplicationName("MyApplication");
    a.setApplicationVersion("0.0.0.1");
    a.setOrganizationName("MyOrganization");
    a.setOrganizationDomain("org.MyOrganization");

    return MyLoaderConfiguration().run();
}
```