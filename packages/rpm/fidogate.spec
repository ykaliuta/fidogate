Summary: Fido-Internet Gateway and Fido Tosser
Name: fidogate
Version: 5.1.0ds
Release: beta1
Copyright: GPL
Group: Fidonet/Gate
Source0: http://node126.narod.ru/fidogate%{version}-%{release}.tar.bz2
#Patch0: %{name}-%{version}-%{release}.patch
BuildRoot: %{_tmppath}/%{name}-root
Requires: 

%description
FIDOGATE-DS Version 5
 * Fido-Internet Gateway
 * Fido FTN-FTN Gateway
 * Fido Mail Processor
 * Fido File Processor
 * Fido Areafix/Filefix

%prep
%setup -q -n %{name}
#%patch0 -p0

./configure --prefix=/usr \
	    --with-sysconfdir=/etc/fido/gate \
	    --with-newsbindir=/usr/lib/news/bin \
	    --with-logdir=/var/log/fido/gate \
	    --with-vardir=/var/lib/fidogate \
	    --with-spooldir=/var/spool/fido/gate \
	    --with-btbasedir=/var/spool/fido/bt \
	    --enable-amiga-out \
	    --disable-desc-dir

%build
make DEBUG=-O2

%install
rm -rf $RPM_BUILD_ROOT

mkdir -p $RPM_BUILD_ROOT/usr/bin
mkdir -p $RPM_BUILD_ROOT/usr/lib
mkdir -p $RPM_BUILD_ROOT/usr/libexec
mkdir -p $RPM_BUILD_ROOT/etc/fidogate
mkdir -p $RPM_BUILD_ROOT/etc/news
mkdir -p $RPM_BUILD_ROOT/etc/postfix
mkdir -p $RPM_BUILD_ROOT/var/lib/fidogate
mkdir -p $RPM_BUILD_ROOT/var/lib/fidogate/seq
mkdir -p $RPM_BUILD_ROOT/var/spool/fido/bt
mkdir -p $RPM_BUILD_ROOT/var/spool/fido/bt/pin/bad
mkdir -p $RPM_BUILD_ROOT/var/spool/fido/bt/pin/tmpunpack
mkdir -p $RPM_BUILD_ROOT/var/spool/fido/bt/in/bad
mkdir -p $RPM_BUILD_ROOT/var/spool/fido/bt/in/tmpunpack
mkdir -p $RPM_BUILD_ROOT/var/spool/fido/bt/amiga.out
mkdir -p $RPM_BUILD_ROOT/var/spool/fido/bt/fbox
mkdir -p $RPM_BUILD_ROOT/var/spool/fido/bt/tick
mkdir -p $RPM_BUILD_ROOT/var/spool/fido/gate
mkdir -p $RPM_BUILD_ROOT/var/spool/fido/gate/outpkt
mkdir -p $RPM_BUILD_ROOT/var/spool/fido/gate/outpkt/mail
mkdir -p $RPM_BUILD_ROOT/var/spool/fido/gate/outpkt/news
mkdir -p $RPM_BUILD_ROOT/var/spool/fido/gate/outrfc
mkdir -p $RPM_BUILD_ROOT/var/spool/fido/gate/outrfc/mail
mkdir -p $RPM_BUILD_ROOT/var/spool/fido/gate/outrfc/news
mkdir -p $RPM_BUILD_ROOT/var/spool/fido/gate/toss
mkdir -p $RPM_BUILD_ROOT/var/spool/fido/gate/toss/bad
mkdir -p $RPM_BUILD_ROOT/var/spool/fido/gate/toss/pack
mkdir -p $RPM_BUILD_ROOT/var/spool/fido/gate/toss/route
mkdir -p $RPM_BUILD_ROOT/var/spool/fido/gate/toss/toss
mkdir -p $RPM_BUILD_ROOT/var/log/fidogate
mkdir -p $RPM_BUILD_ROOT/var/lock/fidogate

make DESTDIR=$RPM_BUILD_ROOT BINDIR=$RPM_BUILD_ROOT/usr/bin LIBEXECDIR=$RPM_BUILD_ROOT/usr/libexec  install

install -o news -g news doc/news/inn/newsfeeds.fidogate	$RPM_BUILD_ROOT/etc/news/newsfeeds-fidogate
install -o news -g news doc/mailer/postfix/master.cf	$RPM_BUILD_ROOT/etc/postfix/master.cf-fidogate
install -o news -g news doc/mailer/postfix/transport	$RPM_BUILD_ROOT/etc/postfix/transport-fidogate

%clean
rm -rf $RPM_BUILD_ROOT

%files
%doc COPYING TODO TODO.rus doc/README doc/README.en

%dir %attr(700,news,news)  /etc/fido/gate
%dir %attr(700,news,news)  /var/log/fido/gate
%attr(-,news,news) /usr/bin/*
%attr(-,news,news) /usr/libexec/*
%attr(-,news,news) /usr/lib/*
%dir %attr(700,news,news) /var/lib/fidogate
%dir %attr(700,news,news) /var/lib/fidogate/seq
%attr(700,news,news) /var/lock/fidogate
%attr(700,news,news) /var/spool/fido/gate
%attr(770,news,news) /var/spool/fido/bt
%attr(600,news,news) /etc/news/newsfeeds-fidogate
%attr(600,news,news) /etc/postfix/master.cf-fidogate
%attr(600,news,news) /etc/postfix/transport-fidogate
%config(noreplace) %attr(600,news,news) /etc/fidogate/acl.sample
%config(noreplace) %attr(600,news,news) /etc/fidogate/aliases.sample
%config(noreplace) %attr(600,news,news) /etc/fidogate/areas.sample
%config(noreplace) %attr(600,news,news) /etc/fidogate/fidogate.conf.sample
%config(noreplace) %attr(600,news,news) /etc/fidogate/fidokill.sample
%config(noreplace) %attr(600,news,news) /etc/fidogate/ftnacl.sample
%config(noreplace) %attr(600,news,news) /etc/fidogate/hosts.sample
%config(noreplace) %attr(600,news,news) /etc/fidogate/packing.sample
%config(noreplace) %attr(600,news,news) /etc/fidogate/passwd.sample
%config(noreplace) %attr(600,news,news) /etc/fidogate/routing.sample
%config(noreplace) %attr(600,news,news) /etc/fidogate/spyes.sample
%config(noreplace) %attr(600,news,news) /var/lib/fidogate/areas.bbs.sample
%config(noreplace) %attr(600,news,news) /var/lib/fidogate/fareas.bbs.sample
