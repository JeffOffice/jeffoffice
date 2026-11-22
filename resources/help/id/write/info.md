# Info: properti, kata sandi, dan Periksa Dokumen
<!-- keywords: periksa aksesibilitas, info, properti, judul, penulis, subjek, kata kunci, kategori, komentar, perusahaan, manajer, properti kustom, docproperty, tandai sebagai final, final, hanya baca, kata sandi, enkripsi, proteksi, kata sandi buka, periksa dokumen, informasi pribadi, teks tersembunyi, xml kustom, periksa kompatibilitas, riwayat versi, versi, simpan versi, kembali, pulihkan, bandingkan versi, tampilkan di folder, salin jalur, folder, jalur -->

Klik **Berkas > Info** untuk melihat informasi tentang dokumen Anda, dan untuk melindunginya sebelum Anda membagikannya. Tekan **Esc** untuk kembali ke dokumen Anda.

## Lokasi berkas

Di bagian atas, halaman menampilkan lokasi tempat berkas disimpan. Klik **Tampilkan di Folder** untuk membuka folder tersebut. Klik **Salin Jalur** untuk menyalin jalur, sehingga Anda dapat menempelkannya ke dalam email atau obrolan.

## Properti

Properti adalah informasi tentang dokumen, seperti judul dan siapa penulisnya. Halaman ini memiliki kotak untuk setiap properti: **Judul**, **Penulis**, **Subjek**, **Kata Kunci**, **Kategori**, **Komentar**, **Perusahaan**, dan **Manajer**. Ketik di dalam kotak lalu tekan **Tab** atau **Enter**. JeffWrite menyimpan perubahan tersebut, dan menyimpannya lagi saat Anda menyimpan dokumen berikutnya.

Di bawah kotak-kotak tersebut, halaman menampilkan informasi yang dihitung JeffWrite: kapan dokumen dibuat, disimpan, dan dicetak, serta jumlah halaman, kata, karakter, dan paragraf.

### Properti Anda sendiri

Klik **Properti Kustom** untuk membuat properti sendiri, seperti **Proyek** atau **Klien**. Klik **Tambah**, ketik nama, pilih jenis nilainya (teks, bilangan bulat, angka, ya atau tidak, atau tanggal), lalu ketik nilainya.

Sebuah bidang dapat menampilkan properti kustom. Klik **Sisipkan > Bidang**, lalu pilih sebuah bidang. Ubah kodenya menjadi `DOCPROPERTY "Klien"`, lalu klik **OK**. Teks akan menampilkan nilainya. Tekan **F9** untuk memperbaruinya setelah Anda mengubah properti. Lihat [Bidang](fields).

## Tandai sebagai Final

Klik **Tandai sebagai Final** untuk memberi tahu pembaca bahwa dokumen sudah selesai. JeffWrite menyimpan dokumen lalu membukanya hanya untuk dibaca. Saat seseorang membukanya, bilah judul menyatakan bahwa dokumen hanya dapat dibaca, dan sebuah catatan memberi tahu bahwa dokumen itu ditandai sebagai final. Ini adalah cara halus untuk berkata "tolong jangan ubah ini". Ini bukan kunci, karena siapa pun dapat memilih **Ditandai sebagai Final** lagi untuk mengeditnya.

## Selalu Buka sebagai Hanya Baca

Klik **Selalu Buka sebagai Hanya Baca** agar dokumen selalu terbuka hanya untuk dibaca, sehingga tidak ada yang mengubahnya secara tidak sengaja. Pilihan ini disimpan bersama berkas saat Anda menyimpannya. Ketika berkas dibuka lagi, bilah status menyatakan bahwa berkas terbuka hanya untuk dibaca. Untuk mengeditnya, buka **Berkas > Info** lalu klik **Selalu Dibuka sebagai Hanya Baca**. Cukup satu klik, jadi ini adalah pengingat, bukan kunci. Program lain tidak membaca pilihan ini, dan berkas .odt tidak dapat menyimpannya, jadi simpan dokumen sebagai berkas .docx terlebih dahulu. Periksa Kompatibilitas mencantumkannya untuk berkas .odt.

