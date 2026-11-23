# Membuka, menyimpan, mencetak, dan mengekspor
<!-- keywords: fonts in the file, embedded fonts, missing font, e-book, ebook, epub, e-reader, chapters, contents, web page, html, save as web page, filtered web page, email, eml, bookmarks, pdf bookmarks, shift+f5, last edit, where you stopped, built-in templates, gallery, letter, report, brief, manuscript, newsletter, open, save, save as, docx, doc, dot, odt, opendocument, rtf, older files, template, dotx, recent, pin, new, close, print, pdf, export, recover, automatic recovery, unsaved, crash, file screen, file formats, save a copy, read-only, read only, open as copy, new window, backup copy, my templates, save as template, ott, repair, open and repair, damaged, password, encrypt, protect with a password -->

Klik **File** di pojok kiri atas untuk melihat layar Berkas. Layar ini memiliki halaman Baru, Buka, Cetak, dan Ekspor, serta perintah Simpan dan Simpan Sebagai. Tekan **Esc** untuk kembali ke dokumen Anda.

## Memulai dokumen baru

Tekan **Ctrl+N**, atau klik **File > Baru** lalu **Dokumen Kosong**. Dokumen baru akan terbuka di jendelanya sendiri. Jika jendela yang Anda gunakan kosong dan belum pernah disimpan, dokumen baru akan menggantikannya.

## Membuka dokumen

Tekan **Ctrl+O**, atau klik **File > Buka**. Klik **Telusuri** untuk memilih berkas. JeffWrite membuka jenis berkas berikut:

- **.docx** adalah dokumen.
- **.dotx** adalah templat. Templat dibuka sebagai dokumen baru yang dibuat darinya, sehingga templat itu sendiri tidak berubah.
- **.docm** dan **.dotm** adalah dokumen dan templat yang dapat memuat **makro**, yaitu program kecil yang menyertai sebuah berkas. JeffWrite tidak pernah menjalankannya. Makro dipertahankan seperti semula saat Anda menyimpan sebagai .docm atau .dotm. Jika Anda menyimpan sebagai .docx atau .odt, makro tidak disertakan, dan JeffWrite akan memberi tahu Anda.
- **.txt** adalah teks biasa, hanya berisi kata-katanya. Saat Anda membuka berkas ini, JeffWrite langsung membacanya jika berupa ASCII biasa atau Unicode. Jika tidak, JeffWrite menanyakan **pengodean** mana yang digunakan untuk membacanya, lalu menampilkan beberapa baris pertama agar Anda dapat memastikan hurufnya tampil benar. Pengodean adalah cara berkas menyimpan huruf, seperti UTF-8 atau Windows 1252. Saat Anda menyimpan sebagai .txt, Anda memilih pengodean dan akhir baris, dan berkas hanya menyimpan kata-katanya: tanpa font, gambar, atau garis tabel.
- **.html** dan **.htm** adalah halaman web. JeffWrite membaca kata-kata, judul, huruf tebal, huruf miring, garis bawah, warna, daftar, dan tabel. Poin dan nomor daftar masuk sebagai tanda yang diketik, sedangkan gambar dan tautan tidak disertakan. JeffWrite tidak pernah mengambil apa pun dari internet dan mengabaikan skrip. Halaman web tidak pernah ditimpa: **Simpan** akan meminta nama baru dan menyimpan berkas .docx. Untuk membuat halaman web, lihat **Simpan sebagai halaman web** di bawah.
- **.doc** dan **.dot** adalah jenis dokumen dan templat lama. JeffWrite membacanya, dan menyimpan perubahan Anda sebagai berkas .docx baru.
- **.odt**, **.ott**, dan **.fodt** adalah berkas OpenDocument. JeffWrite membaca ketiganya. JeffWrite menyimpan berkas .odt, dan menyimpan perubahan pada berkas .ott atau .fodt sebagai berkas .odt baru. Lihat [Buka dan simpan berkas OpenDocument](open-odt).
- **.rtf** adalah Rich Text Format. JeffWrite membaca dan menyimpannya. Lihat [Buka berkas RTF](open-rtf).

