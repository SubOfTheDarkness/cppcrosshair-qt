# Maintainer: SubOfTheDarkness <204970490+SubOfTheDarkness@users.noreply.github.com>
pkgname=cppcrosshair-toolkit
pkgver=1.0.0
pkgrel=1
pkgdesc="Pixel crosshair manager: standalone lightweight X11 overlay and Qt6 editor"
arch=('x86_64')
url="https://github.com/SubOfTheDarkness/cppcrosshair-qt"
license=('GPL-3.0-or-later')
depends=('qt6-base' 'libx11' 'libxext' 'libxpm' 'procps-ng')
makedepends=('cmake')
options=('!debug')

source=()
sha256sums=()

build() {
  cmake -B build -S "$startdir" \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=/usr
  cmake --build build
}

package() {
  DESTDIR="$pkgdir" cmake --install build
}
