# Info: properti, kata sandi, dan Periksa Dokumen
<!-- keywords: periksa aksesibilitas, info, properti, judul, penulis, subjek, kata kunci, kategori, komentar, perusahaan, manajer, properti kustom, docproperty, tandai sebagai final, final, hanya baca, kata sandi, enkripsi, lindungi, kata sandi buka, periksa dokumen, informasi pribadi, teks tersembunyi, xml kustom, periksa kompatibilitas, riwayat versi, versi, simpan versi, kembali, pulihkan, bandingkan versi, tampilkan di folder, salin jalur, folder, jalur -->

Klik **Berkas > Info** untuk melihat informasi tentang dokumen Anda, dan untuk melindunginya sebelum Anda membagikannya. Tekan **Esc** untuk kembali ke dokumen Anda.

## Lokasi berkas

Di bagian atas, halaman menampilkan tempat berkas disimpan. Klik **Tampilkan di Folder** untuk membuka folder tersebut. Klik **Salin Jalur** untuk menyalin jalurnya, sehingga Anda dapat menempelkannya ke email atau obrolan.

## Properti

Properti adalah informasi tentang dokumen, seperti judulnya dan siapa penulisnya. Halaman ini memiliki kotak untuk setiap properti: **Judul**, **Penulis**, **Subjek**, **Kata Kunci**, **Kategori**, **Komentar**, **Perusahaan**, dan **Manajer**. Ketik di dalam kotak lalu tekan **Tab** atau **Enter**. JeffWrite menyimpan perubahan itu, dan menyimpannya lagi saat Anda menyimpan dokumen berikutnya.

Di bawah kotak-kotak tersebut, halaman menampilkan informasi yang dihitung JeffWrite: kapan dokumen dibuat, disimpan, dan dicetak, serta jumlah halaman, kata, karakter, dan paragrafnya.

### Properti kustom Anda

Klik **Properti Kustom** untuk membuat properti Anda sendiri, misalnya **Proyek** atau **Klien**. Klik **Tambah**, ketik nama, pilih jenis nilainya (teks, bilangan bulat, angka, ya atau tidak, atau tanggal), lalu ketik nilainya.

Sebuah bidang dapat menampilkan properti kustom. Klik **Sisipkan > Bidang**, lalu pilih sebuah bidang. Ubah kodenya menjadi `DOCPROPERTY "Klien"`, lalu klik **OK**. Teks akan menampilkan nilainya. Tekan **F9** untuk memperbaruinya setelah Anda mengubah properti. Lihat [Bidang](fields).

## Tandai sebagai Final

Klik **Tandai sebagai Final** untuk memberi tahu pembaca bahwa dokumen sudah selesai. JeffWrite menyimpan dokumen lalu membukanya hanya untuk dibaca. Saat seseorang membukanya, bilah judul menyatakan bahwa dokumen hanya dapat dibaca, dan sebuah catatan memberi tahu bahwa dokumen telah ditandai sebagai final. Ini adalah cara halus untuk mengatakan "mohon jangan ubah ini". Ini bukan kunci, karena siapa pun dapat memilih **Ditandai sebagai Final** lagi untuk mengeditnya.

## Selalu Buka sebagai Baca-Saja

Klik **Selalu Buka sebagai Baca-Saja** agar dokumen selalu terbuka hanya untuk dibaca setiap kali, sehingga tidak ada yang mengubahnya secara tidak sengaja. Pilihan ini disimpan bersama berkas saat Anda menyimpannya. Saat berkas dibuka lagi, bilah status menyatakan bahwa berkas terbuka hanya untuk dibaca. Untuk mengeditnya, buka **Berkas > Info** lalu klik **Selalu Dibuka sebagai Baca-Saja**. Cukup satu klik, jadi ini adalah pengingat dan bukan kunci. Program lain tidak membaca pilihan ini, dan berkas .odt tidak dapat menyimpannya, sehingga simpan dokumen sebagai berkas .docx terlebih dahulu. Periksa Kompatibilitas mencantumkannya untuk berkas .odt.

## Enkripsi dengan Kata Sandi

Klik **Enkripsi dengan Kata Sandi** untuk meminta kata sandi setiap kali berkas dibuka. Ketik kata sandi dua kali, lalu klik **Lindungi**. Berkas diacak saat Anda menyimpannya, sehingga tidak seorang pun dapat membacanya tanpa kata sandi. JeffWrite menggunakan jenis perlindungan kuat yang juga digunakan pengolah kata lain, sehingga program tersebut juga dapat membuka berkas dengan kata sandi.

