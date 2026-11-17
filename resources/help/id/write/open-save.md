# Membuka, menyimpan, mencetak, dan mengekspor
<!-- keywords: e-book, ebook, epub, e-reader, pembaca e-book, bab, daftar isi, isi, halaman web, html, simpan sebagai halaman web, halaman web tersaring, email, eml, penanda, penanda pdf, shift+f5, pengeditan terakhir, tempat terakhir berhenti, templat bawaan, galeri, surat, laporan, ringkasan, naskah, buletin, buka, simpan, simpan sebagai, docx, doc, dot, odt, opendocument, rtf, berkas lama, templat, dotx, terbaru, sematkan, baru, tutup, cetak, pdf, ekspor, pulihkan, pemulihan otomatis, belum disimpan, macet, layar berkas, format berkas, simpan salinan, hanya-baca, hanya baca, buka sebagai salinan, jendela baru, salinan cadangan, templat saya, simpan sebagai templat, ott, perbaiki, buka dan perbaiki, rusak, sandi, kata sandi, enkripsi, lindungi dengan kata sandi -->

Klik **Berkas** di pojok kiri atas untuk membuka layar Berkas. Layar ini memiliki halaman Baru, Buka, Cetak, dan Ekspor, serta perintah Simpan dan Simpan Sebagai. Tekan **Esc** untuk kembali ke dokumen Anda.

## Memulai dokumen baru

Tekan **Ctrl+N**, atau klik **Berkas > Baru** lalu **Dokumen Kosong**. Dokumen baru akan terbuka di jendelanya sendiri. Jika jendela yang Anda gunakan kosong dan belum pernah disimpan, dokumen baru menggantikannya.

## Membuka dokumen

Tekan **Ctrl+O**, atau klik **Berkas > Buka**. Klik **Telusuri** untuk memilih berkas. JeffWrite dapat membuka jenis berkas berikut:

- **.docx** adalah dokumen.
- **.dotx** adalah templat. Templat akan terbuka sebagai dokumen baru yang dibuat darinya, sehingga templat itu sendiri tetap utuh.
- **.docm** dan **.dotm** adalah dokumen dan templat yang dapat memuat **makro**, yaitu program kecil yang menyertai sebuah berkas. JeffWrite tidak pernah menjalankannya. Makro dipertahankan seperti semula saat Anda menyimpan sebagai .docm atau .dotm. Jika Anda menyimpan sebagai .docx atau .odt, makro tidak disertakan, dan JeffWrite akan memberi tahu Anda.
- **.txt** adalah teks biasa, hanya berisi kata-kata. Saat Anda membukanya, JeffWrite langsung membacanya jika berupa ASCII biasa atau Unicode. Jika tidak, JeffWrite menanyakan **pengodean** mana yang akan dipakai dan menampilkan beberapa baris pertama, agar Anda dapat memastikan hurufnya tampil benar. Pengodean adalah cara sebuah berkas menyimpan huruf, misalnya UTF-8 atau Windows 1252. Saat Anda menyimpan sebagai .txt, Anda memilih pengodean dan akhir baris, dan berkas hanya menyimpan kata-katanya: tanpa font, gambar, atau garis tabel.
- **.html** dan **.htm** adalah halaman web. JeffWrite membaca kata-kata, judul, huruf tebal, huruf miring, garis bawah, warna, daftar, dan tabel. Poin dan nomor daftar masuk sebagai tanda yang diketik, sedangkan gambar dan tautan tidak disertakan. JeffWrite tidak pernah mengambil apa pun dari internet dan mengabaikan skrip. Halaman web tidak pernah ditimpa: **Simpan** meminta nama baru dan menyimpan berkas .docx. Untuk membuat halaman web, lihat **Simpan sebagai halaman web** di bawah.
- **.doc** dan **.dot** adalah jenis dokumen dan templat yang lebih lama. JeffWrite dapat membacanya, dan perubahan yang Anda buat disimpan sebagai berkas .docx baru.
- **.odt**, **.ott**, dan **.fodt** adalah berkas OpenDocument. JeffWrite dapat membaca ketiganya. Berkas .odt disimpan langsung, sedangkan perubahan pada berkas .ott atau .fodt disimpan sebagai berkas .odt baru. Lihat [Buka dan simpan berkas OpenDocument](open-odt).
- **.rtf** adalah Rich Text Format. JeffWrite dapat membaca dan menyimpannya. Lihat [Buka berkas RTF](open-rtf).

