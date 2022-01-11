# vcf2hap

## Description

vcf2hap is a simple tool for generating a .hap file from a VCF.  The
.hap file is required by [haplohseq](https://github.com/auerlab/vcf2hap.git).

vcf2hap is extremely fast (about 19 times faster than format_hap.py) and
requires a trivial amount of memory regardless of the size of the VCF file.

## Design and Implementation

The code is organized following basic object-oriented design principals, but
implemented in C to minimize overhead and keep the source code accessible to
scientists who don't have time to master the complexities of C++.

Structures are treated as classes, with accessor and mutator functions
(or macros) provided, so dependent applications and libraries need not access
structure members directly.  Since the C language cannot enforce this, it's
up to application programmers to exercise self-discipline.

## Building and installing

vcf2hap is intended to build cleanly in any POSIX environment.  Please
don't hesitate to open an issue if you encounter problems on any
Unix-like system.

Primary development is done on FreeBSD with clang, but the code is frequently
tested on CentOS, MacOS, and NetBSD as well.  MS Windows is not supported,
unless using a POSIX environment such as Cygwin or Windows Subsystem for Linux.

The Makefile is designed to be friendly to package managers, such as
[Debian packages](https://www.debian.org/distrib/packages),
[FreeBSD ports](https://www.freebsd.org/ports/),
[MacPorts](https://www.macports.org/), [pkgsrc](https://pkgsrc.org/), etc.
End users should install via one of these if at all possible.

I maintain a FreeBSD port and a pkgsrc package.

### Installing vcf2hap on FreeBSD:

FreeBSD is a highly underrated platform for scientific computing, with over
1,900 scientific libraries and applications in the FreeBSD ports collection
(of more than 30,000 total), modern clang compiler, fully-integrated ZFS
filesystem, and renowned security, performance, and reliability.
FreeBSD has a somewhat well-earned reputation for being difficult to set up
and manage compared to user-friendly systems like [Ubuntu](https://ubuntu.com/).
However, if you're a little bit Unix-savvy, you can very quickly set up a
workstation, laptop, or VM using
[desktop-installer](http://www.acadix.biz/desktop-installer.php).

To install the binary package on FreeBSD:

```
pkg install vcf2hap
```

You can just as easily build and install from source.  This is useful for
FreeBSD ports with special build options, for building with non-portable
optimizations such as -march=native, and for 
[work-in-progress ports](https://github.com/outpaddling/freebsd-ports-wip),
for which binary packages are not yet maintained.

```
cd /usr/ports/biology/vcf2hap && env CFLAGS='-march=native -O2' make install
cd /usr/ports/wip/vcf2hap && make install
```

### Installing via pkgsrc

pkgsrc is a cross-platform package manager that works on any Unix-like
platform. It is native to [NetBSD](https://www.netbsd.org/) and well-supported
on [Illumos](https://illumos.org/), [MacOS](https://www.apple.com/macos/),
[RHEL](https://www.redhat.com)/[CentOS](https://www.centos.org/), and
many other Linux distributions.
Using pkgsrc does not require admin privileges.  You can install a pkgsrc
tree in any directory to which you have write access and easily install any
of the nearly 20,000 packages in the collection.  The
[auto-pkgsrc-setup](http://netbsd.org/~bacon/) script can assist you with
basic setup.

First bootstrap pkgsrc using auto-pkgsrc-setup or any
other method.  Then run the following commands:

```
cd pkgsrc-dir/biology/vcf2hap
bmake install clean
```

There may also be binary packages available for your platform.  If this is
the case, you can install by running:

```
pkgin install vcf2hap
```

See the [Joyent Cloud Services Site](https://pkgsrc.joyent.com/) for
available package sets.

### Building vcf2hap locally

Below are cave man install instructions for development purposes, not
recommended for regular use.

vcf2hap depends on [biolibc](https://github.com/auerlab/biolibc).
Install biolibc before attempting to build vcf2hap.

1. Clone the repository
2. Run "make depend" to update Makefile.depend
3. Run "make install"

The default install prefix is ../local.  Clone vcf2hap, biolibc and dependent
apps into sibling directories so that ../local represents a common path to all
of them.

To facilitate incorporation into package managers, the Makefile respects
standard make/environment variables such as CC, CFLAGS, PREFIX, etc.  

Add-on libraries required for the build, such as biolibc, should be found
under ${LOCALBASE}, which defaults to ../local.
The library, headers, and man pages are installed under
${DESTDIR}${PREFIX}.  DESTDIR is empty by default and is primarily used by
package managers to stage installations.  PREFIX defaults to ${LOCALBASE}.

To install directly to /myprefix, assuming biolibc is installed there as well,
using a make variable:

```
make LOCALBASE=/myprefix clean depend install
```

Using an environment variable:

```
# C-shell and derivatives
setenv LOCALBASE /myprefix
make clean depend install

# Bourne shell and derivatives
LOCALBASE=/myprefix
export LOCALBASE
make clean depend install
```

View the Makefile for full details.
