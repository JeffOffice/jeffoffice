# Buka, simpan, cetak, dan ekspor

<!-- keywords: halaman web, html, simpan sebagai halaman web, halaman web tersaring, email, eml, penanda, penanda pdf, shift+f5, pengeditan terakhir, tempat terakhir berhenti, templat bawaan, galeri, surat, laporan, ringkasan, naskah, buletin, buka, simpan, simpan sebagai, docx, doc, dot, odt, opendocument, rtf, berkas lama, templat, dotx, terbaru, sematkan, baru, tutup, cetak, pdf, ekspor, pulihkan, pemulihan otomatis, belum disimpan, macet, layar berkas, format berkas, simpan salinan, hanya baca, read only, buka sebagai salinan, jendela baru, salinan cadangan, templat saya, simpan sebagai templat, ott, perbaiki, buka dan perbaiki, rusak, kata sandi, enkripsi, lindungi dengan kata sandi -->

Klik **Berkas** di pojok kiri atas untuk melihat layar Berkas. Layar ini memiliki halaman Baru, Buka, Cetak, dan Ekspor, serta perintah Simpan dan Simpan Sebagai. Tekan **Esc** untuk kembali ke dokumen Anda.

## Memulai dokumen baru

Tekan **Ctrl+N**, atau klik **Berkas > Baru** lalu **Dokumen Kosong**. Dokumen baru akan terbuka di jendelanya sendiri. Jika jendela yang Anda gunakan kosong dan belum pernah disimpan, dokumen baru akan menggantikannya.

## Membuka dokumen

Tekan **Ctrl+O**, atau klik **Berkas > Buka**. Klik **Telusuri** untuk memilih berkas. JeffWrite dapat membuka jenis berkas berikut:

- **.docx** adalah dokumen.
- **.dotx** adalah templat. Templat akan terbuka sebagai dokumen baru yang dibuat darinya, sehingga templat itu sendiri tetap utuh.
- **.docm** dan **.dotm** adalah dokumen dan templat yang dapat memuat **makro**, yaitu program kecil yang menyertai sebuah berkas. JeffWrite tidak pernah menjalankannya. Makro dipertahankan seperti semula saat Anda menyimpan sebagai .docm atau .dotm. Jika Anda menyimpan sebagai .docx atau .odt, makro tidak disertakan, dan JeffWrite akan memberi tahu Anda.
- **.txt** adalah teks polos, hanya berisi kata-kata. Saat dibuka, JeffWrite langsung membacanya jika berupa ASCII polos atau Unicode. Jika tidak, JeffWrite akan menanyakan **pengodean** mana yang digunakan untuk membacanya, lalu menampilkan beberapa baris pertama agar Anda dapat memastikan huruf-hurufnya tampil benar. Pengodean adalah cara sebuah berkas menyimpan huruf, misalnya UTF-8 atau Windows 1252. Saat Anda menyimpan sebagai .txt, Anda memilih pengodean dan akhir baris, dan berkas hanya menyimpan kata-katanya: tanpa font, gambar, atau garis tabel.
- **.html** dan **.htm** adalah halaman web. JeffWrite membaca kata-kata, judul, huruf tebal, huruf miring, garis bawah, warna, daftar, dan tabel. Butir dan nomor pada daftar masuk sebagai tanda yang diketik, sedangkan gambar dan tautan dilewati. JeffWrite tidak pernah mengambil apa pun dari internet dan mengabaikan skrip. Halaman web tidak pernah ditimpa: **Simpan** akan meminta nama baru dan menyimpan berkas .docx. Untuk membuat halaman web, lihat **Simpan sebagai halaman web** di bawah.
- **.doc** dan **.dot** adalah jenis dokumen dan templat yang lebih lama. JeffWrite dapat membacanya, dan menyimpan hasil pekerjaan Anda sebagai berkas .docx baru.
- **.odt**, **.ott**, dan **.fodt** adalah berkas OpenDocument. JeffWrite dapat membaca ketiganya. JeffWrite menyimpan berkas .odt, dan menyimpan hasil pekerjaan Anda pada berkas .ott atau .fodt sebagai berkas .odt baru. Lihat [Membuka dan menyimpan berkas OpenDocument](open-odt).
- **.rtf** adalah Rich Text Format. JeffWrite dapat membacanya dan menyimpannya. Lihat [Membuka berkas RTF](open-rtf).