Halaman **Buka** juga mencantumkan dokumen yang baru-baru ini Anda buka atau simpan. Klik salah satunya untuk membukanya. Klik kanan pada salah satunya untuk menyematkannya di bagian atas daftar, atau menghapusnya dari daftar.

Jika berkas memiliki bagian yang belum dapat diedit JeffWrite, misalnya tata letak tabel atau header, bagian itu tetap dipertahankan. Lihat [Selamat datang di JeffWrite](start). Jika sebagian berkas sama sekali tidak dapat dibaca, JeffWrite memberi tahu Anda apa yang tidak ikut dibuka. Setelah itu, JeffWrite tidak akan menimpa berkas tersebut: Simpan meminta nama baru, sehingga berkas asli tetap menyimpan bagian yang tidak ikut tersalin.

## Menyimpan dokumen

Tekan **Ctrl+S** untuk menyimpan. Pertama kali, JeffWrite meminta nama berkas. **Simpan Sebagai** (F12) menyimpan salinan dengan nama baru. Pilih **Templat (*.dotx)** untuk menyimpan templat, **Dokumen Berpenyimpanan Makro (*.docm)** untuk mempertahankan makro, **Teks Polos (*.txt)** untuk hanya kata-kata, atau **Rich Text Format (*.rtf)** untuk berkas yang dapat dibaca program lain. Berkas yang berasal dari jenis berkas lama selalu disimpan dengan nama baru, sehingga berkas lama tidak pernah tertimpa.

## Menyimpan salinan

Klik **Berkas > Simpan Salinan**. Pilih nama. JeffWrite menulis dokumen ke berkas tersebut, dan Anda tetap dapat melanjutkan pekerjaan di dokumen yang sedang dibuka. Nama dan perubahan yang belum disimpan tetap seperti semula. Ini berguna ketika Anda ingin mengirim satu versi kepada seseorang dan tetap melanjutkan penulisan.

## Menyimpan salinan cadangan

Klik **Berkas > Opsi**, lalu pada halaman **Umum**, aktifkan **Selalu buat salinan cadangan**. Setiap kali Anda menyimpan, JeffWrite lebih dulu menyalin berkas lama ke berkas bernama "Cadangan dari" diikuti namanya, di folder yang sama. Jika salinan tidak dapat dibuat, JeffWrite menanyakan apakah Anda ingin menyimpan tanpa cadangan. Jawaban standarnya adalah Tidak, kecuali Anda memilih Ya, dan berkas tetap seperti semula. Dengan begitu, Anda dapat kembali ke versi tersimpan terakhir.

## Menyimpan sebagai templat

Templat adalah dokumen yang Anda gunakan sebagai titik awal. Klik **Berkas > Simpan sebagai Templat**, lalu ketik nama. Templat masuk ke **Templat Saya**, dan dokumen yang sedang Anda tulis tidak berubah. Di **Simpan Sebagai**, Anda juga dapat memilih **Templat (*.dotx)** atau **Templat Teks OpenDocument (*.ott)** untuk menyimpan berkas templat di lokasi yang Anda inginkan.

Untuk memulai dokumen baru dari templat Anda, klik **Berkas > Baru**. Templat Anda tercantum di bawah **Templat Saya**. Klik salah satunya. Dokumen baru akan terbuka dengan teks dan gaya dari templat. Untuk menghapus templat dari daftar, klik kanan padanya lalu pilih **Hapus dari Templat Saya**.

## Cara lain membuka berkas

Pada halaman **Buka**, klik **Lebih Banyak Cara untuk Membuka**. Anda juga dapat mengklik kanan nama di daftar Terbaru. Setiap cara meminta berkas, atau menggunakan berkas yang Anda klik:

