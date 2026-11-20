# Buka, simpan, cetak, dan ekspor
<!-- keywords: font dalam berkas, font tersemat, font hilang, e-book, ebook, epub, e-reader, bab, daftar isi, halaman web, html, simpan sebagai halaman web, halaman web tersaring, email, eml, penanda, penanda pdf, shift+f5, pengeditan terakhir, tempat terakhir berhenti, templat bawaan, galeri, surat, laporan, ringkasan, naskah, buletin, buka, simpan, simpan sebagai, docx, doc, dot, odt, opendocument, rtf, berkas lama, templat, dotx, terbaru, sematkan, baru, tutup, cetak, pdf, ekspor, pulihkan, pemulihan otomatis, belum disimpan, macet, layar berkas, format berkas, simpan salinan, hanya baca, read only, buka sebagai salinan, jendela baru, salinan cadangan, templat saya, simpan sebagai templat, ott, perbaiki, buka dan perbaiki, rusak, sandi, enkripsi, lindungi dengan sandi -->

Klik **Berkas** di kiri atas untuk melihat layar Berkas. Layar ini memiliki halaman Baru, Buka, Cetak, dan Ekspor, serta perintah Simpan dan Simpan Sebagai. Tekan **Esc** untuk kembali ke dokumen Anda.

## Memulai dokumen baru

Tekan **Ctrl+N**, atau klik **Berkas > Baru** lalu **Dokumen Kosong**. Dokumen baru akan terbuka di jendelanya sendiri. Jika jendela yang Anda gunakan kosong dan belum pernah disimpan, dokumen baru akan menggantikannya.

## Membuka dokumen

Tekan **Ctrl+O**, atau klik **Berkas > Buka**. Klik **Telusuri** untuk memilih berkas. JeffWrite dapat membuka jenis berkas berikut:

- **.docx** adalah dokumen.
- **.dotx** adalah templat. Templat akan terbuka sebagai dokumen baru yang dibuat darinya, sehingga templat itu sendiri tetap utuh.
- **.docm** dan **.dotm** adalah dokumen dan templat yang dapat memuat **makro**, yaitu program kecil yang menyertai sebuah berkas. JeffWrite tidak pernah menjalankannya. Makro disimpan seperti semula saat Anda menyimpan sebagai .docm atau .dotm. Jika Anda menyimpan sebagai .docx atau .odt, makro tidak disertakan, dan JeffWrite memberi tahu Anda.
- **.txt** adalah teks polos, hanya kata-katanya. Saat Anda membukanya, JeffWrite langsung membacanya jika berupa ASCII atau Unicode biasa. Jika tidak, JeffWrite menanyakan pengodean yang akan digunakan untuk membacanya dan menampilkan beberapa baris pertama, sehingga Anda dapat melihat apakah hurufnya sudah benar. **Pengodean** adalah cara berkas menyimpan huruf, misalnya UTF-8 atau Windows 1252. Saat Anda menyimpan sebagai .txt, Anda memilih pengodean dan akhir baris, dan berkas hanya menyimpan kata-katanya: tanpa font, gambar, atau garis tabel.
- **.html** dan **.htm** adalah halaman web. JeffWrite membaca kata-kata, judul, huruf tebal, huruf miring, garis bawah, warna, daftar, dan tabel. Poin dan nomor daftar dimasukkan sebagai tanda ketikan, sedangkan gambar dan tautan dihilangkan. JeffWrite tidak pernah mengambil apa pun dari internet dan mengabaikan skrip. Halaman web tidak pernah ditimpa: **Simpan** meminta nama baru dan menyimpan berkas .docx. Untuk membuat halaman web, lihat **Simpan sebagai halaman web** di bawah.
- **.doc** dan **.dot** adalah jenis dokumen dan templat yang lebih lama. JeffWrite membacanya, dan menyimpan perubahan yang Anda buat sebagai berkas .docx baru.
- **.odt**, **.ott**, dan **.fodt** adalah berkas OpenDocument. JeffWrite membaca ketiganya. JeffWrite menyimpan berkas .odt, dan menyimpan perubahan pada berkas .ott atau .fodt sebagai berkas .odt baru. Lihat [Buka dan simpan berkas OpenDocument](open-odt).
- **.rtf** adalah Rich Text Format. JeffWrite membaca dan menyimpannya. Lihat [Buka berkas RTF](open-rtf).

