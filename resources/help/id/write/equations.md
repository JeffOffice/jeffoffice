# Persamaan
<!-- keywords: tab persamaan, struktur, simbol, konversi, persamaan, rumus, matematika, formula, pecahan, akar kuadrat, superskrip, subskrip, eksponen, jumlah, sum, integral, yunani, simbol, sisipkan persamaan, alt+=, aksen, topi, vektor, bar, matriks, kurung, persamaan bawaan, galeri, ubah persamaan, edit persamaan -->

Sebuah **persamaan** adalah satu baris matematika, misalnya pecahan atau akar kuadrat. Topik ini menunjukkan cara mengetik persamaan dan memasukkannya ke dalam dokumen.

## Memasukkan persamaan

1. Letakkan kursor di tempat persamaan akan dimasukkan.
2. Klik **Sisipkan > Persamaan**, atau tekan **Alt+=**.
3. Ketik persamaan dalam satu baris. Di bawah kotak, Anda melihat persamaan yang digambar seperti tampilan akhirnya, serta baris teks yang akan ditampilkan halaman. Untuk memulai dari persamaan yang sudah dikenal, pilih satu dari daftar **Persamaan Bawaan** di dalam kotak. Klik **OK**.

Persamaan disimpan dalam dokumen dengan bentuk standar untuk matematika, sehingga program lain dapat membukanya.

## Cara mengetik persamaan

- **Pecahan:** ketik garis miring. `a/b` berarti a per b. `(x+1)/(x-1)` menganggap setiap kelompok dalam kurung sebagai satu bagian. `\frac{1}{2}` juga dapat digunakan.
- **Pangkat:** ketik tanda pangkat (^). `x^2` berarti x kuadrat. `e^(i*pi)` menganggap kelompok dalam kurung sebagai satu pangkat.
- **Subskrip:** ketik garis bawah (_). `x_i` berarti x dengan i kecil di bawahnya. Anda dapat menggunakan keduanya, seperti `x_i^2`.
- **Akar:** ketik `sqrt(x+1)` untuk akar kuadrat. Ketik `sqrt[3]{x}` untuk akar pangkat tiga.
- **Jumlah, integral, dan hasil kali:** ketik `\sum_(i=1)^n i`, `\int_0^1 x`, atau `\prod_(k=1)^n k`. Bagian setelah garis bawah diletakkan di bawah (atau di bagian bawah tanda integral), dan bagian setelah tanda pangkat diletakkan di atas.
- **Simbol:** ketik garis miring terbalik (\) dan sebuah nama. `\alpha` menjadi α, `\pi` menjadi π, `\infty` menjadi ∞, `\leq` menjadi ≤, `\neq` menjadi ≠, `\times` menjadi ×, `\pm` menjadi ±, dan `\to` menjadi →. Sekitar 70 nama dapat digunakan.
- **Aksen:** `\hat{x}`, `\check{x}`, `\tilde{x}`, `\acute{x}`, `\grave{x}`, `\breve{x}`, `\dot{x}`, `\ddot{x}`, `\bar{x}`, dan `\vec{v}` menempatkan tanda di atas huruf. `\overline{ab}` menggambar garis di atas kelompok, dan `\underline{ab}` menggambar garis di bawahnya.
- **Kurung yang membesar:** ketik `\left( ... \right)`, dan kurung akan membesar sesuai tinggi isi di antaranya. Gunakan kurung apa pun setelah `\left` dan `\right`, atau titik untuk menghilangkan salah satu sisi, seperti `\left[ x \right.`.
- **Matriks:** **matriks** adalah kisi angka. Ketik `\matrix{a & b \\ c & d}`. Letakkan `&` di antara sel dalam satu baris, dan dua garis miring terbalik di antara baris. Gunakan `\pmatrix` untuk kurung bulat di sekeliling kisi, `\bmatrix` untuk kurung siku, dan `\vmatrix` untuk garis lurus.
- **Fungsi:** `sin`, `cos`, `tan`, `log`, `ln`, `exp`, `lim`, `min`, dan `max` tetap ditulis utuh.

Di halaman, JeffWrite menampilkan persamaan sebagai satu baris teks. Persamaan dalam berkas tetap menyimpan pecahan dan akarnya, sehingga program lain dapat menatanya.

## Mengetik dalam UnicodeMath atau LaTeX

Di samping kotak terdapat daftar format pengetikan: **Garis Sederhana** (yang di atas), **UnicodeMath**, dan **LaTeX**. Ini adalah dua cara menulis matematika sebagai teks biasa yang sudah dikenal banyak orang. Ketika Anda memilih format lain, apa yang Anda ketik ditulis ulang dalam format tersebut.