- **Buka Hanya-Baca** membuka berkas hanya untuk dibaca. Anda dapat melihat, menggulir, mencari, dan menyalin, tetapi tidak dapat mengubah apa pun. Bilah judul bertuliskan "Hanya Baca". Jika Anda menekan **Simpan**, JeffWrite meminta nama baru, sehingga berkas yang Anda buka tidak berubah.
- **Buka sebagai Salinan** membuka dokumen baru dengan teks dari berkas tersebut. Dokumen ini tidak memiliki berkas sendiri, sehingga **Simpan** akan menanyakan lokasi penyimpanannya. Berkas yang Anda buka tidak pernah diubah.
- **Buka di Jendela Baru** membuka berkas di jendela tersendiri, bahkan saat jendela yang Anda gunakan kosong.
- **Buka dan Perbaiki** digunakan untuk berkas .docx atau .doc yang rusak, misalnya berkas yang terpotong saat disalin. Lihat di bawah.

## Buka dan Perbaiki

Terkadang berkas .docx atau .doc menjadi rusak. Penyalinan terhenti di tengah jalan, atau disk mengalami kesalahan. Berkas .docx adalah kumpulan bagian-bagian kecil, misalnya teks, gaya, dan gambar. **Buka dan Perbaiki** membaca setiap bagian yang dapat dibaca. Bagian yang terhenti di tengah dipertahankan sampai titik terhentinya. Bagian yang tidak dapat dibaca dihilangkan. Jika daftar bagian atau tautan di antara bagian-bagian itu hilang, JeffWrite membuatnya kembali.

Berkas .doc lama adalah satu wadah besar yang berisi beberapa aliran data. Salah satu aliran memuat teks, dan aliran lainnya memuat tabel yang menunjukkan letak setiap potongan teks dan tampilannya. Jika wadahnya terpotong atau daftarnya rusak, **Buka dan Perbaiki** mencari aliran-aliran tersebut berdasarkan isinya dan membaca sebanyak mungkin. Jika tabel hilang, teks dipertahankan sebagai paragraf biasa, tanpa gaya, daftar, tabel, atau gambar. Berkas .doc yang memiliki kata sandi tidak dapat diperbaiki.

Dokumen yang sudah diperbaiki akan terbuka sebagai salinan dengan nama seperti "Surat yang diperbaiki". Sebuah pesan akan memberi tahu Anda apa saja yang dipertahankan sampai titik putus dan apa saja yang dihilangkan. Berkas yang rusak tidak diubah. Simpan salinan yang sudah diperbaiki dengan nama tersendiri, dan periksa kembali sebelum Anda mempercayainya.

## Mengambil kembali pekerjaan yang belum disimpan

JeffWrite menyimpan salinan cadangan pekerjaan yang belum disimpan setiap beberapa menit, sebagai berkas di folder tersendiri. Jika program tertutup secara tidak terduga, program akan menawarkan salinan tersebut saat Anda memulainya kembali. Untuk melihatnya nanti, klik **Berkas > Buka > Pulihkan Pekerjaan yang Belum Disimpan**. Anda dapat memilih mana yang akan dibuka, dan menyimpan setiap salinan di lokasi mana pun yang Anda inginkan.

## Memulai dari templat

**Berkas > Baru** menampilkan **Dokumen Kosong** dan sejumlah templat bawaan JeffWrite, dikelompokkan menurut jenisnya: surat, laporan, ringkasan, naskah, dan buletin. Setiap gambar menampilkan halaman pertama dokumen yang akan Anda dapatkan. **Templat** adalah dokumen yang sudah disiapkan, lengkap dengan gaya, margin, dan contoh kata. Klik salah satunya, lalu ganti contoh kata dengan milik Anda. Templat yang Anda simpan sendiri tercantum di bawah **Templat Saya**.

## Melanjutkan dari tempat Anda berhenti

JeffWrite mengingat letak kursor saat Anda menyimpan atau menutup berkas. Saat Anda membuka berkas itu lagi, kursor berada di tempat yang sama. Tekan **Shift+F5** untuk melompat ke tempat-tempat yang terakhir Anda edit. Setiap kali ditekan, lompatan berpindah ke tempat sebelumnya: tiga tempat terakhir, lalu kembali ke yang pertama.

## Menyimpan sebagai .odt

Berkas .odt tidak dapat memuat semua yang dapat dimuat berkas .docx. Saat Anda menyimpan sebagai .odt, JeffWrite terlebih dahulu menampilkan daftar hal yang akan hilang, dan Anda dapat memilih untuk menyimpan sebagai berkas .docx. Lihat [Info](info).