Halaman **Buka** juga mencantumkan dokumen yang baru-baru ini Anda buka atau simpan. Klik salah satunya untuk membukanya. Klik kanan pada salah satunya untuk menyematkannya di bagian atas daftar, atau menghapusnya dari daftar.

Jika sebuah berkas memiliki bagian yang belum dapat diedit JeffWrite, seperti tata letak tabel atau header, bagian itu tetap dipertahankan. Lihat [Selamat datang di JeffWrite](start). Jika sebagian berkas sama sekali tidak dapat dibaca, JeffWrite memberi tahu Anda apa yang ditinggalkan. Setelah itu JeffWrite tidak menimpa berkas tersebut: Simpan meminta nama baru, sehingga berkas asli tetap menyimpan apa yang hilang dari salinan.

## Menyimpan dokumen

Tekan **Ctrl+S** untuk menyimpan. Pertama kali, JeffWrite meminta nama. **Simpan Sebagai** (F12) menyimpan salinan dengan nama baru. Pilih **Templat (*.dotx)** untuk menyimpan templat, **Dokumen Berpenyimpanan Makro (*.docm)** untuk mempertahankan makro, **Teks Polos (*.txt)** untuk hanya kata-kata, atau **Rich Text Format (*.rtf)** untuk berkas yang dapat dibaca program lain. Berkas yang berasal dari jenis berkas lama selalu disimpan dengan nama baru, sehingga berkas lama tidak pernah tertimpa.

## Menyimpan salinan

Klik **Berkas > Simpan Salinan**. Pilih nama. JeffWrite menulis dokumen ke berkas tersebut, dan Anda tetap dapat bekerja di dokumen yang sedang dibuka. Nama dan perubahan yang belum disimpan tetap seperti semula. Ini berguna saat Anda ingin mengirim satu versi kepada seseorang dan tetap melanjutkan penulisan.

## Menyimpan salinan cadangan

Klik **Berkas > Opsi**, lalu pada halaman **Umum**, aktifkan **Selalu buat salinan cadangan**. Setiap kali Anda menyimpan, JeffWrite lebih dulu menyalin berkas lama ke berkas bernama "Cadangan dari" ditambah namanya, di folder yang sama. Jika salinan tidak dapat dibuat, JeffWrite menanyakan apakah akan menyimpan tanpa salinan itu. Jawabannya adalah Tidak kecuali Anda memilih Ya, dan berkas tetap seperti semula. Dengan begitu Anda dapat kembali ke versi terakhir yang disimpan.

## Menyimpan sebagai templat

Templat adalah dokumen yang Anda gunakan sebagai titik awal. Klik **Berkas > Simpan sebagai Templat**, lalu ketik nama. Templat disimpan di **Templat Saya**, dan dokumen yang sedang Anda tulis tidak berubah. Di **Simpan Sebagai**, Anda juga dapat memilih **Templat (*.dotx)**, atau **OpenDocument Text Template (*.ott)**, untuk menyimpan berkas templat di lokasi yang Anda inginkan.

Untuk memulai dokumen baru dari templat Anda, klik **Berkas > Baru**. Templat Anda tercantum di bawah **Templat Saya**. Klik salah satunya. Dokumen baru akan terbuka dengan teks dan gaya dari templat tersebut. Untuk menghapus templat dari daftar, klik kanan padanya dan pilih **Hapus dari Templat Saya**.

