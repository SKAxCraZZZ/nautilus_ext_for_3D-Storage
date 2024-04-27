## Описание
Расширение для файлового менеджера Nautilus, для работы с виртуальным диском 3D–Storage (https://pilotems.com/ru/products/3d-storage/)
- Отображает иконку состояния на смонтированных файлах;
- Добавляет контекстное меню для взаимодействия с 3D-Storage.

## Установка

Установить зависимости glib2.0 и libnautilus-extension:

    sudo apt install libnautilus-extension-dev libglib2.0-dev

Установить [cmake](https://cmake.org/download/). 



Установить иконки:

    ./icons_install.sh

Собрать и установить расширение:

    ./build.sh.

