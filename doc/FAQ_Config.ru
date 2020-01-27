

Создатель Andrey Slusar 2:467/126, anray@users.sourceforge.net

Самую свежую версию этого документа можно получить написав нетмейлом письмо:
===
To: FAQServer 2:467/126
Subject: FIDOGATE
===
Если вы хотите внести изменения или дополнения в данный документ желательно
пишите по фидошному адресу.

Внимание! Для дополнени данного документа ищется информация по установке и
настройке других MTA: qmail, smail etc и news-серверов.

Также ищутся добровольцы для дополнения и перевода всей или части документации
по fidogate на английский язык.

=============================================================================
                        Содержание:

1.Начальная настройка системы и выбор используемого софта.
2.Установка и начальная настройка ньюссервера inn.
3.Установка и начальная настройка ньюссервера leafnode.
4.Установка и начальная настройка ньюссервера s-news.
5.Компиляция и установка FIDOGATE.
6.Установка и настройка MTA Exim.
7.Установка и настройка MTA Postfix.
8.Окончательная настройка совместной работы с FIDOGATE, MTA и ньюссервера.
9.Работа FIDOGATE.
10.Благодарности.

-----------------------------------------------------------------------------
1.Правим свой хост и домен для нормального хождения почты и ньюсов.
  
  Hапример пишем в /etc/hosts:

  Для "santinel" в домене "home.ua":

  === hosts ===
  127.0.0.1	localhost
  192.168.0.1	santinel santinel.home.ua
  ===

  В hostname:
  === hostname ===
  santinel
  ===

  Для полноценной работы фидогейта необходим MTA, ньюссервер, ньюсочиталка,
  читалка мейла, фидошный мейлер. В данном FAQ будет рассмотрена настройка
  ньюссерверов inn, leafnode, s-news и MTA postfix, exim в совместной работе
  с FIDOGATE. Прежде всего нужно определиться с выбором какой ньюссервер и
  МТА вы будете использовать.

-----------------------------------------------------------------------------
2.Установка inn:

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
  moderatormailer:        %s@santinel.home.ua
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

  Срок хранения в днях(можно с дробной частью 1.5 - полтора дня), по
  истечении которого из системы удаляются идентификаторы статей.

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

  * keep - определяет минимальное число дней хранения тел статей в системе
  	для определемых групп новостей. Любая статья из этих групп не
	может храниться меньше указанного этим полем срока (если Expires-
	заголовок статьи имеет меньший срок, то будет использовано
	значение keep).

  * default - определет число дней хранения для тех статей из определенных 
        групп, у которых отсутствует заголовок Expires.

  * purge - определет максимальное число дней хранения тел статей на
  	локальном сервере для определяемых групп новостей. Любая статья из
	этих групп не может храниться больше указанного этим полем срока 
	(если Expires-заголовок статьи имеет больший срок, то будет
	использовано значение purge).

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