## Cara lain membuka berkas

Pada halaman **Buka**, klik **Lebih Banyak Cara untuk Membuka**. Anda juga dapat mengklik kanan nama di daftar Terbaru. Setiap cara meminta berkas, atau menggunakan berkas yang Anda klik:

- **Buka Hanya-Baca** membuka berkas hanya untuk dibaca. Anda dapat melihat, menggulir, mencari, dan menyalin, tetapi tidak dapat mengubah apa pun. Bilah judul bertuliskan "Hanya Baca". Jika Anda menekan **Simpan**, JeffWrite meminta nama baru, sehingga berkas yang Anda buka tidak berubah.
- **Buka sebagai Salinan** membuka dokumen baru yang berisi teks berkas tersebut. Dokumen ini tidak memiliki berkas sendiri, sehingga **Simpan** menanyakan lokasi penyimpanannya. Berkas yang Anda buka tidak pernah diubah.
- **Buka di Jendela Baru** membuka berkas di jendela tersendiri, meskipun jendela yang Anda gunakan kosong.
- **Buka dan Perbaiki** digunakan untuk berkas .docx atau .doc yang rusak, misalnya berkas yang terpotong saat disalin. Lihat di bawah.

## Buka dan Perbaiki

Terkadang berkas .docx atau .doc menjadi rusak. Penyalinan berhenti di tengah jalan, atau disk mengalami kesalahan. Berkas .docx adalah kumpulan bagian yang lebih kecil, seperti teks, gaya, dan gambar. **Buka dan Perbaiki** membaca setiap bagian yang masih dapat dibaca. Bagian yang terhenti di tengah dipertahankan sampai titik ia berhenti. Bagian yang tidak dapat dibaca ditinggalkan. Jika daftar bagian atau tautan di antara bagian-bagian itu hilang, JeffWrite membuatnya kembali.

Berkas .doc lama adalah satu pembungkus besar yang berisi beberapa aliran data. Satu aliran memuat teks, dan aliran lain memuat tabel yang menunjukkan letak setiap potongan teks dan tampilannya. Jika pembungkus terpotong atau daftarnya rusak, **Buka dan Perbaiki** menemukan aliran-aliran itu dari isinya dan membaca sebanyak mungkin. Jika tabel hilang, teks dipertahankan sebagai paragraf biasa, tanpa gaya, daftar, tabel, atau gambar. Berkas .doc yang memakai sandi tidak dapat diperbaiki.

Dokumen yang sudah diperbaiki akan terbuka sebagai salinan dengan nama seperti "Surat yang diperbaiki". Sebuah pesan memberi tahu Anda apa yang dipertahankan sampai titik putus dan apa yang ditinggalkan. Berkas yang rusak tidak diubah. Simpan salinan yang sudah diperbaiki dengan nama tersendiri, dan periksa dengan teliti sebelum Anda mempercayainya.

## Memulihkan pekerjaan yang belum disimpan

JeffWrite menyimpan salinan cadangan dari pekerjaan yang belum disimpan setiap beberapa menit, sebagai berkas di folder tersendiri. Jika program tertutup tanpa diduga, JeffWrite menawarkan salinan tersebut saat Anda memulainya kembali. Klik **Berkas > Buka > Pulihkan Pekerjaan yang Belum Disimpan** untuk melihatnya nanti. Anda dapat memilih salinan mana yang akan dibuka, dan menyimpan setiap salinan di lokasi yang Anda inginkan.

## Memulai dari templat

**Berkas > Baru** menampilkan **Dokumen Kosong** dan sejumlah templat yang disertakan dengan JeffWrite, dikelompokkan menurut jenisnya: surat, laporan, ringkasan, naskah, dan buletin. Setiap gambar menunjukkan halaman pertama dokumen yang akan Anda dapatkan. **Templat** adalah dokumen yang sudah diatur, lengkap dengan gaya, margin, dan teks contoh. Klik salah satunya, lalu ganti teks contoh dengan teks Anda sendiri. Templat yang Anda simpan sendiri tercantum di bawah **Templat Saya**.

