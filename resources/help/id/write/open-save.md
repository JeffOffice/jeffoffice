# Buka, simpan, cetak, dan ekspor
<!-- keywords: font dalam berkas, font tersemat, font hilang, e-book, ebook, epub, e-reader, pembaca e-book, bab, daftar isi, halaman web, html, simpan sebagai halaman web, halaman web tersaring, email, eml, penanda, penanda pdf, shift+f5, perubahan terakhir, tempat terakhir, templat bawaan, galeri, surat, surat lamaran, surat bisnis, CV, riwayat hidup, faktur, agenda rapat, catatan rapat, selebaran, laporan, ringkasan, naskah, buletin, buka, simpan, simpan sebagai, docx, doc, dot, odt, opendocument, rtf, berkas lama, templat, dotx, terbaru, sematkan, baru, tutup, cetak, pdf, ekspor, pulihkan, pemulihan otomatis, belum disimpan, macet, layar berkas, format berkas, simpan salinan, hanya baca, baca saja, buka sebagai salinan, jendela baru, salinan cadangan, templat saya, simpan sebagai templat, ott, perbaiki, buka dan perbaiki, rusak, sandi, kata sandi, enkripsi, lindungi dengan kata sandi -->

Klik **Berkas** di kiri atas untuk melihat layar Berkas. Layar ini memiliki halaman Baru, Buka, Cetak, dan Ekspor, serta perintah Simpan dan Simpan Sebagai. Tekan **Esc** untuk kembali ke dokumen Anda.

## Memulai dokumen baru

Tekan **Ctrl+N**, atau klik **Berkas > Baru** lalu **Dokumen Kosong**. Dokumen baru akan terbuka di jendelanya sendiri. Jika jendela yang Anda gunakan kosong dan belum pernah disimpan, dokumen baru menggantikannya.

## Membuka dokumen

Tekan **Ctrl+O**, atau klik **Berkas > Buka**. Klik **Telusuri** untuk memilih berkas. JeffWrite dapat membuka jenis berkas berikut:

- **.docx** adalah dokumen.
- **.dotx** adalah templat. Templat akan terbuka sebagai dokumen baru yang dibuat darinya, sehingga templat itu sendiri tetap utuh.
- **.docm** dan **.dotm** adalah dokumen dan templat yang dapat memuat **makro**, yaitu program kecil yang menyertai sebuah berkas. JeffWrite tidak pernah menjalankannya. Makro dipertahankan seperti semula saat Anda menyimpan sebagai .docm atau .dotm. Jika Anda menyimpan sebagai .docx atau .odt, makro tidak disertakan, dan JeffWrite akan memberi tahu Anda.
- **.txt** adalah teks biasa, hanya berisi kata-kata. Saat dibuka, JeffWrite langsung membacanya jika berupa ASCII atau Unicode biasa. Jika tidak, JeffWrite menanyakan **pengodean** yang akan digunakan untuk membacanya dan menampilkan beberapa baris pertama, sehingga Anda dapat memastikan hurufnya tampil dengan benar. Pengodean adalah cara sebuah berkas menyimpan huruf, misalnya UTF-8 atau Windows 1252. Saat menyimpan sebagai .txt, Anda memilih pengodean dan akhir baris, dan berkas hanya menyimpan kata-kata: tanpa font, gambar, atau garis tabel.
- **.html** dan **.htm** adalah halaman web. JeffWrite membaca kata-kata, judul, huruf tebal, huruf miring, garis bawah, warna, daftar, dan tabel. Poin dan nomor daftar dimasukkan sebagai tanda ketikan, sedangkan gambar dan tautan tidak disertakan. JeffWrite tidak pernah mengambil apa pun dari internet dan mengabaikan skrip. Halaman web tidak pernah ditimpa: Simpan meminta nama baru dan menyimpan berkas .docx. Untuk membuat halaman web, lihat **Simpan sebagai halaman web** di bawah.
- **.doc** dan **.dot** adalah jenis dokumen dan templat yang lebih lama. JeffWrite dapat membacanya, dan menyimpan perubahan Anda sebagai berkas .docx baru.
- **.odt**, **.ott**, dan **.fodt** adalah berkas OpenDocument. JeffWrite dapat membaca ketiganya. Berkas .odt disimpan sebagai .odt, sedangkan perubahan pada berkas .ott atau .fodt disimpan sebagai berkas .odt baru. Lihat [Buka dan simpan berkas OpenDocument](open-odt).
- **.rtf** adalah Rich Text Format. JeffWrite dapat membaca dan menyimpannya. Lihat [Buka berkas RTF](open-rtf).

