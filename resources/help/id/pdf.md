# PDF dan tempat cetak

<!-- keywords: pdf, buat pdf, simpan sebagai pdf, ekspor pdf, pdf/x, pdf/a, arsip, tempat cetak, pencetak komersial, cmyk, bleed, tanda potong, profil warna, warna spot, cetak tindih, kata sandi, enkripsi, lindungi dengan kata sandi -->

**PDF** adalah berkas yang tampil sama di setiap komputer. Gunakan PDF untuk mengirim publikasi lewat email, memasangnya di internet, atau mengirimnya ke tempat cetak.

## Membuat PDF

1. Klik **Berkas > Ekspor**, lalu **Buat PDF**.
2. Di bawah **Optimalkan untuk**, pilih kualitas. **dpi** berarti titik per inci. Semakin banyak titik, gambar semakin tajam dan berkas semakin besar.
3. Di bawah **Halaman**, pilih **Semua halaman**, **Halaman saat ini**, atau **Halaman dari** satu halaman **sampai** halaman lain.
4. Klik **Simpan PDF…**, ketik nama, lalu klik **Simpan**.

| Pilihan | Gambar | Cocok untuk |
|---|---|---|
| Ukuran minimum | 96 dpi | Melihat di layar |
| Standar | 150 dpi | Berbagi secara daring |
| Cetak kualitas tinggi | 300 dpi | Printer desktop |
| Percetakan komersial | Resolusi penuh | Tempat cetak. Menambahkan tanda potong, tanda bleed, tanda registrasi, bilah warna, dan informasi pekerjaan. |

Pilihan lainnya:

- **Sertakan properti dokumen** memasukkan judul, penulis, subjek, dan kata kunci ke dalam PDF.
- **Tag struktur dokumen untuk aksesibilitas** (aktif secara default) membuat PDF *bertag*. Tag memberi tahu pembaca layar mana yang berupa judul, paragraf, gambar, atau tabel, serta urutan membacanya: urutan objek di setiap halaman, yang dapat Anda lihat dan ubah di daftar **Urutan Pembacaan** pada Pemeriksa Aksesibilitas. Gambar dibaca bersama teks alternatifnya. Berkas PDF/X tidak memiliki tag.
- **Buat penanda** (aktif secara default) menambahkan penanda di PDF untuk setiap halaman. Nama penanda adalah nama halaman, atau "Halaman 3" untuk halaman tanpa nama. Pembaca PDF menampilkan daftarnya di samping halaman.
- **Buka PDF setelah menyimpannya** langsung menampilkan PDF di penampil Anda.
- **Lembar buklet** muncul untuk buklet. Pilihan ini menempatkan dua halaman di setiap sisi dengan urutan lipatan. Hapus centangnya untuk satu halaman per halaman PDF.

## PDF/A untuk arsip

**PDF/A** adalah PDF yang dibuat agar awet selama bertahun-tahun. PDF ini menyematkan semua font dan menggunakan warna standar, tanpa efek transparan. Centang **PDF/A untuk pengarsipan jangka panjang**, atau klik **Berkas > Ekspor**, lalu **Buat PDF/A untuk Pengarsipan**.

## PDF/X untuk tempat cetak

**PDF/X** adalah PDF yang lebih ketat dan diminta oleh tempat cetak. Warna ditulis sebagai takaran tinta, font tersemat di dalam berkas, dan area potong serta bleed diberi tanda. **Bleed** adalah gambar atau warna tambahan yang melewati tepi halaman, sehingga setelah dipotong tidak ada tepi putih.

1. Klik **Berkas > Ekspor**, lalu **Buat PDF/X untuk Pencetak Komersial**. Atau centang **PDF/X untuk pencetak komersial** di jendela **Buat PDF**. Opsi ini tidak dapat digunakan bersama PDF/A.
2. Di bawah **Kondisi pencetakan**, pilih kondisi yang disebutkan tempat cetak Anda.
3. Jika tempat cetak meminta tanda potong dan bleed, pilih juga **Percetakan komersial** di bawah **Optimalkan untuk**.
4. Klik **Simpan PDF…**.