## Melanjutkan dari tempat terakhir berhenti

JeffWrite mengingat posisi kursor saat Anda menyimpan atau menutup berkas. Saat Anda membuka berkas itu lagi, kursor berada di posisi tersebut. Tekan **Shift+F5** untuk melompat ke tempat terakhir Anda mengedit. Setiap penekanan berpindah ke tempat sebelumnya: tiga tempat terakhir, lalu kembali ke yang pertama.

## Menyimpan sebagai .odt

Berkas .odt tidak dapat memuat semua yang dapat dimuat berkas .docx. Saat Anda menyimpan sebagai .odt, JeffWrite lebih dulu mencantumkan apa saja yang akan hilang, dan memungkinkan Anda menyimpan berkas .docx sebagai gantinya. Lihat [Info](info).

## Mencetak

Tekan **Ctrl+P**, atau klik **Berkas > Cetak**. Pilih printer, jumlah salinan, dan halaman, periksa setiap halaman di pratinjau, lalu klik **Cetak**. Lihat [Cetak](print).

## Membuat PDF

Klik **Berkas > Ekspor > Ekspor ke PDF**. PDF memuat judul, pengarang, subjek, dan kata kunci dokumen dari **Berkas > Info** (jika **Sertakan properti dokumen** diaktifkan), tanggal pembuatannya, dan JeffWrite sebagai program pembuatnya. Pengarang dan judul yang dibuat dari nama berkas tidak disertakan jika informasi pribadi dihapus. Masing-masing dipertahankan utuh hingga 4.000 karakter. Centang **Buka PDF setelah menyimpan** agar berkas dibuka di program PDF komputer Anda saat disimpan. Opsi ini nonaktif sampai Anda mencentangnya. Jika komputer Anda tidak dapat membuka berkas, bilah status akan memberi tahu Anda (berkas tetap tersimpan). Pilih halaman yang akan disertakan: semua, halaman saat ini, atau rentang seperti 1-3, 5 (p2s3 dan s3 juga berlaku di sini). Lalu pilih cara pembuatannya:

- **Optimalkan untuk** memiliki **Standar**, yang mempertahankan gambar Anda apa adanya, dan **Ukuran minimum**, yang membuat setiap gambar tidak lebih halus dari 150 piksel per inci sehingga berkas menjadi lebih kecil. Gunakan Ukuran minimum untuk PDF yang akan Anda pasang di halaman web atau kirim lewat email.
- **Sertakan properti dokumen** memasukkan judul, pengarang, subjek, dan kata kunci dokumen ke dalam PDF.
- **Buat penanda dari judul** menjadikan setiap judul sebagai **penanda**, yaitu entri dalam daftar yang ditampilkan pembaca PDF di samping halaman. Klik penanda untuk melompat ke halamannya.
- **Tag struktur dokumen untuk aksesibilitas** memberi tag pada judul, paragraf, daftar, tabel (beserta baris judulnya), dan gambar (beserta teks alternatifnya), serta menetapkan bahasanya, sehingga pembaca layar membacakan PDF secara berurutan.

Lalu pilih nama. PDF tampak sama di setiap komputer karena font-nya disematkan di dalam berkas. PDF yang dibuat **Cetak** saat Anda memilih printer PDF juga memiliki penanda dan tag. Jika beberapa halaman dicetak dalam satu lembar, setiap penanda dan setiap tautan menuju lembar yang memuat halamannya, dan halaman pada satu lembar dibaca sesuai urutan letaknya: dari kiri ke kanan, lalu dari atas ke bawah.

## Menyimpan sebagai halaman web

Klik **Berkas > Ekspor** dan pilih salah satu dari dua jenis halaman web. **Halaman web** adalah berkas yang dapat Anda buka di peramban web mana pun.

