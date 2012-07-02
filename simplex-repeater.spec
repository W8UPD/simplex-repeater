%define gitcommit  f0abec5
%define gitdate    20120701
Name:           simplex-repeater
Version:        1.0.0
Release:        0.1.%{gitdate}git%{gitcommit}%{?dist}
Summary:        A software-based simplex repeater

Group:          Applications/Communication
License:        GPLv3+
URL:            https://github.com/W8UPD/simplex-repeater
# Source0 retrieved by pulling a Github tarball.
# See: https://github.com/W8UPD/simplex-repeater/downloads
Source0:        W8UPD-%{name}-%{gitcommit}.tar.gz
BuildRoot:      %{_tmppath}/%{name}-%{version}-%{release}-root
BuildRequires:  hamlib-devel
BuildRequires:  festival-devel
BuildRequires:  libconfig-devel
BuildRequires:  libxml2-devel
BuildRequires:  libcurl-devel
BuildRequires:  portaudio-devel
Requires:       hamlib
Requires:       festival
Requires:       libconfig
Requires:       libxml2
Requires:       libcurl
Requires:       portaudio

%description
A software-based simplex repeater which uses hamlib to control a radio for
transmitting/receiving, and festival for voice synthesis.

%prep
%setup -q -n W8UPD-%{name}-%{gitcommit}

%build
make %{?_smp_mflags}

%install
rm -rf %{buildroot}
install -D -m 0755 master %{buildroot}/%{_bindir}/%{name}
install -D -p -m 644 repeater.cfg.dist %{buildroot}%{_sysconfdir}/simplex-repeater/repeater.cfg

%clean
rm -rf %{buildroot}

%files
%defattr(-,root,root,-)
%doc README.md LICENSE
%{_bindir}/simplex-repeater
%{_sysconfdir}/simplex-repeater

%changelog
* Sun Jul 1 2012 Ricky Elrod <codeblock@fedoraproject.org> - 1.0.0-0.1.20120701gitf0abec5
- Initial build.