Kondisi yang tersedia:

- **PDF/X-1a** untuk offset web AS (SWOP) atau offset Eropa (FOGRA39). Efek transparan diratakan.
- **PDF/X-4** untuk kertas berlapis premium (FOGRA51, PSO Coated v3), atau dengan profil warna printer Anda sendiri (berkas .icc yang Anda pilih). Efek transparan tetap dipertahankan.

Profil warna PSO Coated v3 tidak disertakan dengan JeffPub. Pada pertama kali digunakan, JeffPub meminta izin sebelum mengunduh profil tersebut, sekali saja, dari European Color Initiative.

Sebelum membuat berkas, JeffPub memeriksa gambar yang kurang dari 150 ppi (piksel per inci) pada ukuran cetaknya, serta font yang tidak ada di komputer ini. Klik **Tetap Buat PDF**, **Buka Pemeriksa Desain**, atau **Batal**.

## Melindungi PDF dengan kata sandi

Di jendela **Buat PDF**, klik **Enkripsi dengan kata sandi…**. Ketik kata sandi dua kali. Siapa pun yang membuka PDF tersebut harus mengetik kata sandi itu. Kata sandi benar-benar melindungi berkas: halaman diacak dengan enkripsi kuat (AES-256), dan tidak seorang pun dapat membacanya tanpa kata sandi.

Anda juga dapat mencentang **Batasi pencetakan, penyalinan, dan pengeditan**, memberi kata sandi kedua, dan memilih apa saja yang diizinkan. Batasan tersebut hanya berupa permintaan. Sebagian besar pembaca PDF menghormatinya, tetapi batasan itu tidak mencegah orang yang ingin melewatinya, jadi jangan mengandalkannya untuk menjaga kerahasiaan berkas.

Kata sandi yang terlupa tidak dapat dipulihkan, jadi simpan di tempat yang aman. Kata sandi hanya dipakai untuk PDF tersebut dan tidak pernah disimpan. PDF yang berkata sandi tidak dapat berupa PDF/A atau PDF/X, jadi kedua opsi itu dinonaktifkan selama kata sandi diatur. Di **Berkas > Bagikan**, **Enkripsi dengan kata sandi…** di atas **Kirim sebagai PDF** memberi kata sandi pada PDF yang dikirim lewat email.

Saat Anda menempatkan PDF yang berkata sandi sebagai gambar, JeffPub meminta kata sandinya. Jika salah, JeffPub meminta lagi; klik **Batal** untuk berhenti. Halaman di publikasi Anda berupa salinan dan tidak memerlukan kata sandi. Jika pemilik PDF meminta agar berkas tidak disalin, JeffPub tetap menempatkannya dan menampilkan catatan mengenai hal itu; permintaan tersebut bukan kunci.

Gambar yang ditautkan ke PDF berkata sandi akan ditampilkan sebagai **Belum diperbarui** saat Anda membuka publikasi, dan JeffPub tidak pernah meminta kata sandi dengan sendirinya. Klik **Perbarui Tautan** untuk diminta kata sandi.

## Mengatur warna untuk tempat cetak

Klik **Berkas > Info**, lalu kartu **Pengaturan Cetak Komersial**:

- **Model warna** adalah **RGB**, **Warna tunggal (spot)**, **Warna spot**, **Warna proses (CMYK)**, atau **Warna proses ditambah warna spot**. Warna proses adalah tinta sian, magenta, kuning, dan hitam. Warna spot adalah tinta khusus yang dicampur oleh tempat cetak. Tambahkan setiap warna di bawah **Warna spot**.
- **Cetak tindih hitam dalam berkas untuk printer (PDF/X)** membuat warna hitam tercetak di atas warna di bawahnya, sehingga jika pelat bergeser tidak ada tepi putih. Anda memilih apa yang dicakupnya: teks di bawah ukuran tertentu, garis, atau isian.

## Terkait

- [Mencetak publikasi](printing)
- [Simpan sebagai gambar, halaman web, atau e-book](export)
- [Periksa desain Anda](design-checker)