Halaman **Buka** juga mencantumkan dokumen yang baru-baru ini Anda buka atau simpan. Klik salah satunya untuk membukanya. Klik kanan pada salah satu dokumen untuk menyematkannya ke bagian atas daftar, atau menghapusnya dari daftar.

Jika sebuah berkas memiliki bagian yang belum dapat diedit JeffWrite, seperti tata letak tabel atau header, bagian itu tetap dipertahankan. Lihat [Selamat datang di JeffWrite](start). Jika sebagian berkas sama sekali tidak dapat dibaca, JeffWrite akan memberi tahu bagian yang tidak ikut dimuat. Setelah itu, JeffWrite tidak akan menyimpan di atas berkas tersebut: Simpan akan meminta nama baru, sehingga berkas asli tetap menyimpan apa yang tidak ikut dalam salinan.

## Menyimpan dokumen

Tekan **Ctrl+S** untuk menyimpan. Pertama kali, JeffWrite meminta nama. **Simpan Sebagai** (F12) menyimpan salinan dengan nama baru. Pilih **Templat (*.dotx)** untuk menyimpan templat, **Dokumen Berpenyimpanan Makro (*.docm)** untuk mempertahankan makro, **Teks Polos (*.txt)** untuk hanya kata-katanya, atau **Rich Text Format (*.rtf)** untuk berkas yang dapat dibaca program lain. Berkas yang berasal dari jenis berkas lama selalu disimpan dengan nama baru, sehingga berkas lama tidak pernah tertimpa.

## Menyimpan salinan

Klik **File > Simpan Salinan**. Pilih nama. JeffWrite menulis dokumen ke berkas tersebut, dan Anda tetap bekerja di dokumen yang sebelumnya Anda buka. Nama dan perubahan yang belum disimpan tetap seperti semula. Ini berguna jika Anda ingin mengirim satu versi kepada seseorang dan tetap melanjutkan penulisan.

## Menyimpan salinan cadangan

Klik **File > Opsi**, lalu pada halaman **Umum** aktifkan **Selalu buat salinan cadangan**. Setiap kali Anda menyimpan, JeffWrite lebih dulu menyalin berkas lama ke berkas bernama "Cadangan dari" ditambah namanya, di folder yang sama. Jika salinan tidak dapat dibuat, JeffWrite akan menanyakan apakah tetap menyimpan tanpa salinan. Jawabannya adalah Tidak kecuali Anda memilih Ya, dan berkas tetap seperti semula. Dengan begitu, Anda dapat kembali ke versi tersimpan terakhir.

## Menyimpan sebagai templat

Templat adalah dokumen yang Anda gunakan sebagai titik awal. Klik **File > Simpan sebagai Templat**, lalu ketik nama. Templat akan masuk ke **Templat Saya**, dan dokumen yang sedang Anda tulis tidak berubah. Di **Simpan Sebagai**, Anda juga dapat memilih **Templat (*.dotx)**, atau **Templat OpenDocument Text (*.ott)**, untuk menyimpan berkas templat di lokasi yang Anda inginkan.

Untuk memulai dokumen baru dari templat Anda, klik **File > Baru**. Templat Anda tercantum di bawah **Templat Saya**. Klik salah satunya. Dokumen baru akan terbuka dengan teks dan gaya dari templat tersebut. Untuk menghapus templat dari daftar, klik kanan padanya lalu pilih **Hapus dari Templat Saya**.

## Cara lain membuka berkas

Pada halaman **Buka**, klik **Lainnya**. Anda juga dapat mengklik kanan nama di daftar Terbaru. Setiap cara meminta sebuah berkas, atau menggunakan berkas yang Anda klik:

- **Buka Hanya-Baca** membuka berkas hanya untuk dibaca. Anda dapat melihat, menggulir, mencari, dan menyalin, tetapi tidak dapat mengubah apa pun. Bilah judul akan menampilkan "Hanya Baca". Jika Anda menekan **Simpan**, JeffWrite akan meminta nama baru, sehingga berkas yang Anda buka tidak berubah.
- **Buka sebagai Salinan** membuka dokumen baru dengan teks dari berkas tersebut. Dokumen ini tidak memiliki berkas sendiri, sehingga **Simpan** akan menanyakan lokasi penyimpanannya. Berkas yang Anda buka tidak pernah diubah.
- **Buka di Jendela Baru** membuka berkas di jendelanya sendiri, meskipun jendela yang Anda gunakan kosong.
- **Buka dan Perbaiki** digunakan untuk berkas .docx atau .doc yang rusak, misalnya yang terpotong saat disalin. Lihat di bawah.

## Buka dan Perbaiki

Terkadang berkas .docx atau .doc menjadi rusak. Penyalinan terhenti di tengah jalan, atau disk mengalami kesalahan. Berkas .docx adalah kumpulan bagian yang lebih kecil, seperti teks, gaya, dan gambar. **Buka dan Perbaiki** membaca setiap bagian yang mungkin dibaca. Bagian yang terhenti di tengah akan dipertahankan sampai titik ia terhenti. Bagian yang tidak dapat dibaca akan dibuang. Jika daftar bagian atau tautan di antaranya hilang, JeffWrite akan membuatnya kembali.

Berkas .doc lama adalah satu pembungkus besar yang berisi beberapa aliran data. Satu aliran berisi teks, dan aliran lain berisi tabel yang menunjukkan letak setiap potongan teks dan tampilannya. Jika pembungkus terpotong atau daftarnya rusak, **Buka dan Perbaiki** akan menemukan aliran tersebut berdasarkan isinya dan memulihkan sebanyak mungkin. Jika daftar yang menjelaskan bagaimana potongan-potongan itu tersusun rusak, program mengambil setiap potongan untuk mengikuti potongan sebelumnya, lalu menampilkan apa yang ditemukannya. Teks tersebut mungkin berasal dari salinan lama dokumen atau dari bagian lain berkas, sehingga pesan akan menyebutkan paragraf tempat teks itu dimulai, dan meminta Anda membandingkannya dengan cadangan jika Anda memilikinya. Jika tebakan letaknya berada sebelum teks, pesan akan menyatakan bahwa seluruh dokumen disusun ulang; jika berada sesudah teks, pesan akan menyatakan bahwa pemformatannya yang disusun ulang. Jika gambar tidak dapat dipulihkan sepenuhnya, pesan akan menyatakannya. Jika tabel hilang, teks dipertahankan sebagai paragraf biasa, tanpa gaya, daftar, tabel, atau gambar. Berkas .doc yang memiliki sandi tidak dapat diperbaiki.

Dokumen yang telah diperbaiki akan terbuka sebagai salinan dengan nama seperti "Surat yang diperbaiki". Sebuah pesan memberi tahu Anda apa saja yang dipertahankan sampai titik putus dan apa yang dibuang. Berkas yang rusak tidak diubah. Simpan salinan yang telah diperbaiki dengan nama tersendiri, dan periksa kembali sebelum Anda mempercayainya.

## Mendapatkan kembali pekerjaan yang belum disimpan

JeffWrite menyimpan salinan cadangan pekerjaan yang belum disimpan setiap beberapa menit, sebagai berkas di folder tersendiri. Jika program tertutup secara tidak terduga, program akan menawarkan salinan tersebut saat Anda memulainya kembali. Klik **File > Buka > Pulihkan Pekerjaan yang Belum Disimpan** untuk melihatnya kapan saja. Anda dapat memilih mana yang akan dibuka, dan menyimpan setiap salinan di lokasi yang Anda inginkan.

## Memulai dari templat

