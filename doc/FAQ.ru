
		Установка FIDOGATE ver 1.1

Создатель Andrey Slusar 2:467/126, rusfidogate at users.sourceforge.net

Самую свежую версию этого документа можно получить написав нетмейлом письмо:
===
To: FAQServer 2:467/126
Subject: FIDOGATE
===
 Если у вас FreeBSD, можно получить нетмейлом самый свежий порт:
===
To: FAQServer 2:467/126
Subject: GATE-PORT
===
Если вы хотите внести изменения или дополнения в данный документ желательно
пишите по фидошному адресу.

Внимание! Для дополнени данного документа ищется информация по установке и
настройке других MTA: qmail, smail etc и news-серверов.

===============================================================================
                        Содержание:

1.Начальная настройка системы.
2.Установка и начальная настройка Exim4.
3.Установка и начальная настройка Postfix и Procmail.
4.Установка и настройка ньюссервера leafnode.
5.Установка и настройка ньюссервера inn.
6.Компиляция и установка FIDOGATE.
7.Окончательная настройка совместной работы с FIDOGATE MTA и ньюссервера.
8.Работа FIDOGATE-DS.
9.Часто задаваемые вопросы по работе.

------------------------------------------------------------------------------
1.Правим свой хост и домен для нормального хождения почты и ньюсов.
  
  Hапример пишем в /etc/hosts:

  Для "santinel" в домене "home.ua":

  === hosts ===
  127.0.0.1	localhost
  192.168.0.1	santinel.home.ua santinel
  ===

  В hostname:
  === hostname ===
  santinel
  ===
-------------------------------------------------------------------------------
2.Установка и настройка exim4:
  Устанавливаем Exim4 так, чтобы вся фидошная почта *.fidonet.org ходила через
  гейт, интернет-почта через релей провайдера, локальная доставлялась локально:

  === configure ===
  ######################################################################
  #                    MAIN CONFIGURATION SETTINGS                     #
  ######################################################################

  primary_hostname = santinel.home.ua
  domainlist local_domains = @ : @[] : localhost : santinel.home.ua
  domainlist relay_to_domains = localhost : santinel.home.ua
  hostlist   relay_from_hosts = 127.0.0.1

  acl_smtp_rcpt = acl_check_rcpt

  # Баннер мэйлсервера
  smtp_banner = "ESMTP Exim"
  
  # Юзер и группа, от которых запускается демон Exim.
  exim_user  = mailnull
  exim_group = mail

  # Юзер и группа, которые имеют право на sendmail envelope-from
  trusted_users  = mailnull
  trusted_groups = mail

  never_users = root

  host_lookup = santinel.home.ua

  rfc1413_hosts = *
  rfc1413_query_timeout = 30s

  # Баунс-мессаги держим 2 дня в спуле
  ignore_bounce_errors_after = 2d
  
  # Фрозены 7 дней.
  timeout_frozen_after = 7d

  ######################################################################
  #                       ACL CONFIGURATION                            #
  #         Specifies access control lists for incoming SMTP mail      #
  ######################################################################

  begin acl

  acl_check_rcpt:

  accept  hosts = 127.0.0.1/8

  deny    local_parts   = ^.*[@%!/|] : ^\\.

  accept  local_parts   = postmaster
          domains       = +local_domains

  require verify        = sender

  accept  domains       = +local_domains
          endpass
          message       = unknown user
          verify        = recipient

  accept  domains       = +relay_to_domains
          endpass
          message       = unrouteable address
          verify        = recipient

  accept  hosts         = +relay_from_hosts

  accept  authenticated = *

  deny    message       = relay not permitted

  ######################################################################
  #                      ROUTERS CONFIGURATION                         #
  #               Specifies how addresses are handled                  #
  ######################################################################

  begin routers

  # Ищем фидошные адреса. Здесь нужно вписать свой домен в FIDO.

  fidonet:
    driver = manualroute
    domains = ! +local_domains
    route_list = *.fidonet.org f126.n467.z2.fidonet.org
    transport = fidogate
  
  # Все остальное - на релей своего провайдера smtp.my.provider.
  internet:
    driver = manualroute
    domains = ! +local_domains
    route_list = * smtp.my.provider
    transport = remote_smtp
    ignore_target_hosts = 127.0.0.0/8
    no_more
  
  # Обрабатываем системные алиасы.
  system_aliases:
    driver = redirect
    allow_fail
    allow_defer
    data = ${lookup{$local_part}lsearch{/etc/aliases}}
    user = mailnull
    group = mail
    file_transport = address_file
    pipe_transport = address_pipe

  # Локальная почта
  localuser:
    driver = accept
    check_local_user
    transport = local_delivery
  
  ######################################################################
  #                      TRANSPORTS CONFIGURATION                      #
  ######################################################################

  begin transports

  fidogate:
    driver = pipe
    command = "/usr/local/fido/libexec/rfc2ftn -i ${pipe_addresses}"
    user = news
    group = news

  remote_smtp:
    driver = smtp

  # Пользуемся unix mailbox
  local_delivery:
    driver = appendfile
    file   = /var/mail/$local_part
    delivery_date_add
    envelope_to_add
    return_path_add
    group = mail
    user = $local_part
    mode = 0660
    no_mode_fail_narrower

  address_pipe:
    driver = pipe
    return_output

  address_file:
    driver = appendfile
    delivery_date_add
    envelope_to_add
    return_path_add

  ######################################################################
  #                      RETRY CONFIGURATION                           #
  ######################################################################

  begin retry

  *                      *           F,2h,15m; G,16h,1h,1.5; F,4d,6h

  ######################################################################
  #                      REWRITE CONFIGURATION                         #
  ######################################################################

  begin rewrite

  ######################################################################
  #                   AUTHENTICATION CONFIGURATION                     #
  ######################################################################

  begin authenticators
  ===
  
  Проверяем правильность конфигфайла - для этого делаем:
  
  #exim -bV
  
  Если все правильно, то уже запускаем MTA штатным способом через rc-скрипты.
  
