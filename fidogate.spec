Summary: Fido-Internet Gateway and Fido Tosser
Name: fidogate
Version: 4.4.10
Release: 1.fc2
Copyright: GPL
Group: System Environment/Daemons
Source: ftp://ftp.fidogate.org/pub/fidogate/fidogate-%{version}.tar.gz
BuildRoot: /var/tmp/fidogate-root

%description
FIDOGATE Version 4
 * Fido-Internet Gateway
 * Fido FTN-FTN Gateway
 * Fido Mail Processor
 * Fido File Processor
 * Fido Areafix/Filefix

%prep
%setup -q 

%build
make DEBUG=-O2

%install
rm -rf $RPM_BUILD_ROOT

make PREFIX=$RPM_BUILD_ROOT install-dirs
mkdir -p $RPM_BUILD_ROOT/etc/news
make PREFIX=$RPM_BUILD_ROOT install
make PREFIX=$RPM_BUILD_ROOT install-config
rm -rf $RPM_BUILD_ROOT/html

%clean
rm -rf $RPM_BUILD_ROOT

%files
%doc ANNOUNCE COPYING README TODO doc/fidogate.txt doc/gatebau94.txt
%doc doc/gatebau/msgid.txt
%doc doc/*.html doc/html/*.html doc/gatebau/*.html
%dir /etc/fidogate
/usr/lib/fidogate
/usr/info/fidogate.info*
/var/log/fidogate
%dir /var/lib/fidogate
%dir /var/lib/fidogate/seq
/var/lock/fidogate
/var/spool/fidogate
/var/spool/bt
/etc/news/newsfeeds.fidogate
%config(noreplace) /etc/fidogate/aliases
%config(noreplace) /etc/fidogate/areas
%config(noreplace) /etc/fidogate/fidogate.conf
%config(noreplace) /etc/fidogate/hosts
%config(noreplace) /etc/fidogate/packing
%config(noreplace) /etc/fidogate/passwd
%config(noreplace) /etc/fidogate/routing
%config(noreplace) /var/lib/fidogate/areas.bbs
%config(noreplace) /var/lib/fidogate/fareas.bbs

%changelog
* Thu Aug 26 2004 Martin Junius <mj@fidogate.org>
- Updated to version 4.4.10, Fedora Core 2

* Mon Jun 09 2003 Martin Junius <mj@m-j-s.net>
- Updated to version 4.4.9, RedHat 9

* Tue Apr 15 2003 Martin Junius <mj@m-j-s.net>
- Updated to version 4.4.8, RedHat 9

* Thu Apr 10 2003 Martin Junius <mj@m-j-s.net>
- Updated to version 4.4.7, RedHat 7.x (build on 7.3)

* Mon Sep 16 2002 Martin Junius <mj@m-j-s.net>
- Updated to version 4.4.6, RedHat 7.x (build on 7.3)

* Sun Apr 28 2002 Martin Junius <mj@m-j-s.net>
- Updated to version 4.4.5, RedHat 7.x (build on 7.2)

* Mon May 28 2001 Martin Junius <mj@m-j-s.net>
- Updated to version 4.4.4

* Sun Mar 04 2001 Martin Junius <mj@m-j-s.net>
- Updated to version 4.4.3

* Sat Nov 18 2000 Martin Junius <mj@m-j-s.net>
- Updated to version 4.4.2

* Tue Mar  7 2000 Martin Junius <mj@m-j-s.net>
- Updated to version 4.4.1

* Sun Jan 30 2000 Martin Junius <mj@m-j-s.net>
- First try at rpm spec file for 4.4.0
