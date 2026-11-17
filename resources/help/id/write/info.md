# Info: properti, kata sandi, dan Inspect
<!-- keywords: periksa aksesibilitas, info, properti, judul, penulis, subjek, kata kunci, kategori, komentar, perusahaan, manajer, properti kustom, docproperty, tandai sebagai final, final, hanya baca, kata sandi, sandi, enkripsi, lindungi, sandi buka, inspeksi dokumen, inspect document, informasi pribadi, teks tersembunyi, custom xml, periksa kompatibilitas, riwayat versi, versi, simpan versi, kembali, pulihkan, bandingkan versi, tampilkan di folder, salin jalur, folder, jalur -->

Klik **Berkas > Info** untuk melihat informasi tentang dokumen Anda, dan melindunginya sebelum Anda membagikannya. Tekan **Esc** untuk kembali ke dokumen Anda.

## Lokasi berkas

Di bagian atas, halaman ini menampilkan lokasi tempat berkas disimpan. Klik **Tampilkan di Folder** untuk membuka folder tersebut. Klik **Salin Jalur** untuk menyalin jalur, sehingga Anda dapat menempelkannya ke email atau obrolan.

## Properti

Properti adalah informasi tentang dokumen, seperti judul dan siapa penulisnya. Halaman ini memiliki kotak untuk masing-masing properti: **Judul**, **Penulis**, **Subjek**, **Kata Kunci**, **Kategori**, **Komentar**, **Perusahaan**, dan **Manajer**. Ketik di dalam kotak lalu tekan **Tab** atau **Enter**. JeffWrite menyimpan perubahan itu, dan menyimpannya lagi saat Anda menyimpan dokumen berikutnya.

Di bawah kotak-kotak tersebut, halaman menampilkan informasi yang dihitung JeffWrite: kapan dokumen dibuat, disimpan, dan dicetak, serta jumlah halaman, kata, karakter, dan paragrafnya.

### Properti Anda sendiri

Klik **Properti Kustom** untuk membuat properti sendiri, seperti **Proyek** atau **Klien**. Klik **Tambah**, ketik nama, pilih jenis nilainya (teks, bilangan bulat, angka, ya atau tidak, atau tanggal), lalu ketik nilainya.

Sebuah bidang dapat menampilkan properti kustom. Klik **Sisipkan > Bidang**, lalu pilih sebuah bidang. Ubah kodenya menjadi `DOCPROPERTY "Client"`, lalu klik **OK**. Teks akan menampilkan nilainya. Tekan **F9** untuk memperbaruinya setelah Anda mengubah properti. Lihat [Bidang](fields).

## Tandai sebagai Final

Klik **Tandai sebagai Final** untuk memberi tahu pembaca bahwa dokumen sudah selesai. JeffWrite menyimpan dokumen lalu membukanya hanya untuk dibaca. Saat seseorang membukanya, bilah judul menyatakan bahwa dokumen hanya dapat dibaca, dan sebuah catatan memberi tahu bahwa dokumen itu telah ditandai sebagai final. Ini adalah cara halus untuk mengatakan "mohon jangan ubah ini". Ini bukan kunci, karena siapa pun dapat memilih **Ditandai sebagai Final** lagi untuk mengeditnya.

## Enkripsi dengan Kata Sandi

Klik **Enkripsi dengan Kata Sandi** untuk meminta kata sandi setiap kali berkas dibuka. Ketik kata sandi dua kali, lalu klik **Lindungi**. Berkas diacak saat Anda menyimpannya, sehingga tidak seorang pun dapat membacanya tanpa kata sandi. JeffWrite menggunakan jenis perlindungan kuat yang juga dipakai pengolah kata lain, sehingga program tersebut juga dapat membuka berkas dengan kata sandi itu.

- Kata sandi **tidak dapat dipulihkan** jika Anda lupa. Simpan di tempat yang aman.
- Versi yang Anda simpan dan salinan cadangan mengikuti kata sandi: keduanya dilindungi dengan kata sandi itu, dan berubah saat Anda mengubah atau menghapusnya.
- Untuk mengubah kata sandi, klik kartu yang sama lagi. Klik **Hapus Kata Sandi** untuk menghapusnya. Perubahan disimpan saat Anda menyimpan berikutnya.
- Kata sandi berfungsi untuk berkas **.docx** dan berkas **.odt**. Berkas .odt dilindungi sesuai dengan cara yang dijelaskan standar OpenDocument, sehingga program lain yang mengikuti standar tersebut juga dapat membukanya dengan kata sandi. Berkas teks biasa atau .rtf tidak dapat menyimpan kata sandi, sehingga JeffWrite tidak akan menyimpan dokumen yang dilindungi ke dalam format itu. Sebagai gantinya, JeffWrite akan memberi tahu Anda, tanpa menyimpannya tanpa kata sandi.

