%define  ver     0.9.1
%define  rel     1
%define  prefix  /usr

Summary: a graphical, OpenGL based, traceroute
Name: xtraceroute
Version: %ver
Release: %rel
Copyright: GPL
Group: X11/Amusements
Source: http://www.dtek.chalmers.se/~d3august/xt/dl/xtraceroute-%{ver}.tar.gz
URL: http://www.dtek.chalmers.se/~d3august/xt/
BuildRoot: /tmp/xtraceroute-root
 
%description
Xtraceroute is a graphical traceroute utility that shows the path your
IP packets travel on a 3 dimensional rendered globe. Be sure to
download the NDG data files mentioned in the INSTALL document too.

%prep
%setup

%build
if [ ! -f configure ]; then
  CFLAGS="$RPM_OPT_FLAGS" ./autogen.sh --prefix=%prefix
else
  CFLAGS="$RPM_OPT_FLAGS" ./configure --prefix=%prefix
fi

if [ "$SMP" != "" ]; then
  (make "MAKE=make -k -j $SMP"; exit 0)
  make
else
  make
fi

%install
rm -rf $RPM_BUILD_ROOT
mkdir -p $RPM_BUILD_ROOT%{prefix}/share/gnome/apps/Network
mkdir -p $RPM_BUILD_ROOT%{prefix}/share/pixmaps

make prefix=$RPM_BUILD_ROOT%{prefix} install

gzip -9 $RPM_BUILD_ROOT%{prefix}/man/man?/*

install xtraceroute.desktop $RPM_BUILD_ROOT%{prefix}/share/gnome/apps/Network
install xtraceroute.png $RPM_BUILD_ROOT%{prefix}/share/pixmaps

%clean
rm -rf $RPM_BUILD_ROOT

%changelog
* Wed Feb 10 1999 Pablo Saratxaga <srtxg@chanae.alphanet.ch>

- First try at an RPM

%files
%defattr(-, root, root)

%doc AUTHORS ChangeLog NEWS README COPYING TODO

%{prefix}/share/locale/*/LC_MESSAGES/xtraceroute.mo
%{prefix}/bin/xtraceroute
%{prefix}/share/xtraceroute
%{prefix}/share/gnome/apps/Network/xtraceroute.desktop
%{prefix}/share/pixmaps/xtraceroute.png
%{prefix}/man/man?/*