Halaman **Buka** juga mencantumkan dokumen yang baru saja Anda buka atau simpan. Klik salah satunya untuk membukanya. Klik kanan pada salah satunya untuk menyematkannya di bagian atas daftar, atau menghapusnya dari daftar.

Jika sebuah berkas memiliki bagian yang belum dapat diedit JeffWrite, seperti tata letak tabel atau header, bagian tersebut tetap dipertahankan. Lihat [Selamat datang di JeffWrite](start). Jika sebagian berkas tidak dapat dibaca sama sekali, JeffWrite akan memberi tahu Anda apa saja yang tidak disertakan. JeffWrite kemudian tidak akan menimpa berkas tersebut: **Simpan** akan meminta nama baru, sehingga berkas asli tetap menyimpan bagian yang hilang pada salinan.

## Menyimpan dokumen

Tekan **Ctrl+S** untuk menyimpan. Pada penyimpanan pertama, JeffWrite meminta nama berkas. **Simpan Sebagai** (F12) menyimpan salinan dengan nama baru. Pilih **Templat (*.dotx)** untuk menyimpan templat, **Dokumen Berpenyimpanan Makro (*.docm)** untuk mempertahankan makro, **Teks Polos (*.txt)** untuk hanya menyimpan kata-kata, atau **Rich Text Format (*.rtf)** untuk berkas yang dapat dibaca program lain. Berkas yang berasal dari jenis berkas lama selalu disimpan dengan nama baru, sehingga berkas lama tidak pernah tertimpa.

## Menyimpan salinan

Klik **Berkas > Simpan Salinan**. Pilih nama. JeffWrite menulis dokumen ke berkas tersebut, dan Anda tetap dapat bekerja di dokumen yang sedang dibuka. Nama dan perubahan yang belum disimpan pada dokumen tetap seperti semula. Ini berguna jika Anda ingin mengirim satu versi kepada seseorang dan tetap melanjutkan penulisan.

## Menyimpan salinan cadangan

Klik **Berkas > Opsi**, lalu pada halaman **Umum** aktifkan **Selalu buat salinan cadangan**. Setiap kali Anda menyimpan, JeffWrite lebih dulu menyalin berkas lama ke berkas bernama "Cadangan dari" diikuti namanya, di folder yang sama. Jika salinan tidak dapat dibuat, JeffWrite akan menanyakan apakah Anda ingin menyimpan tanpa salinan. Jawabannya adalah Tidak, kecuali Anda memilih Ya, dan berkas tetap seperti semula. Dengan begitu, Anda dapat kembali ke versi terakhir yang disimpan.

## Menyimpan sebagai templat

Templat adalah dokumen yang Anda gunakan sebagai titik awal. Klik **Berkas > Simpan sebagai Templat**, lalu ketik nama. Templat akan disimpan di **Templat Saya**, dan dokumen yang sedang Anda tulis tidak berubah. Di **Simpan Sebagai**, Anda juga dapat memilih **Templat (*.dotx)**, atau **Templat OpenDocument Text (*.ott)**, untuk menyimpan berkas templat di lokasi yang Anda inginkan.

Untuk memulai dokumen baru dari templat Anda, klik **Berkas > Baru**. Templat Anda tercantum di bawah **Templat Saya**. Klik salah satunya. Dokumen baru akan terbuka dengan teks dan gaya dari templat tersebut. Untuk menghapus templat dari daftar, klik kanan padanya dan pilih **Hapus dari Templat Saya**.

## Cara lain membuka berkas

Pada halaman **Buka**, klik **Lebih Banyak Cara Membuka**. Anda juga dapat mengklik kanan nama di daftar Terbaru. Setiap cara meminta berkas, atau menggunakan berkas yang Anda klik:

- **Buka Hanya-Baca** membuka berkas hanya untuk dibaca. Anda dapat melihat, menggulir, mencari, dan menyalin, tetapi tidak dapat mengubah apa pun. Bilah judul bertuliskan "Hanya Baca". Jika Anda menekan **Simpan**, JeffWrite meminta nama baru, sehingga berkas yang Anda buka tidak berubah.
- **Buka sebagai Salinan** membuka dokumen baru dengan teks dari berkas tersebut. Dokumen ini tidak memiliki berkas sendiri, sehingga **Simpan** akan menanyakan tempat penyimpanannya. Berkas yang Anda buka tidak pernah diubah.
- **Buka di Jendela Baru** membuka berkas di jendelanya sendiri, meskipun jendela yang Anda gunakan kosong.
- **Buka dan Perbaiki** digunakan untuk berkas .docx yang rusak, misalnya berkas yang terpotong saat disalin. Lihat di bawah.

## Buka dan Perbaiki

Kadang-kadang berkas .docx menjadi rusak. Penyalinan berhenti di tengah jalan, atau disk mengalami kesalahan. Berkas .docx adalah kumpulan bagian yang lebih kecil, seperti teks, gaya, dan gambar. **Buka dan Perbaiki** membaca setiap bagian yang dapat dibaca. Bagian yang terhenti di tengah dipertahankan sampai titik ia terhenti. Bagian yang tidak dapat dibaca akan dilewati. Jika daftar bagian atau tautan di antaranya hilang, JeffWrite akan membuatnya kembali.

Dokumen yang telah diperbaiki akan terbuka sebagai salinan dengan nama seperti "Surat yang diperbaiki". Sebuah pesan akan memberi tahu Anda apa yang dipertahankan sampai titik putus dan apa yang dilewati. Berkas yang rusak tidak diubah. Simpan salinan yang telah diperbaiki dengan nama tersendiri, lalu periksa kembali sebelum Anda mempercayainya.

## Mendapatkan kembali pekerjaan yang belum disimpan

JeffWrite menyimpan salinan cadangan pekerjaan yang belum disimpan setiap beberapa menit, sebagai berkas di foldernya sendiri. Jika program tertutup secara tidak terduga, JeffWrite akan menawarkan salinan tersebut saat Anda menjalankannya lagi. Klik **Berkas > Buka > Pulihkan Pekerjaan yang Belum Disimpan** untuk melihatnya kemudian. Anda dapat memilih mana yang akan dibuka, dan menyimpan masing-masing di lokasi yang Anda inginkan.

## Memulai dari templat

**Berkas > Baru** menampilkan **Dokumen Kosong** dan sejumlah templat yang disertakan bersama JeffWrite, dikelompokkan menurut jenisnya: surat, laporan, ringkasan, naskah, dan buletin. Setiap gambar adalah halaman pertama dari dokumen yang akan Anda dapatkan. **Templat** adalah dokumen yang sudah disiapkan, lengkap dengan gaya, margin, dan teks contoh. Klik salah satunya, lalu ganti teks contoh dengan teks Anda sendiri. Templat yang Anda simpan sendiri tercantum di bawah **Templat Saya**.

## Melanjutkan dari tempat terakhir berhenti

JeffWrite mengingat posisi kursor saat Anda menyimpan atau menutup berkas. Saat Anda membuka berkas lagi, kursor akan berada di posisi tersebut. Tekan **Shift+F5** untuk melompat ke tempat-tempat yang terakhir Anda edit. Setiap kali Anda menekannya, lompatan berpindah ke tempat sebelumnya: tiga tempat terakhir, lalu kembali ke yang pertama.

## Menyimpan sebagai .odt

Berkas .odt tidak dapat memuat semua yang dapat dimuat berkas .docx. Saat Anda menyimpan sebagai .odt, JeffWrite terlebih dahulu mencantumkan apa saja yang akan hilang, dan memberi Anda pilihan untuk menyimpan sebagai berkas .docx. Lihat [Info](info).

## Cetak

Tekan **Ctrl+P**, atau klik **Berkas > Cetak**. Pilih printer, jumlah salinan, dan halaman, periksa setiap halaman di pratinjau, lalu klik **Cetak**. Lihat [Cetak](print).

## Membuat PDF