**File > Baru** menampilkan **Dokumen Kosong** dan sejumlah templat bawaan JeffWrite, dikelompokkan menurut jenisnya: surat, laporan, ringkasan, naskah, dan buletin. Setiap gambar adalah halaman pertama dari dokumen yang akan Anda dapatkan. **Templat** adalah dokumen yang sudah disiapkan, lengkap dengan gaya, margin, dan teks contoh. Klik salah satunya, lalu ganti teks contoh dengan teks Anda sendiri. Templat yang Anda simpan sendiri tercantum di bawah **Templat Saya**.

## Melanjutkan dari tempat terakhir Anda berhenti

JeffWrite mengingat posisi kursor saat Anda menyimpan atau menutup berkas. Saat Anda membuka berkas itu lagi, kursor akan berada di sana. Tekan **Shift+F5** untuk melompat ke tempat yang terakhir Anda edit. Setiap kali ditekan, kursor berpindah ke tempat sebelumnya: tiga tempat terakhir, lalu kembali ke yang pertama.

## Menyimpan sebagai .odt

Berkas .odt tidak dapat memuat semua yang dapat dimuat berkas .docx. Saat Anda menyimpan sebagai .odt, JeffWrite terlebih dahulu menampilkan daftar hal yang akan hilang, dan memberi Anda pilihan untuk menyimpan sebagai berkas .docx. Lihat [Info](info).

## Mencetak

Tekan **Ctrl+P**, atau klik **File > Cetak**. Pilih printer, jumlah salinan, dan halaman, lihat setiap halaman di pratinjau, lalu klik **Cetak**. Lihat [Cetak](print).

## Membuat PDF

Klik **File > Ekspor > Ekspor ke PDF**. PDF memuat judul, pengarang, subjek, dan kata kunci dokumen dari **File > Info** (jika **Sertakan properti dokumen** diaktifkan), tanggal pembuatannya, serta JeffWrite sebagai program pembuatnya. Pengarang dan judul yang dibuat dari nama berkas tidak disertakan jika informasi pribadi dihapus. Setiap bagian ini disimpan utuh hingga 4.000 karakter. Centang **Buka PDF setelah menyimpan** agar berkas dibuka di program PDF komputer Anda saat disimpan. Pilihan ini nonaktif sampai Anda mencentangnya. Jika komputer Anda tidak dapat membuka berkas, bilah status akan memberi tahu, tetapi berkas tetap tersimpan. Pilih halaman yang akan disertakan: semua, halaman saat ini, atau rentang seperti 1-3, 5 (p2s3 dan s3 juga berlaku di sini). Lalu pilih cara pembuatannya:

- **Optimalkan untuk** memiliki **Standar**, yang mempertahankan gambar Anda apa adanya, dan **Ukuran minimum**, yang membuat setiap gambar tidak lebih tajam dari 150 piksel per inci sehingga berkasnya lebih kecil. Gunakan Ukuran minimum untuk PDF yang akan Anda unggah ke halaman web atau kirim lewat email.
- **Sertakan properti dokumen** memasukkan judul, pengarang, subjek, dan kata kunci dokumen ke dalam PDF.
- **Buat penanda dari judul** menjadikan setiap judul sebagai **penanda**, yaitu entri dalam daftar yang ditampilkan pembaca PDF di samping halaman. Klik penanda untuk langsung ke halamannya.
- **Tag struktur dokumen untuk aksesibilitas** memberi tag pada judul, paragraf, daftar, tabel (beserta baris judulnya), dan gambar (beserta teks alternatifnya), serta menyebutkan bahasanya, sehingga pembaca layar membaca PDF secara berurutan.

Lalu pilih nama. PDF tampak sama di setiap komputer, karena font-nya disimpan di dalamnya. PDF yang dibuat oleh **Cetak** saat Anda memilih berkas PDF juga memiliki penanda dan tag. Jika PDF memuat beberapa halaman pada satu lembar, setiap penanda dan setiap tautan akan menuju lembar yang memuat halamannya, dan halaman pada lembar dibaca sesuai urutan letaknya: dari kiri ke kanan, lalu dari atas ke bawah.

