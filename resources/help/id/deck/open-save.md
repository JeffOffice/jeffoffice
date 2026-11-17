# Membuka dan menyimpan presentasi

<!-- keywords: makro, bermakro, pptm, potm, ppsm, buka, simpan, simpan sebagai, otp, libreoffice, dihilangkan, tutup, baru, berkas terbaru, disematkan, jenis berkas, pptx, potx, ppsx, ppt, odp, templat, tampilkan, ganti nama, Ctrl+S, cadangan, format berkas, berkas lama, dokumen terbuka, disimpan, tidak ditampilkan, font, sematkan font, font tersemat, font hilang, simpan salinan, hanya baca, read only, buka sebagai salinan, jendela baru, salinan cadangan, templat saya, simpan sebagai templat, perbaiki, buka dan perbaiki, rusak -->

JeffDeck menyimpan pekerjaan Anda dalam berkas .pptx dan .odp, serta dapat membuka beberapa jenis berkas presentasi.

## Memulai presentasi baru

Tekan Ctrl+N. Jendela baru terbuka dengan presentasi kosong. Atau klik **Berkas > Baru**, lalu **Presentasi Kosong**. Jika jendela yang sedang Anda gunakan berisi presentasi baru yang belum diubah, presentasi kosong itu akan menggantikannya. Jika tidak, presentasi terbuka di jendela tersendiri.

## Menyimpan

Tekan Ctrl+S, atau klik **Berkas > Simpan**. Untuk pertama kali, JeffDeck meminta nama dan lokasi. Setelah itu, Simpan menggantikan berkas dengan pekerjaan terbaru Anda. Pesan di bagian bawah menampilkan kata "Disimpan" dan nama berkasnya.

Tanda bintang (*) setelah nama di bilah judul berarti ada perubahan yang belum disimpan. Simpanlah sering-sering. Jika JeffDeck berhenti secara tiba-tiba, lihat [Pulihkan pekerjaan yang belum disimpan](recover).

Jika pesan menyatakan beberapa objek "tidak dapat disimpan dalam jenis berkas ini", berkas tetap tersimpan, tetapi JeffDeck tidak memiliki cara untuk menulis objek tersebut. Simpan juga berkas asli Anda. Saat Anda menyimpan berkas .odp, pesan akan mencantumkan apa saja yang tidak dapat ditampung oleh jenis berkas itu. Bagian "Menyimpan sebagai berkas .odp" di bawah menjelaskan hal-hal yang tidak dapat ditampungnya.

## Menyimpan dengan nama atau jenis baru

Klik **Berkas > Simpan Sebagai**, atau tekan F12. Ketik nama, pilih jenisnya, lalu klik **Simpan**. Pilihannya adalah:

- **Presentasi (*.pptx)** adalah pilihan yang biasa.
- **Templat (*.potx)** menyimpan slaid sebagai pola untuk memulai presentasi baru.
- **Tayangan (*.ppsx)** menyimpan presentasi untuk diputar.
- **Presentasi Bermakro (*.pptm)**, **Templat Bermakro (*.potm)**, dan **Tayangan Bermakro (*.ppsm)** adalah tiga jenis yang sama untuk presentasi yang berisi **makro**. Makro adalah program kecil yang menyertai sebuah berkas. JeffDeck tidak pernah menjalankan makro, dan tidak pernah membaca atau menampilkannya. Jika Anda membuka salah satu berkas ini dan menyimpannya dalam jenis yang sama, makronya dipertahankan seperti semula. Jika Anda menyimpannya sebagai .pptx, .potx, atau .ppsx, makronya dihilangkan, dan JeffDeck memberi tahu Anda. Presentasi yang tidak memiliki makro juga dapat disimpan dalam jenis ini, tetapi tidak ada makro yang dapat dipertahankan. Jika makro dalam sebuah berkas terlalu besar untuk dipertahankan (lebih dari 32 MB dalam satu bagian, atau 64 MB secara keseluruhan), JeffDeck menghilangkannya dan memberi tahu Anda saat berkas dibuka.
- **Presentasi OpenDocument (*.odp)** menyimpan slaid dalam jenis berkas yang digunakan LibreOffice dan program gratis lainnya.
- **Templat Presentasi OpenDocument (*.otp)** adalah bentuk templat dari jenis tersebut.

