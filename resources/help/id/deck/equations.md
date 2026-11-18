# Persamaan
<!-- keywords: tab persamaan, struktur, simbol, konversi, persamaan, matematika, rumus, pecahan, akar kuadrat, pangkat, subskrip, eksponen, jumlah, integral, yunani, simbol, sisipkan persamaan, aksen, matriks, kurung, persamaan bawaan, galeri, ubah persamaan, edit persamaan, pptx, odp, gambar, pdf, presentasi slaid, cetak -->

Sebuah **persamaan** adalah baris matematika, seperti pecahan atau akar kuadrat. Pada slaid, persamaan adalah sebuah objek, seperti gambar atau kotak teks. Anda dapat memindahkannya, mengubah ukurannya, memutarnya, menyalinnya, dan menghapusnya.

## Memasukkan persamaan ke slaid

1. Klik **Sisipkan > Simbol > Persamaan** (atau tekan **Alt+=**).
2. Ketik persamaan dalam satu baris. Di bawah kotak, Anda melihat tampilannya nanti. Klik **OK**.

Persamaan akan muncul di tengah slaid, dalam keadaan terpilih. Huruf-hurufnya kira-kira setinggi judul. Persamaan yang sangat lebar akan diperkecil agar muat di slaid. Untuk memulai dari persamaan yang sudah dikenal, klik **Persamaan Bawaan** lalu pilih satu, misalnya rumus kuadrat atau luas lingkaran.

## Cara mengetik

- **Pecahan:** ketik garis miring. `a/b` berarti a per b. `(x+1)/(x-1)` memperlakukan setiap kelompok dalam kurung sebagai satu bagian.
- **Pangkat dan subskrip:** `x^2` berarti x kuadrat, dan `x_i` berarti x dengan i kecil di bawahnya.
- **Akar:** ketik `sqrt(x+1)` untuk akar kuadrat.
- **Jumlah, integral, dan perkalian:** ketik `\sum_(i=1)^n i`, `\int_0^1 x`, atau `\prod_(k=1)^n k`.
- **Simbol:** ketik garis miring terbalik lalu nama. `\alpha` menjadi α, `\pi` menjadi π, `\infty` menjadi ∞, dan `\leq` menjadi ≤.
- **Aksen, kurung, dan matriks:** `\hat{x}`, `\left( ... \right)`, dan `\pmatrix{a & b \\ c & d}`.

## Mengetik dalam UnicodeMath atau LaTeX

Di samping kotak terdapat daftar format pengetikan: **Baris Sederhana** (yang di atas), **UnicodeMath**, dan **LaTeX**. Keduanya adalah dua cara menulis matematika sebagai teks biasa yang sudah dikenal banyak orang. Saat Anda memilih format lain, apa yang sudah Anda ketik akan ditulis ulang dalam format tersebut.

- **UnicodeMath:** `a/b` adalah pecahan, `x^2_i` memiliki pangkat dan subskrip, `√(x)` adalah akar, dan `∑_(i=1)^n i` adalah jumlah. Letakkan kata di antara tanda kutip, seperti `"jika x"`, maka kata itu tetap sebagai teks biasa, bukan huruf matematika. `(a|b)` membuat kurung dengan dua bagian, dan `\matrix(a&b@c&d)` membuat matriks (`&` di antara sel, `@` di antara baris).
- **LaTeX:** `\frac{a}{b}` adalah pecahan, `\sqrt[3]{x}` adalah akar pangkat tiga, `\sum_{i=1}^n i` adalah jumlah, `\text{jika x}` adalah teks biasa, dan `\begin{pmatrix} a & b \\ c & d \end{pmatrix}` adalah matriks.

Dua tombol **Profesional** dan **Linear** mengubah cara kotak menampilkan persamaan Anda. Profesional menggambarnya secara bertingkat, seperti tampilan akhirnya. Linear hanya menampilkan baris yang Anda ketik. Jika sebuah persamaan tidak dapat ditulis dalam format yang Anda pilih, kotak akan memberi tahu Anda dan tetap menyimpan apa yang sudah Anda ketik.

## Mengubah, memindahkan, dan mengubah ukuran

Klik dua kali persamaan, atau pilih persamaan lalu klik **Sisipkan > Simbol > Persamaan**, untuk membuka kotak dengan barisnya. Ubah barisnya lalu klik **OK**. Persamaan tetap di tempatnya dan mempertahankan ukurannya, dan perubahan ini dapat dibatalkan dalam satu langkah. Seret persamaan untuk memindahkannya. Seret pegangan untuk mengubah ukurannya; persamaan mempertahankan proporsinya di dalam kotaknya.

