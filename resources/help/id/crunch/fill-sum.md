# Mengisi sel dan menjumlahkannya
<!-- keywords: isi, seri, seret, pegangan, klik dua kali, daftar kustom, Isi Otomatis, Ctrl+D, Ctrl+R, bawah, kanan, atas, kiri, angka, tanggal, hari kerja, bulan, tahun, langkah, berhenti, salin, jumlah, Sum, sum, rata-rata, hitung, maks, min, Alt+sama dengan, total, Isi Kilat, Ctrl+E, pola, pisah nama -->

## Mengisi

Mengisi melanjutkan apa yang sudah Anda mulai. Pilih sel yang sudah Anda isi, lalu seret kotak kecil di sudut pilihan (**pegangan isi**) sepanjang kolom atau baris, lalu lepaskan. Garis putus-putus menunjukkan sampai mana isian akan berlanjut.

| Yang Anda mulai dengan | Hasil pengisian |
| --- | --- |
| 1, 2 | 3, 4, 5, dan seterusnya: selisih di antara keduanya dilanjutkan |
| Satu angka, misalnya 5 | 5, 5, 5: disalin (tahan Ctrl saat menyeret untuk pilihan kebalikannya) |
| Sebuah tanggal | Hari-hari berikutnya (gunakan tombol **Opsi Isian**, atau **Isi > Seri**, untuk hari kerja, bulan, atau tahun) |
| Senin, atau Januari | Selasa, Rabu, ... atau Februari, Maret, ... dalam bahasa Anda |
| Item 1 | Item 2, Item 3, ... |
| Sebuah kata dari daftar kustom, misalnya Kecil | Kata berikutnya dalam daftar, dan kembali ke yang pertama setelah yang terakhir |
| Sebuah rumus | Rumus disalin, dengan referensinya berpindah seperti pada tempel |

Klik dua kali pegangan isi untuk mengisi ke bawah sejauh data di sebelah pilihan. JeffCrunch memeriksa kolom di sebelah kiri terlebih dahulu, lalu kolom di sebelah kanan. Pengisian berhenti pada sel kosong pertama di kolom tersebut.

## Daftar kustom

**Daftar kustom** adalah deretan kata yang dilalui Isi, seperti hari dalam seminggu. Untuk membuatnya, klik **Berkas > Opsi > Daftar Kustom**. Ketik satu daftar di setiap baris, dengan koma di antara kata-katanya, misalnya `Kecil, Sedang, Besar`. Klik **OK**. Sekarang ketik **Kecil** di sebuah sel dan seret pegangan isi: sel-sel akan terisi Sedang, Besar, Kecil, dan seterusnya. Huruf besar dan kecil tidak berpengaruh saat JeffCrunch mencari kata dalam daftar, dan sel baru akan menyalin huruf besar dari sel awal Anda. Sebuah daftar memerlukan setidaknya dua kata. Nama hari dalam seminggu dan nama bulan sudah tersedia secara bawaan.

**Isi > Ke Bawah** (**Ctrl+D**) dan **Isi > Ke Kanan** (**Ctrl+R**) menyalin baris atau kolom pertama pilihan ke bagian lainnya. **Isi > Ke Atas** dan **Isi > Ke Kiri** bekerja dari baris atau kolom terakhir. **Isi > Seri** meminta jenisnya (linear, pertumbuhan, tanggal), satuan tanggal, langkah, dan tempat berhenti. Setiap sel yang terisi mengambil format sel yang mendahuluinya. Sel yang tidak dapat diubah (bagian dari larik, sel terkunci, atau judul tabel) akan menghentikan pengisian, dan tidak ada yang berubah.

Setelah Anda menyeret pegangan isi, tombol kecil **Opsi Isian** muncul di sudut area yang diisi. Menunya mengulangi pengisian dengan cara lain, dan pengisian tetap merupakan satu langkah Urungkan: **Salin Sel**, **Isi Seri**, **Isi Pemformatan Saja** (sel tetap menyimpan isinya dan mengambil format), **Isi Tanpa Pemformatan** (sel mempertahankan formatnya sendiri), serta, untuk tanggal, **Isi Hari**, **Isi Hari Kerja**, **Isi Bulan**, dan **Isi Tahun**. Tombol ini hilang saat Anda melakukan hal lain.

Pengisian tidak masuk ke atau keluar dari sel gabungan, karena akan meninggalkan nilai di sel yang tersembunyi di dalam area gabungan. Pisahkan sel tersebut terlebih dahulu.

**Isi > Rata Kiri-Kanan** menata teks dari sel-sel yang dipilih pada satu kolom menjadi baris-baris selebar kolom, mulai dari sel pertama ke bawah; sel sisa pada pilihan dikosongkan. Jika teksnya memerlukan lebih banyak sel daripada yang Anda pilih, JeffCrunch akan menanyakan apakah sel di bawahnya akan digunakan. Fitur ini hanya bekerja pada teks.

**Isi > Isi Kilat** (**Ctrl+E**) mengisi sebuah kolom untuk Anda berdasarkan pola yang Anda tunjukkan. Ketik sendiri hasil yang Anda inginkan di satu atau dua sel pertama pada sebuah kolom, di samping sel yang menjadi sumbernya. Lalu letakkan kursor di kolom tersebut dan pilih Isi Kilat. JeffCrunch memahami bagaimana contoh Anda dibuat dari sel di sebelahnya, lalu mengisi sel-sel kosong dengan cara yang sama. Contoh: kolom A berisi nama seperti **Smith, Ann**. Di B1 Anda mengetik **Ann Smith**. Isi Kilat akan mengisi B2 dan seterusnya dengan nama lain yang dibalik dengan cara yang sama. Fitur ini dapat mengambil sebuah kata dari sel, huruf pertama sebuah kata, atau beberapa huruf pertama atau terakhir, dalam huruf besar, huruf kecil, atau seperti aslinya, dan dapat menambahkan teks yang sama di setiap contoh, seperti spasi atau titik. Jika tidak menemukan pola, JeffCrunch akan memberi tahu Anda. Ketik satu atau dua contoh lagi, lalu coba lagi. Ini juga merupakan satu langkah Urungkan.

**Isi > Di Seluruh Lembar Kerja** menyalin sel-sel yang dipilih ke sel yang sama pada lembar-lembar yang Anda centang dalam daftar: semuanya, hanya isinya, atau hanya formatnya. Ini merupakan satu langkah Urungkan.

## Jumlah

**Beranda > Sum** (**Alt dan tanda sama dengan**) memasukkan total ke sel yang Anda pilih. Fitur ini melihat angka di atas sel; jika tidak ada, angka di sebelah kirinya. Rumus dimasukkan ke sel agar Anda dapat memeriksanya, dan **Enter** akan mempertahankannya. Panah di sebelah Sum memberikan total lainnya: **Rata-rata**, **Berapa Banyak Angka**, **Maks**, dan **Min**.

Pilih satu blok terlebih dahulu, dan setiap kolom dalam blok akan mendapatkan rumusnya sendiri di sel kosong di bawahnya (atau, untuk satu baris saja, di sebelah kanannya).