## Menyimpan sebagai halaman web

Klik **File > Ekspor** dan pilih salah satu dari dua jenis halaman web. **Halaman web** adalah berkas yang dapat dibuka di peramban web mana pun.

- **Simpan sebagai Halaman Web** membuat satu berkas .html yang memuat kata-kata, tampilannya, dan gambar Anda. Anda dapat mengirim berkas tunggal itu saja. Tampilan yang dipertahankan adalah font, ukuran, warna, sorotan, huruf tebal, huruf miring, garis bawah, perataan, dan spasi di sekitar paragraf.
- **Simpan sebagai Halaman Web Terfilter** membuat berkas .html kecil yang hanya berisi struktur: judul, huruf tebal dan miring, daftar, tabel, tautan, dan gambar. Berkas ini tidak memiliki font, ukuran, atau warna, sehingga Anda dapat memberinya tampilan sendiri nanti.

Keduanya mempertahankan judul, daftar berpoin dan bernomor, tabel dengan sel yang digabungkan, tautan, dan gambar. Teks tersembunyi, kata-kata yang dihapus oleh perubahan yang dilacak, komentar, header, footer, dan catatan tidak disertakan. Tautan yang akan menjalankan program atau skrip disimpan sebagai teks biasa.

## Menyimpan sebagai e-book

**E-book** adalah buku yang Anda baca di pembaca e-book, ponsel, atau tablet. Berkasnya berupa **EPUB**. Teks menyesuaikan diri dengan layar apa pun, sehingga pembaca dapat memperbesar atau memperkecil huruf.

1. Klik **File > Ekspor > Simpan sebagai E-book (EPUB)**.
2. Periksa **Judul** dan **Pengarang**. Keduanya awalnya berasal dari judul dan pengarang dalam properti dokumen Anda (**File > Info**). Anda dapat mengubahnya khusus untuk buku ini.
3. Aktifkan **Gunakan gambar pertama sebagai sampul** jika Anda ingin gambar pertama dalam dokumen menjadi sampul. Tanpa itu, buku tidak memiliki sampul.
4. Klik **OK**, lalu pilih lokasi untuk menyimpan berkas.

Cara buku dibuat:

- Setiap **Judul 1** memulai bab baru. Teks sebelum Judul 1 pertama menjadi bab tersendiri.
- Daftar **isi** menampilkan setiap Judul 1, Judul 2, dan Judul 3, dan pembaca dapat langsung menuju masing-masing. Judul yang lebih kecil tetap ada di dalam buku, tetapi tidak tercantum di daftar isi.
- Gambar dan tabel tetap berada di tempat Anda meletakkannya. Teks alternatif gambar ikut disertakan, sehingga pembaca layar dapat menjelaskan apa yang ditampilkan gambar tersebut.
- Catatan kaki dan catatan akhir ditempatkan di akhir bab tempat catatan itu berada. Setiap catatan memiliki tautan kembali ke posisinya dalam teks, dan setiap nomor dalam teks menjadi tautan ke catatannya.

Buku dibatasi agar berkas yang rusak tidak menghasilkan buku berukuran sangat besar: hingga 500 bab, 4.000 baris dalam daftar isi, 1.000 gambar (tidak ada yang lebih dari 32 MB), dan 20.000 catatan. Apa pun yang melewati batas akan tetap berada di bab terakhir, atau menjadi kata-kata biasa, dan kata-kata Anda tidak pernah dibuang: gambar yang melewati batas diganti dengan teks alternatifnya, dan catatan yang melewati batas tidak memiliki tampilan. Gambar yang tidak dapat ditampilkan pembaca (metafile, bitmap, atau TIFF) diubah menjadi PNG. Mengekspor ulang dokumen yang sama dan telah disimpan akan menghasilkan buku yang sama, sehingga pembaca tetap pada posisinya.

## Melindungi PDF dengan sandi

