#pragma once
// The formatting marks a page can show, as bits (apart from textprops.h so that a header that only carries the choice need not include it).

namespace jp::tp {

// The formatting marks a page can show (Show/Hide, and Options > Display for each kind), as bits.
namespace special {
enum { Space = 1, Tab = 2, Paragraph = 4, Hidden = 8, OptionalHyphen = 16, Anchor = 32, All = Space | Tab | Paragraph | Hidden | OptionalHyphen | Anchor };
}

}  // namespace jp::tp
