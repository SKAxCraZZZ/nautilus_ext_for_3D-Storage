Расширение для файлового менеджера Nautilus, для работы с виртуальным диском 3D–Storage (https://pilotems.com/ru/products/3d-storage/)
Отображает иконку состояния на смонтированных файлах.

Установить зависимости glib2.0 и libnautilus-extension
sudo apt install libnautilus-extension-dev libglib2.0-dev

Установить cmake 
https://cmake.org/download/

Скопировать иконки ('..\src\icons') в каталог эмблем используемой визуальной темы Linux (узнать название темы можно с помощью команды 'gsettings get org.gnome.desktop.interface icon-theme').
Пример пути для темы 'Yaru' - '/usr/share/icons/Yaru/48x48/emblems'
Может потребоваться дать права на чтение пользователю, от имени которого запускается nautilus.

запустить build.sh