## Enkripsi dengan Kata Sandi

Klik **Enkripsi dengan Kata Sandi** untuk meminta kata sandi setiap kali berkas dibuka. Ketik kata sandi dua kali, lalu klik **Lindungi**. Berkas diacak saat Anda menyimpannya, sehingga tidak ada yang dapat membacanya tanpa kata sandi. JeffWrite menggunakan jenis proteksi kuat yang juga digunakan pengolah kata lain, sehingga program tersebut juga dapat membuka berkas dengan kata sandi.

- Kata sandi **tidak dapat dipulihkan** jika Anda lupa. Simpan di tempat yang aman.
- Versi yang Anda simpan dan salinan cadangan mengikuti kata sandi: keduanya dilindungi dengan kata sandi tersebut, dan berubah saat Anda mengubah atau menghapusnya.
- Untuk mengubah kata sandi, klik kartu yang sama lagi. Klik **Hapus Kata Sandi** untuk menghapusnya. Perubahan disimpan saat Anda menyimpan berikutnya.
- Kata sandi berlaku untuk berkas **.docx** dan berkas **.odt**. Berkas .odt dilindungi sesuai dengan standar OpenDocument, sehingga program lain yang mengikuti standar tersebut juga dapat membukanya dengan kata sandi. Berkas teks biasa atau .rtf tidak dapat menyimpan kata sandi, sehingga JeffWrite tidak akan menyimpan dokumen yang dilindungi sebagai berkas tersebut. JeffWrite akan memberi tahu Anda, bukan menyimpan tanpa kata sandi.

Untuk membuka berkas yang dilindungi, buka seperti biasa. JeffWrite akan meminta kata sandi. Jika kata sandinya salah, Anda dapat mencoba lagi, atau klik **Batal**.

JeffWrite juga membuka berkas **.doc** lama yang meminta kata sandi. Caranya sama. JeffWrite tidak pernah menyimpan berkas .doc, sehingga **Simpan Sebagai** meminta nama baru, dan salinan .docx mempertahankan kata sandi yang sama.

## Periksa Dokumen

Klik **Periksa Dokumen** sebelum Anda mengirim dokumen kepada seseorang. Fitur ini mendaftar hal-hal yang ada di dalam berkas yang mungkin tidak ingin Anda bagikan, beserta jumlah masing-masing yang ditemukan:

- **Komentar** adalah catatan yang ditinggalkan orang di margin.
- **Perubahan yang dilacak** adalah suntingan yang belum diterima atau ditolak. **Terima Semua** menyimpan setiap perubahan sebagai bagian dari teks.
- **Properti dokumen dan informasi pribadi** adalah penulis, orang yang terakhir menyimpan, perusahaan, dan manajer.
- **Teks tersembunyi** adalah teks yang diatur agar disembunyikan. Teks ini tidak tampil atau tercetak, tetapi tetap ada di dalam berkas.
- **Header dan footer** adalah teks di bagian atas dan bawah halaman.
- **Data XML kustom** adalah data yang ditambahkan program lain ke dalam berkas. Anda tidak dapat melihatnya pada halaman.
- **Tautan eksternal** adalah tautan ke berkas lain, bukan tautan web: gambar atau objek yang hanya ditautkan (menampilkan berkas yang tidak ada di dalam dokumen), dan bidang yang mengambil teks atau gambarnya dari berkas lain (INCLUDEPICTURE, INCLUDETEXT, LINK, DDE, dan DDEAUTO). **Hapus Semua** mengubah setiap bidang menjadi teks yang ditampilkannya saat ini, menghapus setiap gambar atau objek yang ditautkan, dan tidak menyertakan tautan tersebut saat Anda menyimpan berkas. Berkas lain tidak diubah. Tautan di header, footer, dan catatan kaki tidak dihitung, dan tautan web tetap dibiarkan.

