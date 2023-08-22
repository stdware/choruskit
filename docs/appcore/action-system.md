# ActionSystem

`Core::ActionSystem` manages the metadata of menu items and organizing rules of each menu context, including `QToolBar`, `QMenuBar`, `QMenu`.

## Menu Context

+ Editing applications usually have menubars, toolbars and pop-up menus, we call them as top level menu.

+ Each top level menu has its sub-menus and actions and even widgets, we call theme as menu item.

+ For a scalable application, we can't hard-code the menu contexts construction, because the plugins usually need to add their own menu items to the existing menus. As a result, we should set some rules to support building scalable, flexible and well-organized menu contexts.

+ Users may need to adjust the layout of the menu contexts.

## Classes

### ActionSpec

`Core::ActionSpec` is a metadata class holding the properties of a menu item specified by a unique ID.

+ `commandName`: Name on command palette, effective only on `QAction`
+ `commandDisplayName`: Display name, can be none
+ `shortcuts`: Shortcuts, effective only on `QAction`
+ `icon`: Icon

### ActionDomain

`Core::ActionDomain` is a metadata class holding the layout of a top level menu.

### ActionItem

`Core::ActionItem` is the wrapper of a menu item instance. If the menu item is a widget, you should construct it by passing the constructing factory of the widget.