Jika Anda tidak mengetik ekstensinya, JeffDeck menambahkan ekstensi yang sesuai dengan jenisnya. Setelah menyimpan, bilah judul menampilkan nama baru, dan Anda tetap bekerja di berkas baru itu.

Presentasi yang Anda buka dari templat disimpan sebagai presentasi baru: **Simpan** meminta nama, sehingga templat tetap seperti semula. Hal yang sama berlaku untuk presentasi yang dibuka dari berkas .ppt, .pps, atau .pot yang lama. Presentasi yang Anda buka dari berkas .odp disimpan kembali sebagai berkas .odp, menimpa berkas yang sama.

## Menyimpan salinan

Klik **Berkas > Simpan Salinan** lalu pilih nama. JeffDeck menulis presentasi ke berkas tersebut, dan Anda tetap bekerja di presentasi yang tadi. Nama dan perubahan yang belum disimpan tetap seperti sebelumnya.

## Menyimpan salinan cadangan

Klik **Berkas > Opsi**, lalu pada tab **Umum** aktifkan **Selalu buat salinan cadangan**. Setiap kali Anda menyimpan, JeffDeck terlebih dahulu menyalin berkas lama ke berkas bernama "Cadangan dari" ditambah nama berkasnya, di folder yang sama. Jika salinan tidak dapat dibuat, aplikasi bertanya apakah tetap ingin menyimpan tanpa cadangan. Jawabannya adalah Tidak, kecuali Anda memilih Ya, dan berkas tetap seperti semula.

## Menyimpan sebagai templat

Templat adalah presentasi yang Anda gunakan sebagai titik awal. Klik **Berkas > Simpan sebagai Templat**, lalu ketik nama. Templat (berkas **.potx**) disimpan di **Templat Saya**, dan presentasi Anda tidak berubah. Untuk memulai presentasi baru dari templat itu, klik **Berkas > Baru**, lalu klik templat di bawah **Templat Saya**. Presentasi baru belum memiliki berkas sendiri, sehingga templat tidak berubah. Untuk menghapus templat dari daftar, klik kanan templat tersebut lalu pilih **Hapus dari Templat Saya**.

## Membuka

Tekan Ctrl+O untuk langsung memilih berkas. Atau klik **Berkas > Buka**, lalu klik **Telusuri**, atau nama di bawah **Disematkan** atau **Presentasi Terbaru**.

Berkas dibuka di jendela ini jika jendela ini berisi presentasi baru yang belum diubah. Jika tidak, berkas dibuka di jendela tersendiri.

Klik kanan nama dalam daftar untuk **Sematkan ke daftar** (nama tetap di bagian atas), **Lepas dari daftar**, atau **Hapus dari daftar**. Nama yang berwarna abu-abu berarti JeffDeck tidak dapat menemukan berkas tersebut saat ini.

## Cara lain membuka berkas

Pada halaman **Buka**, klik **Lebih Banyak Cara Membuka**. Anda juga dapat mengklik kanan nama dalam daftar Terbaru.

- **Buka Hanya-Baca** membuka presentasi hanya untuk dibaca. Anda dapat melihat, berpindah antarslaid, dan menyalin, tetapi tidak dapat mengubah apa pun. Bilah judul menampilkan "Hanya Baca". Jika Anda menekan **Simpan**, JeffDeck meminta nama baru, sehingga berkas yang Anda buka tidak berubah.
- **Buka sebagai Salinan** membuka slaid sebagai presentasi baru tanpa berkas sendiri. **Simpan** menanyakan di mana menyimpannya.
- **Buka di Jendela Baru** membuka berkas di jendela tersendiri, meskipun jendela yang sedang Anda gunakan kosong.
- **Buka dan Perbaiki** digunakan untuk berkas .pptx atau .ppt yang rusak, misalnya berkas yang terpotong saat disalin. JeffDeck membaca setiap bagian yang dapat dibaca, mempertahankan bagian yang terhenti di tengah sampai titik ia berhenti, dan membuang bagian yang tidak dapat dibaca. Untuk berkas .ppt lama, JeffDeck menemukan slaid berdasarkan isinya ketika daftar yang menunjukkan lokasinya rusak atau hilang. Jika daftar yang menentukan urutan slaid hilang, slaid dimasukkan sesuai urutan penyimpanannya, yang mungkin tidak sama dengan urutan semula. Berkas yang memiliki kata sandi tidak dapat diperbaiki. Slaid yang sudah diperbaiki dibuka sebagai salinan bernama "Diperbaiki" ditambah nama berkasnya, dan sebuah pesan memberi tahu bagian apa saja yang dibuang. Berkas yang rusak tidak diubah.