## Mencetak

Tekan **Ctrl+P**, atau klik **Berkas > Cetak**. Pilih printer, jumlah salinan, dan halaman, lihat setiap halaman di pratinjau, lalu klik **Cetak**. Lihat [Cetak](print).

## Membuat PDF

Klik **Berkas > Ekspor > Ekspor ke PDF**. Pilih halaman yang akan disertakan: semua, halaman saat ini, atau rentang seperti 1-3, 5 (p2s3 dan s3 juga berlaku di sini). Lalu pilih cara pembuatannya:

- **Optimalkan untuk** memiliki **Standar**, yang mempertahankan gambar apa adanya, dan **Ukuran minimum**, yang membuat setiap gambar memiliki resolusi paling tinggi 150 piksel per inci, sehingga berkas lebih kecil. Gunakan Ukuran minimum untuk PDF yang akan Anda pasang di halaman web atau kirim lewat email.
- **Sertakan properti dokumen** memasukkan judul, pengarang, subjek, dan kata kunci dokumen ke dalam PDF.
- **Buat penanda dari judul** menjadikan setiap judul sebagai **penanda**, yaitu entri dalam daftar yang ditampilkan pembaca PDF di samping halaman. Klik penanda untuk melompat ke halamannya.
- **Tag struktur dokumen untuk aksesibilitas** memberi tag pada judul, paragraf, daftar, tabel (beserta baris header-nya), dan gambar (beserta teks alternatifnya), serta menyebutkan bahasanya, sehingga pembaca layar membacakan PDF secara berurutan.

Kemudian pilih nama. PDF tampil sama di setiap komputer karena font-nya disematkan di dalam berkas. PDF yang dibuat **Cetak** saat Anda memilih berkas PDF juga memiliki penanda dan tag. Jika beberapa halaman ditempatkan pada satu lembar, setiap penanda dan setiap tautan mengarah ke lembar yang memuat halamannya, dan halaman pada satu lembar dibaca sesuai urutan letaknya: dari kiri ke kanan, lalu dari atas ke bawah.

## Menyimpan sebagai halaman web

Klik **Berkas > Ekspor** dan pilih salah satu dari dua jenis halaman web. **Halaman web** adalah berkas yang dapat dibuka di peramban web apa pun.

- **Simpan sebagai Halaman Web** membuat satu berkas .html yang memuat kata-kata, tampilannya, dan gambar Anda. Anda dapat mengirim berkas tunggal itu saja. Tampilan yang dipertahankan adalah font, ukuran, warna, sorotan, huruf tebal, huruf miring, garis bawah, perataan, dan jarak di sekitar paragraf.
- **Simpan sebagai Halaman Web Tersaring** membuat berkas .html kecil yang hanya berisi struktur: judul, huruf tebal dan miring, daftar, tabel, tautan, dan gambar. Berkas ini tidak memiliki font, ukuran, atau warna, sehingga Anda dapat memberinya tampilan sendiri nanti.

Keduanya mempertahankan judul, daftar berpoin dan bernomor, tabel dengan sel yang digabungkan, tautan, dan gambar. Teks tersembunyi, kata-kata yang dihapus oleh perubahan yang dilacak, komentar, header, footer, dan catatan tidak disertakan. Tautan yang akan menjalankan program atau skrip disimpan sebagai teks biasa.

## Menyimpan sebagai e-book

**E-book** adalah buku yang Anda baca di pembaca e-book, ponsel, atau tablet. Berkasnya berformat **EPUB**. Kata-katanya menyesuaikan diri dengan layar apa pun, sehingga pembaca dapat memperbesar atau memperkecil huruf.

1. Klik **Berkas > Ekspor > Simpan sebagai E-book (EPUB)**.
2. Periksa **Judul** dan **Pengarang**. Keduanya awalnya diambil dari judul dan pengarang di properti dokumen Anda (**Berkas > Info**). Anda dapat mengubahnya khusus untuk buku ini.
3. Aktifkan **Gunakan gambar pertama sebagai sampul** jika Anda ingin gambar pertama dalam dokumen menjadi sampul. Tanpa opsi ini, buku tidak memiliki sampul.
4. Klik **OK**, lalu pilih lokasi penyimpanan berkas.

