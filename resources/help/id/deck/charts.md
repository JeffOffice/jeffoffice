# Bagan

<!-- keywords: bagan, chart, grafik, bagan kolom, bagan batang, bagan garis, bagan lingkaran, bagan donat, area, sebaran, sisipkan bagan, data bagan, edit data, judul bagan, keterangan, label data, jenis bagan, format bagan, sumbu, judul sumbu, garis kisi, gaya bagan, bagian bagan, tata letak, warna, tampilan, tukar baris dan kolom, jenis bagan, format sumbu, bilah kesalahan, garis tren, filter bagan, elemen bagan -->

Sebuah **bagan** mengubah angka menjadi gambar, seperti kolom, garis, atau irisan lingkaran. Topik ini menunjukkan cara memasukkan bagan ke slaid, cara mengetik angkanya, dan cara mengubah tampilannya.

## Memasukkan bagan ke slaid

1. Buka slaid tempat bagan akan diletakkan.
2. Klik **Sisipkan > Bagan**, lalu pilih jenisnya: kolom, kolom bertumpuk, kolom bertumpuk 100%, batang, batang bertumpuk, garis, garis dengan penanda, lingkaran, donat, area, area bertumpuk, sebar, radar, radar dengan penanda, radar terisi, high-low-close, open-high-low-close, volume-high-low-close, atau volume-open-high-low-close.

Bagan **saham** menunjukkan harga: sebuah garis untuk setiap kategori dari harga tertinggi ke harga terendah, dengan titik kecil untuk penutupan, atau, jika ada pembukaan, sebuah batang di antara pembukaan dan penutupan (terang untuk kenaikan, gelap untuk penurunan). Bagan dibuat dari kolom Anda persis seperti urutannya, dari kiri ke kanan, dan tidak ada pemeriksaan apakah angkanya sesuai dengan posisinya, jadi tempatkan dalam urutan ini: tinggi, rendah, dan tutup; buka, tinggi, rendah, dan tutup; volume, tinggi, rendah, dan tutup; atau volume, buka, tinggi, rendah, dan tutup. Jika kolom berada dalam urutan lain, bagan tetap digambar, tetapi tandanya memiliki arti lain (titik penutupan berada pada kolom harga ketiga, dan kotaknya membentang dari kolom harga pertama ke kolom harga terakhir). Dengan volume, volume dibaca sebagai kolom pada sumbu kiri dan harga dibaca pada sumbu kedua di sebelah kanan. Jika jumlah kolom tidak sesuai dengan jenisnya, JeffOffice memberi tahu berapa kolom yang dibutuhkan dan tidak membuat bagan.

Bagan **bertumpuk** menumpuk seri di atas satu sama lain. Bagan **bertumpuk 100%** membuat setiap tumpukan memiliki tinggi yang sama, sehingga Anda melihat proporsinya. Bagan **sebar** menempatkan setiap titik sesuai dengan dua angkanya.

Bagan muncul dengan angka contoh, di tengah slaid, dan tab **Format Bagan** terbuka. Bagan adalah objek seperti gambar atau bentuk: seret untuk memindahkannya, seret pegangan untuk mengubah ukurannya (bagan digambar ulang agar sesuai, sehingga tulisannya tetap berbentuk benar), dan putar dengan **Putar** di grup **Gambar** pada tab **Beranda**.

## Mengetik angka Anda sendiri

Pilih bagan lalu klik **Format Bagan > Edit Data**. Sebuah tabel terbuka.

- Kolom pertama berisi kata-kata di bawah titik (**kategori**), seperti nama bulan.
- Baris pertama berisi nama setiap **seri**, yaitu satu kumpulan angka, misalnya satu produk.
- Sel lainnya berisi angka.

Klik **Tambah Baris** atau **Tambah Kolom** untuk menambah, atau **Hapus Baris Terakhir** atau **Hapus Kolom Terakhir** untuk mengurangi. Klik **OK**. Jika sebuah sel bukan angka, bagan tidak diubah, dan bilah status memberi tahu baris mana yang bermasalah. Sebuah bagan dapat memiliki hingga 200 kategori dan 24 seri.

