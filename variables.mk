# Support for autoconf enabled install target

prefix=/usr/local
exec_prefix=${prefix}
bindir=${exec_prefix}/bin
sbindir=${exec_prefix}/sbin
libdir=${exec_prefix}/lib
libexecdir=${exec_prefix}/libexec
mandir=${datarootdir}/man
mansubdir=@mansubdir@
sysconfdir=${prefix}/etc
sysmoddir=@sysmoddir@
piddir=@piddir@
srcdir=.
top_srcdir=.
includedir=${prefix}/include
datarootdir=${prefix}/share
datadir=$(datarootdir)

octave_path=/usr
glpk_path=/usr