Klik **Berkas > Ekspor > Ekspor ke PDF**. Pilih halaman yang akan disertakan: semua, halaman saat ini, atau rentang seperti 1-3, 5 (p2s3 dan s3 juga berlaku di sini). Lalu pilih nama. PDF akan tampil sama di setiap komputer, karena font disematkan di dalamnya. Judul, pengarang, subjek, dan kata kunci dokumen akan dimasukkan ke dalam PDF. Setiap judul menjadi **penanda**, yaitu entri dalam daftar yang ditampilkan pembaca PDF di samping halaman. Klik penanda untuk melompat ke halamannya.

## Menyimpan sebagai halaman web

Klik **Berkas > Ekspor** dan pilih salah satu dari dua jenis halaman web. **Halaman web** adalah berkas yang dapat Anda buka di peramban web mana pun.

- **Simpan sebagai Halaman Web** membuat satu berkas .html yang memuat kata-kata, tampilannya, dan gambar Anda. Anda dapat mengirim satu berkas itu saja. Tampilan yang dipertahankan meliputi font, ukuran, warna, sorotan, huruf tebal, huruf miring, garis bawah, perataan, dan jarak di sekitar paragraf.
- **Simpan sebagai Halaman Web Tersaring** membuat berkas .html kecil yang hanya memuat strukturnya: judul, huruf tebal dan miring, daftar, tabel, tautan, dan gambar. Berkas ini tidak memiliki font, ukuran, atau warna, sehingga Anda dapat memberinya tampilan sendiri nanti.

Keduanya mempertahankan judul, daftar berpoin dan bernomor, tabel dengan sel yang digabungkan, tautan, dan gambar. Teks tersembunyi, kata-kata yang dihapus dalam perubahan yang dilacak, komentar, header, footer, dan catatan dilewati. Tautan yang akan menjalankan program atau skrip akan disimpan sebagai teks biasa.

## Melindungi PDF dengan kata sandi

Di kotak **Ekspor ke PDF**, klik **Enkripsi dengan kata sandi…**. Ketik kata sandi dua kali. Siapa pun yang membuka PDF harus mengetikkannya. Kata sandi benar-benar melindungi berkas: halaman diacak dengan enkripsi kuat (AES-256), dan tidak ada yang dapat membacanya tanpa kata sandi tersebut.

Anda juga dapat mencentang **Batasi pencetakan, penyalinan, dan pengeditan**, memberi kata sandi kedua, dan memilih apa yang diizinkan. Batasan tersebut hanya berupa permintaan. Sebagian besar pembaca PDF menghormatinya, tetapi batasan ini tidak menghentikan orang yang ingin melewatinya, jadi jangan mengandalkannya untuk menjaga kerahasiaan berkas.

Kata sandi yang terlupakan tidak dapat dipulihkan, jadi simpan di tempat yang aman. Kata sandi hanya digunakan untuk PDF tersebut dan tidak pernah disimpan. Kotak ini menonaktifkan **PDF/A** selama kata sandi diatur, karena berkas PDF/A tidak dapat memiliki kata sandi.

## Membuat PDF/A

Di kotak **Ekspor ke PDF**, centang **PDF/A, untuk disimpan dalam jangka panjang**. Berkas PDF/A memuat setiap font yang digunakannya dan mengikuti standar yang dibuat untuk menyimpan berkas selama bertahun-tahun. Berkas ini belum diberi tag untuk pembaca layar, dan tidak dapat memiliki kata sandi.

## Membuat email

Klik **Berkas > Ekspor > Buat Email** untuk membuat berkas pesan (.eml). Tidak ada yang dikirim. Anda memilih apa yang dimasukkan ke dalam pesan: dokumen, dokumen sebagai PDF, atau halaman yang ditampilkan sebagai gambar dalam pesan (30 halaman pertama). Buka berkas tersebut di program email Anda, tambahkan tujuannya, lalu kirim. Untuk PDF, Anda dapat mengklik **Enkripsi dengan kata sandi…** di kotak; sampaikan kata sandinya kepada penerima melalui cara lain, bukan di dalam pesan.

## Terkait

- [Selamat datang di JeffWrite](start)
- [Cetak](print)
- [Menggunakan JeffWrite dengan papan ketik](keyboard)