Klik **Hapus Semua** di samping suatu jenis untuk menghapusnya. Anda dapat mengurungkan perubahan yang memengaruhi teks dengan **Ctrl+Z**. Simpan dokumen setelahnya untuk mempertahankan salinan yang sudah bersih.

**Hapus informasi pribadi dari berkas ini saat menyimpan**, di bawah kartu-kartu, akan melakukannya setiap kali Anda menyimpan, untuk berkas apa pun, tanpa mengubah dokumen yang sedang terbuka. Ini adalah kotak yang sama dengan yang ada di **Opsi**, dan sama di setiap aplikasi JeffOffice.

## Periksa Kompatibilitas

Klik **Periksa Kompatibilitas**, lalu pilih jenis berkas untuk melihat apa yang tidak dapat disimpannya. JeffWrite menyimpan berkas **.docx**, **.odt**, **.rtf**, dan **.txt**. Berkas .docx menyimpan semua isi dokumen Anda. Jenis lainnya tidak dapat menyimpan beberapa hal, dan daftar ini menunjukkan apa saja. Daftar ini juga menyebutkan pilihan yang disimpan di dalam berkas itu sendiri, yaitu Tandai sebagai Final dan Selalu Buka sebagai Hanya Baca, jika aktif, serta kata sandi untuk jenis yang tidak dapat menyimpannya. Jika tidak ada yang tercantum, berarti tidak ada yang hilang.

Saat Anda menyimpan sebagai berkas .odt, JeffWrite mencantumkan apa saja yang tidak dapat disimpan berkas tersebut **sebelum** menulis apa pun. Anda dapat memilih **Tetap Simpan**, **Simpan sebagai .docx** (berkas .docx disimpan di sebelahnya, dan tidak ada yang hilang), atau **Batal**. Centang **Jangan tanyakan lagi untuk berkas ini**, dan JeffWrite berhenti menanyakannya untuk berkas tersebut sampai Anda menutup program.

## Periksa Aksesibilitas

Klik **Periksa Aksesibilitas** untuk membuka daftar hal yang mungkin sulit dibaca atau digunakan oleh sebagian orang, misalnya gambar tanpa deskripsi. Pemeriksaan ini sama dengan **Tinjau > Periksa Aksesibilitas**. Lihat [Hitung kata, atur bahasa, periksa aksesibilitas, dan cari kata](proofing-tools).

## Riwayat Versi

Klik **Riwayat Versi** untuk menyimpan salinan dokumen yang diberi nama di komputer Anda. Tidak ada yang dikirim ke mana pun. Versi adalah salinan utuh dokumen pada saat itu. Simpan satu versi sebelum perubahan besar, agar Anda dapat kembali ke versi tersebut. Dokumen harus disimpan terlebih dahulu, karena versi disimpan di bawah nama berkas.

- **Simpan Versi** meminta nama, misalnya "Sebelum revisi", lalu menyimpan salinan.
- **Buka Salinan** membuka versi yang dipilih di jendela baru. Versi itu sendiri tidak berubah.
- **Bandingkan** menempatkan dokumen dan versi yang dipilih berdampingan sebagai satu dokumen, dengan perbedaannya ditampilkan sebagai perubahan yang dilacak. Lihat [Lacak perubahan](track-changes).
- **Kembali ke Versi Ini** membuat dokumen sama dengan versi yang dipilih. JeffWrite lebih dulu menyimpan dokumen seperti keadaannya sebagai versi baru, sehingga tidak ada yang hilang. Simpan dokumen untuk mempertahankan perubahan.
- **Hapus** menghapus versi yang dipilih.

JeffWrite menyimpan hingga 50 versi sebuah berkas dan menghapus yang paling lama saat jumlahnya melebihi batas.