- **Simpan sebagai Halaman Web** membuat satu berkas .html yang memuat kata-kata, tampilannya, dan gambar Anda. Anda dapat mengirim berkas tunggal itu saja. Tampilan yang dipertahankan adalah font, ukuran, warna, sorotan, huruf tebal, huruf miring, garis bawah, perataan, dan spasi di sekitar paragraf.
- **Simpan sebagai Halaman Web Tersaring** membuat berkas .html kecil yang hanya berisi struktur: judul, huruf tebal dan miring, daftar, tabel, tautan, dan gambar. Berkas ini tidak memiliki font, ukuran, atau warna, sehingga Anda dapat memberinya tampilan sendiri nanti.

Keduanya mempertahankan judul, daftar berpoin dan bernomor, tabel dengan sel gabungan, tautan, dan gambar. Teks tersembunyi, kata-kata yang dihapus oleh perubahan yang dilacak, komentar, header, footer, dan catatan tidak disertakan. Tautan yang akan menjalankan program atau skrip disimpan sebagai teks biasa.

## Menyimpan sebagai e-book

**E-book** adalah buku yang Anda baca di e-reader, ponsel, atau tablet. Berkasnya berupa **EPUB**. Kata-kata menyesuaikan diri dengan layar apa pun, sehingga pembaca dapat memperbesar atau memperkecil huruf.

1. Klik **Berkas > Ekspor > Simpan sebagai E-book (EPUB)**.
2. Periksa **Judul** dan **Pengarang**. Keduanya awalnya berisi judul dan pengarang dari properti dokumen Anda (**Berkas > Info**). Anda dapat mengubahnya khusus untuk buku ini.
3. Aktifkan **Gunakan gambar pertama sebagai sampul** jika Anda ingin gambar pertama dalam dokumen menjadi sampul. Tanpa opsi ini, buku tidak memiliki sampul.
4. Klik **OK**, lalu pilih lokasi penyimpanan berkas.

Cara buku ini dibuat:

- Setiap **Judul 1** memulai bab baru. Kata-kata sebelum Judul 1 pertama menjadi bab tersendiri.
- Daftar **isi** memuat setiap Judul 1, Judul 2, dan Judul 3, dan pembaca dapat melompat ke masing-masing. Judul yang lebih kecil ada di dalam buku tetapi tidak di daftar isi.
- Gambar dan tabel tetap berada di tempat Anda meletakkannya. Teks alternatif gambar ikut bersamanya, sehingga pembaca layar dapat menyebutkan apa yang ditampilkannya.
- Catatan kaki dan catatan akhir ditampilkan di akhir bab tempat catatan itu berada. Setiap catatan memiliki tautan kembali ke posisinya dalam teks, dan setiap nomor dalam teks adalah tautan ke catatannya.

Sebuah buku dibatasi agar berkas yang rusak tidak menghasilkan buku yang sangat besar: maksimal 500 bab, 4.000 baris dalam daftar isi, 1.000 gambar (tidak ada yang lebih dari 32 MB), dan 20.000 catatan. Apa pun yang melampaui batas tetap berada di bab terakhir, atau menjadi kata-kata biasa, dan kata-kata Anda tidak pernah dibuang: gambar yang melampaui batas digantikan teks alternatifnya, dan catatan yang melampaui batas tidak memiliki tampilan. Gambar yang tidak dapat ditampilkan pembaca (metafile, bitmap, atau TIFF) diubah menjadi PNG. Mengekspor dokumen tersimpan yang sama lagi akan menghasilkan buku yang sama, sehingga pembaca tetap pada posisinya.

## Melindungi PDF dengan sandi

