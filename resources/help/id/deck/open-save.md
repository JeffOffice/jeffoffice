# Membuka dan menyimpan presentasi
<!-- keywords: makro, bermakro, pptm, potm, ppsm, buka, simpan, simpan sebagai, otp, libreoffice, tidak disertakan, tutup, baru, berkas terbaru, disematkan, jenis berkas, pptx, potx, ppsx, ppt, odp, templat, tayangan, ganti nama, Ctrl+S, cadangan, format berkas, berkas lama, dokumen terbuka, disimpan, tidak ditampilkan, font, sematkan font, font tersemat, font yang hilang, simpan salinan, hanya baca, read only, buka sebagai salinan, jendela baru, salinan cadangan, templat saya, simpan sebagai templat, perbaiki, buka dan perbaiki, rusak -->

JeffDeck menyimpan pekerjaan Anda dalam berkas .pptx dan .odp, serta dapat membuka beberapa jenis berkas presentasi.

## Membuat presentasi baru

Tekan Ctrl+N. Jendela baru akan terbuka dengan presentasi kosong. Atau klik **Berkas > Baru**, lalu **Presentasi Kosong**. Jika jendela yang Anda gunakan berisi presentasi baru yang belum diubah, presentasi kosong itu akan menggantikannya. Jika tidak, presentasi akan terbuka di jendela tersendiri.

## Menyimpan

Tekan Ctrl+S, atau klik **Berkas > Simpan**. Pertama kali, JeffDeck meminta nama dan lokasi. Setelah itu, Simpan akan menggantikan berkas dengan pekerjaan terbaru Anda. Pesan di bagian bawah menampilkan "Tersimpan" dan nama berkas.

Tanda bintang (*) setelah nama di bilah judul berarti ada perubahan yang belum disimpan. Simpanlah sering-sering. Jika JeffDeck berhenti secara tiba-tiba, lihat [Pulihkan pekerjaan yang belum disimpan](recover).

Jika muncul pesan bahwa beberapa objek "belum dapat disimpan dalam jenis berkas ini", berkas tetap disimpan, tetapi JeffDeck tidak memiliki cara untuk menulis objek tersebut. Simpan juga berkas asli. Saat Anda menyimpan berkas .odp, pesan akan mencantumkan hal-hal yang tidak dapat dimuat oleh jenis berkas tersebut. Bagian "Menyimpan sebagai berkas .odp" di bawah menjelaskan apa saja yang tidak dapat dimuat.

## Menyimpan dengan nama atau jenis baru

Klik **Berkas > Simpan Sebagai**, atau tekan F12. Ketik nama, pilih jenisnya, lalu klik **Simpan**. Pilihannya adalah:

- **Presentasi (*.pptx)** adalah pilihan yang biasa.
- **Templat (*.potx)** menyimpan slaid sebagai pola untuk memulai presentasi baru.
- **Tayangan (*.ppsx)** menyimpan presentasi untuk diputar.
- **Presentasi Mendukung Makro (*.pptm)**, **Templat Mendukung Makro (*.potm)**, dan **Tayangan Mendukung Makro (*.ppsm)** adalah tiga jenis yang sama untuk presentasi yang berisi **makro**. Makro adalah program kecil yang menyertai sebuah berkas. JeffDeck tidak pernah menjalankan makro, dan tidak pernah membaca atau menampilkannya. Jika Anda membuka salah satu berkas ini dan menyimpannya dengan jenis yang sama, makro akan tetap dipertahankan seperti semula. Jika Anda menyimpannya sebagai .pptx, .potx, atau .ppsx, makro tidak akan disertakan, dan JeffDeck akan memberi tahu Anda. Presentasi tanpa makro juga dapat disimpan dengan jenis-jenis ini, tetapi tidak ada makro yang perlu dipertahankan. Jika makro dalam sebuah berkas terlalu besar untuk dipertahankan (lebih dari 32 MB dalam satu bagian, atau 64 MB seluruhnya), JeffDeck tidak akan menyertakannya dan memberi tahu Anda saat berkas dibuka.
- **Presentasi OpenDocument (*.odp)** menyimpan slaid dalam jenis berkas yang digunakan LibreOffice dan program gratis lainnya.
- **Templat Presentasi OpenDocument (*.otp)** adalah bentuk templat dari jenis tersebut.