Halaman **Buka** juga mencantumkan dokumen yang baru-baru ini Anda buka atau simpan. Klik salah satunya untuk membukanya. Klik kanan pada salah satunya untuk menyematkannya di bagian atas daftar, atau menghapusnya dari daftar.

Jika sebuah berkas memiliki bagian yang belum dapat diedit JeffWrite, seperti tata letak tabel atau header, bagian itu tetap dipertahankan. Lihat [Selamat datang di JeffWrite](start). Jika sebagian berkas tidak dapat dibaca sama sekali, JeffWrite memberi tahu Anda apa saja yang tidak disertakan. JeffWrite tidak akan menimpa berkas tersebut: Simpan meminta nama baru, sehingga berkas asli tetap menyimpan isi yang tidak ada di salinan.

## Menyimpan dokumen

Tekan **Ctrl+S** untuk menyimpan. Pertama kali menyimpan, JeffWrite meminta nama berkas. **Simpan Sebagai** (F12) menyimpan salinan dengan nama baru. Pilih **Templat (*.dotx)** untuk menyimpan templat, **Dokumen Berpenyimpanan Makro (*.docm)** untuk mempertahankan makro, **Teks Polos (*.txt)** untuk hanya kata-kata, atau **Rich Text Format (*.rtf)** agar berkas dapat dibaca program lain. Berkas yang berasal dari jenis berkas lama selalu disimpan dengan nama baru, sehingga berkas lama tidak pernah tertimpa.

## Menyimpan salinan

Klik **Berkas > Simpan Salinan**. Pilih nama. JeffWrite menulis dokumen ke berkas itu, dan Anda tetap dapat melanjutkan pekerjaan di dokumen yang sedang dibuka. Nama dokumen dan perubahan yang belum disimpan tetap seperti semula. Ini berguna jika Anda ingin mengirim satu versi kepada seseorang sambil terus menulis.

## Menyimpan salinan cadangan

Klik **Berkas > Opsi**, lalu pada halaman **Umum** aktifkan **Selalu buat salinan cadangan**. Setiap kali Anda menyimpan, JeffWrite terlebih dahulu menyalin berkas lama ke berkas bernama "Cadangan dari" ditambah nama berkas, di folder yang sama. Jika salinan tidak dapat dibuat, JeffWrite menanyakan apakah Anda ingin menyimpan tanpa salinan itu. Jawabannya adalah Tidak, kecuali Anda memilih Ya, dan berkas tetap seperti semula. Dengan begitu Anda dapat kembali ke versi tersimpan terakhir.

## Menyimpan sebagai templat

Templat adalah dokumen yang Anda gunakan sebagai titik awal. Klik **Berkas > Simpan sebagai Templat**, lalu ketik nama. Templat disimpan di **Templat Saya**, dan dokumen yang sedang Anda tulis tidak berubah. Di **Simpan Sebagai**, Anda juga dapat memilih **Templat (*.dotx)** atau **Templat OpenDocument Text (*.ott)** untuk menyimpan berkas templat di lokasi yang Anda inginkan.

Untuk memulai dokumen baru dari templat Anda, klik **Berkas > Baru**. Templat Anda tercantum di bawah **Templat Saya**. Klik salah satunya. Dokumen baru akan terbuka dengan teks dan gaya dari templat tersebut. Untuk menghapus templat dari daftar, klik kanan padanya dan pilih **Hapus dari Templat Saya**.

## Cara lain membuka berkas

Di halaman **Buka**, klik **Lebih Banyak Cara Membuka**. Anda juga dapat mengklik kanan nama di daftar Terbaru. Setiap cara meminta berkas, atau menggunakan berkas yang Anda klik:

- **Buka Hanya-Baca** membuka berkas hanya untuk dibaca. Anda dapat melihat, menggulir, mencari, dan menyalin, tetapi tidak dapat mengubah apa pun. Bilah judul menampilkan "Hanya-Baca". Jika Anda menekan **Simpan**, JeffWrite meminta nama baru, sehingga berkas yang Anda buka tidak berubah.
- **Buka sebagai Salinan** membuka dokumen baru yang berisi teks berkas tersebut. Dokumen ini tidak memiliki berkas sendiri, sehingga **Simpan** akan menanyakan lokasi penyimpanannya. Berkas yang Anda buka tidak pernah diubah.
- **Buka di Jendela Baru** membuka berkas di jendela tersendiri, meskipun jendela yang sedang Anda gunakan kosong.
- **Buka dan Perbaiki** digunakan untuk berkas .docx atau .doc yang rusak, misalnya berkas yang terpotong saat disalin. Lihat di bawah.

## Buka dan Perbaiki

Kadang berkas .docx atau .doc menjadi rusak. Penyalinan berhenti di tengah jalan, atau disk mengalami kesalahan. Berkas .docx adalah kumpulan bagian-bagian kecil, seperti teks, gaya, dan gambar. **Buka dan Perbaiki** membaca setiap bagian yang masih dapat dibaca. Bagian yang terhenti di tengah dipertahankan sampai titik terhentinya. Bagian yang tidak dapat dibaca dibuang. Jika daftar bagian atau tautan di antaranya hilang, JeffWrite membuatnya kembali.

Berkas .doc lama adalah satu wadah besar yang berisi beberapa aliran data. Satu aliran menyimpan teks, dan aliran lain menyimpan tabel yang menunjukkan di mana setiap potongan teks berada dan bagaimana tampilannya. Jika wadah terpotong atau daftarnya rusak, **Buka dan Perbaiki** mencari aliran-aliran itu berdasarkan isinya, lalu memulihkan sebanyak mungkin. Jika daftar yang menentukan urutan potongan-potongan itu rusak, program mengambil setiap potongan dan mengikutkannya pada potongan sebelumnya, lalu menampilkan apa yang ditemukan di sana. Teks tersebut mungkin berasal dari salinan lama dokumen atau dari bagian lain berkas, sehingga pesan akan menyebutkan paragraf tempat teks itu dimulai dan meminta Anda membandingkannya dengan cadangan jika Anda memilikinya. Jika tebakan muncul sebelum teks, pesan menyatakan bahwa seluruh dokumen disusun ulang, dan jika tebakan muncul sesudah teks, pesan menyatakan bahwa pemformatannya yang disusun ulang. Jika gambar tidak dapat dipulihkan sepenuhnya, pesan akan menyatakannya. Jika tabel hilang, teks dipertahankan sebagai paragraf biasa, tanpa gaya, daftar, tabel, atau gambar. Berkas .doc yang memakai sandi tidak dapat diperbaiki.

Dokumen yang sudah diperbaiki akan terbuka sebagai salinan dengan nama seperti "Surat yang diperbaiki". Sebuah pesan memberi tahu Anda apa yang dipertahankan sampai titik putus dan apa yang tidak disertakan. Berkas yang rusak tidak diubah. Simpan salinan yang sudah diperbaiki dengan nama tersendiri, lalu periksa kembali sebelum Anda mempercayainya.

## Mendapatkan kembali pekerjaan yang belum disimpan

JeffWrite menyimpan salinan cadangan dari pekerjaan yang belum disimpan setiap beberapa menit, sebagai berkas di folder tersendiri. Jika program tertutup secara tidak terduga, JeffWrite menawarkan salinan tersebut saat Anda memulainya kembali. Untuk melihatnya nanti, klik **Berkas > Buka > Pulihkan Pekerjaan yang Belum Disimpan**. Anda dapat memilih mana yang ingin dibuka, dan menyimpan masing-masing di lokasi yang Anda inginkan.

## Memulai dari templat

