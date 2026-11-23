# Membuka dan menyimpan presentasi
<!-- keywords: makro, bermakro, pptm, potm, ppsm, buka, simpan, simpan sebagai, otp, libreoffice, dibiarkan, tutup, baru, berkas terbaru, disematkan, jenis berkas, pptx, potx, ppsx, ppt, odp, templat, tayangan, ganti nama, Ctrl+S, cadangan, format berkas, berkas lama, dokumen OpenDocument, disimpan, tidak ditampilkan, font, sematkan font, font tersemat, font hilang, simpan salinan, hanya baca, buka sebagai salinan, jendela baru, salinan cadangan, templat saya, simpan sebagai templat, perbaiki, buka dan perbaiki, rusak -->

JeffDeck menyimpan pekerjaan Anda dalam berkas .pptx dan .odp, serta dapat membuka beberapa jenis berkas presentasi.

## Memulai presentasi baru

Tekan Ctrl+N. Jendela baru akan terbuka dengan presentasi kosong. Atau klik **Berkas > Baru**, lalu **Presentasi Kosong**. Jika jendela yang Anda gunakan berisi presentasi baru yang belum disentuh, presentasi kosong itu menggantikannya. Jika tidak, presentasi akan terbuka di jendelanya sendiri.

## Menyimpan

Tekan Ctrl+S, atau klik **Berkas > Simpan**. Untuk pertama kali, JeffDeck meminta nama dan lokasi. Setelah itu, Simpan menggantikan berkas dengan pekerjaan terbaru Anda. Pesan di bagian bawah akan menampilkan kata "Disimpan" beserta nama berkas.

Tanda bintang (*) setelah nama di bilah judul berarti ada perubahan yang belum Anda simpan. Simpanlah sering-sering. Jika JeffDeck berhenti secara tiba-tiba, lihat [Mengambil kembali pekerjaan yang belum disimpan](recover).

Jika sebuah pesan menyatakan bahwa beberapa objek "belum dapat disimpan dalam jenis berkas ini", berkas tetap disimpan, tetapi JeffDeck tidak memiliki cara untuk menulis objek tersebut. Simpan juga berkas aslinya. Saat Anda menyimpan berkas .odp, pesan akan mencantumkan hal-hal yang tidak dapat dimuat oleh jenis berkas tersebut. Bagian "Menyimpan sebagai berkas .odp" di bawah menjelaskan apa saja yang tidak dapat dimuat.

## Menyimpan dengan nama atau jenis baru

Klik **Berkas > Simpan Sebagai**, atau tekan F12. Ketik nama, pilih jenisnya, lalu klik **Simpan**. Pilihannya adalah:

- **Presentasi (*.pptx)** adalah pilihan yang biasa.
- **Templat (*.potx)** menyimpan slaid sebagai pola untuk memulai presentasi baru.
- **Presentasi Slaid (*.ppsx)** menyimpan presentasi untuk diputar.
- **Presentasi Mendukung Makro (*.pptm)**, **Templat Mendukung Makro (*.potm)**, dan **Tayangan Mendukung Makro (*.ppsm)** adalah tiga jenis yang sama untuk presentasi yang memuat **makro**. Makro adalah program kecil yang menyertai sebuah berkas. JeffDeck tidak pernah menjalankan makro, dan tidak membaca atau menampilkannya. Jika Anda membuka salah satu berkas ini lalu menyimpannya dengan jenis yang sama, makro tetap dipertahankan seperti semula. Jika Anda menyimpannya sebagai .pptx, .potx, atau .ppsx, makro akan dibiarkan dan JeffDeck akan memberi tahu Anda. Presentasi yang tidak memiliki makro juga dapat disimpan dengan jenis-jenis ini, tetapi tidak ada makro yang perlu dipertahankan. Jika makro dalam sebuah berkas terlalu besar untuk dipertahankan (lebih dari 32 MB dalam satu bagian, atau 64 MB seluruhnya), JeffDeck akan membiarkannya dan memberi tahu Anda saat berkas dibuka.
- **Presentasi OpenDocument (*.odp)** menyimpan slaid dalam jenis berkas yang digunakan LibreOffice dan program gratis lainnya.
- **Templat Presentasi OpenDocument (*.otp)** adalah bentuk templat dari jenis ini.

Jika Anda tidak menuliskan akhiran, JeffDeck menambahkan akhiran yang sesuai dengan jenisnya. Setelah menyimpan, bilah judul menampilkan nama baru, dan Anda tetap bekerja di berkas yang baru.