- **UnicodeMath:** `a/b` adalah pecahan, `x^2_i` memiliki pangkat dan subskrip, `√(x)` adalah akar, dan `∑_(i=1)^n i` adalah jumlah. Tulis kata di antara tanda kutip, misalnya `"jika x"`, maka kata tersebut tetap menjadi teks biasa dan tidak berubah menjadi huruf matematika. `(a|b)` membuat kurung dengan dua bagian, dan `\matrix(a&b@c&d)` membuat matriks (`&` di antara sel, `@` di antara baris).
- **LaTeX:** `\frac{a}{b}` adalah pecahan, `\sqrt[3]{x}` adalah akar pangkat tiga, `\sum_{i=1}^n i` adalah jumlah, `\text{jika x}` adalah teks biasa, dan `\begin{pmatrix} a & b \\ c & d \end{pmatrix}` adalah matriks.

Kedua tombol **Profesional** dan **Linear** mengubah cara kotak menampilkan persamaan Anda. Profesional menggambarnya secara bertingkat, sesuai tampilan akhirnya. Linear hanya menampilkan baris yang Anda ketik. Jika sebuah persamaan tidak dapat ditulis dalam format yang Anda pilih, kotak akan memberi tahu Anda dan tetap menyimpan apa yang Anda ketik.

## Tab Persamaan

Pilih sebuah persamaan (klik, atau pindahkan kursor di atasnya dengan Shift), dan tab bernama **Persamaan** muncul di ujung pita. Tab itu hilang ketika Anda memilih hal lain.

- **Alat:** **Edit Persamaan** membuka kotak. **Profesional** dan **Linear** memilih cara kotak dibuka, bertingkat atau hanya baris. Dokumen itu sendiri selalu menampilkan persamaan sebagai baris teks. **Teks Normal** membuka kotak dengan sepasang tanda kutip, untuk kata yang bukan matematika.
- **Konversi:** pilih format tempat kotak dibuka: **Garis Sederhana**, **UnicodeMath**, atau **LaTeX**. Saat Anda mengedit persamaan berikutnya, persamaan itu ditulis dalam format yang Anda pilih. Persamaan yang tidak dapat ditulis dengan baris sederhana, misalnya jumlah dengan batas atas dan bawahnya, dibuka dalam UnicodeMath.
- **Simbol:** huruf Yunani, operator, relasi, panah, serta himpunan dan logika. Pilih simbol, dan kotak dibuka dengan simbol itu sudah ditambahkan pada baris.
- **Struktur:** pecahan, skrip, radikal, integral, operator besar, kurung, fungsi, aksen, limit, operator, dan matriks. Klik tombol untuk bentuk biasanya, atau panah di sebelahnya untuk bentuk lain, seperti turunan, akar dengan pangkat, atau matriks identitas.

Struktur dimasukkan di akhir baris, dengan huruf contoh di dalamnya, dan bagian pertama dipilih agar Anda dapat langsung mengetik di atasnya. Kotak memiliki struktur yang sama dalam deretan tombol kecil di atas baris. Di sana, struktur dimasukkan di posisi kursor, dan jika sebagian baris dipilih, bagian itu dimasukkan ke tempat pertama dalam struktur.

## Persamaan bawaan

Klik **Sisipkan > Persamaan Bawaan** untuk melihat daftar persamaan yang sudah dikenal, seperti rumus kuadrat dan luas lingkaran. Klik salah satu untuk memasukkannya pada posisi kursor.

## Mengubah persamaan

Letakkan kursor di dekat persamaan, atau pilih persamaan itu, lalu klik **Sisipkan > Persamaan**. Kotak akan terbuka dengan persamaan yang sudah ditulis dalam satu baris. Ubah persamaan itu lalu klik **OK**. Untuk membatalkannya cukup satu langkah.

## Persamaan dalam dokumen lain

JeffWrite menampilkan persamaan yang dimiliki sebuah dokumen, dan mempertahankannya seperti semula. Persamaan yang dibuat program lain dibuka dalam UnicodeMath, atau dalam LaTeX jika UnicodeMath tidak dapat menulisnya, sehingga Anda dapat mengubahnya. Jika keduanya tidak dapat menulis semua bagiannya, seperti tanda pengelompokan, kotak akan dibuka dengan teks persamaan tersebut. Klik **Batal** untuk mempertahankan persamaan apa adanya, atau ketik persamaan baru untuk menggantinya.