------------------------------------------------------------------------------
3.Устанавливаем Postfix и Procmail(не обязательно) так, чтобы ходила локальная 
  почта:

  Вот пример работающего main.cf Postfix'а c procmail'ом:

  === main.cf ===
  command_directory = /usr/sbin
  daemon_directory  = /usr/lib/postfix
  program_directory = /usr/lib/postfix

  smtpd_banner = $myhostname ESMTP $mail_name
  setgid_group = postdrop
  biff = no

  # appending .domain is the MUA's job.
  append_dot_mydomain = yes
  myhostname = santinel.home.ua
  mydomain = home.ua
  alias_maps = hash:/etc/aliases
  transport_maps = hash:/etc/postfix/transport
  alias_database = hash:/etc/aliases
  myorigin = /etc/mailname
  mydestination = santinel.home.ua
  mynetworks = 127.0.0.0/8
  mailbox_command = procmail -a "$EXTENSION"
  mailbox_size_limit = 0
  ===
  
  В большинстве случаев достаточно просто вписать путь к transport_maps

  Редактируем файл /etc/aliases:
  Желательно прописать юзера, который будет получать письма для root'а:

  root: pupkin

  Даем команду(от root) newaliases. Желательно почитать man sendmail.

-------------------------------------------------------------------------------
4.Установка leafnode:
  
   leafnode лучше всего брать последнюю бету не "плюс", так как "плюс" имеет
  некоторые отличия в формате groupinfo и interesting.groups.
   Для freebsd порт и готовый пакадж leafnode-2.0b8_ma9 можно взять на моем 
  сайте http://node126.narod.ru
   Сорсы для сборки можно взять здесь:
  
   ftp://wpxx02.toxi.uni-wuerzburg.de/pub/leafnode-2.0b8_ma9.tar.gz
  
  Распаковываем tar -xzf leafnode-2.0b8_ma9.tar.gz, потом cd leafnode-2.0b8_ma9,
  ./configure с нужными опциями, допустим c такими:

  ./configure --with-lockfile=/var/spool/leafnode/leaf.node/lock.file \
	      --with-spooldir=/var/spool/leafnode
  
  Делаем make && make install, cd /usr/local/leafnode/etc,
  cp config.example config, редактируем config. В config необходимо только 
  определить expire - максимальное время хранения непрочитанных статей и server,
  откуда будем брать сами статьи. Все остальное, закомментировать, стереть или
  отредактировать по вкусу. man leafnode было бы неплохо почитать.
  
  === config ===
  server = localhost
  expire = 30
  ===
  
  Прописываем свой сервер в inetd.conf:
  === inetd.conf ===
  nntp stream tcp nowait news /usr/libexec/tcpd /usr/local/sbin/leafnode
  ===
  В services прописываем порт:
  === services ===
  nntp     119/tcp
  ===
  Очень желательно еще почитать INSTALL в пакете с leafnode по поводу hosts.allow,
  hosts.deny и конечно man inetd.conf.
  Потом делаем #kill -HUP `cat /var/run/inetd.pid` и проверяем отвечает ли сервер:
  $telnet localhost 119
  
  Должны увидеть приглашение сервера.  
  Теперь нам понадобятся leafnode-util от Elohin Igor. Скачиваем их с сайта
  http://maint.unona.ru, распаковываем, смотрим README и делаем все, что там 
  сказано, не забывая отредактировать common.h. Например так:
  === common.h ===
  [...]
  #define OUTGOING "/var/spool/leafnode/out.going"
  #define FAILED_POSTING "/var/spool/leafnode/failed.postings"
  #define IN_COMING "/var/spool/leafnode/in.coming"
  #define DUPE_POST "/var/spool/leafnode/dupe.post"
  #define INTERESTING_GROUPS "/var/spool/leafnode/interesting.groups"
  #define GROUPINFO "/var/spool/leafnode/leaf.node/groupinfo"
  #define LOCAL_GROUPINFO "/usr/local/etc/leafnode/local.groups"
  #define ACTIVE "/var/spool/leafnode/leaf.node/active"
  #define RFC2FTN  "/usr/local/fido/libexec/rfc2ftn -n -v"
  #define DELETE_CTRL_D  "/usr/local/fido/leafnode/ctrld"
  #define DIRLOG  "/var/log/fidogate"
  #define NEWSLOGDIR  "/var/log/leafnode"
  #define MSGBUF 512
  #define LEAFNODE_OWNER "news"
  #define LOGNAME "leafnode"
  ===
  Потом делаем make && make install
  