Di kotak **Ekspor ke PDF**, klik **Enkripsi dengan sandi…**. Ketik sandi dua kali. Siapa pun yang membuka PDF harus mengetikkan sandi tersebut. Sandi benar-benar melindungi berkas: halamannya diacak dengan enkripsi kuat (AES-256), dan tidak seorang pun dapat membacanya tanpa sandi.

Anda juga dapat mencentang **Batasi pencetakan, penyalinan, dan pengeditan**, memberi sandi kedua, dan memilih apa yang diizinkan. Batasan tersebut hanya bersifat permintaan. Sebagian besar pembaca PDF mematuhinya, tetapi batasan itu tidak menghentikan orang yang ingin melewatinya, jadi jangan mengandalkannya untuk menjaga kerahasiaan berkas.

Sandi yang terlupa tidak dapat dipulihkan, jadi simpan di tempat yang aman. Sandi hanya dipakai untuk PDF tersebut dan tidak pernah disimpan. Kotak ini akan menonaktifkan **PDF/A** selama sandi diatur, karena berkas PDF/A tidak dapat memiliki sandi.

## Membuat PDF/A

Di kotak **Ekspor ke PDF**, centang **PDF/A, untuk penyimpanan jangka panjang**. Berkas PDF/A (PDF/A-1b) memuat setiap font yang digunakan, menamai warnanya, dan mengikuti standar yang dibuat untuk menyimpan berkas selama bertahun-tahun. Berkas ini dapat diberi tag untuk pembaca layar, tetapi tidak dapat memiliki sandi.

## Membuat email

Klik **File > Ekspor > Buat Email** untuk membuat berkas pesan (.eml). Tidak ada yang dikirim. Anda memilih apa yang masuk ke dalam pesan: dokumen, dokumen sebagai PDF, atau halaman yang ditampilkan sebagai gambar dalam pesan (30 halaman pertama). Buka berkas di program email Anda, tambahkan tujuan pengiriman, lalu kirim. Untuk PDF, Anda dapat mengklik **Enkripsi dengan sandi…** di kotak; sampaikan sandinya kepada penerima dengan cara lain, bukan di dalam pesan.

## Font dalam berkas

Berkas .docx dapat memuat font yang digunakan untuk teksnya, sehingga dokumen tampil seperti yang dibuat penulisnya, bahkan di komputer yang tidak memiliki font tersebut. JeffWrite membacanya saat membuka berkas. Font tersebut hanya digunakan untuk dokumen itu, selama dokumen terbuka. Font tidak pernah dipasang di komputer Anda, tidak dapat dilihat dokumen atau program lain, dan hilang saat dokumen ditutup. Font yang dimuat berkas tidak dihitung sebagai font yang hilang.

JeffWrite tidak menggunakan font yang lisensi pembuatnya melarang untuk disematkan, font yang rusak, atau font yang melebihi 64 font pertama dalam satu berkas. Jika itu terjadi, pesan saat Anda membuka berkas akan menyatakannya, dan teks ditampilkan dengan font yang mirip. Berkas mempertahankan font-nya seperti saat Anda menyimpannya.

Untuk memasukkan font yang digunakan teks Anda ke dalam berkas yang Anda simpan, klik **File > Opsi**, buka halaman **Simpan**, dan aktifkan **Simpan font dalam berkas**. Pilihan ini nonaktif pada awalnya. JeffWrite kemudian memasukkan font yang disertakan bersamanya dan yang digunakan teks, dalam setiap gaya yang dipakai dokumen, saat menyimpan berkas .docx. JeffWrite tidak pernah memasukkan font yang lisensinya tidak mengizinkan, dan jika ada font yang dilewati, pesan setelah penyimpanan akan menyebutkan namanya. Berkas menjadi lebih besar, dan paling banyak 64 font dan 256 MB dapat dimuat dalam satu berkas.

## Terkait

- [Selamat datang di JeffWrite](start)
- [Cetak](print)
- [Menggunakan JeffWrite dengan keyboard](keyboard)
