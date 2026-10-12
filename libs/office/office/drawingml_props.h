#pragma once
// Properties DrawingML text keeps on a paragraph beyond the ones the text engine has.

#include <QTextFormat>

namespace jp::office {

// On an empty paragraph the file says has no bullet (a:buNone in its own properties): true. An empty paragraph shows no bullet either way, and
// one that says nothing takes the bullet of its layout; this tells the two apart, so a presentation that opens and saves again keeps what the
// file said, and typing into the paragraph does not give it a bullet.
constexpr int kNoBulletParagraph = QTextFormat::UserProperty + 300;

} // namespace jp::office