-------------------------------------------------------------------------------
5.Установка inn:

  Допустим хост называется santinel.home.ua:

  === inn.conf ===
  innflags:               -c0 -u
  [...]
  server:                 santinel.home.ua
  pathhost:               santinel.home.ua
  fromhost:               santinel.home.ua
  domain:                 home.ua
  nnrpdposthost:          santinel.home.ua
  nnrpdposrport:          119
  moderatormailer:        %s@f126.n467.z2.fidonet.org
  ===

  Все остальное можно оставить по умолчанию или почитать man inn.conf и в
  соответствии с ним отредактировать.

  Редактируем expire.ctl:

  === expire.ctl ===
  /remember/:20
  *:A:1:7:15
  ===

  Формат:

  /remember/:время

  Срок хранения в днях(можно с дробной частью 1.5 - полтора дня), по истечении
  которого из системы удаляются идентификаторы статей.

  Вторая строка определет, когда из системы нужно удалять тела статей:
 
  pattern:modflag:keep:default:purge

  * pattern - образец для групп новостей в wildmat-формате, для которого
    применяется остаток данной строки;

  * modflag - флаг, используемый для дальнейшего ограничения списка новостей, 
  для которых использовать данную строку. 
  
  Описание полей:
          o М - то применть остаток данной строки только для групп с ведущим
            (moderated groups) из тех, которые соответствуют образцу pattern
	     первого поля;
          o U - то применть остаток данной строки только для групп без
            ведущего (unmoderated groups) из тех, которые соответствуют
            образцу pattern первого поля;
          o A - то применть остаток данной строки для всех групп новостей,
            соответствущих образцу pattern первого поля.
  
  Обычно достаточно "A" в modflag'е.
  
  Следующие 3 поля определют какое количество дней хранить тела статей на
  локальном сервере новостей:

  * keep - определяет минимальное число дней хранения тел статей в системе для
         определемых групп новостей. Любая статья из этих групп не может
	 хранится меньше указанного этим полем срока (если Expires-заголовок 
	 статьи имеет меньший срок, то будет использовано значение keep).

  * default - определет число дней хранения для тех статей из определенных 
         групп, у которых отсутствует заголовок Expires.

  * purge - определет максимальное число дней хранения тел статей на локальном
         сервере для определяемых групп новостей. Любая статья из этих групп не
  может храниться больше указанного этим полем срока (если Expires-
  заголовок статьи имеет больший срок, то будет использовано значение
  purge).

  Желательно почитать man expire.ctl

  Редактируем readers.conf:

  readers.conf определет доступ пользователей к серверу новостей:

  === readers.conf ===
  [skip]
  auth santinel.home.ua {
    hosts: "localhost, santinel.home.ua, 127.0.0.1, stdin"
    default: <LOCAL>
   }

  access full {
    newsgroups: *
   }
  ===
  man readers.conf было бы неплохо почитать.

   Этим самым дается полный доступ по своему локальному хосту ко всем группам 
  новостей на своем сервере.
   
    Редактируем incoming.conf:
  
   В этом файле прописываются хосты, с которых берутся группы новостей для 
  сервера:
  
  === incoming.conf ===
  peer ME {
  hostname:  "santinel.home.ua, localhost, 127.0.0.1"
  }
  ===
  
   Пишем в storage.conf:
  
  === storage.conf ===
  method tradspool {
  newsgroups: *
  class: 0
                   }
  ===