**Berkas > Baru** menampilkan **Dokumen Kosong** dan sejumlah templat yang disertakan bersama JeffWrite, dikelompokkan menurut jenisnya: surat (surat pribadi, surat lamaran, dan surat bisnis), laporan (laporan, laporan dengan halaman sampul dan daftar isi, serta ringkasan), naskah, publikasi (buletin dan selebaran), riwayat hidup (satu yang klasik dan satu dengan kolom samping), dan dokumen bisnis (faktur, agenda rapat, dan catatan rapat). Arahkan kursor ke gambar untuk melihat kegunaan templat. Setiap gambar adalah halaman pertama dari dokumen yang akan Anda dapatkan. **Templat** adalah dokumen yang sudah disiapkan, dengan gaya, margin, dan teks contoh. Klik salah satunya, lalu ganti teks contoh dengan teks Anda sendiri. Templat yang Anda simpan sendiri tercantum di bawah **Templat Saya**.

## Lanjutkan dari tempat terakhir

JeffWrite mengingat letak kursor saat Anda menyimpan atau menutup berkas. Saat Anda membuka berkas itu lagi, kursor kembali di tempat tersebut. Tekan **Shift+F5** untuk melompat ke tempat terakhir Anda melakukan pengeditan. Setiap kali menekan tombol ini, Anda pindah ke tempat sebelumnya: tiga tempat terakhir, lalu kembali ke yang pertama.

## Menyimpan sebagai .odt

Berkas .odt tidak dapat memuat semua yang dapat dimuat berkas .docx. Saat Anda menyimpan sebagai .odt, JeffWrite terlebih dahulu mendaftar apa saja yang akan hilang, dan memberi Anda pilihan untuk menyimpan sebagai berkas .docx. Lihat [Info](info).

## Mencetak

Tekan **Ctrl+P**, atau klik **Berkas > Cetak**. Pilih printer, jumlah salinan, dan halamannya, periksa setiap halaman di pratinjau, lalu klik **Cetak**. Lihat [Cetak](print).

## Membuat PDF

Klik **Berkas > Ekspor > Ekspor ke PDF**. PDF membawa judul, pengarang, subjek, dan kata kunci dokumen dari **Berkas > Info** (jika **Sertakan properti dokumen** diaktifkan), tanggal pembuatannya, serta JeffWrite sebagai program pembuatnya. Pengarang dan judul yang dibuat dari nama berkas tidak disertakan jika informasi pribadi dihapus. Setiap bagian tersebut disimpan utuh hingga 4.000 karakter. Centang **Buka PDF setelah menyimpan** agar berkas dibuka di program PDF komputer Anda saat disimpan. Pilihan ini nonaktif sampai Anda mencentangnya. Jika komputer Anda tidak dapat membuka berkas, bilah status akan menyatakannya (berkas tetap tersimpan). Pilih halaman yang akan disertakan: semua, halaman saat ini, atau rentang seperti 1-3, 5 (p2s3 dan s3 juga dapat digunakan di sini). Kemudian pilih cara pembuatannya:

- **Optimalkan untuk** memiliki pilihan **Standar**, yang mempertahankan gambar Anda apa adanya, dan **Ukuran minimum**, yang membuat setiap gambar tidak lebih tajam dari 150 piksel per inci agar berkas lebih kecil. Gunakan Ukuran minimum untuk PDF yang akan Anda pasang di halaman web atau kirim lewat email.
- **Sertakan properti dokumen** memasukkan judul, pengarang, subjek, dan kata kunci dokumen ke dalam PDF.
- **Buat penanda dari judul** menjadikan setiap judul sebagai **penanda**, yaitu entri dalam daftar yang ditampilkan pembaca PDF di samping halaman. Klik penanda untuk melompat ke halamannya.
- **Tag struktur dokumen untuk aksesibilitas** memberi tag pada judul, paragraf, daftar, tabel (beserta baris judulnya), dan gambar (beserta teks alternatifnya), serta menyatakan bahasanya, sehingga pembaca layar membaca PDF secara berurutan.