- Kata sandi **tidak dapat dipulihkan** jika Anda lupa. Simpan di tempat yang aman.
- Versi yang Anda simpan dan salinan cadangan mengikuti kata sandi: keduanya dilindungi dengan kata sandi itu, dan berubah saat Anda mengubah atau menghapusnya.
- Untuk mengubah kata sandi, klik kartu yang sama lagi. Klik **Hapus Kata Sandi** untuk menghapusnya. Perubahan disimpan saat Anda menyimpan berikutnya.
- Kata sandi berfungsi untuk berkas **.docx** dan berkas **.odt**. Berkas .odt dilindungi sesuai dengan standar OpenDocument, sehingga program lain yang mengikuti standar tersebut juga dapat membukanya dengan kata sandi. Berkas teks biasa atau .rtf tidak dapat menyimpan kata sandi, sehingga JeffWrite tidak akan menyimpan dokumen yang dilindungi ke dalam format tersebut. JeffWrite akan memberi tahu Anda, bukan menyimpannya tanpa kata sandi.

Untuk membuka berkas yang dilindungi, buka seperti biasa. JeffWrite akan meminta kata sandi. Jika kata sandinya salah, Anda dapat mencoba lagi, atau klik **Batal**.

JeffWrite juga dapat membuka berkas **.doc** lama yang meminta kata sandi. Cara memintanya sama. JeffWrite tidak pernah menyimpan berkas .doc, sehingga **Simpan Sebagai** meminta nama baru, dan salinan .docx tetap memakai kata sandi yang sama.

## Periksa Dokumen

Klik **Periksa Dokumen** sebelum Anda mengirim dokumen kepada seseorang. Fitur ini menampilkan isi berkas yang mungkin tidak ingin Anda bagikan, dan jumlah setiap jenisnya:

- **Komentar** adalah catatan yang ditinggalkan orang di margin.
- **Perubahan yang dilacak** adalah suntingan yang belum diterima atau ditolak. **Terima Semua** menjadikan setiap perubahan sebagai bagian dari teks.
- **Properti dokumen dan informasi pribadi** adalah penulis, orang yang terakhir menyimpan, perusahaan, dan manajer.
- **Teks tersembunyi** adalah teks yang diatur agar tersembunyi. Teks ini tidak tampil atau tercetak, tetapi tetap ada di dalam berkas.
- **Header dan footer** adalah teks di bagian atas dan bawah halaman.
- **Data XML kustom** adalah data yang ditambahkan program lain ke dalam berkas. Anda tidak dapat melihatnya pada halaman.
- **Tautan eksternal** adalah tautan ke berkas lain, bukan tautan web: gambar atau objek yang hanya ditautkan (menampilkan berkas yang tidak ada di dalam dokumen), dan bidang yang mengambil teks atau gambarnya dari berkas lain (INCLUDEPICTURE, INCLUDETEXT, LINK, DDE, dan DDEAUTO). **Hapus Semua** mengubah setiap bidang menjadi teks yang ditampilkannya saat ini, menghapus setiap gambar atau objek yang ditautkan, dan membuang tautan tersebut dari berkas saat Anda menyimpan. Berkas lain tidak diubah. Tautan di header, footer, catatan kaki, dan catatan akhir juga dihitung dan dihapus. Tautan web tetap dibiarkan. Tautan di dalam komentar, kotak teks, dan bagian yang disimpan program apa adanya (seperti teks gambar) juga dicari. Gambar yang ditautkan tetapi juga memuat salinan gambarnya sendiri tetap dipertahankan sebagai gambar tanpa tautan, dan tautan dibuang dari berkas saat Anda menyimpan. Penghapusan tautan di dalam komentar dan bagian lain yang disimpan apa adanya tidak dapat diurungkan; kotak teks dapat diurungkan dalam satu langkah Urungkan. Templat tempat dokumen dilampirkan (jalurnya sering memuat nama pengguna) juga dihitung sebagai tautan eksternal: **Hapus Semua** melepasnya, dan dokumen tetap memakai gayanya sendiri. Tautan lain ke berkas tidak dihapus, dan satu baris di bawah jumlah menyebutkannya: sumber data cetak surat massal, bingkai, subdokumen di dalam bagian lain, data eksternal bagan, film dan suara yang ditautkan, serta lembar gaya yang ditautkan. Baris yang sama juga menghitung tautan lain di dalam bidang, komentar, kotak teks, atau daftar tautan berkas yang tidak dipakai dokumen, dan menyebutkannya jika alamat gambar yang ditautkan tidak ada di dalam berkas, atau jika sebuah bagian memakai pengodean yang tidak dapat dibaca pemeriksaan. Baris tersebut kemudian menyatakan "Belum diperiksa sepenuhnya", bukan "Tidak ditemukan". Satu kali **Hapus Semua** menghapus paling banyak 100.000 tautan di bagian yang disimpan; jika masih ada yang tersisa, baris tersebut menyebutkan jumlahnya, dan Anda dapat menjalankan **Hapus Semua** lagi.
- **Gambar asli yang disimpan di bawah efek gambar** adalah gambar tempat efek seperti Kabur atau Mosaik dibuat. Berkas menyimpan gambar asli di samping gambar akhir, dan siapa pun yang menerima berkas dapat mengeluarkannya. **Hapus Asli** hanya menyimpan gambar akhir, dan **Atur Ulang Gambar** kemudian tidak dapat lagi mengembalikan gambar asli.
- **Bagian gambar yang dipotong** adalah bagian yang Anda potong dari pandangan. Bagian itu tetap ada di dalam gambar, dan siapa pun yang menerima berkas dapat melihatnya. **Hapus Bagian yang Dipotong** menghapusnya, dan halaman tampak persis seperti sebelumnya. Gambar yang memiliki efek akan menjadi gambar akhirnya terlebih dahulu.