-------------------------------------------------------------------------------
6. Устанавливаем FIDOGATE:

6.1.Берем исходники последнего FIDOGATE с cvs(все в одну строчку):
  ===
  cvs -z3 -d:pserver:anonymous@cvs.rusfidogate.sourceforge.net:/cvsroot/rusfidogate \
  co fidogate
  ===
  
   Распаковываем исходники в папку fidogate, накладываем нужные патчи, заходим 
  в нее, смотрим ./configure --help, делаем ./configure с нужными опцими, 
  
  Например с такими:

  ----------------
  ./configure --prefix=/usr/local/fido \
              --with-logdir=/var/log/fido/gate \
              --with-vardir=/var/db/fidogate \
              --with-spooldir=/var/spool/fido/gate \
              --with-btbasedir=/var/spool/fido/bt \
              --enable-amiga-out \
              --disable-desc-dir
  -----------------

  Внимание!
  Если вы в качестве сервера используете НЕ inn, то fidogate необходимо 
  собирать с опцией --without-news.
  
  Описание токенов в конфигах можно прочитать в README или README.en поставки
  FIDOGATE.

  Делаем "make", "make install".

6.2.Можно просто взять у меня на сайте http://node126.narod.ru файл 
  fidogate5.1.0-beta1.src.rpm и перекомпилировать:

  Копируем этот файл в /usr/src/rpm/SRPMS, делаем: 
  
   cd /usr/src/rpm,
   rpm -i SRPMS/fidogate5.1.0-beta1.src.rpm,
   rpm -ba SPECS/fidogate.spec
  
  Устанавливаем fidogate:
   
   rpm -i RPMS/i386/fidogate5.1.0-beta1.rpm;

  или воспользоватьс готовым бинарным пакетом fidogate-5.1.0-beta1.rpm:
  
   rpm -i fidogate5.1.0-beta1.rpm.

  Для Debian:
   Пакеты для Debian лучше всего брать не у меня, а на kaliuta.basnet.by/debian,
  так как я больше не собираю пакетов для Debian.
   Устанавливаем пакет:
  dpkg -i fidogate5.1.0-beta1.deb

6.3. Для freebsd можно взять пакадж с моего сайта:
  http://node126.narod.ru/files/fg{дата последней сборки}.tgz
  и установить командой pkg_add.
  Или воспользоваться моим портом fidogate5.tar.gz, положив его в 
  /usr/ports/news/fidogate5