Cara buku dibuat:

- Setiap **Judul 1** memulai bab baru. Kata-kata sebelum Judul 1 pertama menjadi bab tersendiri.
- Daftar **isi** menampilkan setiap Judul 1, Judul 2, dan Judul 3, dan pembaca dapat melompat ke masing-masingnya. Judul yang lebih kecil ada di dalam buku tetapi tidak tercantum di daftar isi.
- Gambar dan tabel tetap berada di tempat yang Anda tentukan. Teks alternatif gambar ikut dibawa, sehingga pembaca layar dapat menyebutkan apa yang ditampilkan gambar tersebut.
- Catatan kaki dan catatan akhir ditempatkan di akhir bab tempat catatan itu berada. Setiap catatan memiliki tautan kembali ke posisinya di teks, dan setiap nomor di teks merupakan tautan ke catatannya.

Buku dibatasi agar berkas yang rusak tidak membuat berkas yang sangat besar: hingga 500 bab, 4.000 baris di daftar isi, 1.000 gambar (tidak ada yang lebih dari 32 MB), dan 20.000 catatan. Apa pun yang melebihi batas tetap berada di bab terakhir, atau menjadi teks biasa, dan kata-kata Anda tidak pernah hilang: gambar yang melebihi batas diganti dengan teks alternatifnya, dan catatan yang melebihi batas tidak memiliki tampilan. Gambar yang tidak dapat ditampilkan pembaca (metafile, bitmap, atau TIFF) diubah menjadi PNG. Mengekspor dokumen tersimpan yang sama lagi akan menghasilkan buku yang sama, sehingga pembaca tetap pada posisinya.

## Melindungi PDF dengan kata sandi

Di kotak **Ekspor ke PDF**, klik **Enkripsi dengan kata sandi…**. Ketik kata sandi dua kali. Siapa pun yang membuka PDF harus mengetikkan kata sandi tersebut. Kata sandi benar-benar melindungi berkas: halaman diacak dengan enkripsi kuat (AES-256), dan tidak seorang pun dapat membacanya tanpa kata sandi.

Anda juga dapat mencentang **Batasi pencetakan, penyalinan, dan pengeditan**, memberikan kata sandi kedua, dan memilih apa saja yang diizinkan. Pembatasan ini hanya berupa permintaan. Sebagian besar pembaca PDF mematuhinya, tetapi pembatasan ini tidak menghentikan orang yang ingin mengakalinya, jadi jangan mengandalkannya untuk menjaga kerahasiaan berkas.

Sandi yang terlupa tidak dapat dipulihkan, jadi simpan di tempat yang aman. Kata sandi hanya digunakan untuk PDF tersebut dan tidak pernah disimpan. Kotak ini menonaktifkan **PDF/A** selama kata sandi diatur, karena berkas PDF/A tidak dapat memiliki kata sandi.

## Membuat PDF/A

Di kotak **Ekspor ke PDF**, centang **PDF/A, untuk disimpan dalam jangka panjang**. Berkas PDF/A (PDF/A-1b) memuat setiap font yang digunakan, memberi nama pada warnanya, dan mengikuti standar yang dibuat untuk menyimpan berkas selama bertahun-tahun. Berkas ini dapat diberi tag untuk pembaca layar, tetapi tidak dapat diberi kata sandi.

## Membuat email

Klik **Berkas > Ekspor > Buat Email** untuk membuat berkas pesan (.eml). Tidak ada yang dikirim. Anda memilih isi pesan: dokumen, dokumen sebagai PDF, atau halaman yang ditampilkan sebagai gambar dalam pesan (30 halaman pertama). Buka berkas di program email Anda, tambahkan penerimanya, lalu kirim. Untuk PDF, Anda dapat mengklik **Enkripsi dengan kata sandi…** di kotak; sampaikan kata sandinya kepada penerima dengan cara lain, bukan di dalam pesan.

## Terkait

- [Selamat datang di JeffWrite](start)
- [Cetak](print)
- [Menggunakan JeffWrite dengan papan ketik](keyboard)