Lalu pilih nama. PDF akan tampak sama di setiap komputer, karena font disematkan di dalamnya. PDF yang dibuat oleh **Cetak** saat Anda memilih berkas PDF juga memiliki penanda dan tag. Jika beberapa halaman dimuat dalam satu lembar, setiap penanda dan setiap tautan mengarah ke lembar yang memuat halamannya, dan halaman pada satu lembar dibaca sesuai urutan letaknya di lembar itu: dari kiri ke kanan, lalu dari atas ke bawah.

## Simpan sebagai halaman web

Klik **Berkas > Ekspor** dan pilih salah satu dari dua jenis halaman web. **Halaman web** adalah berkas yang dapat dibuka di peramban web mana pun.

- **Simpan sebagai Halaman Web** membuat satu berkas .html yang memuat kata-kata, tampilannya, dan gambar Anda. Anda dapat mengirim berkas tunggal itu saja. Tampilan yang dipertahankan adalah font, ukuran, warna, sorotan, huruf tebal, huruf miring, garis bawah, perataan, dan jarak di sekitar paragraf.
- **Simpan sebagai Halaman Web Tersaring** membuat berkas .html kecil yang hanya berisi struktur: judul, huruf tebal dan miring, daftar, tabel, tautan, dan gambar. Berkas ini tidak memiliki font, ukuran, atau warna, sehingga Anda dapat memberinya tampilan sendiri nanti.

Keduanya mempertahankan judul, daftar berpoin dan bernomor, tabel dengan sel yang digabung, tautan, dan gambar. Teks tersembunyi, kata-kata yang dihapus oleh perubahan yang dilacak, komentar, header, footer, dan catatan tidak disertakan. Tautan yang akan menjalankan program atau skrip disimpan sebagai teks biasa.

## Simpan sebagai e-book

**E-book** adalah buku yang dibaca di pembaca e-book, ponsel, atau tablet. Berkasnya berformat **EPUB**. Teks menyesuaikan diri dengan layar apa pun, sehingga pembaca dapat memperbesar atau memperkecil huruf.

1. Klik **Berkas > Ekspor > Simpan sebagai E-book (EPUB)**.
2. Periksa **Judul** dan **Pengarang**. Keduanya awalnya diambil dari judul dan pengarang dalam properti dokumen (**Berkas > Info**). Anda dapat mengubahnya khusus untuk buku ini.
3. Aktifkan **Gunakan gambar pertama sebagai sampul** jika Anda ingin gambar pertama dalam dokumen menjadi sampul. Jika tidak, buku tidak memiliki sampul.
4. Klik **OK**, lalu pilih lokasi penyimpanan berkas.

Cara buku ini dibuat:

- Setiap **Judul 1** memulai bab baru. Teks sebelum Judul 1 pertama menjadi bab tersendiri.
- Daftar isi menampilkan setiap Judul 1, Judul 2, dan Judul 3, dan pembaca dapat langsung melompat ke masing-masing. Judul yang lebih kecil tetap ada di buku, tetapi tidak muncul di daftar isi.
- Gambar dan tabel tetap berada di tempat yang Anda tentukan. Teks alternatif gambar ikut disertakan, sehingga pembaca layar dapat menyebutkan apa yang ditampilkan gambar itu.
- Catatan kaki dan catatan akhir ditempatkan di akhir bab tempat catatan itu berada. Setiap catatan memiliki tautan kembali ke tempatnya di teks, dan setiap nomor di teks menjadi tautan ke catatannya.

Buku dibatasi agar berkas yang rusak tidak menghasilkan buku yang sangat besar: hingga 500 bab, 4.000 baris dalam daftar isi, 1.000 gambar (tidak ada yang lebih dari 32 MB), dan 20.000 catatan. Apa pun yang melewati batas akan tetap berada di bab terakhir, atau menjadi teks biasa, dan kata-kata Anda tidak pernah hilang: gambar yang melewati batas diganti teks alternatifnya, dan catatan yang melewati batas tidak memiliki tampilan. Gambar yang tidak dapat ditampilkan pembaca (metafile, bitmap, atau TIFF) diubah menjadi PNG. Mengekspor dokumen tersimpan yang sama lagi akan menghasilkan buku yang sama, sehingga pembaca tetap pada posisinya.

## Melindungi PDF dengan kata sandi