-------------------------------------------------------------------------------
7.Окончательная настройка совместной работы с FIDOGATE.
 
 7.1. leafnode.
  Прежде всего необходимо прочитать FAQ в архиве с leafnode-util и INSTALL, FAQ,
  README в поставке leafnode.
  
  Правим fidogate.conf:

  === fidogate.conf ===
  [...]
  INN_BATCHDIR  /var/spool/leafnode/out.going
  NEWSVARDIR    /usr/local/leafnode/etc
  NEWSLIBDIR    /usr/local/leafnode
  NEWSBINDIR    /usr/local/leafnode/sbin
  NEWSSPOOLDIR  /var/spool/leafnode
  [...]
  # rnews program path
  FTNInRnews      /usr/local/fido/leafnode/rnews -c
  [...]
  # Newsserver scrips to create/renumber/remove newsgroups
  AutoCreateNewgroupCmd /usr/local/fido/leafnode/leafnode-group -M %s
  AutoCreateRenumberCmd /usr/local/fido/leafnode/leafnode-group -L %s
  AutoCreateRmgroupCmd  /usr/local/fido/leafnode/leafnode-group -r %s
  [...]
  ===
  
  Делаем #crontab -e -u news и редактруем crontab:
  === news ===
  # Каждые 15 минут гейтуем запощенные на ньюссервер статьи.
  */15  *  * * *  /usr/local/libexec/fidogate/send-fidogate
  # Каждый день выполняем expire.
  0     21 * * *  /usr/local/sbin/texpire
  # Каждый день отписываемся от эх без даунлинков.
  00   22  * * *   /usr/local/bin/ftnafutil expire
  # Каждую неделю чистим areas.bbs от отписанных эх, т.е. со статусом "U".
  00   23  * * 1   /usr/local/bin/ftnafutil delete
  ===
  
   Все настроено. Можно переходить к пункту 9.
  
 7.2.Настройка inn.
  
  Редактируем newsfeeds inn'a:

  Комментируем все, что там незакомментировано и пишем:

  === newsfeeds ===
  ME:*/!junk,!control::

  fidogate\
     :*,!cc,\
     !junk,\
     !control\
     :Tf,Wnb:fidogate
  ===

  От юзера news даем команду crontab -e и прописываем там запуск скрипта
  send-fidogate допустим каждые 15мин., прописываем запуск news.daily,
  flush cache на каждый день, rnews -U:

  #crontab -e -u news и пишем:
  === news ===
  # Каждые 15 минут гейтуем запощенные на ньюссервер статьи.
  */15  *  * * *   /usr/local/libexec/fidogate/send-fidogate
  # Каждый день выполняем expire.
  30   21  * * *   news.daily expireover lowmark delayrm
  # Каждый день чистим cache inn-а.
  40   21  * * *   ctlinnd -t 300 -s reload incoming.conf "flush cache"
  # Каждый час постим залежалые в incoming inn-a артикли.
  10    *  * * *   test -x /usr/local/news/bin/rnews && rnews -U
  # Каждый день отписываемся от эх без даунлинков.
  00   22  * * *   /usr/local/bin/ftnafutil expire
  # Каждую неделю чистим areas.bbs от отписанных эх, т.е. со статусом "U".
  00   23  * * 1   /usr/local/bin/ftnafutil delete
  ===
  Перезапускаем innd.

 7.3.Postfix.
 
  Следующий этап нужен только при использовании MTA Postfix. Если у Вас Exim,
  то переходим к этапу 6.3
  
  Редактируем transport Postfix'a:

  Добавлем туда содержимое <fidogate src>/doc/mailer/postfix/transport.
  От root даем команду "postmap /etc/postfix/transport" для создания
  transport.db

  Желательно почитать man 5 transport.

  Добавляем содержимое <fidogate src>/doc/mailer/postfix/master.cf

  Вот пример правильного master.cf(фидошная часть):

  === master.cf ===
  [skip]
  ftn   unix - n n - - pipe
    flags=F user=news argv=/usr/local/fidogate/libexec/ftnmail -- $recipient
  ftni   unix - n n - - pipe
    flags=F user=news argv=/usr/local/fidogate/libexec/rfc2ftn -i -- $recipient
  ftna   unix - n n - - pipe
     flags=F user=news argv=/usr/local/fidogate/libexec/rfc2ftn -a $nexthop -i \
  -- $recipient
  ftno   unix - n n - - pipe
    flags=F user=news argv=/usr/local/fidogate/libexec/ftnmail -a $nexthop -O \
  outpkt/$nexthop -i -- $recipient
  ===

---------------------------------------------------------------------------------  
 8.Работа fidogate:

  Hа входящих для тоссинга прописать в мейлере или по крону запуск runinc.
  Hа исходящих для запаковки почты запускать runinc -o

=================================================================================
=================================================================================

 9.Ответы на часто задаваемые вопросы.

=================================================================================
=================================================================================
  
  Q1:У меня inn не запускается. Пишет, что нет history-файла, хотя такой файл
     на самом деле существует.
   
  A1:Необходимо создать корректный history inn'a:

    От root ввести:
    ===
    su news
    makehistory -b -f history -O -l 30000 -I
    makedbz -f history -i -o -s 30000
    exit
    /etc/init.d/inn2 start
    ===
    Все.
  
