# Знакомство с Linux и bash

## Задание
Пройти курс ["Введение в Linux"](https://stepik.org/course/73/syllabus)

- баллы за выполнение: 2
- работа сдается в виде сертификата за прохождение курса 

## Теоретическая справка
### UNIX подобные системы
Uniplexed Information and Computing Service - многопользовательская операционная система. Современными версиями ОС UNIX 
являются GNU/Linux, MacOS, Solaris, NetBSD, ...

Операционные системы семейства UNIX переносимы, основаны на принципе вытесняющей многозадачности, поддерживают 
асинхронные процессы, имеют иерархическую файловую систему, имеют абстракцию над устройствами ввода-вывода
### Дистрибутивы Linux
Дистрибутив представляет собой ряд решений для разных задач, объединенный единой системой установки и управления. Отметим наиболее распространенные семейства дистрибутивов Linux. Это семейства, основанные на дистрибутивах:
- Debian (относятся Debian, Ubuntu)
- RedHat (относятся RHEL, Fedora, Centos)
- Arch
- Android
- ...

У дистрибутивов первых двух семейств используются разные пакетные менеджеры (yum и rpm; apt-get), также они местами 
отличается структура системных каталогов.

Для первичного знакомства с Linux, как правило, подходит любой растпространенный дистрибутив, так как при использовании
не будут затрагиваться особенности дистрибутива. Лабораторные работы будут проверяться на Ubuntu.

### Структура файловой системы
Файлы программ в системе UNIX распределены по корневой файловой системе. Каждый подкаталог корневого 
каталога отвечает за определенные файлы программы. Знание структуры подкаталогов поможет при выполнении лабораторных 
работ, при отладке разработанных программ
- `/` - Корневой каталог. Рдительский каталог для всех файлов и каталогов системы. 
- `/home` - Каталог домашних каталогов всех пользователей, в которых хранятся личные файлы, настройки программ. Данный 
  каталог как правило размещен на отдельном разделе диска, чтобы при переустановке системы личные файлы сохранялись на диске.
- `/proc` - Каталог файлов запущенных процессов. Это файлы с pid процесса,параметрами запуска, информацией о 
  использовании RAM. В каталоге содержится информация о потреблении системных ресурсов, о системе.
- `/var` - Каталог часто изменяемых файлов с постоянно увеличивающимся размером. Это файлы системных журналов, кеши. 
  Подкаталоги этого каталога - `/var/log` содержит большинство файлов логов программ 
  (добавить ли про squid `/var/log/squid` ?...); `/var/lib` содержит файлы баз данных, файлы загруженные пакетным менеджером
- `/etc` - Каталог конфигурационных файлов программ и скрипты инициализации программ, скрипты запуска и завершения 
  системных демонов, автозагрузки программ и т.д.
- `/bin` - Каталог бинарных файлов пользователя. Содержит исполняемые файлы, которые могут использоваться, 
  когда не подключен каталог пользователя `/usr/`, например в режиме восстановления и в однопользовательском режиме.
- `/sbin` - Каталог системных бинарных файлов. Содержит исполняемые файлы, служащие дли обслуживания системы. 
  Также могут использоваться без каталога пользователя, на ранних этапах загрузки. 
- `/usr` - Каталог файлов пользователя: исполняемые файлы, исходные коды программ, ресурсы приложений
- `/dev` - Каталог файлов, связанных с подключенными к системе устройствами (напр. флешки, камеры). При загрузке 
  системы для каждого подключенного устройства создается такой файл.
- `/lib` - Каталог файлов системных библиотек.
- `/opt` - Каталог программ, состоящих из отдельного исполняемого файла, созданного производителями программы. 
  Конфигурационные файлы, библиотеки, бинарники таких программ хранятся в одной директории.
- `/run` - Каталог pid файлов запущенных процессов, в котором файла при перезагрузке удаляются.
- `/sys` - Каталог файлов информации о системе, получаемой от ядра. Данные файлы позволяют конфигурировать систему.
- `/boot` - Каталог файлов загрузчика.

Доступ к файлу и дериктории регулируется правами доступа:
- `u` - user: пользователь
- `g` - group: группа пользователей, которым необходим доступ к одним и тем же ресурсам
- `o` - other: все

Права доступа можно настраивать на каждое действие: чтение (`r`), запись (`w`) и исполнение (`x`, `X`). В случае 
директории право на исполнение означает возможность делать ее текущей и обращаться к файлам и поддиректориям.

Команда `chmod` служит для изменения прав доступа - [мануал](https://linuxcommand.org/lc3_man_pages/chmod1.html)

### Базовые команды

- `pwd` выводит путь в системе от корневого каталога до каталога, в котором находится пользователь
- `cd` переключает пользователя между каталогами по абсолютному или относительному пути
- `ls` выводит список файлов и каталогов, содержащихся в текущем каталоге
- `cat` выводит содержимое файла
- `mkdir` создает директорию по относительному и абсолютному пути
- `rmdir` удаляет директорию по относительному и абсолютному пути
- `touch` создает файл по указанному адресу
- `cp` копирует содержимое файла(директории)-параметра1 в файл(директорию)-параметр2
- `mv` перемещает файл из адреса1 в адрес2
- `man` выводит справку по другой команде


- `kill` (`pkill`,`killall`) отправить процессу сигнал SIGKILL по идентификатору (по имени, отправить всем с данным 
именем)
- `ps` (`ps axu`) выводит список с инфорацией о всех запущенных процессах (о всех активных процессах)
- `pidof` выдает идентификатор процесса по его имени
- `shutdown` выключает или перезагружает систему
### Синтаксис bash

Код, иллюстрирующий [синтаксис](https://learnxinyminutes.com/docs/bash/)