Di kotak **Ekspor ke PDF**, klik **Enkripsi dengan sandi…**. Ketik kata sandi dua kali. Siapa pun yang membuka PDF harus mengetikkan kata sandi tersebut. Kata sandi benar-benar melindungi berkas: halaman diacak dengan enkripsi kuat (AES-256), dan tidak seorang pun dapat membacanya tanpa kata sandi.

Anda juga dapat mencentang **Batasi pencetakan, penyalinan, dan pengeditan**, memberi kata sandi kedua, dan memilih apa saja yang diizinkan. Batasan itu hanya berupa permintaan. Sebagian besar pembaca PDF menghormatinya, tetapi batasan tidak menghentikan orang yang ingin mengakalinya, jadi jangan mengandalkannya untuk menjaga kerahasiaan berkas.

Kata sandi yang terlupa tidak dapat dipulihkan, jadi simpan di tempat yang aman. Kata sandi hanya berlaku untuk PDF itu dan tidak pernah disimpan. Kotak ini menonaktifkan **PDF/A** selama kata sandi diatur, karena berkas PDF/A tidak dapat memiliki kata sandi.

## Membuat PDF/A

Di kotak **Ekspor ke PDF**, centang **PDF/A, untuk disimpan dalam jangka panjang**. Berkas PDF/A (PDF/A-1b) memuat setiap font yang dipakainya, memberi nama pada warnanya, dan mengikuti standar yang dibuat untuk menyimpan berkas selama bertahun-tahun. Berkas ini dapat diberi tag untuk pembaca layar, tetapi tidak dapat memiliki kata sandi.

## Membuat email

Klik **Berkas > Ekspor > Buat Email** untuk membuat berkas pesan (.eml). Tidak ada yang dikirim. Anda memilih apa yang masuk ke dalam pesan: dokumennya, dokumen sebagai PDF, atau halaman yang ditampilkan sebagai gambar di dalam pesan (30 halaman pertama). Buka berkas itu di program email Anda, tambahkan penerimanya, lalu kirim. Untuk PDF, Anda dapat mengklik **Enkripsi dengan sandi…** di kotak tersebut; sampaikan kata sandinya kepada penerima dengan cara lain, jangan di dalam pesan.

## Font dalam berkas

Berkas .docx dapat memuat font yang digunakan untuk teksnya, sehingga dokumen tampil seperti yang dibuat penulisnya, bahkan di komputer yang tidak memiliki font tersebut. JeffWrite membacanya saat membuka berkas. Font itu hanya digunakan untuk dokumen tersebut, selama dokumen terbuka. Font tidak pernah dipasang di komputer Anda, tidak dapat dilihat oleh dokumen atau program lain, dan hilang saat Anda menutup dokumen. Font yang dimuat berkas tidak dihitung sebagai font yang hilang.

JeffWrite tidak menggunakan font yang lisensinya dari pembuatnya melarang penyematan, font yang rusak, atau font di luar 64 font pertama dalam satu berkas. Jika itu terjadi, pesan yang Anda lihat saat membuka berkas akan menyatakannya, dan teks ditampilkan dengan font yang mirip. Berkas mempertahankan font-fontnya seperti saat Anda menyimpannya.

Untuk menyematkan font yang digunakan teks dalam berkas yang Anda simpan, klik **Berkas > Opsi**, buka halaman **Simpan**, lalu aktifkan **Simpan font dalam berkas**. Pilihan ini nonaktif pada awalnya. JeffWrite kemudian memasukkan font yang disertakan bersamanya dan yang digunakan teks, dalam setiap gaya yang dipakai dokumen, saat menyimpan berkas .docx. JeffWrite tidak pernah memasukkan font yang lisensinya tidak mengizinkan, dan jika ada font yang dilewati, pesan setelah penyimpanan akan menyebutkan namanya. Pilihan ini membuat berkas lebih besar, dan paling banyak 64 font serta 256 MB dapat dimasukkan dalam satu berkas.

## Terkait

- [Selamat datang di JeffWrite](start)
- [Cetak](print)
- [Menggunakan JeffWrite dengan papan ketik](keyboard)
