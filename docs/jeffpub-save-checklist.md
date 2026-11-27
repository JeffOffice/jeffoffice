# What each format JeffPub saves keeps

One row for each part of a publication: the test that proves it for JeffPub's own `.jpub` format, and the test that proves it for `.pub`. "not kept" means the format cannot hold it and a save says so (Check Compatibility and the warning when saving as `.pub` list it; the tests named in the last section prove the list). Last checked November 27, 2026.

| Part | `.jpub` | `.pub` |
| --- | --- | --- |
| Pages and master pages | `PubFileTests::documentRoundTrip` (the whole publication compared as saved and read back) | `pubWriterPages`, `pubWriterMaster`, `pubWriterTwoMasters`, `pubPageWithoutMasterObjectsRoundTrip` |
| Text with its runs | `documentRoundTrip` | `pubWriterTextFormats`, `pubWriterTabsLists`, `pubWriterRoundTrip` |
| Styles | `styleLinksAreSavedAndOldDocumentsLookTheSame` | `pubWriterStyles` |
| Fields | `documentRoundTrip` (a page number field) | `pubFieldsAndLinksRoundTrip` (page, next and previous page, date and time) |
| Links in text | `aJpubKeepsLinksInText` | `pubFieldsAndLinksRoundTrip` |
| Links on objects and on mouse-over | `aPublicationWithWebAddressesFetchesNothing` (kept, never followed) | not kept, and said: `aPubFileSaysWhatItDropsOfMacrosAndStoredFonts` |
| Shapes | `documentRoundTrip` | `pubWriterShapes`, `pubWriterFills`, `pubGradientsRoundTrip` |
| Lines | `aJpubKeepsLines` | `pubWriterObjects`, `pubDashStylesRoundTrip` |
| Pictures | `jpubPicturesAreHeldToThePictureChecks` | `pubWriterPictures`, `pubWriterKeepsJpegPhotos`, `pubPictureTurnsAndFill` |
| Picture crops and effects | `aJpubKeepsPictureCropsAndEffects` | `pubWriterPictures`, `pubPictureOpacityFromTertiaryProperty` |
| Tables with merged cells | `pastedTableSurvivesSavingAsJpub`, `aRingMergedAgainKeepsItsHoleThroughSaveAndOpen` | `pubWriterTable`, `pubTableFillsInLaterRows` |
| Booklets and cards | `aJpubKeepsBookletsAndCards` | `pubBookletRoundTrip`, `pubFoldedCardAndEnvelope` |
| Text Art | `aJpubKeepsTextArt` | `pubWriterTextArt`, `pubTextArtPatternFill` |
| Building blocks | not part of a publication: they live in the shared saved-blocks store (tested by `savedBuildingBlocksInsert`) | not part of a publication |
| Alt text | `aJpubKeepsAltText` | not kept, and said: `aPubFileSaysWhatItDropsOfMacrosAndStoredFonts` |
| The publication's properties | `propertiesStillRoundTripThroughASave` | `propertiesStillRoundTripThroughASave` |
| A mail merge list | `recipientsLeftOutAndTheirKeyColumnAreKeptInTheFile` | not kept, and said: `aPubFileCopyListsTheMailMergeListAsNotKept` |
| Comments | `jpubKeepsComments` | not kept, and said: `pubFileNamesTheCommentsAndWritesNothingElse` |
| Equations | `aJpubKeepsTheEquation` | saved as a picture, and said: `aPubSavesAPictureAndSaysSo` |
| Macro project of an opened `.pub` | not kept (JeffPub never runs one), and said: `savingAsJpubSaysWhatItDropsOfMacrosAndStoredFonts` | not kept, and said: `aPubFileSaysWhatItDropsOfMacrosAndStoredFonts` |
| Fonts stored in an opened `.pub` | used while the file is open; not kept, and said: `savingAsJpubSaysWhatItDropsOfMacrosAndStoredFonts` | not kept, and said: `aPubFileSaysWhatItDropsOfMacrosAndStoredFonts` |

The `.jpub` tests (`aJpubKeeps...` and the one on macros and fonts) save and reopen through the window's own save and open, and each fails when the part it names is left out of the file. Saving as `.jpub` after opening a `.pub` file with a macro project or stored fonts says so in the status bar, as saving as `.pub` says what it leaves out.
