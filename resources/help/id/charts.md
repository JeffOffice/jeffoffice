# Bagan
<!-- keywords: bagan, grafik, diagram, bagan kolom, bagan batang, bagan garis, bagan lingkaran, bagan donat, bagan area, sebar, sisipkan bagan, data bagan, edit data, judul bagan, keterangan, label data, jenis bagan, format bagan, sumbu, judul sumbu, garis kisi, gaya bagan, garis tren, bilah kesalahan, warna bagan, tata letak cepat, format bagian bagan, bagan saham, kolom bertumpuk, bagan radar, bagan pivot]]>

Sebuah **bagan** mengubah angka menjadi gambar, seperti kolom, garis, atau irisan lingkaran. Topik ini menjelaskan cara menempatkan bagan pada halaman, cara mengetik angka Anda sendiri, dan cara mengubah tampilannya.

## Menempatkan bagan pada halaman

1. Buka halaman tempat bagan akan diletakkan.
2. Klik **Sisipkan > Bagan**, lalu pilih jenisnya: kolom, kolom bertumpuk, kolom 100% bertumpuk, batang, batang bertumpuk, garis, garis dengan penanda, lingkaran, donat, area, area bertumpuk, sebar, radar, radar dengan penanda, radar terisi, tinggi-rendah-tutup, buka-tinggi-rendah-tutup, volume-tinggi-rendah-tutup, atau volume-buka-tinggi-rendah-tutup.

Bagan **saham** menunjukkan harga: sebuah garis untuk setiap kategori dari harga tertinggi hingga terendah, dengan titik kecil untuk harga penutupan, atau, jika ada harga pembukaan, sebuah batang di antara harga pembukaan dan penutupan (terang untuk kenaikan, gelap untuk penurunan). Bagan dibuat dari kolom Anda persis seperti urutannya, dari kiri ke kanan, dan tidak ada pemeriksaan apakah angkanya sesuai dengan posisinya. Karena itu, susunlah angkanya dalam urutan ini: tinggi, rendah, dan tutup; buka, tinggi, rendah, dan tutup; volume, tinggi, rendah, dan tutup; atau volume, buka, tinggi, rendah, dan tutup. Jika kolomnya dalam urutan lain, bagan tetap digambar, tetapi tandanya berarti hal lain (titik penutupan berada pada kolom harga ketiga, dan kotaknya membentang dari kolom harga pertama ke kolom harga terakhir). Pada bagan volume, volume dibaca sebagai kolom pada sumbu kiri, dan harga dibaca pada sumbu kedua di sebelah kanan. Jika jumlah kolom tidak sesuai dengan jenisnya, JeffPub memberi tahu berapa banyak kolom yang dibutuhkan dan tidak membuat bagan.

Bagan **bertumpuk** menumpuk seri satu di atas yang lain. Bagan **100% bertumpuk** membuat setiap tumpukan memiliki tinggi yang sama, sehingga Anda melihat proporsinya. Bagan **sebar** menempatkan setiap titik sesuai dengan dua angkanya.

Bagan muncul dengan angka contoh, di tengah halaman, dan tab **Format Bagan** terbuka. Bagan adalah objek seperti gambar atau bentuk: seret untuk memindahkannya, seret pegangan untuk mengubah ukurannya (bagan digambar ulang agar pas, sehingga tulisannya tetap berbentuk benar), dan putar dengan **Putar** pada grup **Atur**.

## Mengetik angka Anda sendiri

Pilih bagan, lalu klik **Format Bagan > Edit Data**. Sebuah tabel akan terbuka.

- Kolom pertama berisi kata-kata di bawah titik-titik (**kategori**), misalnya nama bulan.
- Baris pertama berisi nama setiap **seri**, yaitu satu kumpulan angka, misalnya satu produk.
- Sel lainnya berisi angkanya.

Klik **Tambah Baris** atau **Tambah Kolom** untuk menambah, atau **Hapus Baris Terakhir** atau **Hapus Kolom Terakhir** untuk mengurangi. Klik **OK**. Jika sebuah sel bukan angka, bagan tidak berubah, dan bilah status memberi tahu baris mana yang bermasalah. Sebuah bagan dapat memiliki hingga 200 kategori dan 24 seri.

Untuk bagan sebar, kolom pertama berisi angka yang menempatkan titik dari kiri ke kanan.

## Mengubah tampilannya

Perintah-perintah ini berada pada tab **Format Bagan**. Setiap perintah dapat dibatalkan dalam satu langkah, dan bagan tetap mempertahankan warnanya serta tampilan lainnya.