Jika Anda tidak menuliskan akhiran, JeffDeck akan menambahkan akhiran yang sesuai dengan jenisnya. Setelah menyimpan, bilah judul menampilkan nama baru, dan Anda tetap bekerja di berkas baru tersebut.

Presentasi yang Anda buka dari templat disimpan sebagai presentasi baru: **Simpan** akan meminta nama, sehingga templat tetap seperti semula. Hal yang sama berlaku untuk presentasi yang dibuka dari berkas .ppt, .pps, atau .pot lama. Presentasi yang dibuka dari berkas .odp akan disimpan kembali sebagai berkas .odp, menimpa berkas yang sama.

## Menyimpan salinan

Klik **Berkas > Simpan Salinan** dan pilih nama. JeffDeck menulis presentasi ke berkas tersebut, dan Anda tetap bekerja di berkas yang tadi. Nama dan perubahan yang belum disimpan tetap seperti sebelumnya.

## Menyimpan salinan cadangan

Klik **Berkas > Opsi**, lalu pada tab **Umum**, aktifkan **Selalu buat salinan cadangan**. Setiap kali Anda menyimpan, JeffDeck terlebih dahulu menyalin berkas lama ke berkas bernama "Cadangan dari" ditambah namanya, di folder yang sama. Jika salinan tidak dapat dibuat, aplikasi akan menanyakan apakah berkas tetap disimpan tanpa cadangan. Jawabannya adalah Tidak, kecuali Anda memilih Ya, dan berkas tetap seperti semula.

## Menyimpan sebagai templat

Templat adalah presentasi yang Anda gunakan sebagai titik awal. Klik **Berkas > Simpan sebagai Templat**, lalu ketik nama. Templat (berkas **.potx**) disimpan di **Templat Saya**, dan presentasi Anda tidak berubah. Untuk memulai presentasi baru dari templat tersebut, klik **Berkas > Baru**, lalu klik templat di bawah **Templat Saya**. Presentasi baru belum memiliki berkas sendiri, sehingga templat tidak berubah. Untuk menghapus templat dari daftar, klik kanan templat tersebut dan pilih **Hapus dari Templat Saya**.

## Membuka

Tekan Ctrl+O untuk langsung memilih berkas. Atau klik **Berkas > Buka**, lalu klik **Telusuri**, atau nama di bawah **Disematkan** atau **Presentasi Terbaru**.

Berkas akan terbuka di jendela ini jika jendela berisi presentasi baru yang belum diubah. Jika tidak, berkas akan terbuka di jendela tersendiri.

Klik kanan nama dalam daftar untuk **Sematkan ke daftar** (nama akan tetap berada di bagian atas), **Lepas dari daftar**, atau **Hapus dari daftar**. Nama yang berwarna abu-abu berarti JeffDeck tidak dapat menemukan berkas tersebut saat ini.

## Cara lain untuk membuka berkas

Pada halaman **Buka**, klik **Cara Lain untuk Membuka**. Anda juga dapat mengklik kanan nama dalam daftar Terbaru.

- **Buka Hanya-Baca** membuka presentasi hanya untuk dibaca. Anda dapat melihat, berpindah antarslaid, dan menyalin, tetapi tidak dapat mengubah apa pun. Bilah judul menampilkan "Hanya Baca". Jika Anda menekan **Simpan**, JeffDeck akan meminta nama baru, sehingga berkas yang Anda buka tidak berubah.
- **Buka sebagai Salinan** membuka slaid sebagai presentasi baru tanpa berkas sendiri. **Simpan** akan menanyakan lokasi penyimpanannya.
- **Buka di Jendela Baru** membuka berkas di jendela tersendiri, meskipun jendela yang sedang Anda gunakan kosong.
- **Buka dan Perbaiki** digunakan untuk berkas .pptx yang rusak, misalnya berkas yang terpotong saat disalin. JeffDeck membaca setiap bagian yang dapat dibaca, mempertahankan bagian yang terhenti di tengah sampai titik ia berhenti, dan membuang bagian yang tidak dapat dibaca. Slaid yang sudah diperbaiki akan dibuka sebagai salinan bernama "Diperbaiki" ditambah nama berkas, dan pesan akan memberi tahu apa saja yang dibuang. Berkas yang rusak tidak diubah.