-----------------------------------------------------------------------------
3.Установка leafnode:
  
   Нужен leafnode2 последняя альфа или бета не "плюс", так как "плюс" имеет
  некоторые отличия в формате groupinfo и interesting.groups.
   Сорсы leafnode2-alpha для сборки можно взять здесь:
  
   http://home.pages.de/~mandree/leafnode/beta/

  Распаковываем tar -xzf leafnode-2.0.0.alpha20040513a.tar.bz2, потом 
  cd leafnode-2.0.0.alpha20040513a, ./configure с нужными опциями, допустим
  c такими:

  ./configure --with-lockfile=/var/spool/leafnode/leaf.node/lock.file \
	      --with-spooldir=/var/spool/leafnode
  
  Делаем make && make install, cd /usr/local/leafnode/etc,
  cp config.example config, редактируем config. В config необходимо только 
  определить expire - максимальное время хранения непрочитанных статей и
  server, откуда будем брать сами статьи. Все остальное, закомментировать,
  стереть или отредактировать по вкусу. man leafnode было бы неплохо
  почитать.
  
  === config ===
  expire = 30
  ===

  Для inetd:
  Прописываем свой сервер в inetd.conf:
  === inetd.conf ===
  nntp stream tcp nowait news /usr/libexec/tcpd /usr/local/sbin/leafnode
  ===
  В services прописываем порт:
  === services ===
  nntp     119/tcp
  ===
  Очень желательно еще почитать INSTALL в пакете с leafnode по поводу
  hosts.allow, hosts.deny и конечно man inetd.conf.

  Для xinetd:
  === xinetd.conf ===
  service nntp
  {
    flags           = NAMEINARGS NOLIBWRAP
    socket_type     = stream
    protocol        = tcp
    wait            = no
    user            = news
    server          = /usr/sbin/tcpd
    server_args     = /usr/local/sbin/leafnode
    instances       = 7
    per_source      = 3
  }
  ===
  Потом делаем #kill -HUP `cat /var/run/[x]inetd.pid` и проверяем отвечает ли
  сервер:
  
  $telnet localhost 119
  
  Должны увидеть приглашение сервера.  
  Теперь нам понадобятся leafnode-util от Elohin Igor. Скачиваем их с сайта
  http://maint.unona.ru, распаковываем, смотрим README и делаем все, что
  там сказано, не забывая отредактировать common.h. Например так:
  
  === common.h ===
  [...]
  #define OUTGOING "/var/spool/leafnode2/out.going"
  #define FAILED_POSTING "/var/spool/leafnode2/failed.postings"
  #define IN_COMING "/var/spool/leafnode2/in.coming"
  #define DUPE_POST "/var/spool/leafnode2/dupe.post"
  #define INTERESTING_GROUPS "/var/spool/leafnode2/interesting.groups"
  #define GROUPINFO "/var/spool/leafnode2/leaf.node/groupinfo"
  #define LOCAL_GROUPINFO "/usr/local/etc/leafnode/local.groups"
  #define ACTIVE "/var/spool/leafnode2/leaf.node/active"
  #define RFC2FTN  "/usr/local/fidogate/libexec/rfc2ftn -n -v"
  #define DELETE_CTRL_D  "/usr/local/fido/leafnode/ctrld"
  #define DIRLOG  "/var/log/fidogate"
  #define NEWSLOGDIR  "/var/log/leafnode"
  #define MSGBUF 512
  #define LEAFNODE_OWNER "news"
  #define LOGNAME "leafnode"
  ===
  Потом делаем make && make install

------------------------------------------------------------------------------
4. Настройка s-news.
  Если у вас FreeBSD, то s-news лучше всего устанавливать прямо с портов.
  Для Linux берем последний s-news например с:

            http://www.lost.org.uk/s-news-0.1.11.tar.gz

  Можно взять патчи от Tony Houghton с http://www.realh.co.uk и наложить на сорсы.
  Патчи дают дополнительные возможности. В том числе если вы пользуетесь newsstar,
  то они вам понадобятся. Порт s-news FreeBSD уже содержит эти патчи.

   Компилируем, накладываем патчи, если у вас inetd, то прописываем s-news в
  inetd.conf:
  ===
  nntp stream tcp nowait news:news /usr/local/sbin/nntpd nntpd
  ===
  В services должен быть сервис nntp:
  ===
  nntp tcp/119
  ===
  По вкусу меняем hosts.allow/hosts.deny, killall -1 inetd и проверяем ответ
  сервера:
  $telnet localhost nntp

  Если отвечает, то делаем ньюсовые фиды для фидошных групп на fidogate от
  юзера news:
  $newfeed fidogate fido.*
  $newfeed fidogate cc.*
  Делаем карбонгруппу cc.carbon:
  $newgroup cc.carbon y my personal carbongroup
  Прописываем в expire длительность хранения артиклей на сервере для групп:
  ===
  *	30
  ===
  Все группы храним 30 дней.
  В access:
  ===
  stdin     rpx
  localhost rpx
  ===
  Разрешаем через stdin и локалхост постить артикли.
  Запускаем makehistory для создания history.

-------------------------------------------------------------------------------

5. Устанавливаем FIDOGATE:

5.1.Берем исходники последнего FIDOGATE с cvs(все в одну строчку):
  ===
  cvs -z3 -d:pserver:anonymous@cvs.rusfidogate.sourceforge.net:/cvsroot/rusfidogate \
  co fidogate
  ===
  
   Распаковываем исходники в папку fidogate, заходим в нее, смотрим
  ./configure --help, делаем ./configure с нужными опциями, 
  
  Например с такими:

  ----------------
  ./configure --prefix=/usr/local/fido \
              --with-logdir=/var/log/fido/gate \
              --with-vardir=/var/db/fidogate \
              --with-spooldir=/var/spool/fido/gate \
              --with-btbasedir=/var/spool/fido/bt \
              --disable-desc-dir
  -----------------

  Внимание!
  Если вы в качестве сервера используете НЕ inn, то fidogate необходимо 
  собирать с опцией --without-news.
  
  Описание токенов в конфигах можно прочитать в README.ru поставки
  FIDOGATE.

  Делаем "make", "make install".

5.2.Можно просто взять у меня на сайте http://node126.narod.ru файл 
  fidogate5.1.0_ds.src.rpm и перекомпилировать:

  Копируем этот файл в /usr/src/rpm/SRPMS, делаем: 
  
   cd /usr/src/rpm,
   rpm -i SRPMS/fidogate5.1.0_ds.src.rpm,
   rpm -ba SPECS/fidogate.spec
  
  Устанавливаем fidogate:
   
   rpm -i RPMS/i386/fidogate5.1.0_ds.rpm;

  или воспользоватьс готовым бинарным пакетом fidogate-5.1.0_ds.rpm:
  
   rpm -i fidogate5.1.0_ds.rpm.

  Для Debian:
   Пакеты для Debian лучше всего брать не у меня, а на
  http://kaliuta.basnet.by/debian, так как я больше не собираю пакетов для
  Debian.
   Устанавливаем пакет:
  dpkg -i fidogate5.1.0_ds.deb

-----------------------------------------------------------------------------

6.Установка и настройка exim4:
  Устанавливаем Exim4 так, чтобы вся фидошная почта *.fidonet.org ходила
  через гейт, интернет-почта через релей провайдера, локальная доставлялась
  локально:

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

-----------------------------------------------------------------------------

7.Установка и настройка Postfix:

  Устанавливаем Postfix и Procmail(не обязательно) так, чтобы ходила
  локальная почта:

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
    flags=F user=news argv=/usr/local/fido/libexec/ftnmail -- $recipient
  ftni   unix - n n - - pipe
    flags=F user=news argv=/usr/local/fido/libexec/rfc2ftn -i -- $recipient
  ftna   unix - n n - - pipe
     flags=F user=news argv=/usr/local/fido/libexec/rfc2ftn -a $nexthop -i \
  -- $recipient
  ftno   unix - n n - - pipe
    flags=F user=news argv=/usr/local/fido/libexec/ftnmail -a $nexthop -O \
  outpkt/$nexthop -i -- $recipient
  ===

