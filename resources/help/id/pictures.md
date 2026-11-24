# Menambahkan dan menyesuaikan gambar
<!-- keywords: efek gambar, bayangan, pantulan, cahaya, tepi lembut, bevel, rotasi 3-D, atur ulang gambar dan ukuran, foto, citra, sisipkan gambar, potong, clip art, halaman PDF, jpg, png, svg, ubah warna, teks alternatif, keterangan, placeholder, pengelola grafis, gambar daring, tautkan ke berkas, gambar tertaut, sisipkan dan tautkan, tanamkan, gambar hilang, tautkan ulang, gambar, sisipkan gambar, tautan gambar -->

Sebuah gambar berada di dalam bingkai. Anda dapat memotong gambar, serta memindahkan atau mengubah ukuran bingkainya.

## Menyisipkan gambar dari berkas

1. Klik **Sisipkan > Ilustrasi > Gambar**.
2. Di jendela **Sisipkan Gambar**, pilih berkas. JeffPub membuka berkas PNG, JPEG, GIF, BMP, TIFF, WebP, SVG, WMF, EMF, ICO, dan PDF.
3. Pilih **Sisipkan**, **Tautkan ke Berkas**, atau **Sisipkan dan Tautkan** di kotak **Sisipkan sebagai**, yang berada bersama daftar berkas dan dapat dicapai dengan tombol Tab. **Sisipkan** dipilih setiap kali jendela dibuka. Lihat "Tautkan gambar ke berkasnya," di bawah.
4. Klik **Buka**.

Gambar akan muncul di tengah halaman. Anda juga dapat menyeret berkas gambar ke halaman, dan cara ini selalu menggunakan **Sisipkan**.

JeffPub membaca berkas gambar hingga 256 MB. Berkas yang lebih besar ditolak, begitu pula berkas kosong atau apa pun yang bukan berkas biasa di disk, seperti perangkat atau pipa. Pesannya menyatakan bahwa JeffPub tidak dapat membacanya sebagai gambar.

Jika Anda memilih beberapa gambar sekaligus, gambar-gambar tersebut akan tersusun sebagai miniatur di area coret-coret, yaitu ruang abu-abu di samping halaman. Seret gambar yang Anda inginkan ke halaman.

Jika PDF memiliki lebih dari satu halaman, jendela **Sisipkan Halaman PDF** menampilkan setiap halaman. Pilih satu dan klik **OK**.

## Tautkan gambar ke berkasnya

**Sisipkan** menyalin gambar ke dalam publikasi. Salinan itu ikut ke mana pun publikasi pergi, tetapi publikasi dengan banyak foto besar juga menjadi besar. Tautan menyimpan foto di berkasnya sendiri.

- **Sisipkan** menyalin gambar ke dalam publikasi. Ini pilihan yang umum.
- **Tautkan ke Berkas** hanya menyimpan lokasi berkas dan pratinjau kecil, dengan sisi terpanjang tidak lebih dari 512 piksel. Gambar di halaman, serta saat dicetak, dalam PDF, atau e-book, diambil dari berkas dengan kualitas penuh. Jika berkas dipindahkan atau dihapus, halaman menampilkan pratinjau kecil.
- **Sisipkan dan Tautkan** melakukan keduanya. Publikasi menyimpan gambar utuh beserta tautannya. Saat Anda membuka publikasi dan berkas telah berubah, JeffPub membaca ulang berkas dan mengganti salinannya.

JeffPub mengingat lokasi lengkap berkas, dan juga lokasinya relatif terhadap publikasi, misalnya "folder Foto di sebelahnya". Jika Anda memindahkan publikasi dan gambarnya bersama-sama, lokasi kedua yang berlaku, sehingga gambar ditemukan kembali. Biarkan folder tetap seperti semula.

