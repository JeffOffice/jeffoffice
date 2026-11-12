# Info: properti, kata sandi, dan Inspect
<!-- keywords: periksa aksesibilitas, info, properti, judul, penulis, subjek, kata kunci, kategori, komentar, perusahaan, manajer, properti kustom, docproperty, tandai sebagai final, final, hanya baca, kata sandi, enkripsi, lindungi, kata sandi buka, inspect dokumen, informasi pribadi, teks tersembunyi, xml kustom, periksa kompatibilitas, riwayat versi, versi, simpan versi, kembali, pulihkan, bandingkan versi, tampilkan di folder, salin jalur, folder, jalur, path -->

Klik **Berkas > Info** untuk melihat informasi tentang dokumen Anda, dan melindunginya sebelum Anda membagikannya. Tekan **Esc** untuk kembali ke dokumen Anda.

## Lokasi berkas

Di bagian atas, halaman menampilkan lokasi tempat berkas disimpan. Klik **Tampilkan di Folder** untuk membuka folder tersebut. Klik **Salin Jalur** untuk menyalin jalur, sehingga Anda dapat menempelkannya ke email atau obrolan.

## Properti

Properti adalah informasi tentang dokumen, seperti judul dan siapa penulisnya. Halaman ini memiliki kotak untuk masing-masing properti: **Judul**, **Penulis**, **Subjek**, **Kata Kunci**, **Kategori**, **Komentar**, **Perusahaan**, dan **Manajer**. Ketik di dalam kotak lalu tekan **Tab** atau **Enter**. JeffWrite menyimpan perubahan itu, dan menyimpannya lagi saat Anda menyimpan dokumen berikutnya.

Di bawah kotak-kotak tersebut, halaman menampilkan informasi yang dihitung JeffWrite: kapan dokumen dibuat, disimpan, dan dicetak, serta jumlah halaman, kata, karakter, dan paragrafnya.

### Properti kustom Anda

Klik **Properti Kustom** untuk membuat properti sendiri, misalnya **Proyek** atau **Klien**. Klik **Tambah**, ketik nama, pilih jenis nilainya (teks, bilangan bulat, angka, ya atau tidak, atau tanggal), lalu ketik nilainya.

Sebuah bidang dapat menampilkan properti kustom. Klik **Sisipkan > Bidang**, lalu pilih bidang. Ubah kodenya menjadi `DOCPROPERTY "Klien"`, lalu klik **OK**. Teks akan menampilkan nilainya. Tekan **F9** untuk memperbaruinya setelah Anda mengubah properti. Lihat [Bidang](fields).

## Tandai sebagai Final

Klik **Tandai sebagai Final** untuk memberi tahu pembaca bahwa dokumen sudah selesai. JeffWrite menyimpan dokumen lalu membukanya hanya untuk dibaca. Saat seseorang membukanya, bilah judul menyatakan bahwa dokumen hanya dapat dibaca, dan sebuah catatan memberi tahu mereka bahwa dokumen telah ditandai sebagai final. Ini adalah cara halus untuk mengatakan "mohon jangan ubah ini". Ini bukan kunci, karena siapa pun dapat memilih **Ditandai sebagai Final** lagi untuk mengeditnya.

## Enkripsi dengan Kata Sandi

Klik **Enkripsi dengan Kata Sandi** untuk meminta kata sandi setiap kali berkas dibuka. Ketik kata sandi dua kali, lalu klik **Lindungi**. Berkas diacak saat Anda menyimpannya, sehingga tidak seorang pun dapat membacanya tanpa kata sandi. JeffWrite menggunakan jenis perlindungan yang kuat seperti yang dipakai pengolah kata lain, sehingga pengolah kata tersebut juga dapat membuka berkas dengan kata sandi.

- Kata sandi **tidak dapat dipulihkan** jika Anda lupa. Simpanlah di tempat yang aman.
- Versi yang Anda simpan dan salinan cadangan mengikuti kata sandi: keduanya dilindungi dengan kata sandi tersebut, dan berubah saat Anda mengubah atau menghapusnya.
- Untuk mengubah kata sandi, klik kartu yang sama lagi. Klik **Hapus Kata Sandi** untuk menghapusnya. Perubahan disimpan saat Anda menyimpan berikutnya.
- Kata sandi berfungsi untuk berkas **.docx**. Berkas **.odt** belum dapat menyimpan kata sandi, sehingga JeffWrite tidak akan menyimpan dokumen yang dilindungi sebagai .odt. JeffWrite akan memberi tahu Anda alih-alih menyimpannya tanpa kata sandi.

