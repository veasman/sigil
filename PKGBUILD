pkgname=sigil-git
pkgver=r0.0000000
pkgrel=1
pkgdesc="Lean X11 screenshot tool for Arch-family systems"
arch=('x86_64')
url="https://github.com/veasman/sigil"
license=('MIT')
depends=('libx11' 'libxrandr' 'libxrender' 'libpng' 'xclip')
makedepends=('base-devel' 'git' 'pkgconf')
provides=('sigil')
conflicts=('sigil')
source=('git+https://github.com/veasman/sigil.git')
sha256sums=('SKIP')

pkgver() {
	cd "$srcdir/sigil"
	printf "r%s.%s" \
		"$(git rev-list --count HEAD)" \
		"$(git rev-parse --short=7 HEAD)"
}

build() {
	cd "$srcdir/sigil"
	make
}

package() {
	cd "$srcdir/sigil"
	make DESTDIR="$pkgdir" PREFIX=/usr install
}