JeffPub membuka berkas gambar tertaut secara otomatis hanya jika berkas itu berada di folder publikasi atau di dalam folder di dalamnya. Gambar yang ditautkan ke berkas di tempat lain menampilkan pratinjau tersimpannya, buram, dan diberi tanda **Belum diperbarui**, sampai Anda memilihnya dan mengklik **Perbarui Tautan** di Pengelola Grafis. Ini mencegah publikasi yang dikirim orang lain mengakses komputer Anda. Agar gambar dimuat otomatis, simpan gambar di folder publikasi.

**Pengelola Grafis** menunjukkan apakah setiap gambar **Tersemat**, **Tertaut**, **Hilang**, **Diubah**, atau **Belum diperbarui**, serta memiliki tombol untuk memperbarui tautan, mengarahkannya ke berkas lain, atau mengubahnya menjadi salinan. Lihat [Pengelola Grafis](graphics-manager).

Saat Anda menyimpan publikasi sebagai berkas `.pub`, membuat berkas Kemas dan Bawa, atau mengirimnya lewat email, JeffPub memasukkan setiap gambar tertaut ke dalam berkas secara utuh, sehingga tidak bergantung pada berkas yang tertinggal. Jika berkas tertaut hilang saat itu, atau gambar berstatus **Belum diperbarui**, pratinjau kecilnya yang dimasukkan. Berkas `.pub` yang Anda buka tidak pernah memiliki tautan: gambarnya masuk sebagai salinan.

## Menambahkan placeholder gambar

Placeholder adalah bingkai kosong yang menyediakan tempat untuk gambar. Klik **Sisipkan > Ilustrasi > Placeholder Gambar**, lalu klik dua kali bingkai dan pilih berkas. Gambar mengisi bingkai, dan bagian tepi yang menonjol disembunyikan. Placeholder kosong tidak tercetak.

## Mencari gambar gratis secara daring

Klik **Sisipkan > Ilustrasi > Gambar Daring**. Panel **Gambar Daring** mencari dua pustaka gambar gratis dengan lisensi terbuka: Openverse, yang mencakup banyak koleksi, dan Wikimedia Commons, pustaka gambar Wikipedia.

1. Pilih pustaka, ketik yang Anda cari, misalnya "lumbung merah", lalu tekan Enter.
2. Klik gambar untuk melihat pembuatnya dan lisensinya. Lisensi adalah aturan penggunaan gambar. Sebagian besar meminta Anda mencantumkan nama pembuatnya.
3. Klik **Sisipkan**. JeffPub menempatkan gambar di halaman dan menggunakan judulnya sebagai teks alternatif. Dengan **Tambahkan kredit di bawah gambar** dicentang, baris seperti "Lumbung merah" oleh Pat Lee, CC BY 2.0 akan ditambahkan di bawahnya.

Kata pencarian Anda dikirim ke pustaka yang Anda pilih. Periksa bahwa lisensi setiap gambar sesuai dengan cara Anda akan menggunakannya.

Pencarian menampilkan hingga 24 gambar. JeffPub hanya menggunakan alamat web aman (https) dari sebuah pustaka, dan membuang gambar yang alamatnya tidak aman. Dari Wikimedia Commons, gambar yang lebarnya lebih dari 1.920 piksel diambil sebagai salinan selebar itu, bukan unggahan aslinya. Unduhan lebih dari 50 MB, atau yang berlangsung terlalu lama, dihentikan, dan panel menyatakan bahwa gambar tidak dapat diunduh atau dimuat.

## Memotong gambar

Memotong menyembunyikan bagian tepi gambar.

1. Klik gambar, lalu klik **Format Gambar > Potong > Potong**.
2. Seret pegangan untuk menyembunyikan sebagian gambar. Seret gambar untuk menggesernya di dalam bingkai.
3. Klik **Potong** sekali lagi, atau tekan **Esc**.

