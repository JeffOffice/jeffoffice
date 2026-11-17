# Info: properti, kata sandi, dan Inspect
<!-- keywords: periksa aksesibilitas, info, properti, judul, penulis, subjek, kata kunci, kategori, komentar, perusahaan, manajer, properti kustom, docproperty, tandai sebagai final, final, hanya baca, kata sandi, sandi, enkripsi, lindungi, sandi buka, inspeksi dokumen, inspect, informasi pribadi, teks tersembunyi, xml kustom, periksa kompatibilitas, riwayat versi, versi, simpan versi, kembali, pulihkan, bandingkan versi, tampilkan di folder, salin jalur, folder, jalur -->

Klik **Berkas > Info** untuk melihat keterangan tentang dokumen Anda, dan untuk melindunginya sebelum Anda membagikannya. Tekan **Esc** untuk kembali ke dokumen Anda.

## Lokasi berkas

Di bagian atas, halaman ini menampilkan tempat berkas disimpan. Klik **Tampilkan di Folder** untuk membuka folder tersebut. Klik **Salin Jalur** untuk menyalin jalur, sehingga Anda dapat menempelkannya ke email atau obrolan.

## Properti

Properti adalah keterangan tentang dokumen, seperti judul dan siapa penulisnya. Halaman ini memiliki kotak untuk setiap properti: **Judul**, **Penulis**, **Subjek**, **Kata Kunci**, **Kategori**, **Komentar**, **Perusahaan**, dan **Manajer**. Ketik di dalam kotak lalu tekan **Tab** atau **Enter**. JeffWrite menyimpan perubahan itu, dan menyimpannya lagi saat Anda menyimpan dokumen berikutnya.

Di bawah kotak-kotak tersebut, halaman menampilkan informasi yang dihitung JeffWrite: kapan dokumen dibuat, disimpan, dan dicetak, serta jumlah halaman, kata, karakter, dan paragrafnya.

### Properti kustom Anda

Klik **Properti Kustom** untuk membuat properti sendiri, misalnya **Proyek** atau **Klien**. Klik **Tambah**, ketik nama, pilih jenis nilainya (teks, bilangan bulat, angka, ya atau tidak, atau tanggal), lalu ketik nilainya.

Sebuah bidang dapat menampilkan properti kustom. Klik **Sisipkan > Bidang**, lalu pilih bidang. Ubah kodenya menjadi `DOCPROPERTY "Klien"`, lalu klik **OK**. Teks akan menampilkan nilainya. Tekan **F9** untuk memperbaruinya setelah Anda mengubah properti. Lihat [Bidang](fields).

## Tandai sebagai Final

Klik **Tandai sebagai Final** untuk memberi tahu pembaca bahwa dokumen sudah selesai. JeffWrite menyimpan dokumen lalu membukanya hanya untuk dibaca. Saat seseorang membukanya, bilah judul menyatakan bahwa dokumen hanya dapat dibaca, dan sebuah catatan memberi tahu bahwa dokumen itu ditandai sebagai final. Ini cara halus untuk mengatakan "mohon jangan ubah ini". Ini bukan kunci, sebab siapa pun dapat memilih **Ditandai sebagai Final** lagi untuk mengeditnya.

## Selalu Buka sebagai Baca-Saja

Klik **Selalu Buka sebagai Baca-Saja** agar dokumen selalu terbuka hanya untuk dibaca, sehingga tidak ada yang mengubahnya secara tidak sengaja. Pilihan ini disimpan bersama berkas saat Anda menyimpannya. Saat berkas dibuka lagi, bilah status menyatakan bahwa berkas terbuka hanya untuk dibaca. Untuk mengeditnya, buka **Berkas > Info** lalu klik **Selalu Dibuka sebagai Baca-Saja**. Caranya cukup satu klik, jadi ini berupa pengingat dan bukan kunci. Program lain tidak membaca pilihan ini, dan berkas .odt tidak dapat menyimpannya, jadi simpan dokumen sebagai berkas .docx terlebih dahulu. Periksa Kompatibilitas akan mencantumkannya untuk berkas .odt.

## Enkripsi dengan Kata Sandi

Klik **Enkripsi dengan Kata Sandi** untuk meminta kata sandi setiap kali berkas dibuka. Ketik kata sandi dua kali, lalu klik **Lindungi**. Berkas diacak saat Anda menyimpannya, sehingga tidak ada yang dapat membacanya tanpa kata sandi. JeffWrite menggunakan jenis perlindungan kuat yang juga dipakai pengolah kata lain, sehingga program tersebut juga dapat membuka berkas dengan kata sandi itu.

- Kata sandi **tidak dapat dipulihkan** jika Anda lupa. Simpan di tempat yang aman.
- Versi yang Anda simpan dan salinan cadangan mengikuti kata sandi: keduanya dilindungi dengan kata sandi itu, dan berubah saat Anda mengubah atau menghapusnya.
- Untuk mengubah kata sandi, klik kartu yang sama lagi. Klik **Hapus Kata Sandi** untuk menghapusnya. Perubahan disimpan saat Anda menyimpan berikutnya.
- Kata sandi berlaku untuk berkas **.docx** dan berkas **.odt**. Berkas .odt dilindungi sesuai dengan standar OpenDocument, sehingga program lain yang mengikuti standar tersebut juga dapat membukanya dengan kata sandi. Berkas teks biasa atau .rtf tidak dapat menyimpan kata sandi, sehingga JeffWrite tidak akan menyimpan dokumen terlindungi ke dalam format itu. Sebagai gantinya, JeffWrite memberi tahu Anda, bukan menyimpannya tanpa kata sandi.