- **Ubah Jenis Bagan** mengubah bagan menjadi jenis lain dengan angka yang sama. Jika diubah ke jenis saham, seri bagan harus berada dalam urutan yang diminta jenis tersebut; jika tidak, JeffPub memberi tahu urutannya dan membiarkan bagan apa adanya.
- **Judul Bagan** menampilkan atau menyembunyikan judul. **Edit Judul Bagan** mengubah kata-katanya.
- **Keterangan** adalah kunci yang menunjukkan warna mana untuk seri mana. Letakkan di kanan, atas, kiri, atau bawah, atau hilangkan.
- **Sumbu** menampilkan atau menyembunyikan garis kata-kata di bawah bagan (**kategori**) dan garis angka di sampingnya (**nilai**), serta memberi judul pada masing-masing. Bagan lingkaran tidak memiliki sumbu.
- **Garis Kisi** menggambar garis tipis melintasi bagan agar mata Anda lebih mudah mengikuti angkanya. Garis **utama** berada pada angka utama di sisi, dan garis **kecil** berada di antaranya.
- **Gaya Bagan** memilih salah satu dari delapan gaya keseluruhan.
- **Label Data** menuliskan setiap angka pada kolom, titik, atau irisannya.

## Menambahkan garis tren, bilah kesalahan, dan warna

Pilih bagan. Tiga tombol kecil berada di sampingnya, dan perintah yang sama juga ada pada tab **Format Bagan**.

- **Bagian Bagan** (tombol plus) menambah atau menghapus sebuah bagian bagan: judul, label, tabel angka, dan lainnya. Entri **Garis Tren** menggambar garis melalui titik-titik yang menunjukkan arah tujuan titik-titik tersebut (linear, eksponensial, dan jenis lainnya), dan dapat menuliskan persamaan garisnya pada bagan. Entri **Bilah Kesalahan** menggambar batang kecil pada setiap titik untuk menunjukkan seberapa jauh angka sebenarnya mungkin berbeda dari angka yang ditampilkan: jumlah tetap, persentase, atau sebaran angka.
- **Tata Letak** memilih salah satu dari beberapa penempatan siap pakai untuk judul, keterangan, dan label.
- **Warna** mengubah warna yang digunakan untuk menggambar seri. **Gaya** (tombol kuas) memilih tampilan keseluruhan.
- **Filter Bagan** (tombol corong) menyembunyikan sebuah seri tanpa menghapus angkanya.
- **Format Bagan** membuka jendela untuk tampilan satu bagian: seluruh bagan, area plot, atau satu seri. **Format Sumbu Horizontal** dan **Format Sumbu Vertikal** mengatur rentang, langkah, dan format angka sebuah sumbu.
- **Pilihan Saat Ini** mencantumkan bagian-bagian bagan: area bagan, area plot, dan setiap seri. Pilih salah satu, lalu **Format Pilihan** membuka jendela format pada bagian tersebut. **Atur Ulang agar Sesuai Gaya** menghapus pengaturan yang dibuat pada bagian yang dipilih dan mengembalikan apa yang diberikan oleh gaya bagan. Untuk area bagan, yang dikembalikan adalah isiannya, garisnya, dan warna teks yang diberikan oleh sebuah tampilan (teks terang pada tampilan gelap kembali ke warna teks tema). Untuk area plot, yang dikembalikan adalah isian dan garisnya. Untuk sebuah seri, yang dikembalikan adalah warnanya (warna yang diberikan oleh pilihan Warna atau gaya bagan), garis tepinya, penandanya sesuai jenis bagan, dan warna titik-titiknya; garis halus tetap halus jika jenis bagannya memang halus. Perintah ini tidak mengubah judul, keterangan, label, sumbu, garis kisi, atau data, dan tidak berpengaruh pada bagian yang sudah sesuai. Perintah ini adalah satu langkah Urungkan.
- **Data Bagan** dan **Tukar Baris dan Kolom** mengubah angka mana yang menjadi seri dan mana yang menjadi kategori.

Setiap perintah tersebut dapat dibatalkan dalam satu langkah.

## Menyimpan, mencetak, dan PDF

- Berkas **.jpub** menyimpan bagan sebagai bagan, sehingga Anda dapat membukanya nanti dan mengubah angkanya.
- Berkas **.pub** tidak dapat menyimpan bagan yang dibuka sebagai bagan oleh program lain. JeffPub menyimpan bagan sebagai gambar dari bagan tersebut, dan memberi tahu Anda berapa banyak bagan yang disimpan dengan cara itu. Simpan salinan **.jpub** jika Anda ingin mengubah angkanya nanti.
- **PDF** dan pencetakan menggambar bagan sebagai garis, bentuk, dan teks asli, bukan sebagai gambar, sehingga tetap tajam pada ukuran berapa pun.
