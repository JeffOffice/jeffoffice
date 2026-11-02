# Cross-references
<!-- keywords: cross-reference, cross reference, REF, PAGEREF, NOTEREF, see page, see figure, above, below, heading number, link -->

A **cross-reference** is words in your text that point to another place, such as "see Figure 2 on page 7". It is a field, so when the figure or the page changes, you update the field and the words follow.

## Insert a cross-reference

1. Click where the words should go.
2. Click **References > Cross-reference**.
3. Pick the **Reference type**: a heading, a bookmark, a footnote, an endnote, a numbered item, or a caption such as Figure or Table.
4. Pick what to **Insert reference to**: the text, the number, the page number, or "above" and "below".
5. Choose the item from the list, leave **Insert as hyperlink** checked if you want to jump to it, and click **Insert**.

For example, to write "See the heading Results on page 4": type `See the heading `, insert a reference to the heading with **Heading text**, type ` on page `, and insert a second reference to the same heading with **Page number**.

## Follow a reference

Hold **Ctrl** and click a reference that is a hyperlink. The caret goes to the place it names.

## Keep the words right

After you add pages, change headings, or move figures, select the text and press **F9**. Every reference works out its words again. If the place a reference points to has been deleted, the field shows `Error: the bookmark this refers to is missing.`, and you can pick a new target.

## What happens behind the scenes

JeffWrite marks the target with a hidden bookmark (its name begins with `_Ref`) and makes a field that names it: `REF` for text and numbers, `PAGEREF` for page numbers, and `NOTEREF` for notes. Another program opens the file with the references still working.