Untuk membuka berkas yang dilindungi, bukalah seperti biasa. JeffWrite akan meminta kata sandi. Jika salah, Anda dapat mencoba lagi, atau klik **Batal**.

## Inspect Dokumen

Klik **Inspect Dokumen** sebelum Anda mengirim dokumen kepada seseorang. Fitur ini mendaftar hal-hal yang ada di dalam berkas dan mungkin tidak ingin Anda bagikan, beserta jumlah masing-masing yang ditemukan:

- **Komentar** adalah catatan yang ditinggalkan orang di margin.
- **Perubahan yang dilacak** adalah suntingan yang belum diterima atau ditolak. **Terima Semua** mempertahankan setiap perubahan sebagai bagian dari teks.
- **Properti dokumen dan informasi pribadi** adalah penulis, orang yang terakhir menyimpan, perusahaan, dan manajer.
- **Teks tersembunyi** adalah teks yang diatur agar disembunyikan. Teks ini tidak tampil atau tercetak, tetapi tetap ada di dalam berkas.
- **Header dan footer** adalah teks di bagian atas dan bawah halaman.
- **Data XML kustom** adalah data yang ditambahkan program lain ke dalam berkas. Anda tidak dapat melihatnya di halaman.

Klik **Hapus Semua** di samping suatu jenis untuk menghapusnya. Anda dapat membatalkan perubahan pada teks dengan **Ctrl+Z**. Simpan dokumen setelahnya untuk mempertahankan salinan yang sudah bersih.

**Hapus informasi pribadi dari berkas ini saat menyimpan**, di bawah kartu-kartu, melakukannya setiap kali Anda menyimpan, untuk berkas apa pun, tanpa mengubah dokumen yang sedang Anda buka. Kotak ini sama dengan yang ada di **Opsi**, dan sama di setiap aplikasi JeffOffice.

## Periksa Kompatibilitas

Klik **Periksa Kompatibilitas** untuk melihat apa yang tidak dapat disimpan oleh jenis berkas yang lebih lama. JeffWrite menyimpan berkas **.docx** dan **.odt**. Berkas .docx menyimpan seluruh isi dokumen Anda. Berkas .odt tidak dapat menyimpan beberapa hal, dan daftar ini menyebutkannya. Jika tidak ada yang tercantum, tidak ada yang hilang.

Saat Anda menyimpan sebagai berkas .odt, JeffWrite mendaftar apa saja yang tidak dapat disimpan oleh berkas tersebut **sebelum** menulis apa pun. Anda dapat memilih **Tetap Simpan sebagai .odt**, **Simpan sebagai .docx Saja** (berkas .docx akan dibuat di sebelahnya, dan tidak ada yang hilang), atau **Batal**.

## Periksa Aksesibilitas

Klik **Periksa Aksesibilitas** untuk membuka daftar hal yang mungkin sulit dibaca atau digunakan oleh sebagian orang, misalnya gambar tanpa deskripsi. Pemeriksaan ini sama dengan **Tinjau > Periksa Aksesibilitas**. Lihat [Hitung kata, atur bahasa, periksa aksesibilitas, dan cari kata](proofing-tools).

## Riwayat Versi

Klik **Riwayat Versi** untuk menyimpan salinan dokumen yang diberi nama di komputer Anda. Tidak ada yang dikirim ke mana pun. Sebuah versi adalah salinan utuh dokumen sebagaimana adanya pada saat itu. Simpanlah satu versi sebelum perubahan besar, agar Anda dapat kembali ke versi tersebut. Dokumen harus disimpan terlebih dahulu, karena versi disimpan di bawah nama berkas.

- **Simpan Versi** meminta sebuah nama, misalnya "Sebelum revisi", lalu menyimpan salinannya.
- **Buka Salinan** membuka versi yang dipilih di jendela baru. Versi itu sendiri tidak berubah.
- **Bandingkan** menampilkan dokumen dan versi yang dipilih berdampingan sebagai satu dokumen, dengan perbedaannya ditampilkan sebagai perubahan yang dilacak. Lihat [Lacak perubahan](track-changes).
- **Kembali ke Versi Ini** membuat dokumen sama dengan versi yang dipilih. JeffWrite lebih dulu menyimpan dokumen seperti keadaannya sebagai versi baru, sehingga tidak ada yang hilang. Simpan dokumen untuk mempertahankan perubahan.
- **Hapus** menghapus versi yang dipilih.

JeffWrite menyimpan hingga 50 versi sebuah berkas dan menghapus versi yang paling lama saat jumlahnya melebihi batas.