---------------------------------------------------------------------------------

  Q2:У меня постоянные проблемы с электроэнергией, а UPS нет. В результате часто
     падает inn или не хотят обрабатываться ньюсовые батчи - все летят в bad.
    
  A2:Для повышения надежности работы ньюссервера можно вместо storage метода 
     tradspool поставить timehash. Для этого достаточно просто прописать в 
     storage.conf:
  
  === storage.conf ===
  method timehash {
  newsgroups: *
  class: 0
                  }
  ===		
       
    Как временный метод - можно исправить active-файл и overview таким скриптом:
  
  === inn-recover.sh ===
  #!/bin/sh
  /usr/local/etc/rc.d/innd.sh stop
  su news -c "/usr/local/news/bin/makehistory -b -f history -O -l 30000 -I"
  /usr/local/etc/rc.d/innd.sh start
  for act in `cat /usr/local/news/db/active | awk '{print $1}'`
  do
   su news -c "/usr/local/news/bin/ctlinnd renumber $act"
  done
  ===
   Пути к соответствующим файлам подправить.

-----------------------------------------------------------------------------------
  
  Q3:Использую inn в качестве ньюссервера. Почему send-fidogate не гейтует в pkt
    исходящие мессаги, а в log-news сыпятся следующие ошибки:
    
    === log-news ===
    Aug 21 00:04:51 rfc2ftn WARNING: can't open /usr/local/news/spool/articles/ \
    @050000000017000017AB0000000000000000@ (errno=2: No such file or directory)
    ===

  A3:Дело в том, что в последних версиях INN используется storage API и для
    правильной работы fidogate нужно поправить send-fidogate:

    Ищем в send-fidogate строку: 
    
     time $RFC2FTN -f $BATCHFILE -m 500

    И меняем ее на такую:

     time $PATHBIN/batcher -N $QUEUEJOBS -b500000 -p"$RFC2FTN -b -n" $SITE $BATCHFILE

    Также рекомендуется man batcher.

-------------------------------------------------------------------------------------

  Q4:Все вроде настроил правильно, но при запуске runinc почему-то ничего не делает
     - не запускает тоссинга. В логах все пусто. Что делать?
    
  A4:Убедиться, что локдир фидогейта существует и что runinc-у хватает прав на 
     запись, чтение и запуск. 

-------------------------------------------------------------------------------------
    
  Q5:Поставил leafnode 1.x и leafnode-util от Elohin Igor, создаю группы 
     leafnode-group. groupinfo меняется, но leafnode не видит созданных групп.
    
  A5:leafnode-group работает только с leafnode 2.x не "плюс". С остальными версиями
     leafnode он не будет работать.

-------------------------------------------------------------------------------------
    
  Q6:Поставил leafnode, прописал его как сказано в данном FAQ в inetd.conf и services,
     сделал kill -HUP `cat /var/run/inetd.pid`, но telnet localhost 119 не работает.
  
  A6:Необходимо наконец прочитать INSTALL в пакете leafnode и прописать правильно
     доступ в hosts.allow и, если у вас Linux то hosts.deny.

-------------------------------------------------------------------------------------

  Q7:Стоит inn. Почему при запуске configure не может найти rnews и не хочет из-за
     этого ничего конфигурить и создавать мэйкфайлы?
  
  А7:Дело в том, что rnews обычно имеет права news:news а юзер, запустивший скрипт
     configure, не имеет на rnews прав. Для того, чтоб configure проработал корректно,
     необходимо либо добавить юзера, собирающего фидогейт, в группу news либо собирать
     от root.

---------------------------------------------------------------------------------------

  Q8:Почему эхомейл тоссится, но в ньюсгруппах сообщения не появляются? В логах
     следующее:
     ===
     Oct 18 22:21:16 ftntoss packet /var/spool/bt/pin/9192da0c.pkt (1622b) from
     2:450/256.0 to 2:450/256.1
     Oct 18 22:21:16 ftntoss WARNING: node 2:450/256.0 have null password
     ===

  A8:Если у вас ходят непарольные пакеты, то не следует указывать в passwd на них пароли.
     Логично, не правда ли? В общем удали в passwd строки вида:
     === passwd ===
     packet  2:5030/1469             XXXXXXXX
     packet  2:5030/1229.0           XXXXXXXX
     packet  2:5030/1229.5           XXXXXXXX
     packet  2:5030/1229.6           XXXXXXXX
     packet  2:5030/1229.7           XXXXXXXX
     packet  2:5030/1229.8           XXXXXXXX
     ===

======================================================================================
======================================================================================
