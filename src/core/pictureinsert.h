#pragma once

namespace jp {

// How a picture file joins a publication: copied in (Embed), only tied to
// its file (Link), or both (EmbedAndLink).
enum class PictureInsert { Embed, Link, EmbedAndLink };

} // namespace jp