Juga di grup **Potong**: **Rasio Aspek** memotong bingkai menjadi bentuk seperti persegi, 4 lebar kali 3 tinggi, atau 16 kali 9, dari bagian tengahnya (gambar tetap di tempatnya pada halaman); **Pas** menampilkan seluruh gambar dalam bingkainya; **Isi** mengisi bingkai; **Potong ke Bentuk** memangkas gambar ke bentuk seperti oval; dan **Hapus Potongan** menampilkan kembali seluruh gambar.

## Mengubah tampilan gambar

Pada tab **Format Gambar**, gunakan grup **Sesuaikan**:

- **Hapus Latar Belakang**: menghilangkan latar belakang gambar, misalnya dinding polos di belakang orang atau area putih di sekitar logo. Klik gambar, lalu klik **Hapus Latar Belakang**. JeffPub mencari warna di sepanjang tepi gambar dan memberi warna merah muda pada bagian yang tampak sebagai latar belakang. Jika terlalu banyak atau terlalu sedikit yang dihapus, klik **Tandai Area untuk Dipertahankan** atau **Tandai Area untuk Dihapus** pada tab **Penghapusan Latar Belakang**, lalu seret di atas gambar; jika dua tanda bertemu, tanda yang Anda buat terakhir yang berlaku. **Simpan Perubahan** (atau Enter) mengakhiri dan menyimpan hasilnya, dan **Buang Semua Perubahan** (atau Esc) mengembalikan semuanya. Seluruh sesi ini dihitung sebagai satu langkah untuk diurungkan. Gambar tidak pernah terpotong: klik **Hapus Latar Belakang** lagi untuk melanjutkan penandaan, atau **Atur Ulang Gambar** untuk mengembalikan seluruh gambar. Gambar yang sangat besar menampilkan kotak dengan **Batal** selama JeffPub bekerja. Gambar berupa gambar garis dan bentuk (SVG) serta gambar animasi tidak dapat dihapus latar belakangnya. Berkas .jpub menyimpan penghapusan sehingga dapat diedit lagi. **Atur Ulang Gambar** membatalkan penghapusan yang dibuat di sini; **Ganti Gambar** tidak tersedia saat Anda menandai. Berpindah ke halaman lain, menghapus gambar, atau menyimpan akan mengakhiri Hapus Latar Belakang dan menyimpan apa yang Anda tandai. Berkas .pub dan PDF, halaman web, atau e-book hanya menyimpan gambar jadi, dan Periksa Kompatibilitas memberi tahu hal ini untuk .pub. Gambar jadi disimpan pada ukuran tampilannya, sehingga dapat terlihat kabur jika program lain memperbesarnya, dan JeffPub membangunnya kembali dari gambar asli setiap kali Anda menyimpan di sini.
- **Koreksi**: pilih pasangan kecerahan dan kontras, dari minus 40 hingga plus 40 persen. **Format Objek…**, di bagian bawah menu, memiliki penggeser.
- **Ubah Warna**: jadikan gambar Skala Abu-abu, Sepia, Pudar, Hitam Putih, atau rona aksen. **Atur Warna Transparan** membuat warna di sudut kiri atas gambar menjadi tembus pandang.
- **Transparansi…**: memudarkan seluruh gambar.
- **Atur Ulang Gambar**: menghapus semua perubahan dalam daftar ini, serta bingkai, bentuk, dan efek gambar. Potongan ikut dihapus, sehingga bingkai menampilkan seluruh gambar. Klik panah di sebelahnya untuk **Atur Ulang Gambar & Ukuran**, yang juga mengembalikan gambar ke ukuran yang dimilikinya di berkasnya.
- **Kompres Gambar…**: memperkecil berkas dengan menurunkan detail menjadi 300, 220, atau 96 ppi (piksel per inci). Secara bawaan, semua gambar diubah dan area yang dipotong dihapus.
- **Ubah ke Bentuk**: mengubah gambar SVG (gambar yang terdiri dari garis dan bentuk) menjadi bentuk yang dapat diedit, dipotong di tempat gambar memotongnya. Teks dan gambar di dalamnya tidak disertakan.