## Berkas dengan kata sandi

Berkas **.pptx** atau **.odp** yang meminta kata sandi akan dibuka seperti biasa: JeffDeck meminta kata sandi, dan Anda dapat mencoba lagi jika kata sandinya salah, atau mengklik **Batal**. Berkas tetap dilindungi saat Anda menyimpannya. Untuk mengubah atau menghapus kata sandi, gunakan **Berkas > Info > Enkripsi dengan Kata Sandi** (lihat Layar Berkas). Salinan yang disimpan sebagai berkas .odp juga mempertahankan kata sandinya.

## Font dalam berkas

Font adalah bentuk huruf. Tidak setiap komputer memiliki semua font. Jika komputer tidak memiliki font yang digunakan slaid Anda, komputer akan menggambar huruf dengan font lain. Akibatnya, slaid dapat tampak berbeda.

JeffDeck dapat menyimpan font yang digunakan slaid Anda di dalam berkas. Dengan begitu, slaid tampak sama di komputer mana pun. Cara ini disebut penyematan. JeffDeck melakukannya setiap kali Anda menyimpan. Untuk menonaktifkannya, klik **Berkas > Opsi**, buka tab **Simpan**, lalu hapus centang **Sematkan font dalam berkas**.

- JeffDeck hanya menyematkan font yang disertakan bersama program, seperti Fraunces, Lato, dan Open Sans. Komputer lain tidak memiliki font tersebut. JeffDeck tidak pernah menyematkan font yang dimiliki hampir setiap komputer, seperti Calibri atau Arial. Font yang lisensinya melarang penyalinan ke dalam berkas akan dilewati.
- JeffDeck menyematkan setiap gaya yang digunakan slaid: reguler, tebal, miring, dan tebal miring. Jika slaid hanya menggunakan gaya reguler, hanya gaya itu yang disematkan.
- JeffDeck menyematkan setiap huruf dari setiap font, tidak hanya huruf yang Anda gunakan. Dengan begitu, orang yang membuka berkas tetap dapat mengubah kata-katanya.
- Berkas akan menjadi lebih besar. Setiap gaya font menambah sekitar 40 hingga 700 KB (satu KB adalah seribu byte). Berkas dimampatkan saat disimpan, sehingga ukurannya bertambah sekitar setengah dari jumlah itu.

Saat Anda membuka berkas yang berisi font tersemat, JeffDeck menggunakan font tersebut selama presentasi terbuka. Slaid tampak seperti saat dibuat. JeffDeck tidak memasang font di komputer Anda dan tidak menyalinnya ke tempat lain. Font itu hanya milik presentasi tersebut: tidak muncul di daftar font Anda, dan presentasi lain tidak menggunakannya. Font akan hilang saat presentasi ditutup. Jika komputer Anda sudah memiliki font tersebut, JeffDeck menggunakan salinan milik Anda sendiri.

JeffDeck membaca font dari berkas .pptx yang dibuat program presentasi lain, bahkan jika program tersebut memampatkan font itu. JeffDeck juga membaca font dari berkas .odp.

Sebuah berkas dapat memuat font yang tidak dimiliki JeffDeck. Saat Anda menyimpan, JeffDeck akan memasukkan kembali font tersebut ke dalam berkas, selama slaid masih menggunakannya. Hal ini berlaku baik opsi **Sematkan font dalam berkas** aktif maupun tidak. Jika sebuah font dalam berkas rusak atau tidak dapat dibaca, pesan yang muncul saat Anda membuka berkas akan menyebutkannya.

## Jenis berkas yang dibaca JeffDeck

