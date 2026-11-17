# Menambahkan dan menyesuaikan gambar
<!-- keywords: efek gambar, bayangan, pantulan, cahaya, tepi lembut, bevel, rotasi 3-D, atur ulang gambar dan ukuran, foto, citra, sisipkan gambar, potong, clip art, halaman PDF, jpg, png, svg, ubah warna, teks alternatif, keterangan, placeholder, pengelola grafis, gambar daring, tautkan ke berkas, gambar tertaut, sisipkan dan tautkan, sematkan, gambar hilang, tautkan ulang, gambar, pemotongan, tautan gambar, bingkai gambar -->

Gambar berada di dalam sebuah bingkai. Anda dapat memotong gambar, serta memindahkan atau mengubah ukuran bingkainya.

## Menyisipkan gambar dari berkas

1. Klik **Sisipkan > Ilustrasi > Gambar**.
2. Di jendela **Sisipkan Gambar**, pilih berkas. JeffPub membuka berkas PNG, JPEG, GIF, BMP, TIFF, WebP, SVG, WMF, EMF, ICO, dan PDF.
3. Pilih **Sisipkan**, **Tautkan ke Berkas**, atau **Sisipkan dan Tautkan** di kotak **Sisipkan sebagai**, yang berada bersama daftar berkas dan dapat dijangkau dengan tombol Tab. **Sisipkan** dipilih setiap kali jendela dibuka. Lihat "Menautkan gambar ke berkasnya" di bawah.
4. Klik **Buka**.

Gambar akan muncul di tengah halaman. Anda juga dapat menyeret berkas gambar ke halaman; cara ini selalu menggunakan **Sisipkan**.

JeffPub membaca berkas gambar hingga 256 MB. Berkas yang lebih besar akan ditolak, begitu pula berkas kosong atau sesuatu yang bukan berkas biasa di disk, seperti perangkat atau pipa. Pesannya menyatakan bahwa JeffPub tidak dapat membacanya sebagai gambar.

Jika Anda memilih beberapa gambar sekaligus, gambar tersebut akan tersusun sebagai miniatur di area coret-coret, yaitu ruang abu-abu di samping halaman. Seret gambar yang Anda inginkan ke halaman.

Jika sebuah PDF memiliki lebih dari satu halaman, jendela **Sisipkan Halaman PDF** menampilkan setiap halaman. Pilih satu halaman dan klik **OK**.

## Menautkan gambar ke berkasnya

**Sisipkan** menyimpan salinan gambar di dalam publikasi. Salinan itu ikut ke mana pun publikasi dibawa, tetapi publikasi dengan banyak foto besar akan ikut membesar. Tautan membuat foto tetap berada di berkasnya sendiri.

- **Sisipkan** menyalin gambar ke dalam publikasi. Ini pilihan yang biasa dipakai.
- **Tautkan ke Berkas** hanya menyimpan lokasi berkas dan pratinjau kecil, dengan sisi terpanjang tidak lebih dari 512 piksel. Gambar di halaman, serta pada cetakan, PDF, atau e-book, diambil dari berkas dengan kualitas penuh. Jika berkas dipindahkan atau dihapus, halaman hanya menampilkan pratinjau kecil.
- **Sisipkan dan Tautkan** melakukan keduanya. Publikasi menyimpan gambar utuh beserta tautannya. Saat Anda membuka publikasi dan berkasnya telah berubah, JeffPub membaca ulang berkas itu dan menggantikan salinannya.

JeffPub mengingat lokasi lengkap berkas, dan juga letaknya relatif terhadap publikasi, misalnya "folder Foto di sebelahnya". Jika Anda memindahkan publikasi dan gambarnya bersama-sama, lokasi relatif menjadi acuan, sehingga gambar dapat ditemukan kembali. Biarkan struktur folder tetap seperti semula.

JeffPub hanya membuka berkas gambar tertaut secara otomatis jika berkas itu berada di folder publikasi atau di folder di dalamnya. Gambar yang ditautkan ke berkas di tempat lain akan menampilkan pratinjau tersimpan yang buram dan ditandai **Belum diperbarui**, sampai Anda memilihnya dan mengklik **Perbarui Tautan** di Pengelola Grafis. Cara ini mencegah publikasi yang dikirim orang lain mengakses komputer Anda. Agar gambar dimuat sendiri, simpan gambar di folder publikasi.

