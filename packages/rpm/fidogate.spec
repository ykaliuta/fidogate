Summary: Fido-Internet Gateway and Fido Tosser
Name: fidogate
Version: 5.2.0ds
Release: alpha1
Copyright: GPL
Group: Fidonet/Gate
Source0: http://node126.narod.ru/fidogate%{version}-%{release}.tar.bz2
#Patch0: %{name}-%{version}-%{release}.patch
BuildRoot: %{_tmppath}/%{name}-root

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
mkdir -p $RPM_BUILD_ROOT/etc/fido/gate
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
mkdir -p $RPM_BUILD_ROOT/var/log/fido/gate
mkdir -p $RPM_BUILD_ROOT/var/lock/fidogate

make DESTDIR=$RPM_BUILD_ROOT BINDIR=$RPM_BUILD_ROOT/usr/bin LIBEXECDIR=$RPM_BUILD_ROOT/usr/libexec install

%clean
rm -rf $RPM_BUILD_ROOT

%files
%doc COPYING TODO TODO.rus ChangeLog Changes.ru doc/README doc/FAQ.ru doc/README.ru

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
%config(noreplace) %attr(600,news,news) /etc/fido/gate/acl.sample
%config(noreplace) %attr(600,news,news) /etc/fido/gate/aliases.sample
%config(noreplace) %attr(600,news,news) /etc/fido/gate/areas.sample
%config(noreplace) %attr(600,news,news) /etc/fido/gate/fidogate.conf.sample
%config(noreplace) %attr(600,news,news) /etc/fido/gate/fidokill.sample
%config(noreplace) %attr(600,news,news) /etc/fido/gate/ftnacl.sample
%config(noreplace) %attr(600,news,news) /etc/fido/gate/hosts.sample
%config(noreplace) %attr(600,news,news) /etc/fido/gate/packing.sample
%config(noreplace) %attr(600,news,news) /etc/fido/gate/passwd.sample
%config(noreplace) %attr(600,news,news) /etc/fido/gate/routing.sample
%config(noreplace) %attr(600,news,news) /etc/fido/gate/spyes.sample
%config(noreplace) %attr(600,news,news) /var/lib/fidogate/areas.bbs.sample
%config(noreplace) %attr(600,news,news) /var/lib/fidogate/fareas.bbs.sample