## Berkas dengan kata sandi

Berkas **.pptx** atau **.odp** yang meminta kata sandi dibuka seperti biasa: JeffDeck meminta kata sandi, dan Anda dapat mencoba lagi jika salah, atau klik **Batal**. Berkas tetap dilindungi saat Anda menyimpannya. Untuk mengubah atau menghapus kata sandi, gunakan **Berkas > Info > Enkripsi dengan Kata Sandi** (lihat Layar Berkas). Salinan yang disimpan sebagai berkas .odp juga mempertahankan kata sandi.

## Font dalam berkas

Font adalah bentuk huruf. Tidak semua komputer memiliki semua font. Jika sebuah komputer tidak memiliki font yang dipakai slaid Anda, komputer itu menggambar huruf dengan font lain. Akibatnya, slaid bisa tampak berbeda.

JeffDeck dapat memasukkan font yang dipakai slaid Anda ke dalam berkas. Dengan begitu, slaid tampak sama di komputer mana pun. Ini disebut penyematan. JeffDeck melakukannya setiap kali Anda menyimpan. Untuk mematikannya, klik **Berkas > Opsi**, buka tab **Simpan**, lalu hapus centang **Sematkan font dalam berkas**.

- JeffDeck hanya memasukkan font yang disertakan bersamanya, seperti Fraunces, Lato, dan Open Sans. Komputer lain tidak memiliki font tersebut. JeffDeck tidak pernah memasukkan font yang dimiliki setiap komputer, seperti Calibri atau Arial. JeffDeck juga tidak memasukkan font yang lisensinya menyatakan tidak boleh disalin ke dalam berkas.
- JeffDeck memasukkan setiap gaya yang dipakai slaid: biasa, tebal, miring, dan tebal miring. Jika slaid hanya memakai gaya biasa, hanya gaya itu yang dimasukkan.
- JeffDeck memasukkan setiap huruf dari setiap font, tidak hanya huruf yang Anda pakai. Dengan begitu, orang yang membuka berkas tetap dapat mengubah kata-katanya.
- Berkas menjadi lebih besar. Setiap gaya font menambah sekitar 40 hingga 700 KB (satu KB adalah seribu bita). Berkas dimampatkan saat disimpan, sehingga pertambahannya sekitar separuh dari angka itu.

Saat Anda membuka berkas yang memuat font di dalamnya, JeffDeck menggunakannya selama presentasi terbuka. Slaid tampak seperti saat dibuat. JeffDeck tidak memasang font itu di komputer Anda dan tidak menyalinnya ke mana pun. Font itu hanya milik presentasi tersebut: tidak muncul di daftar font Anda, dan tidak dipakai presentasi lain. Font itu hilang ketika presentasi ditutup. Jika komputer Anda sudah memiliki font tersebut, JeffDeck menggunakan salinan milik Anda sendiri.

JeffDeck membaca font dari berkas .pptx yang dibuat program presentasi lain, meskipun program itu telah memampatkannya. JeffDeck juga membaca font dari berkas .odp.

Sebuah berkas dapat memuat font yang tidak dimiliki JeffDeck. Saat menyimpan, JeffDeck mengembalikan font itu ke dalam berkas selama slaid masih memakainya. Hal ini berlaku baik opsi **Sematkan font dalam berkas** aktif maupun tidak. Jika sebuah font dalam berkas rusak atau tidak dapat dibaca, pesan yang muncul saat Anda membuka berkas akan menyebutkannya.

## Jenis berkas yang dapat dibaca JeffDeck