**Pengelola Grafis** menunjukkan apakah setiap gambar berstatus **Tersemat**, **Tertaut**, **Hilang**, **Diubah**, atau **Belum diperbarui**, dan memiliki tombol untuk memperbarui tautan, mengarahkannya ke berkas lain, atau menjadikannya salinan. Lihat [Pengelola Grafis](graphics-manager).

Saat Anda menyimpan publikasi sebagai berkas `.pub`, membuat berkas Pack and Go, atau mengirimnya lewat e-mail, JeffPub memasukkan setiap gambar tertaut secara utuh ke dalam berkas, sehingga tidak bergantung pada berkas yang tertinggal. Jika berkas tertaut hilang saat itu, atau gambar berstatus **Belum diperbarui**, pratinjau kecilnya yang dimasukkan. Berkas `.pub` yang Anda buka tidak pernah memiliki tautan: gambarnya dimuat sebagai salinan.

## Menambahkan placeholder gambar

Placeholder adalah bingkai kosong yang menyediakan tempat untuk gambar. Klik **Sisipkan > Ilustrasi > Placeholder Gambar**, lalu klik dua kali bingkai dan pilih berkas. Gambar akan mengisi bingkai, dan bagian tepi yang menonjol disembunyikan. Placeholder kosong tidak tercetak.

## Mencari gambar gratis di internet

Klik **Sisipkan > Ilustrasi > Gambar Daring**. Panel **Gambar Daring** mencari dua pustaka gambar gratis yang berlisensi terbuka: Openverse, yang mencakup banyak koleksi, dan Wikimedia Commons, pustaka gambar Wikipedia.

1. Pilih pustaka, ketik yang Anda cari, misalnya "lumbung merah", lalu tekan Enter.
2. Klik sebuah gambar untuk melihat siapa pembuatnya dan lisensinya. Lisensi adalah aturan penggunaan gambar. Sebagian besar meminta Anda mencantumkan nama pembuatnya.
3. Klik **Sisipkan**. JeffPub menempatkan gambar di halaman dan memakai judulnya sebagai teks alternatif. Dengan **Tambahkan kredit di bawah gambar** dicentang, baris seperti "Lumbung merah" oleh Pat Lee, CC BY 2.0 akan ditambahkan di bawahnya.

Kata pencarian Anda dikirim ke pustaka yang Anda pilih. Pastikan lisensi setiap gambar sesuai dengan cara Anda menggunakannya.

Satu pencarian menampilkan hingga 24 gambar. JeffPub hanya menggunakan alamat web aman (https) dari sebuah pustaka, dan membuang gambar yang alamatnya bukan https. Dari Wikimedia Commons, gambar yang lebarnya lebih dari 1.920 piksel diambil sebagai salinan selebar itu, bukan berkas asli yang diunggah. Unduhan yang lebih dari 50 MB, atau yang terlalu lama, akan dihentikan, dan panel memberi tahu bahwa gambar tidak dapat diunduh atau dimuat.

## Memotong gambar

Memotong berarti menyembunyikan tepi gambar.

1. Klik gambar, lalu klik **Format Gambar > Potong > Potong**.
2. Seret pegangan untuk menyembunyikan sebagian gambar. Seret gambar untuk menggeser posisinya di dalam bingkai.
3. Klik **Potong** lagi, atau tekan **Esc**.

Juga di grup **Potong**: **Rasio Aspek** memotong bingkai menjadi bentuk seperti persegi, lebar 4 kali tinggi 3, atau 16 kali 9, dengan berpusat di tengahnya (gambar tetap berada di posisinya di halaman). **Sesuaikan** menampilkan seluruh gambar di dalam bingkainya, **Isi** mengisi bingkai, **Potong ke Bentuk** memangkas gambar menjadi bentuk seperti lingkaran, dan **Hapus Potongan** menampilkan kembali seluruh gambar.

## Mengubah tampilan gambar

Pada tab **Format Gambar**, gunakan grup **Sesuaikan**:

- **Koreksi**: pilih pasangan kecerahan dan kontras, dari minus 40 hingga plus 40 persen. **Format Objek…**, di bagian bawah menu, menyediakan penggeser.
- **Ubah Warna**: jadikan gambar Skala abu-abu, Sepia, Pudar, Hitam Putih, atau rona warna aksen. **Atur Warna Transparan** membuat warna di sudut kiri atas gambar menjadi tembus pandang.
- **Transparansi…**: memudarkan seluruh gambar.
- **Atur Ulang Gambar**: menghapus semua perubahan dalam daftar ini, serta bingkai, bentuk, dan efek gambar. Pemotongan juga dihapus, sehingga bingkai menampilkan seluruh gambar. Klik panah di sebelahnya untuk **Atur Ulang Gambar & Ukuran**, yang juga mengembalikan ukuran gambar seperti yang tersimpan di berkasnya.
- **Kompres Gambar…**: memperkecil berkas dengan menurunkan detail menjadi 300, 220, atau 96 ppi (piksel per inci). Secara bawaan, semua gambar diubah dan area yang dipotong dihapus.
- **Ubah ke Bentuk**: mengubah gambar SVG (gambar yang terdiri atas garis dan bentuk) menjadi bentuk yang dapat diedit, dipotong sesuai bagian yang dipangkas oleh gambar. Teks dan gambar di dalamnya tidak disertakan.

## Gaya, bingkai, dan efek gambar

Gunakan grup **Gaya Gambar** pada tab **Format Gambar**:

- **Galeri** menampilkan sekitar dua puluh tampilan siap pakai: bingkai, bayangan, pantulan, tepi lembut, bentuk membulat dan oval, putaran dalam ruang, cahaya, dan bevel. Bevel adalah tepi miring yang membuat gambar terlihat timbul. Setiap kotak menampilkan warna Anda. Klik salah satu untuk memberi gambar tampilan tersebut. Gaya mengatur bentuk, bingkai, dan efek sekaligus, dan menggantikan yang sudah ada pada gambar. Potongan, ukuran, dan warna tetap dipertahankan.
- **Bingkai Gambar** mengatur warna, **Ketebalan** (seberapa tebal garisnya), dan **Garis Putus-putus** (garis padat, titik-titik, atau putus-putus). **Tanpa Garis Tepi** menghapus bingkai.
- **Efek Gambar** membuka **Bayangan**, **Pantulan**, **Cahaya**, **Tepi Lembut**, **Bevel**, dan **Rotasi 3-D**. Setiap menu memiliki pilihan siap pakai. Pilih salah satu untuk menambahkannya. Efek lain tetap ada. Pilih entri pertama, seperti **Tanpa Bayangan**, untuk menghapus satu efek. **Opsi Efek Lainnya…** membuka **Format Objek**.

Setiap langkah dapat dibatalkan.

## Keterangan, teks alternatif, dan penukaran

- **Format Gambar > Gaya Gambar > Keterangan** menambahkan kotak teks di bawah gambar.
- Teks alternatif adalah deskripsi singkat untuk orang yang tidak dapat melihat gambar, misalnya pengguna pembaca layar. Klik kanan gambar, pilih **Format Objek…**, buka tab **Teks Alternatif**, lalu ketik di **Teks alternatif**.
- **Format Gambar > Sisipkan > Ganti Gambar** memasukkan berkas lain ke bingkai yang sama. Bingkai mempertahankan ukuran, posisi, dan tampilannya: bingkai, bentuk, efek, dan perubahan warna. Jika gambar baru memiliki bentuk yang sama dengan yang lama, pemotongan juga dipertahankan. Jika bentuknya berbeda, gambar baru mengisi bingkai.
- Untuk menukar dua gambar, seret salah satunya ke gambar yang lain. Saat tooltip menampilkan **Lepaskan untuk menukar gambar**, lepaskan tombol mouse: gambar berpindah bingkai, dan bingkai yang Anda seret kembali ke tempat semula. Atau tahan Shift, klik kedua gambar, lalu klik **Tukar**. Tahan Ctrl saat menyeret untuk menyalin gambar. Salinan yang Anda lepaskan di atas gambar lain tetap berada di tempat Anda meletakkannya, dan tidak ada yang ditukar.

## Melihat semua gambar

Klik **Tampilan > Tampilkan > Pengelola Grafis** untuk menampilkan setiap gambar dalam panel **Pengelola Grafis**. Pilih satu gambar, lalu klik **Buka** untuk menuju ke gambar tersebut, **Simpan Sebagai…** untuk menyimpan salinan, atau **Ganti…** untuk memilih berkas lain. Untuk gambar tertaut, tersedia juga **Perbarui Tautan**, **Ubah Tautan…**, dan **Sematkan Gambar**. Lihat [Pengelola Grafis](graphics-manager).

## Terkait

- [Pengelola Grafis](graphics-manager)
- [Menggambar bentuk dan garis](shapes)
- [Memindahkan, merapikan, mengelompokkan, dan mengatur lapisan objek](arrange)
- [Tab Format Gambar](tab-picture-format)