Untuk bagan sebar, kolom pertama berisi angka yang menempatkan titik dari kiri ke kanan.

## Bagan yang disimpan program lain

Beberapa bagan yang disimpan program lain menyimpan angkanya di dalam sebuah **buku kerja** kecil (berkas lembar kerja) yang tersimpan di dalam berkas Anda. Pilih bagan seperti itu lalu klik **Format Bagan > Edit Data**. Tabel menampilkan sel-sel buku kerja tersebut yang dibaca bagan. Ubah sebuah angka, nama di bawah titik, atau nama seri, lalu klik **OK**. Perubahan masuk ke buku kerja dan ke bagan sekaligus, sebagai satu langkah yang dapat diurungkan, sehingga program lain juga menampilkan angka baru.

Anda tidak dapat menambah atau menghapus baris atau kolom pada bagan seperti ini, dan **Data Bagan** serta **Tukar Baris dan Kolom** dibuat redup, karena bagan terikat pada sel-sel tersebut. Hal yang sama berlaku jika bagan membaca sel dalam berkas lain, dalam blok yang bukan satu kolom atau satu baris, atau pada sel yang berisi rumus, tanggal, atau kata-kata di tempat yang seharusnya berisi angka. Perintahnya dibuat redup, dan tips-nya menjelaskan alasannya.

## Mengubah tampilannya

Perintah-perintah ini ada pada tab **Format Bagan**. Setiap perintah adalah satu langkah yang dapat diurungkan, dan bagan tetap mempertahankan warnanya serta tampilan lainnya.

- **Ubah Jenis Bagan** mengubah bagan menjadi jenis lain dengan angka yang sama. Jika diubah ke jenis saham, seri bagan harus berada dalam urutan yang diminta jenis tersebut; jika tidak, program memberi tahu urutannya dan membiarkan bagan apa adanya.
- **Judul Bagan** menampilkan atau menyembunyikan judul. **Edit Judul Bagan** mengubah kata-katanya.
- **Keterangan** adalah kunci yang menunjukkan warna mana untuk seri mana. Letakkan di kanan, atas, kiri, atau bawah, atau hilangkan.
- **Sumbu** menampilkan atau menyembunyikan garis kata-kata di bawah bagan (**kategori**) dan garis angka di sampingnya (**nilai**), serta menetapkan judul untuk masing-masing. Bagan lingkaran tidak memiliki sumbu.
- **Garis Kisi** menggambar garis tipis melintang pada bagan untuk membantu mata Anda mengikuti angka. Garis **utama** berada pada angka-angka utama di sisi, dan garis **kecil** berada di antaranya.
- **Gaya Bagan** memilih salah satu dari delapan gaya keseluruhan.
- **Label Data** menuliskan setiap angka pada kolom, titik, atau irisannya.

## Tab Bagan

Pilih bagan dan tab **Bagan** terbuka di samping tab **Format Bagan**. Semua yang ada di dalamnya mengubah bagan dalam satu langkah yang dapat Anda urungkan.