-----------------------------------------------------------------------------
8.Окончательная настройка совместной работы с FIDOGATE.
 
 8.1. Если вы выбрали для работы с фидогейтом ньюссервер leafnode:
  Прежде всего необходимо прочитать FAQ в архиве с leafnode-util и INSTALL,
  FAQ, README в поставке leafnode.
  
  Правим fidogate.conf:

  === fidogate.conf ===
  [...]
  INN_BATCHDIR  /var/spool/leafnode2/out.going
  NEWSVARDIR    /usr/local/leafnode/etc
  NEWSLIBDIR    /usr/local/leafnode2/leaf.node
  NEWSBINDIR    /usr/local/leafnode/sbin
  NEWSSPOOLDIR  /var/spool/leafnode2
  [...]
  # rnews program path
  FTNInRnews      /usr/local/bin/rnews
  [...]
  # Newsserver scrips to create/renumber/remove newsgroups
  AutoCreateNewgroupCmd /usr/local/fido/leafnode/leafnode-group -M %s
  AutoCreateRenumberCmd /usr/local/fido/leafnode/leafnode-group -L %s
  AutoCreateRmgroupCmd  /usr/local/fido/leafnode/leafnode-group -r %s
  [...]
  ===
  
  Делаем #crontab -eu news и редактруем crontab:
  === news ===
  # Каждые 15 минут гейтуем запощенные на ньюссервер статьи.
  */15  *  * * *  /usr/local/fido/libexec/send-fidogate
  # Каждый день выполняем expire.
  0     21 * * *  /usr/local/sbin/texpire
  # Каждый день отписываемся от эх без даунлинков.
  00   22  * * *   /usr/local/bin/ftnafutil expire
  # Каждую неделю чистим areas.bbs от отписанных эх, т.е. со статусом "U".
  00   23  * * 1   /usr/local/bin/ftnafutil delete
  ===
  
   Все настроено. Можно переходить к пункту 9.

 8.2.Если вы выбрали для работы с fidogate ньюссервер s-news - окончательная
  настройка s-news:
  Правим fidogate.conf:
  ===
  INN_BATCHDIR    /usr/local/etc/s-news/suck
  NEWSVARDIR      /usr/local/etc/s-news
  NEWSLIBDIR      /usr/local/etc/s-news
  NEWSBINDIR      /usr/local/sbin
  NEWSSPOOLDIR    /var/spool/s-news
  [...]
  # Это обязательно. s-news работает только с отдельными артиклями, а не батчами.
  SingleArticles
  FTNInRnews      /usr/local/sbin/rnews
  [...]
  AutoCreateNewgroupCmd /usr/local/sbin/newgroup %s y fido group
  # Renumber закомментировать - он для s-news не нужен.
  #AutoCreateRenumberCmd
  AutoCreateRmgroupCmd  /usr/local/sbin/rmgroup %s
  ===
  Нужен специальный send-fidogate.snews, который есть в fidogate-devel. В нем нужно
  поправить путь до qnews. И еще - s-news для обновления overview артиклей требует
  запуска overview. Необходимо его запуск прописать после запуска runinc.
  Делаем crontab -eu news и пишем:
  ===
  # Каждые 15 минут гейтуем запощенные на ньюссервер статьи.
  */15  *  * * *  /usr/local/libexec/fidogate/send-fidogate.snews
  # Каждый день выполняем expire.
  0    21  * * * /usr/local/sbin/expire
  # Каждый день отписываемся от эх без даунлинков.
  00   22  * * *   /usr/local/bin/ftnafutil expire
  # Каждую неделю чистим areas.bbs от отписанных эх, т.е. со статусом "U".
  00   23  * * 1   /usr/local/bin/ftnafutil delete
  ===

  Все настроено. Можно переходить к пункту 9.

 8.3.Окончательная настройка inn:
  
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

  Прописываем в крон запуск send-fidogate, news.daily, flush cache, rnews -U:

  #crontab -e -u news и пишем:
  === news ===
  # Каждые 15 минут гейтуем запощенные на ньюссервер статьи.
  */15  *  * * *   /usr/local/fido/libexec/send-fidogate
  # Каждый день выполняем expire.
  30   21  * * *   /usr/local/news/bin/news.daily expireover lowmark delayrm
  # Каждый день чистим cache inn-а.
  40   21  * * *   /usr/local/news/bin/ctlinnd -t 300 -s reload \
  			incoming.conf "flush cache"
  # Каждый час постим залежалые в incoming inn-a артикли.
  10    *  * * *   test -x /usr/local/news/bin/rnews && \
  			/usr/local/news/bin/rnews -U
  # Каждый день отписываемся от эх без даунлинков.
  00   22  * * *   /usr/local/fido/bin/ftnafutil expire
  # Каждую неделю чистим areas.bbs от отписанных эх, т.е. со статусом "U".
  00   23  * * 1   /usr/local/fido/bin/ftnafutil delete
  ===
  Перезапускаем innd.

-----------------------------------------------------------------------------  
 9.Работа fidogate:

  Hа входящих для тоссинга прописать в мейлере или по крону запуск runinc.
  Hа исходящих для запаковки почты запускать runinc -o

=============================================================================
 
 10. Благодарности всем, кто помогал мне в составлении данного FAQ, оживлении
     проекта fidogate и активно присылающие патчи и пожелания.

 1. Alexandr Dobroslavskiy 2:5020/1356
 2. Andrew Zhuravlev 2:5035/67
 3. Peter Levchenko 2:463/672
 4. Elohin Igor 2:5070/222.52
 5. Zhenya Kaluta 2:450/254

 А также составителям "старого" FAQ по FIDOGATE.
 
=============================================================================