Presentasi yang Anda buka dari templat akan disimpan sebagai presentasi baru: **Simpan** akan meminta nama, sehingga templat tetap seperti semula. Hal yang sama berlaku untuk presentasi yang dibuka dari berkas .ppt, .pps, atau .pot lama. Presentasi yang Anda buka dari berkas .odp akan disimpan kembali sebagai berkas .odp, menggantikan berkas yang sama.

## Menyimpan salinan

Klik **Berkas > Simpan Salinan**, lalu pilih nama. JeffDeck menulis presentasi ke berkas tersebut, dan Anda tetap bekerja pada presentasi yang sebelumnya. Nama serta perubahan yang belum disimpan pada presentasi itu tetap seperti semula.

## Menyimpan salinan cadangan

Klik **Berkas > Opsi**, lalu pada tab **Umum**, aktifkan **Selalu buat salinan cadangan**. Setiap kali Anda menyimpan, JeffDeck lebih dulu menyalin berkas lama ke berkas bernama "Cadangan dari" ditambah nama berkas, di folder yang sama. Jika salinan tidak dapat dibuat, aplikasi akan menanyakan apakah berkas tetap disimpan tanpa cadangan. Jawabannya adalah Tidak, kecuali Anda memilih Ya, dan berkas akan tetap seperti semula.

## Menyimpan sebagai templat

Templat adalah presentasi yang Anda gunakan sebagai titik awal. Klik **Berkas > Simpan sebagai Templat** lalu ketik nama. Templat (berkas **.potx**) akan disimpan di **Templat Saya**, dan presentasi Anda tidak berubah. Untuk memulai presentasi baru dari templat itu, klik **Berkas > Baru**, lalu klik templat di bawah **Templat Saya**. Presentasi baru belum memiliki berkas sendiri, sehingga templat tidak berubah. Untuk menghapus templat dari daftar, klik kanan templat itu lalu pilih **Hapus dari Templat Saya**.

## Membuka

Tekan Ctrl+O untuk langsung memilih berkas. Atau klik **Berkas > Buka**, lalu klik **Telusuri**, atau klik nama di bawah **Disematkan** atau **Presentasi Terbaru**.

Berkas akan terbuka di jendela ini jika jendela berisi presentasi baru yang belum disentuh. Jika tidak, berkas akan terbuka di jendelanya sendiri.

Klik kanan nama dalam daftar untuk **Sematkan ke Daftar** (nama tetap berada di atas), **Lepas Sematan dari Daftar**, atau **Hapus dari Daftar**. Nama yang berwarna abu-abu berarti JeffDeck tidak dapat menemukan berkas tersebut saat ini.

## Cara lain membuka berkas

Pada halaman **Buka**, klik **Cara Lain untuk Membuka**. Anda juga dapat mengklik kanan nama dalam daftar Terbaru.

- **Buka Hanya-Baca** membuka presentasi hanya untuk dibaca. Anda dapat melihat, berpindah antarslaid, dan menyalin, tetapi tidak dapat mengubah apa pun. Bilah judul menampilkan "Hanya Baca". Jika Anda menekan **Simpan**, JeffDeck akan meminta nama baru, sehingga berkas yang Anda buka tidak berubah.
- **Buka sebagai Salinan** membuka slaid sebagai presentasi baru tanpa berkas sendiri. **Simpan** akan menanyakan lokasi penyimpanannya.
- **Buka di Jendela Baru** membuka berkas di jendelanya sendiri, meskipun jendela yang Anda gunakan kosong.
- **Buka dan Perbaiki** diperuntukkan bagi berkas .pptx atau .ppt yang rusak, misalnya berkas yang terpotong saat disalin. JeffDeck memulihkan sebanyak mungkin: ia membaca setiap bagian yang dapat dibaca, mempertahankan bagian yang terhenti di tengah sampai titik ia berhenti, dan membiarkan bagian yang tidak dapat dibaca. Untuk berkas .ppt lama, JeffDeck menemukan slaid berdasarkan isinya jika daftar yang menunjukkan letak slaid rusak atau hilang. Jika daftar yang menjelaskan bagaimana potongan-potongan berkas saling terhubung rusak, program mengambil setiap potongan untuk mengikuti potongan sebelumnya, lalu menampilkan apa yang ditemukan di sana. Bagian itu mungkin berasal dari salinan presentasi yang lebih lama atau dari bagian lain berkas, sehingga pesan akan menyebutkan slaid tempat bagian itu dimulai dan meminta Anda membandingkannya dengan cadangan jika Anda memilikinya. Jika tebakan muncul sebelum slaid pertama, pesan akan menyatakan bahwa seluruh presentasi telah disusun ulang. Jika gambar tidak dapat dipulihkan sepenuhnya, pesan akan menyatakannya. Jika daftar yang memuat urutan slaid hilang, slaid akan muncul sesuai urutan saat disimpan, yang mungkin tidak sama dengan urutan semula. Berkas yang memiliki kata sandi tidak dapat diperbaiki. Slaid yang telah diperbaiki akan dibuka sebagai salinan bernama "Diperbaiki" ditambah nama berkas, dan sebuah pesan akan memberi tahu apa yang telah dilakukan. Berkas yang rusak tidak diubah.