Untuk membuka berkas yang dilindungi, bukalah seperti biasa. JeffWrite akan meminta kata sandi. Jika kata sandi salah, Anda dapat mencoba lagi, atau klik **Batal**.

JeffWrite juga dapat membuka berkas **.doc** lama yang meminta kata sandi. Caranya sama. JeffWrite tidak pernah menyimpan berkas .doc, sehingga **Simpan Sebagai** akan meminta nama baru, dan salinan .docx tetap memakai kata sandi yang sama.

## Inspeksi Dokumen

Klik **Inspect Document** sebelum Anda mengirim dokumen kepada seseorang. Daftar ini memperlihatkan apa saja yang dimuat berkas dan mungkin tidak ingin Anda bagikan, beserta jumlah setiap jenisnya yang ditemukan:

- **Komentar** adalah catatan yang ditinggalkan orang di margin.
- **Perubahan yang dilacak** adalah suntingan yang belum diterima atau ditolak. **Terima Semua** menjadikan setiap perubahan sebagai bagian dari teks.
- **Properti dokumen dan informasi pribadi** adalah penulis, orang yang terakhir menyimpan, perusahaan, dan manajer.
- **Teks tersembunyi** adalah teks yang diatur untuk disembunyikan. Teks ini tidak tampil atau tercetak, tetapi tetap ada di dalam berkas.
- **Header dan footer** adalah teks di bagian atas dan bawah halaman.
- **Data XML kustom** adalah data yang ditambahkan program lain ke dalam berkas. Anda tidak dapat melihatnya di halaman.

Klik **Hapus Semua** di samping suatu jenis untuk menghapusnya. Anda dapat membatalkan perubahan pada teks dengan **Ctrl+Z**. Simpan dokumen setelahnya untuk mempertahankan salinan yang bersih.

**Hapus informasi pribadi dari berkas ini saat menyimpan**, di bawah kartu-kartu, melakukannya setiap kali Anda menyimpan, untuk berkas apa pun, tanpa mengubah dokumen yang sedang Anda buka. Ini adalah kotak yang sama dengan yang ada di **Opsi**, dan sama di setiap aplikasi JeffOffice.

## Periksa Kompatibilitas

Klik **Periksa Kompatibilitas** untuk melihat apa saja yang tidak dapat disimpan oleh jenis berkas yang lebih lama. JeffWrite menyimpan berkas **.docx** dan **.odt**. Berkas .docx memuat semua isi dokumen Anda. Berkas .odt tidak dapat memuat beberapa hal, dan daftar ini menunjukkan apa saja. Jika tidak ada yang tercantum, tidak ada yang hilang.

Saat Anda menyimpan sebagai berkas .odt, JeffWrite mencantumkan apa saja yang tidak dapat dimuat berkas itu **sebelum** menulis apa pun. Anda dapat memilih **Tetap Simpan**, **Simpan sebagai .docx** (berkas .docx akan disimpan di sebelahnya, dan tidak ada yang hilang), atau **Batal**. Centang **Jangan tanyakan lagi untuk berkas ini**, dan JeffWrite berhenti bertanya tentang berkas itu sampai Anda menutup program.

## Periksa Aksesibilitas

Klik **Periksa Aksesibilitas** untuk membuka daftar hal yang mungkin sulit dibaca atau digunakan oleh sebagian orang, seperti gambar tanpa deskripsi. Ini adalah pemeriksaan yang sama dengan **Tinjau > Periksa Aksesibilitas**. Lihat [Hitung kata, atur bahasa, periksa aksesibilitas, dan cari kata](proofing-tools).

## Riwayat Versi

Klik **Riwayat Versi** untuk menyimpan salinan dokumen yang diberi nama di komputer Anda. Tidak ada yang dikirim ke mana pun. Sebuah versi adalah salinan utuh dokumen seperti keadaannya pada saat itu. Simpan satu versi sebelum perubahan besar, agar Anda dapat kembali ke keadaan itu. Dokumen harus disimpan terlebih dahulu, karena versi disimpan dengan nama berkas.

- **Simpan Versi** meminta nama, misalnya "Sebelum revisi", lalu menyimpan salinan.
- **Buka Salinan** membuka versi yang dipilih di jendela baru. Versi itu sendiri tidak berubah.
- **Bandingkan** menempatkan dokumen dan versi yang dipilih berdampingan sebagai satu dokumen, dengan perbedaannya ditampilkan sebagai perubahan yang dilacak. Lihat [Lacak perubahan](track-changes).
- **Kembali ke Versi Ini** membuat dokumen sama dengan versi yang dipilih. JeffWrite menyimpan dokumen seperti keadaannya sebagai versi baru lebih dulu, sehingga tidak ada yang hilang. Simpan dokumen untuk mempertahankan perubahan.
- **Hapus** menghapus versi yang dipilih.

JeffWrite menyimpan hingga 50 versi sebuah berkas dan membuang versi tertua saat jumlahnya melebihi batas.
