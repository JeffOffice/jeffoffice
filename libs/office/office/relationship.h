#pragma once
// A relationship of a package part (see package.h), kept apart so a header that only holds a list of them need not include the package reader.

#include <QString>

namespace jp::office {

// One link from a part (or from the package) to another part or to an
// address outside. For an internal link `target` is the part name it was
// resolved to; for an external one it is the address as written.
struct Relationship {
    QString id, type, target;
    bool external = false;
};

} // namespace jp::office