- **.pptx, .potx, dan .ppsx** adalah satu format dalam tiga bentuk: presentasi, templat, dan tayangan. JeffDeck membaca dan menulis ketiganya.
- **.ppt, .pps, dan .pot** adalah jenis lama. JeffDeck membacanya, tetapi tidak dapat menulisnya, sehingga **Simpan** akan meminta nama baru dan menyimpan sebagai .pptx (atau .ppsx untuk .pps). Berkas asli tidak pernah ditimpa.
- **.odp dan .otp** adalah presentasi dokumen terbuka beserta templatnya. JeffDeck membaca dan menulis keduanya. Berkas .odp yang dibuka di sini akan disimpan sebagai .odp. Berkas .otp adalah templat, sehingga **Simpan** akan meminta nama baru dan menyarankan .odp.

Jika sebagian berkas tidak dapat dibaca, JeffDeck tetap membuka bagian lainnya dan mencantumkan apa yang dilewati. Jika seluruh berkas tidak dapat dibaca, JeffDeck memberi tahu alasannya.

## Menyimpan sebagai berkas .odp

Berkas .odp dapat dibuka di LibreOffice Impress dengan slaid, master, gambar, catatan, dan komentar Anda, serta tampilan yang ditampilkan JeffDeck. Berkas ini menyimpan animasi, transisi slaid, presentasi khusus, dan pengaturan tayangan. Font setiap kata disimpan berdasarkan nama, sehingga komputer yang tidak memiliki font tersebut akan menampilkan font lain.

JeffDeck sendiri belum membaca kembali animasi, komentar, presentasi khusus, atau pengaturan tayangan dari berkas .odp. Semuanya tersimpan dalam berkas dan dapat dijalankan oleh LibreOffice, tetapi saat Anda membuka berkas .odp di JeffDeck, hal-hal tersebut akan tercantum sebagai tidak dimuat. Simpan berkas .pptx Anda jika Anda ingin terus mengeditnya di sini.

Berkas .odp tidak memiliki tempat untuk beberapa hal. Setelah penyimpanan, JeffDeck mencantumkannya dalam pesan di bagian bawah jendela:

- **Bagan, diagram, dan persamaan** disimpan sebagai gambar. Anda dapat melihatnya, tetapi tidak dapat mengedit angka atau bagiannya dalam berkas .odp.
- **Bagian** tidak disimpan.
- **Bevel dan bayangan dalam** tidak dimuat, begitu pula pendaran dan pantulan pada teks. Bayangan pada teks tetap dipertahankan. Pantulan pada bentuk, garis dengan gradasi, garis dengan dua atau tiga goresan, dan ukuran bayangan tetap dipertahankan dengan cara yang hanya dibaca JeffDeck. Program lain akan menampilkan bentuk tanpa pantulan, garis dengan warna pertamanya, satu goresan, dan bayangan sebesar objek.
- **Efek animasi** yang dimulai saat Anda mengklik objek, serta efek yang mengubah warna, tidak dimuat. Klik pada film untuk menjeda atau memutarnya kembali tetap dipertahankan.
- **Film atau suara tertaut** dari berkas lain, bukan yang tersimpan di dalam presentasi, tetap menyimpan alamat dan gambarnya dalam berkas .odp. LibreOffice akan memutarnya jika dapat menemukan berkas tersebut. Film dan suara yang berada di dalam presentasi tersimpan dalam berkas .odp dan dapat diputar.

Simpan juga berkas .pptx Anda jika Anda masih membutuhkan hal-hal tersebut.

## Yang disimpan tetapi tidak ditampilkan

Beberapa bagian berkas adalah hal yang belum dapat ditampilkan atau diubah oleh JeffDeck. JeffDeck menyimpannya bersama slaidnya dan menuliskannya kembali saat Anda menyimpan, sehingga tidak hilang. Ini mencakup animasi, yang diputar dalam tayangan, film dan suara, yang belum dapat diputar, serta efek transisi yang tidak memiliki bentuk di JeffDeck, yang diputar sebagai pudar. Bagan dan diagram ditampilkan seperti saat disimpan. Lihat [Selamat datang di JeffDeck](start).

## Terkait

- [Pulihkan pekerjaan yang belum disimpan](recover)
- [Layar Berkas](file-screen)
- [Gunakan ulang slaid dari presentasi lain](reuse)
- [Cetak, PDF, dan gambar slaid](printing)