Persamaan yang berasal dari berkas juga dapat diubah. Sampai Anda mengubahnya, JeffDeck menampilkan gambar yang disimpan berkas dan menyimpannya kembali persis seperti semula. Klik dua kali (atau pilih lalu klik **Persamaan**). Kotak akan dibuka dengan satu baris yang menjelaskan persamaan tersebut. Jika Anda mengubah baris itu, gambar akan menjadi persamaan di tempat yang sama, dan JeffDeck menyusun ulang matematikanya dari apa yang Anda ketik, sehingga font khusus di matematika lama akan hilang. Jika Anda mengklik **OK** tanpa mengubah baris itu, tidak ada yang berubah.

## Tab Persamaan

Pilih sebuah persamaan, dan tab bernama **Persamaan** akan muncul di ujung pita. Tab ini hilang saat Anda memilih hal lain.

- **Alat:** **Edit Persamaan** membuka kotak. **Profesional** dan **Linear** mengubah cara persamaan yang dipilih ditampilkan di slaid: Profesional menggambarnya secara bertingkat, dan Linear menampilkan baris tempat persamaan itu diketik. Pilihan ini hanya melekat pada persamaan selama presentasi terbuka. Berkas `.pptx` yang disimpan selalu menyimpan persamaan dalam bentuk bertingkat, sehingga saat dibuka kembali akan tampil seperti itu. **Teks Normal** membuka kotak dengan sepasang tanda kutip ditambahkan, untuk kata-kata yang bukan matematika.
- **Konversi:** pilih format tempat kotak dibuka: **Baris Sederhana**, **UnicodeMath**, atau **LaTeX**. Saat Anda mengedit persamaan berikutnya, persamaan itu akan ditulis dalam format yang Anda pilih. Persamaan yang tidak dapat ditulis dengan baris sederhana, seperti jumlah dengan batasnya di atas dan di bawah, akan dibuka dalam UnicodeMath.
- **Simbol:** huruf Yunani, operator, relasi, panah, serta himpunan dan logika. Pilih satu simbol, dan kotak akan dibuka dengan simbol itu sudah ditambahkan ke baris.
- **Struktur:** pecahan, indeks, radikal, integral, operator besar, kurung, fungsi, aksen, limit, operator, dan matriks. Klik tombol untuk bentuk biasanya, atau panah di sampingnya untuk bentuk lain, seperti turunan, akar dengan pangkat, atau matriks identitas.

Struktur ditambahkan di akhir baris, dengan huruf contoh di dalamnya, dan bagian pertama dipilih sehingga Anda dapat langsung mengetik di atasnya. Kotak memiliki struktur yang sama dalam deretan tombol kecil di atas baris. Di sana, struktur ditambahkan pada posisi kursor, dan jika sebagian baris dipilih, bagian itu masuk ke posisi pertama struktur.

## Presentasi slaid, pencetakan, dan PDF

Presentasi slaid, pencetakan, dan berkas PDF menggambar persamaan sebagai bentuk, sehingga tetap tajam pada ukuran berapa pun.

## Menyimpan

- **.pptx** menyimpan persamaan sebagai persamaan, dalam format standar untuk matematika, dengan gambar persamaan di sebelahnya untuk program yang tidak dapat membaca matematika. Saat Anda membuka berkas itu lagi, JeffDeck menampilkan gambar dan tetap menyimpan matematikanya, dan Anda tetap dapat mengubahnya seperti dijelaskan di atas. Sebuah presentasi memuat paling banyak 2.000 persamaan dan 16 MB matematika secara keseluruhan. Melebihi batas itu, JeffDeck tidak akan menambahkan persamaan lain.
- **.odp** tidak menyimpan persamaan dalam salinan JeffDeck. Setiap persamaan disimpan sebagai gambar, di tempat yang sama, dan pesan setelah penyimpanan akan memberi tahu Anda.
- Kotak teks dengan kata-kata di sekitar matematikanya bukan persamaan di sini. Kotak itu ditampilkan sebagai gambar yang disimpan berkas dan disimpan kembali secara utuh.

## Terkait

- [Bentuk, gambar, tabel, dan Seni Teks](objects)
- [Tab Sisipkan](tab-insert)