- **.pptx, .potx, dan .ppsx** adalah satu format dalam tiga bentuk: presentasi, templat, dan tayangan. JeffDeck dapat membaca dan menulis ketiganya.
- **.ppt, .pps, dan .pot** adalah jenis yang lebih lama. JeffDeck dapat membacanya tetapi tidak dapat menulisnya, sehingga **Simpan** meminta nama baru dan menyimpan berkas .pptx (.ppsx untuk .pps). Berkas asli tidak pernah ditimpa.
- **.odp dan .otp** adalah presentasi dokumen terbuka beserta templatnya. JeffDeck dapat membaca dan menulis keduanya. Berkas .odp yang dibuka di sini disimpan kembali sebagai .odp. Berkas .otp adalah templat, sehingga **Simpan** meminta nama baru dan menyarankan .odp.

Jika sebagian berkas tidak dapat dibaca, JeffDeck tetap membuka sisanya dan mencantumkan bagian yang dibuang. Jika seluruh berkas tidak dapat dibaca, JeffDeck memberi tahu alasannya.

## Menyimpan sebagai berkas .odp

Berkas .odp dapat dibuka di LibreOffice Impress dengan slaid, master, gambar, catatan, dan komentar Anda, serta tampilan yang ditampilkan JeffDeck. Berkas ini menyimpan animasi, transisi slaid, presentasi khusus, dan pengaturan tayangan. Font setiap kata disimpan berdasarkan nama, sehingga komputer yang tidak memiliki font tersebut akan menampilkan font lain.

JeffDeck sendiri belum membaca kembali animasi, komentar, presentasi khusus, atau pengaturan tayangan dari berkas .odp. Semuanya tetap ada di dalam berkas, dan LibreOffice dapat memutarnya, tetapi saat Anda membuka berkas .odp di JeffDeck, semuanya dicantumkan sebagai tidak disertakan. Simpan berkas .pptx Anda jika ingin terus mengeditnya di sini.

Berkas .odp tidak memiliki tempat untuk beberapa hal. Setelah penyimpanan, JeffDeck mencantumkannya dalam pesan di bagian bawah jendela:

- **Bagan, diagram, dan persamaan** disimpan sebagai gambar. Anda dapat melihatnya, tetapi tidak dapat mengedit angka atau bagiannya dalam berkas .odp.
- **Bagian** tidak disimpan.
- **Efek bevel dan bayangan dalam** tidak disertakan, begitu pula cahaya dan pantulan pada teks. Bayangan pada teks tetap disimpan. Pantulan pada bentuk, garis dengan gradasi, garis dengan dua atau tiga goresan, dan ukuran bayangan disimpan dengan cara yang hanya dibaca oleh JeffDeck. Program lain menampilkan bentuk tanpa pantulan, garis dengan warna pertamanya, satu goresan saja, dan bayangan dengan ukuran objek.
- **Efek animasi** yang dimulai saat Anda mengklik objek, dan efek yang mengubah warna, tidak disertakan. Klik pada film, untuk menjeda atau memutarnya lagi, tetap disimpan.
- **Film atau suara yang ditautkan** dari berkas lain, bukan disimpan di dalam presentasi, menyimpan alamat dan gambarnya di berkas .odp. LibreOffice dapat memutarnya jika menemukan berkasnya. Film dan suara yang ada di dalam presentasi disimpan di berkas .odp dan dapat diputar.

Simpan juga berkas .pptx Anda jika Anda memerlukan hal-hal ini kembali.

## Yang disimpan tetapi tidak ditampilkan

Beberapa bagian berkas adalah hal yang belum dapat ditampilkan atau diubah oleh JeffDeck. JeffDeck tetap menyimpannya bersama slaidnya dan menuliskannya kembali saat Anda menyimpan, sehingga tidak hilang. Ini mencakup animasi, yang diputar dalam tayangan, film dan suara, yang belum dapat diputar, serta efek transisi yang tidak memiliki bentuk di JeffDeck, yang diputar sebagai pudar. Bagan dan diagram tampil seperti saat disimpan. Lihat [Selamat datang di JeffDeck](start).

## Terkait

- [Pulihkan pekerjaan yang belum disimpan](recover)
- [Layar Berkas](file-screen)
- [Gunakan ulang slaid dari presentasi lain](reuse)
- [Cetak, PDF, dan gambar slaid](printing)
