
Add a release note to NEWS.md.

Set 2GEOM_MAJOR_VERSION, 2GEOM_MINOR_VERSION, and 2GEOM_PATCH_VERSION in CMakeLists.txt

$ cmake .
$ make
$ make test
$ make dist

Next, copy lib2geom-${VERSION}.tar.bz2 to /tmp/, extract it, and verify
it builds.

$ gpg --armor --detach-sign --output lib2geom-${VERSION}.tar.bz2.sig lib2geom-${VERSION}.tar.bz2 
$ git tag -s ${MAJOR}.${MINOR}
$ git push --tags