## Berkas dengan kata sandi

Berkas **.pptx** atau **.odp** yang meminta kata sandi akan terbuka seperti biasa: JeffDeck meminta kata sandi, dan Anda dapat mencoba lagi jika kata sandinya salah, atau klik **Batal**. Berkas tetap dilindungi saat Anda menyimpannya. Untuk mengubah atau menghapus kata sandi, gunakan **Berkas > Info > Enkripsi dengan Kata Sandi** (lihat Layar Berkas). Salinan yang disimpan sebagai berkas .odp juga tetap memiliki kata sandi.

## Font dalam berkas

Font adalah bentuk huruf. Tidak semua komputer memiliki semua font. Jika sebuah komputer tidak memiliki font yang digunakan slaid Anda, komputer itu menggambar huruf dengan font lain. Akibatnya, slaid dapat tampak berbeda.

JeffDeck dapat memasukkan font yang digunakan slaid Anda ke dalam berkas. Dengan begitu, slaid tampak sama di komputer mana pun. Proses ini disebut penyematan. JeffDeck melakukannya setiap kali Anda menyimpan. Untuk mematikannya, klik **Berkas > Opsi**, buka tab **Simpan**, lalu hapus centang **Sematkan font dalam berkas**.

- JeffDeck hanya memasukkan font yang disertakan bersamanya, seperti Fraunces, Lato, dan Open Sans. Komputer lain tidak memiliki font tersebut. JeffDeck tidak pernah memasukkan font yang sudah dimiliki setiap komputer, seperti Calibri atau Arial. Font yang lisensinya melarang penyalinan ke dalam berkas akan dibiarkan.
- JeffDeck memasukkan setiap gaya yang digunakan slaid: biasa, tebal, miring, dan tebal miring. Jika slaid hanya memakai gaya biasa, hanya gaya itu yang dimasukkan.
- JeffDeck memasukkan setiap huruf dari setiap font, bukan hanya huruf yang Anda gunakan. Dengan begitu, orang yang membuka berkas tetap dapat mengubah kata-katanya.
- Ukuran berkas menjadi lebih besar. Setiap gaya font menambah sekitar 40 hingga 700 KB (satu KB adalah seribu byte). Berkas dipadatkan saat disimpan, sehingga pertambahannya sekitar setengah dari ukuran tersebut.

Saat Anda membuka berkas yang berisi font tersemat, JeffDeck menggunakan font tersebut selama presentasi terbuka. Slaid tampak seperti saat dibuat. JeffDeck tidak memasang font itu di komputer Anda dan tidak menyalinnya ke tempat lain. Font itu hanya milik presentasi tersebut: font tidak muncul di daftar font Anda, dan presentasi lain tidak menggunakannya. Font akan hilang saat presentasi ditutup. Jika komputer Anda sudah memiliki font tersebut, JeffDeck menggunakan salinan milik Anda.

JeffDeck dapat membaca font dari berkas .pptx yang dibuat oleh program presentasi lain, meskipun program tersebut memadatkan font itu. JeffDeck juga dapat membaca font dari berkas .odp.

Sebuah berkas dapat membawa font yang tidak dimiliki JeffDeck. Saat menyimpan, JeffDeck memasukkan kembali font itu ke dalam berkas, selama slaid masih menggunakannya. Hal ini dilakukan baik opsi **Sematkan font dalam berkas** aktif maupun tidak. Jika sebuah font dalam berkas rusak atau tidak dapat dibaca, pesan yang muncul saat Anda membuka berkas akan menyatakannya.

## Jenis berkas yang dibaca JeffDeck