Klik **Hapus Semua** di samping suatu jenis untuk menghapusnya. Anda dapat mengurungkan perubahan pada teks dengan **Ctrl+Z**. Tautan yang dihapus dari komentar dan bagian lain yang disimpan, serta data XML kustom, tidak dapat diurungkan. Simpan dokumen setelahnya untuk menyimpan salinan yang bersih.

Centang **Hapus informasi pribadi dari berkas ini saat menyimpan**, di bawah kartu-kartu, untuk melakukannya setiap kali Anda menyimpan, untuk berkas apa pun, tanpa mengubah dokumen yang sedang Anda buka. Ini adalah kotak yang sama dengan yang ada di **Opsi**, dan sama di setiap aplikasi JeffOffice.

## Periksa Kompatibilitas

Klik **Periksa Kompatibilitas**, lalu pilih jenis berkas untuk melihat apa yang tidak dapat dimuatnya. JeffWrite menyimpan berkas **.docx**, **.odt**, **.rtf**, dan **.txt**. Berkas .docx memuat semua isi dokumen Anda. Jenis lainnya tidak dapat memuat beberapa hal, dan daftar ini menyebutkannya. Daftar ini juga menyebutkan pilihan yang disimpan di dalam berkas itu sendiri, yaitu Tandai sebagai Final dan Selalu Buka sebagai Baca-Saja, jika aktif, serta kata sandi untuk jenis yang tidak dapat menyimpannya. Jika tidak ada yang tercantum, berarti tidak ada yang hilang.

Saat Anda menyimpan sebagai berkas .odt, JeffWrite mencantumkan apa saja yang tidak dapat dimuat berkas itu **sebelum** menulis apa pun. Anda dapat memilih **Tetap Simpan**, **Simpan sebagai .docx** (berkas .docx disimpan di sebelahnya, dan tidak ada yang hilang), atau **Batal**. Centang **Jangan tanyakan lagi untuk berkas ini**, dan JeffWrite berhenti menanyakannya untuk berkas tersebut sampai Anda menutup program.

## Periksa Aksesibilitas

Klik **Periksa Aksesibilitas** untuk membuka daftar hal yang mungkin sulit dibaca atau digunakan sebagian orang, seperti gambar tanpa deskripsi. Ini adalah pemeriksaan yang sama dengan **Tinjau > Periksa Aksesibilitas**. Lihat [Hitung kata, atur bahasa, periksa aksesibilitas, dan cari kata](proofing-tools).

## Riwayat Versi

Klik **Riwayat Versi** untuk menyimpan salinan dokumen yang diberi nama di komputer Anda. Tidak ada yang dikirim ke mana pun. Sebuah versi adalah salinan utuh dokumen seperti keadaannya pada saat itu. Simpan satu versi sebelum perubahan besar, agar Anda dapat kembali ke versi tersebut. Dokumen harus disimpan terlebih dahulu, karena versi disimpan di bawah nama berkas.

- **Simpan Versi** meminta nama, misalnya "Sebelum revisi", lalu menyimpan salinan.
- **Buka Salinan** membuka versi yang dipilih di jendela baru. Versi itu sendiri tidak berubah.
- **Bandingkan** menampilkan dokumen dan versi yang dipilih berdampingan sebagai satu dokumen, dengan perbedaannya ditampilkan sebagai perubahan yang dilacak. Lihat [Lacak perubahan](track-changes).
- **Kembali ke Versi Ini** membuat dokumen sama dengan versi yang dipilih. JeffWrite lebih dulu menyimpan dokumen seperti keadaannya sebagai versi baru, sehingga tidak ada yang hilang. Simpan dokumen untuk menyimpan perubahan.
- **Hapus** menghapus versi yang dipilih.

JeffWrite menyimpan hingga 50 versi untuk setiap berkas dan membuang versi tertua jika jumlahnya melebihi itu.