## Gaya, bingkai, dan efek gambar

Gunakan grup **Gaya Gambar** pada tab **Format Gambar**:

- **Galeri** menampilkan sekitar dua puluh lebih tampilan siap pakai: bingkai, bayangan, pantulan, tepi lembut, sudut membulat dan oval, putaran dalam ruang, cahaya, dan bevel. Bevel adalah tepi miring yang membuat gambar tampak timbul. Setiap kotak menampilkan warna Anda. Klik salah satu untuk memberi gambar tampilan itu. Gaya mengatur bentuk, bingkai, dan efek sekaligus, dan menggantikan yang dimiliki gambar. Potongan, ukuran, dan warna tetap.
- **Bingkai Gambar** mengatur warna, **Ketebalan** (seberapa tebal garisnya), dan **Garis Putus-putus** (garis padat, titik-titik, atau putus-putus). **Tanpa Garis Tepi** menghapus bingkai.
- **Efek Gambar** membuka **Bayangan**, **Pantulan**, **Cahaya**, **Tepi Lembut**, **Bevel**, dan **Rotasi 3-D**. Setiap menu memiliki pilihan siap pakai. Pilih satu untuk menambahkannya. Efek lain tetap ada. Pilih entri pertama, seperti **Tanpa Bayangan**, untuk menghapus salah satu efek. **Opsi Efek Lainnya…** membuka **Format Objek**.

Setiap perubahan ini dihitung sebagai satu langkah untuk diurungkan.

## Keterangan, teks alternatif, dan penukaran

- **Format Gambar > Gaya Gambar > Keterangan** menambahkan kotak teks di bawah gambar.
- Teks alternatif adalah deskripsi singkat untuk orang yang tidak dapat melihat gambar, misalnya pengguna pembaca layar. Klik kanan gambar, pilih **Format Objek…**, buka tab **Teks Alternatif**, lalu ketik di **Teks alternatif**.
- **Format Gambar > Sisipkan > Ganti Gambar** memasukkan berkas lain ke bingkai yang sama. Bingkai mempertahankan ukuran, tempat, dan tampilannya: bingkai, bentuk, efek, dan perubahan warna. Jika gambar baru memiliki bentuk yang sama dengan gambar lama, potongannya juga dipertahankan. Jika bentuknya berbeda, gambar baru mengisi bingkai.
- Untuk menukar dua gambar, seret salah satunya ke gambar lain. Ketika tips menampilkan **Lepaskan untuk menukar gambar**, lepaskan tombol mouse: gambar bertukar bingkai, dan bingkai yang Anda seret kembali ke tempat semula. Atau tahan Shift, klik kedua gambar, lalu klik **Tukar**. Tahan Ctrl saat menyeret untuk menyalin gambar. Salinan yang Anda lepaskan di atas gambar lain tetap berada di tempat Anda meletakkannya, dan tidak ada yang ditukar.

## Melihat semua gambar

Klik **Tampilan > Tampilkan > Pengelola Grafis** untuk mendaftar setiap gambar di panel **Pengelola Grafis**. Pilih satu gambar, lalu klik **Buka** untuk langsung menuju ke gambar itu, **Simpan Sebagai…** untuk menyimpan salinan, atau **Ganti…** untuk memilih berkas lain. Untuk gambar tertaut, tersedia juga **Perbarui Tautan**, **Ubah Tautan…**, dan **Tanamkan Gambar**. Lihat [Pengelola Grafis](graphics-manager).

## Terkait

- [Pengelola Grafis](graphics-manager)
- [Menggambar bentuk dan garis](shapes)
- [Memindahkan, merapikan, mengelompokkan, dan mengatur lapisan objek](arrange)
- [Tab Format Gambar](tab-picture-format)