- **Bagian Bagan** menambah, menghapus, atau menempatkan bagian bagan: **Sumbu**, **Judul Sumbu**, **Judul Bagan**, **Label Data** (isinya, letaknya, dan tanda di antaranya), **Garis Kisi**, **Keterangan**, **Bilah Kesalahan** (batang pendek yang menunjukkan seberapa jauh sebuah angka mungkin meleset), **Garis** (garis jatuh, garis tinggi-rendah, dan batang naik-turun), **Tabel Data** (kisi angka di bawah bagan, dengan kolom untuk setiap kategori; daftarnya menyalakan dan mematikan tabel, serta mengatur garis di antara baris, garis di antara kolom, garis tepi, dan kunci keterangan, yaitu warna setiap seri di sebelah namanya; tersedia untuk bagan kolom, garis, dan area), dan **Garis Tren** (garis yang menunjukkan arah angka menuju, dengan persamaannya jika diinginkan).
- **Tata Letak** mengatur beberapa bagian sekaligus, seperti judul, keterangan, label, dan garis kisi.
- **Warna** memberi seri kumpulan warna baru. **Tampilan** memberi seluruh bagan sebuah tampilan, seperti polos, bersih, gelap, atau lembut.
- **Tukar Baris dan Kolom** membalik angka: seri menjadi kata-kata di bawah titik, dan kata-kata tersebut menjadi seri.
- **Data Bagan** mendaftar seri. Centang seri yang akan digambar, ubah namanya, dan pindahkan ke atas atau ke bawah.
- **Edit Data** membuka tabel angka.
- **Jenis Bagan** menampilkan gambar setiap jenis. Pilih satu jenis untuk seluruh bagan, atau pilih jenis untuk setiap seri, misalnya kolom dengan garis di atasnya.
- **Format Bagian Bagan** mengatur isian dan garis dari halaman bagan, dari area plot (kotak tempat tanda-tanda berada), atau dari satu seri. Untuk garis, juga diatur apakah garis dihaluskan dan tanda apa yang berada di setiap titik.
- **Pilihan Saat Ini** mendaftar bagian bagan: area bagan, area plot, dan setiap seri. Pilih satu, dan **Format Pilihan** membuka jendela format pada bagian tersebut. **Atur Ulang agar Sesuai Gaya** menghapus apa yang telah diatur pada bagian yang dipilih dan mengembalikan apa yang diberikan gaya bagan kepadanya. Untuk area bagan, itu adalah isian, garisnya, dan warna teks yang diatur oleh sebuah tampilan (teks terang pada tampilan gelap kembali ke warna teks tema). Untuk area plot, itu adalah isian dan garisnya. Untuk seri, itu adalah warnanya (yang diberikan oleh pilihan Warna atau gaya bagan), garis tepinya, tandanya sesuai jenis bagan, dan warna titik-titiknya; garis halus tetap halus jika jenis bagannya memang halus. Ini tidak mengubah judul, keterangan, label, sumbu, garis kisi, atau data, dan tidak berpengaruh pada bagian yang sudah sesuai. Ini adalah satu langkah Urungkan.
- **Format Sumbu Horizontal** dan **Format Sumbu Vertikal** mengatur angka terendah dan tertinggi, ukuran setiap langkah, urutannya, tempat sumbu lain memotong, tampilan angka, dan tanda skala.

## Tombol di samping bagan

Tiga tombol kecil berada di sisi kanan bagan yang dipilih.

- **Elemen Bagan** (tanda plus) membuka menu yang sama dengan **Bagian Bagan**.
- **Warna dan Gaya Bagan** (kuas) mendaftar kumpulan warna dan tampilan.
- **Filter Bagan** (corong) mendaftar seri. Hilangkan centang pada seri untuk tidak menampilkannya dalam gambar. Berkas tetap menyimpannya, sehingga seri itu kembali saat Anda mencentangnya lagi. Bagan selalu menampilkan setidaknya satu seri.

## Bagan dalam presentasi lain

Bagan dalam berkas **.pptx** terbuka sebagai bagan yang dapat diubah, jika angkanya tersimpan di dalam bagan. Jika bagan membaca angkanya dari lembar kerja yang disimpan bersamanya, Anda dapat mengubah semuanya kecuali angkanya. Bagan dengan bagian yang tidak diubah JeffDeck, seperti efek khusus, dibiarkan apa adanya.

## Menyimpan, mencetak, dan PDF

- Berkas **.pptx** menyimpan bagan sebagai bagan, sehingga terbuka sebagai bagan di sini dan di program lain.
- Berkas **.odp** tidak memiliki tempat untuk bagan yang dapat terbuka sebagai bagan di sini. JeffDeck menyimpan bagan sebagai gambar, dan pesan setelah penyimpanan memberi tahu hal itu.
- **PDF** dan pencetakan menggambar bagan sebagai garis, bentuk, dan teks asli, bukan sebagai gambar, sehingga tetap tajam pada ukuran berapa pun.