Untuk membuka berkas terlindungi, bukalah seperti biasa. JeffWrite akan meminta kata sandi. Jika kata sandinya salah, Anda dapat mencoba lagi atau klik **Batal**.

JeffWrite juga dapat membuka berkas **.doc** lama yang meminta kata sandi. Caranya sama. JeffWrite tidak pernah menyimpan berkas .doc, sehingga **Simpan Sebagai** akan meminta nama baru, dan salinan .docx akan memakai kata sandi yang sama.

## Inspeksi Dokumen

Klik **Inspeksi Dokumen** sebelum Anda mengirim dokumen kepada seseorang. Fitur ini mencantumkan apa saja yang terdapat dalam berkas dan mungkin tidak ingin Anda bagikan, beserta jumlah masing-masing yang ditemukan:

- **Komentar** adalah catatan yang ditinggalkan orang di margin.
- **Perubahan yang dilacak** adalah suntingan yang belum diterima atau ditolak. **Terima Semua** mempertahankan setiap perubahan sebagai bagian dari teks.
- **Properti dokumen dan informasi pribadi** adalah penulis, orang yang terakhir menyimpan, perusahaan, dan manajer.
- **Teks tersembunyi** adalah teks yang diatur agar disembunyikan. Teks ini tidak tampil atau tercetak, tetapi tetap ada dalam berkas.
- **Header dan footer** adalah teks di bagian atas dan bawah halaman.
- **Data XML kustom** adalah data yang ditambahkan program lain ke dalam berkas. Data ini tidak terlihat di halaman.
- **Tautan eksternal** adalah tautan ke berkas lain, bukan tautan web: gambar atau objek yang hanya ditautkan (menampilkan berkas yang tidak ada di dalam dokumen), dan bidang yang mengambil teks atau gambarnya dari berkas lain (INCLUDEPICTURE, INCLUDETEXT, LINK, DDE, dan DDEAUTO). **Hapus Semua** mengubah setiap bidang menjadi teks yang ditampilkannya sekarang, menghapus setiap gambar atau objek yang ditautkan, dan membuang tautan tersebut dari berkas saat Anda menyimpan. Berkas lain tidak diubah. Tautan di header, footer, dan catatan kaki tidak dihitung, dan tautan web tetap dibiarkan.

Klik **Hapus Semua** di samping suatu jenis untuk menghapusnya. Anda dapat membatalkan perubahan yang mengubah teks dengan **Ctrl+Z**. Simpan dokumen setelahnya untuk mempertahankan salinan yang sudah bersih.

**Hapus informasi pribadi dari berkas ini saat menyimpan**, di bawah kartu-kartu, berlaku setiap kali Anda menyimpan, untuk berkas apa pun, tanpa mengubah dokumen yang sedang terbuka. Ini adalah kotak yang sama dengan yang ada di **Opsi**, dan sama di setiap aplikasi JeffOffice.

## Periksa Kompatibilitas

Klik **Periksa Kompatibilitas** untuk melihat apa saja yang tidak dapat disimpan oleh jenis berkas yang lebih lama. JeffWrite menyimpan berkas **.docx** dan **.odt**. Berkas .docx menampung seluruh isi dokumen Anda. Berkas .odt tidak dapat menampung beberapa hal, dan daftar ini menyebutkannya. Jika tidak ada yang tercantum, tidak ada yang hilang.

Saat Anda menyimpan sebagai berkas .odt, JeffWrite mencantumkan apa saja yang tidak dapat ditampung berkas itu **sebelum** menulis apa pun. Anda dapat memilih **Tetap Simpan**, **Simpan sebagai .docx** (berkas .docx ditempatkan di sebelahnya, dan tidak ada yang hilang), atau **Batal**. Centang **Jangan tanyakan lagi untuk berkas ini**, dan JeffWrite berhenti bertanya tentang berkas tersebut sampai Anda menutup program.

## Periksa Aksesibilitas

Klik **Periksa Aksesibilitas** untuk membuka daftar hal yang mungkin sulit dibaca atau digunakan sebagian orang, misalnya gambar tanpa deskripsi. Ini adalah pemeriksaan yang sama dengan **Tinjau > Periksa Aksesibilitas**. Lihat [Hitung kata, atur bahasa, periksa aksesibilitas, dan cari kata](proofing-tools).

## Riwayat Versi

Klik **Riwayat Versi** untuk menyimpan salinan dokumen yang diberi nama di komputer Anda. Tidak ada yang dikirim ke mana pun. Sebuah versi adalah salinan utuh dokumen seperti keadaannya pada saat itu. Simpan satu versi sebelum perubahan besar, agar Anda dapat kembali ke sana. Dokumen harus disimpan terlebih dahulu, karena versi disimpan dengan nama berkas.

- **Simpan Versi** meminta sebuah nama, misalnya "Sebelum revisi", dan menyimpan salinan.
- **Buka Salinan** membuka versi yang dipilih di jendela baru. Versi itu sendiri tidak berubah.
- **Bandingkan** menampilkan dokumen dan versi yang dipilih berdampingan sebagai satu dokumen, dengan perbedaannya ditampilkan sebagai perubahan yang dilacak. Lihat [Lacak perubahan](track-changes).
- **Kembali ke Versi Ini** membuat dokumen sama dengan versi yang dipilih. JeffWrite lebih dulu menyimpan dokumen seperti keadaannya sebagai versi baru, sehingga tidak ada yang hilang. Simpan dokumen untuk mempertahankan perubahan.
- **Hapus** menghapus versi yang dipilih.

JeffWrite menyimpan hingga 50 versi untuk setiap berkas dan membuang versi terlama jika jumlahnya melebihi batas.
