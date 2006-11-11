# -*- rpm-spec -*-

%define snapshot 20061031

Summary: Fido-Internet Gateway and Fido Tosser
Name: fidogate
Version: 5.2.3
Release: alt1
License: GPL
Packager: FTN Development Team <ftn@packages.altlinux.org>
Group: Networking/Other

Source0: fidogate-%{version}.cvs%{snapshot}.tar.bz2
Source1: send-fidogate.conf
Source2: fidogate-README.ALT
Source3: fidogate.logrotate
Source4: fidogate.cron.d


Patch0: fidogate-send-fidogate.patch
Patch1: fidogate-newspath.patch
Patch2: fidogate-permissions.patch
Patch3: fidogate-nocxx.patch

#BuildRequires: gcc3.4-c++

PreReq: ftn

#BuildRoot: %buildroot

%description
FIDOGATE Version 5
 * Fido-Internet Gateway
 * Fido FTN-FTN Gateway
 * Fido Mail Processor
 * Fido File Processor
 * Fido Areafix/Filefix

%prep
%setup -q -n %name
%patch0 -p1
%patch1 -p1
%patch2 -p1
%patch3 -p2

%__subst 's/\(INSTALL_.*\)-g ${GROUP} -o ${OWNER}/\1/' configure.in

%build

%__autoconf
SENDMAIL=/usr/sbin/sendmail ./configure --sysconfdir=%_sysconfdir/%name \
	--bindir=%_bindir \
	--libdir=%_libdir \
	--libexecdir=%_libdir/%name \
	--with-logdir=%_logdir/%name \
	--with-vardir=%_localstatedir/%name \
	--with-spooldir=%_spooldir/%name \
	--with-btbasedir=%_spooldir/ftn \
	--with-newsbindir=%_bindir\
	--without-news \
	--enable-dbc-history \
	--with-owner=ftn --with-group=ftn

%__make

%install
%__mkdir_p %buildroot%_sysconfdir/%name
%__mkdir_p %buildroot/%_sysconfdir/logrotate.d
%__mkdir_p %buildroot/%_sysconfdir/cron.d
%__mkdir_p %buildroot%_bindir
%__mkdir_p %buildroot%_libdir/%name
%__mkdir_p %buildroot%_logdir/%name
%__mkdir_p %buildroot%_lockdir/%name
%__mkdir_p %buildroot%_localstatedir/%name/seq
%__mkdir_p %buildroot%_spooldir/%name/toss/bad
%__mkdir_p %buildroot%_spooldir/%name/toss/pack
%__mkdir_p %buildroot%_spooldir/%name/toss/route
%__mkdir_p %buildroot%_spooldir/%name/toss/toss
%__mkdir_p %buildroot%_spooldir/%name/outpkt/mail
%__mkdir_p %buildroot%_spooldir/%name/outpkt/news
%__mkdir_p %buildroot%_spooldir/%name/outrfc/mail
%__mkdir_p %buildroot%_spooldir/%name/outrfc/news

%__make DESTDIR=%buildroot prefix=%_prefix LD_LIBRARY_PATH=%buildroot%_libdir install
%__subst 's|log/fidogate|log/news|g' %buildroot%_libdir/%name/send-fidogate
%__mv %buildroot%_libdir/%name/send-fidogate %buildroot%_bindir/send-fidogate
touch %buildroot%_localstatedir/%name/areas.bbs
touch %buildroot%_localstatedir/%name/fareas.bbs

%__install -p -m 0644 %SOURCE1 %buildroot%_sysconfdir/%name
%__cp %SOURCE2 ./README.ALT
%__install -p -m 0644 %SOURCE3 %buildroot%_sysconfdir/logrotate.d/%name
%__install -p -m 0644 %SOURCE4 %buildroot%_sysconfdir/cron.d/%name

# #8871
mv %buildroot%_bindir/outb %buildroot%_bindir/fg-outb

%post
/usr/sbin/usermod -G ftn$(groups news | cut -d ':' -f 2 | sed 's/ /,/g') news ||:
/usr/sbin/usermod -G news$(groups ftn | cut -d ':' -f 2 | sed 's/ /,/g') ftn ||:

%files
%doc TODO TODO.rus Changes.ru ChangeLog ChangeLog.O ChangeLog.OO doc scripts README.ALT
%defattr(755,root,root)
%_bindir/*
%_libdir/%name/*
%attr(750,news,ftn) %_bindir/ngoper
%attr(750,ftn,ftn) %_libdir/%name/rfc2ftn
%attr(4770,ftn,ftn) %_logdir/%name
%defattr(644,root,root)
%_libdir/lib%name.so*

%defattr(640,ftn,ftn)
%config(noreplace) %_sysconfdir/%name/send-fidogate.conf
%config(noreplace) %_sysconfdir/%name/*.sample
%config(noreplace) %_sysconfdir/%name/bounce.*
%config(noreplace) %_sysconfdir/%name/areafix.*
%config(noreplace) %attr(644,root,root) %_sysconfdir/logrotate.d/%name
%config(noreplace) %attr(644,root,root) %_sysconfdir/cron.d/%name

%defattr(775,ftn,ftn)
%_spooldir/%name

%config(noreplace) %_localstatedir/%name/*areas.bbs
%_localstatedir/%name/*areas.bbs.sample
%dir %_localstatedir/%name/seq
%dir %_localstatedir/%name

%_logdir/%name
%_lockdir/%name

%changelog
* Fri Nov 10 2006 Zhenja Kaliuta <tren@altlinux.ru> 5.2.3-alt1
- New upstream snapshot (20061031)
- Removed C++ dependency

* Thu Apr 20 2006 Vladimir V Kamarzin <vvk@altlinux.ru> 5.2.2-alt3
- 20060301 snapshot (Closes #8650)
- Fixed conflict with xorg-x11-server (Closes: #8871)
- {f}areas.bbs packaged as %%config(noreplace) (Closes: #9184)
- Spec: s/Copyright/License

* Fri Dec  9 2005 Zhenja Kaliuta <tren@altlinux.ru> 5.2.2-alt2
- New upstream snapshot
- bzip2 sources
- Removed full link from spec, it is not updated

* Mon Nov  7 2005 Zhenja Kaliuta <tren@altlinux.ru> 5.2.2-alt1
- New upstream snapshot
- Fixed build requirements (Closes: #8297).

* Wed Jul  6 2005 Zhenja Kaliuta <tren@altlinux.ru> 5.2.1-alt5
- Fixed build from sources (added fidogate-build.patch)

* Fri May 20 2005 Zhenja Kaliuta <tren@altlinux.ru> 5.2.1-alt4
- New upstream snapshot
- Fixed path in cronjob file (Closes: #6905).
- Fixed spool permissions (Closes: #6906).
- Build with --enable-dbc-history (Closes: #6907). 

* Tue Mar 22 2005 Zhenja Kaliuta <tren@altlinux.ru> 5.2.1-alt3
- Fixed packed files (Closes: #6295)

* Thu Mar 17 2005 Zhenja Kaliuta <tren@altlinux.ru> 5.2.1-alt2
- Removed suid bits

* Fri Mar 11 2005 Zhenja Kaliuta <tren@altlinux.ru> 5.2.1-alt1
- New spec version

* Fri Mar  4 2005 Zhenja Kaluta <tren@altlinux.ru> 5.2.0-alt5
- Fixed permissions
- Fixed group membership
- New cvs snapshot
- Fixed newspath path
- Fixed ctlinnd path for send-fidogate
- Cron jobs are diasabled by default
- Updated README.ALT

* Fri Mar 26 2004 Zhenja Kaluta <tren@altlinux.ru> 5.2.0-alt4
- New cvs snapshot
- Added rw permissions to group
- Fixed path to inn binaries

* Thu Feb 26 2004 Zhenja Kaluta <tren@altlinux.ru> 5.2.0-alt3
- New cvs snapshot

* Tue Feb 24 2004 Zhenja Kaluta <tren@altlinux.ru> 5.2.0-alt2
- New cvs snapshot

* Fri Feb 20 2004 Zhenja Kaluta <tren@altlinux.ru> 5.2.0-alt1
- New cvs snapshot
- send-fidogate back
- added config file for send-fidogate
- added README.ALT

* Fri Oct 31 2003 Zhenja Kaluta <tren@altlinux.ru> 5.0.0-alt4
- Removed fidogateconf

* Mon Oct 27 2003 Zhenja Kaluta <tren@altlinux.ru> 5.0.0-alt3
- New upstream release

* Tue Jun 15 2003 Zhenja Kaluta <tren@altlinux.ru> 5.0.0-alt2
- added faq

* Fri Jun 13 2003 Zhenja Kaluta <tren@altlinux.ru> 5.0.0-alt1
- Intitial release (cvs snapshot of Apr 09, 2003)