Di kotak **Ekspor ke PDF**, klik **Enkripsi dengan sandi…**. Ketik sandi dua kali. Siapa pun yang membuka PDF harus mengetikkan sandi tersebut. Sandi benar-benar melindungi berkas: halamannya diacak dengan enkripsi kuat (AES-256), dan tidak seorang pun dapat membacanya tanpa sandi.

Anda juga dapat mencentang **Batasi pencetakan, penyalinan, dan pengeditan**, memberikan sandi kedua, dan memilih apa saja yang diizinkan. Pembatasan itu hanya berupa permintaan. Sebagian besar pembaca PDF menghormatinya, tetapi pembatasan itu tidak menghentikan orang yang ingin melewatinya, sehingga jangan mengandalkannya untuk menjaga kerahasiaan berkas.

Sandi yang lupa tidak dapat dipulihkan, jadi simpan di tempat yang aman. Sandi hanya dipakai untuk PDF tersebut dan tidak pernah disimpan. Kotak ini menonaktifkan **PDF/A** selama sandi diatur, karena berkas PDF/A tidak dapat memiliki sandi.

## Membuat PDF/A

Di kotak **Ekspor ke PDF**, centang **PDF/A, untuk penyimpanan jangka panjang**. Berkas PDF/A (PDF/A-1b) memuat setiap font yang dipakainya, memberi nama pada warnanya, dan mengikuti standar yang dibuat untuk menyimpan berkas selama bertahun-tahun. Berkas ini dapat diberi tag untuk pembaca layar, tetapi tidak dapat memakai sandi.

## Membuat email

Klik **Berkas > Ekspor > Buat Email** untuk membuat berkas pesan (.eml). Tidak ada yang dikirim. Anda memilih apa yang masuk ke pesan: dokumen, dokumen sebagai PDF, atau halaman yang ditampilkan sebagai gambar di dalam pesan (30 halaman pertama). Buka berkas di program email Anda, tambahkan tujuannya, lalu kirim. Untuk PDF, Anda dapat mengklik **Enkripsi dengan sandi…** di kotak; sampaikan sandinya kepada penerima dengan cara lain, bukan di dalam pesan.

## Font dalam berkas

Berkas .docx dapat memuat font tempat teksnya diatur, sehingga dokumen tampak seperti yang dibuat penulisnya, bahkan di komputer yang tidak memiliki font tersebut. JeffWrite membacanya saat membuka berkas. Font itu hanya dipakai untuk dokumen tersebut, selama dokumen dibuka. Font itu tidak pernah dipasang di komputer Anda, tidak dapat dilihat dokumen atau program lain, dan hilang saat dokumen ditutup. Font yang dimuat berkas tidak dihitung sebagai font yang hilang.

JeffWrite tidak menggunakan font yang lisensi pembuatnya melarang untuk disematkan, font yang rusak, atau font apa pun di luar 64 font pertama dalam satu berkas. Jika itu terjadi, pesan yang Anda lihat saat membuka berkas akan menyebutkannya, dan teks ditampilkan dengan font yang mirip. Berkas tetap menyimpan font-nya seperti semula saat Anda menyimpannya.

Untuk menyertakan font yang dipakai teks Anda dalam berkas yang Anda simpan, klik **Berkas > Opsi**, buka halaman **Simpan**, dan aktifkan **Simpan font dalam berkas**. Opsi ini nonaktif pada awalnya. JeffWrite kemudian memasukkan font yang disertakan bersamanya dan yang dipakai teks, dalam setiap gaya yang dipakai dokumen, saat menyimpan berkas .docx. JeffWrite tidak pernah memasukkan font yang lisensinya tidak mengizinkan, dan saat ada font yang dilewati, pesan setelah penyimpanan akan menyebutkan namanya. Berkas akan menjadi lebih besar, dan paling banyak 64 font serta 256 MB yang dimasukkan dalam satu berkas.

## Terkait

- [Selamat datang di JeffWrite](start)
- [Cetak](print)
- [Menggunakan JeffWrite dengan keyboard](keyboard)