- **.pptx, .potx, dan .ppsx** adalah satu format dalam tiga bentuk: presentasi, templat, dan tayangan. JeffDeck dapat membaca dan menulis ketiganya.
- **.ppt, .pps, dan .pot** adalah jenis yang lebih lama. JeffDeck dapat membacanya tetapi tidak dapat menulisnya, sehingga **Simpan** akan meminta nama baru dan menyimpan berkas .pptx (atau .ppsx untuk .pps). Berkas asli tidak pernah ditimpa.
- **.odp dan .otp** adalah presentasi dokumen terbuka beserta templatnya. JeffDeck dapat membaca dan menulis keduanya. Berkas .odp yang dibuka di sini akan disimpan sebagai .odp. Berkas .otp adalah templat, sehingga **Simpan** akan meminta nama baru dan menyarankan .odp.

Jika sebagian berkas tidak dapat dibaca, JeffDeck tetap membuka bagian lainnya dan mencantumkan hal-hal yang dibiarkan. Jika seluruh berkas tidak dapat dibaca, JeffDeck memberi tahu alasannya.

## Menyimpan sebagai berkas .odp

Berkas .odp dapat dibuka di LibreOffice Impress dengan slaid, master, gambar, catatan, dan komentar Anda, serta tampilan yang ditunjukkan JeffDeck. Berkas ini menyimpan animasi, transisi slaid, pertunjukan khusus, dan pengaturan presentasi slaid. Font setiap kata disimpan berdasarkan namanya, sehingga komputer yang tidak memiliki font tersebut akan menampilkan font lain.

JeffDeck sendiri belum dapat membaca kembali animasi, komentar, pertunjukan khusus, atau pengaturan tayangan dari berkas .odp. Semua itu tetap ada di dalam berkas, dan LibreOffice dapat memutarnya, tetapi saat Anda membuka berkas .odp di JeffDeck, program akan mencatatnya sebagai bagian yang dibiarkan. Simpan berkas .pptx Anda jika Anda ingin terus mengeditnya di sini.

Berkas .odp tidak memiliki tempat untuk beberapa hal. JeffDeck mencantumkannya setelah penyimpanan, di pesan pada bagian bawah jendela:

- **Bagan, diagram, dan persamaan** disimpan sebagai gambar. Anda dapat melihatnya, tetapi tidak dapat mengedit angka atau bagiannya di berkas .odp.
- **Bagian** tidak disimpan.
- **Bevel dan bayangan dalam** dibiarkan, begitu pula cahaya dan pantulan pada teks. Bayangan pada teks tetap disimpan. Pantulan pada bentuk, garis dengan gradasi, garis dua atau tiga goresan, dan ukuran bayangan tetap disimpan dengan cara yang hanya dibaca oleh JeffDeck. Program lain akan menampilkan bentuk tanpa pantulan, garis dengan warna pertamanya, goresan tunggal, dan bayangan sebesar objeknya.
- **Efek animasi** yang dimulai saat Anda mengklik objek, dan efek yang mengubah warna, dibiarkan. Klik pada film, untuk menjeda atau memutarnya kembali, tetap disimpan.
- **Film atau suara yang ditautkan** dari berkas lain, bukan yang disimpan di dalam presentasi, tetap menyimpan alamat dan gambarnya di berkas .odp. LibreOffice dapat memutarnya jika menemukan berkasnya. Film dan suara yang berada di dalam presentasi disimpan di berkas .odp dan dapat diputar.

Simpan juga berkas .pptx Anda jika Anda membutuhkan hal-hal tersebut kembali.

## Yang disimpan tetapi tidak ditampilkan

Sebagian bagian berkas adalah hal-hal yang belum dapat ditampilkan atau diubah oleh JeffDeck. Program tetap menyimpannya bersama slaid dan menuliskannya kembali saat Anda menyimpan, sehingga tidak hilang. Hal ini mencakup animasi, yang diputar dalam tayangan, film dan suara, yang belum dapat diputar, serta efek transisi yang tidak memiliki bentuk di JeffDeck, yang diputar sebagai pudar. Bagan dan diagram ditampilkan seperti saat disimpan. Lihat [Selamat datang di JeffDeck](start).

## Terkait

- [Mengambil kembali pekerjaan yang belum disimpan](recover)
- [Layar Berkas](file-screen)
- [Menggunakan ulang slaid dari presentasi lain](reuse)
- [Cetak, PDF, dan gambar slaid](printing)
